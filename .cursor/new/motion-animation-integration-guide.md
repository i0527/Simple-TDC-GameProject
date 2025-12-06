# パーツアニメーション統合ガイド

> 対象: 新アーキテクチャ（`include/new/`, `src/new/`）でモーションエディタ出力をゲーム本体へ統合する手順。

## ゴール

- `motion-editor-requirements.md` で定義した JSON を読み込み、パーツベースのアニメーションを実行。
- キャラクター状態（Idle/Walk/Attack/Hit/Death 等）に応じてアニメーションを切替。
- RenderTexture経由でゲームビューとエディタ双方で共通のレンダリングパイプラインを使う。
- F1〜F4 エディタ統一レイアウトに MotionEditor(F4) を組み込み、ホットリロードを反映。

## 全体フロー

```
JSON (CharacterMotionData)
    ↓ Loader
AnimationRegistry (per character)
    ↓
RuntimeAnimationComponent (per entity)
    ↓
AnimationPlaybackSystem
    ↓
SpritePartRenderer / PartBatcher
    ↓ RenderTexture / Screen
```

## データモデル

- `CharacterMotionData`（JSON）: パーツツリー、アニメーションセット、フレーム列、各フレームのパーツ姿勢（pos/rot/scale/zOrder/flip/alpha）。
- `AnimationClip`:
  - `id`, `fps`, `loop`, `frames[]`（フレームごとのパーツ姿勢）。
  - 追加メタ: `transitionTags`（AttackEnd/HitReact などの終了タグ）。
- `AnimationRegistry`（キャラ単位）:
  - `std::unordered_map<std::string, AnimationClip> clips`
  - `std::unordered_map<std::string, Texture2D> textures`（スプライトシートやパーツ個別テクスチャ）
  - 参照元: `assets/motions/<character>.json`

## ランタイムコンポーネント

```cpp
struct RuntimeAnimationComponent {
    std::string characterId;      // "player_unit_01"
    std::string clipId;           // "idle", "attack"
    int frame = 0;
    float timer = 0.0f;
    float fps = 12.0f;
    bool loop = true;
    bool playing = true;
    bool flipX = false;
    bool flipY = false;
    Color tint = WHITE;

    // 追加オプション
    std::optional<Vector2> overridePos;
    std::optional<float> overrideScale;
    std::optional<float> overrideRotation;
};
```

- `AnimationStateComponent`（任意拡張）:
  - `currentState`, `nextState`, `blendTime`, `eventQueue`（アニメーションイベント）

## システム構成

### AnimationPlaybackSystem

- 入力: `RuntimeAnimationComponent`, `AnimationRegistry`（参照）
- 処理:
  - `timer += dt;` → `frame` を進める (`frame += floor(timer * fps)`)
  - 非ループかつ最終フレーム到達時は `playing=false` または `transitionTags` を発火
  - `clipId` 変更時に `frame=0`, `timer=0`, `fps/loop` を clip から再設定
- 出力: 現在フレームのパーツ姿勢リストを `SpritePartRenderQueue` に積む

### StateSystem（責務の明確化）

- 役割: 入力イベント→状態遷移の判定、割込み可否/優先度、ブレンド設定、`RuntimeAnimationComponent` への clip/fps/loop 反映。
- AnimationPlaybackSystem への委譲: フレーム進行・イベント抽出・描画キュー生成は Playback が担当。
- 共有コンテキスト例:

```cpp
struct AnimationPlaybackContext {
    AnimationRegistry* registry;
    StateDefinition* stateDef;
    RuntimeAnimationComponent* anim;
    AnimationStateComponent* state;
};
```

このコンテキストを StateSystem がセットアップし、PlaybackSystem は純粋に「進行とイベント抽出」に集中させる。

### SpritePartRenderer / PartBatcher

- 役割: パーツ単位の描画指示をまとめ、zOrder順に描画。
- 入力: `SpritePartRenderQueue`
- 実装案:
  - `struct PartDrawCommand { Texture2D* tex; Rectangle src; Vector2 pos; Vector2 origin; float rot; Vector2 scale; int z; Color tint; bool flipX; bool flipY; };`
  - `std::stable_sort` による zOrder ソート後に描画
  - バッチング: テクスチャ単位でグルーピングし描画回数を削減

### AnimationEventSystem（任意）

- 役割: クリップ内に埋め込まれたイベント（例: 攻撃ヒットフレーム）をゲームロジックに伝達。
- インターフェース: `std::function<void(const AnimationEvent&)>`

## ローダ（HotReload対応）

- ファイル: `assets/motions/<character>.json`
- 処理:
  1. JSONパース（例外ハンドリング必須）
  2. `AnimationRegistry` を構築し、テクスチャロード
  3. `HotReloadSystem` にパスを登録し、変更検知で再ロード
- キャッシュ: `AnimationRegistryCache` を DI で共有（GameContext 配下）

## 状態とクリップの対応付け

- 推奨: キャラごとに状態テーブルを定義

```cpp
struct AnimationStateEntry {
    std::string state;     // "Idle", "Move", "Attack", "Hit", "Death"
    std::string clip;      // "idle", "walk", "attack", ...
    bool interruptible;    // 攻撃中に上書き可否
    std::string onEndState; // 非ループ終了後に遷移する状態
};
```

- 例: `state="Attack"` → `clip="attack"`（非ループ, onEndState="Idle"）
- 入力イベント（Move/Attack/Hit/Death）で状態を変え、`clipId` を切替。

## MotionEditor(F4) との連携

- 保存: `Save` で `assets/motions/<character>.json` を生成（既存要件準拠）。
- ホットリロード: `HotReloadSystem` が差分検出→`AnimationRegistry`再ロード→`RuntimeAnimationComponent` が再参照。
- プレビュー: F4 Viewport で `SpritePartRenderer` をそのまま利用し、ゲームと同じ描画経路を保証。

## 描画座標系

- 基準: 仮想FHD (1920x1080)
- パーツの `pos` はキャラクター基準のローカル座標。ワールド座標は `Transform` と合成。
- `scale`/`rot`/`flip` はパーツローカルに適用、`zOrder` で奥行きを表現。

## 推奨コンポーネント組み合わせ

- `Transform` + `RuntimeAnimationComponent` + （任意で）`AnimationStateComponent`
- 物理/衝突を伴う場合: `CollisionComponent`（別設計で定義予定）と併用

## 状態管理と StateSystem 設計

### 状態テーブル（JSONスキーマ案）

ファイル: `assets/characters/<character>/states.json`

```json
{
  "characterId": "meowt_pink",
  "states": [
    {
      "state": "Idle",
      "clips": ["idle"],
      "interruptible": true,
      "onComplete": { "toState": "Idle" },
      "transitions": [
        { "trigger": "Move", "toState": "Move", "interruptible": true },
        { "trigger": "Attack", "toState": "Attack", "interruptible": false }
      ]
    },
    {
      "state": "Attack",
      "clips": ["attack"],
      "interruptible": false,
      "onComplete": { "toState": "Idle" },
      "events": [
        { "frame": 5, "type": "hitbox_on", "args": { "emitterId": "sword_melee" } },
        { "frame": 10, "type": "hitbox_off" }
      ]
    }
  ]
}
```

### AnimationStateComponent（明示版）

```cpp
struct AnimationStateComponent {
    std::string currentState;     // "Idle"
    std::string nextState;        // 要求された遷移先
    std::string playingClip;      // 実際に再生中のクリップID
    float blendTime = 0.0f;       // 現在のブレンド経過
    float blendDuration = 0.0f;   // ブレンド総時間
    bool interruptible = true;    // 現在状態が割込みを許可するか
    std::queue<AnimationEvent> eventQueue; // クリップ内イベント
    StatePriority priority = StatePriority::Idle;
};
```

### StateSystem

- 入力: `AnimationStateComponent`, `RuntimeAnimationComponent`, `InputEvents`, `StateDefinition`(JSON)
- 処理フロー:
  1. 入力イベント（Move/Attack/Hit/Deathなど）を `nextState` 候補として受け取る。
  2. 現在状態の `interruptible` が false の場合、攻撃終了フレームや `onComplete` まで遷移を保留。
  3. 遷移許可時に `playingClip` と `RuntimeAnimationComponent.clipId` を切替、`frame=0`, `timer=0`, `fps/loop` を反映。
  4. ブレンド: `blendDuration > 0` の場合、旧クリップと新クリップを線形補間（最小実装: 位置/回転/スケールを lerp）。
  5. クリップ内イベント: `AnimationPlaybackSystem` がフレーム進行時に `eventQueue` に `AnimationEvent` を積み、`AnimationEventSystem` が処理（例: `hitbox_on/off` を CombatSystem へ通知）。
- 出力: `RuntimeAnimationComponent` 更新、`AnimationEvent` 発火、状態変更イベント（UI/ログ用）。

### 入力→状態→アニメーションの経路

```
InputSystem (Move/Attack/Hit...) 
    → StateSystem (状態許可判定・遷移・ブレンド設定)
        → RuntimeAnimationComponent に clipId/loop/fps をセット
        → AnimationPlaybackSystem がフレーム進行・イベント抽出
            → AnimationEventSystem がヒットボックス等へ伝達
```

### 割り込みポリシー例

- `interruptible = false` の状態（Attackなど）は `onComplete` まで維持し、Hit/Death だけ割り込み可などの優先度テーブルを別途持つ:

```cpp
enum class StatePriority { Idle=0, Move=1, Attack=2, Hit=3, Death=4 };
// 高優先度の状態は割込みを許可。Attack→Hit/Death は上書き可、Hit→Attack は不可。
```

### 状態定義の共有

- 使い回し: 同型キャラは `characterId` ごとに `states.json` を分けるか、共通プリセットを `assets/characters/shared/states_<archetype>.json` に置き、キャラ定義で参照する。

### DevModeでの状態遷移可視化

- F1デバッグパネルに状態遷移グラフを表示（現在状態をハイライト、許可される遷移矢印を描画）。
- Attack→Hit など割込みが失敗した場合は理由を注記（例: `interruptible=false` / `priority低`）。
- ログ: 最新の遷移履歴を時系列で表示（state-from → state-to, trigger, frame）。

### パーティクルイベントとの連携（MotionEditor側で編集）

- クリップ内イベントに `particle_*` を登録し、フレームごとに `particleId` とオフセット/スケールを指定。
- 例: `{"frame":5,"type":"particle","args":{"particleId":"slash_small","offset":[10,-5],"scale":1.0}}`
- **責務分離**: PlaybackSystem はイベント検出とキュー積みのみ、StateSystem は遷移系（onComplete 等）を扱い、AnimationEventSystem がゲームロジック系イベントを処理する。`type=="particle"` は AnimationEventSystem で `ParticleEmitter` に橋渡しする。

### AnimationEvent の発火責務まとめ

- PlaybackSystem: クリップ内イベントを検出し、`AnimationStateComponent.eventQueue` に積む（検出/キュー）。
- StateSystem: キューを見て「状態遷移に関わるイベント」（onComplete など）を処理（遷移系）。
- AnimationEventSystem: 戦闘・パーティクル・サウンド等のゲームロジック系イベントを処理（ゲームロジック系）。`type=="particle"` はここで `ParticleEmitter`/パーティクルスポーンに橋渡しする。

### イベント種類と優先順位

- 同一フレームに複数イベントがある場合の優先順位: `hitbox_on/off` → `particle` → `sound` → `custom`（同種は定義順）。遷移系（onCompleteなど）は StateSystem で別枠処理。
- 処理者:
  - `hitbox_on/off`: AnimationEventSystem → Combat/Collision 系へ通知
  - `particle`: AnimationEventSystem → ParticleEmitter へ橋渡し
  - `sound`: AnimationEventSystem → AudioSystem 呼び出し
  - `custom`: AnimationEventSystem → 登録ハンドラへ委譲

---

このガイドは `json-schema-design.md` に定義された `states.json` / `effects.json` スキーマを前提とします。

## 実装ステップ（チェックリスト）

- [ ] `AnimationRegistry` / ローダを実装（HotReload対応）
- [ ] `RuntimeAnimationComponent` / `AnimationStateComponent` を定義
- [ ] `AnimationPlaybackSystem` でフレーム進行＋イベント発火
- [ ] `SpritePartRenderer`（パーツ描画バッチャ）を実装
- [ ] MotionEditor(F4) から出力した JSON を読んでプレビュー一致を確認
- [ ] ホットリロードでゲーム画面が即時更新されることを確認
- [ ] `StateSystem` を実装し、入力イベント→状態遷移→クリップ切替→イベント発火を一連で扱う
- [ ] `states.json` のスキーマを読み込むローダを追加し、キャラごと/共有プリセットを解決する
