# 内部エディタ設計（ショートカット統一版）

> **重要**: このドキュメントは新アーキテクチャ（`include/new/`, `src/new/`）の内部エディタ設計を定義します。DockSpaceによる自由レイアウトを廃止し、F1〜F4ショートカットで固定レイアウトのエディタを切り替えます。

## 目的

- ImGuiベースのゲーム内エディタでJSON定義を直接編集
- F1〜F4ショートカットでエディタを即時切替（工数削減・操作統一）
- レイアウトは4エディタ共通の3ペイン＋タイムライン構成で再利用性を最大化
- ホットリロードによるリアルタイム反映
- ワークスペース保存/読み込み（開いているリソースと各エディタ状態を保持）

---

## 1. DevModeManager（開発者モード管理）

### 1.1 ショートカットと役割

- **F1**: UIエディタ
- **F2**: エンティティエディタ
- **F3**: ステージエディタ
- **F4**: モーションエディタ（パーツアニメーション）
- 開発者モードON/OFFのトグルと、現在エディタインスタンスの差し替えを担当。

### 1.2 クラス設計

```cpp
namespace New::Game::Editor {

class DevModeManager {
public:
    enum class EditorMode { UI, Entity, Stage, Motion };

    bool Initialize(Core::GameContext& context);
    void Shutdown();

    void Update(float deltaTime);   // 入力処理（F1〜F4）と現在エディタのUpdate
    void Render();                  // 現在エディタのRender

    void ToggleDevMode();
    bool IsActive() const { return active_; }

    void SaveWorkspace(const std::string& name);
    void LoadWorkspace(const std::string& name);

    RenderTexture2D& GetGameView() { return gameViewRenderer_.GetRenderTarget(); }

private:
    bool active_ = false;
    bool initialized_ = false;
    Core::GameContext* context_ = nullptr;

    GameViewRenderer gameViewRenderer_;
    std::unique_ptr<Workspace> workspace_;
    std::unique_ptr<Core::HotReloadSystem> hotReload_;

    std::unique_ptr<EditorBase> currentEditor_;
    EditorMode currentMode_ = EditorMode::UI;

    void SwitchEditor(EditorMode mode);
};

} // namespace New::Game::Editor
```

### 1.3 初期化フロー

```cpp
bool DevModeManager::Initialize(Core::GameContext& context) {
    context_ = &context;
    if (!gameViewRenderer_.Initialize(1920, 1080)) return false;

    workspace_ = std::make_unique<Workspace>();
    hotReload_ = std::make_unique<Core::HotReloadSystem>();
    hotReload_->Initialize("assets/new/definitions");

    SwitchEditor(EditorMode::UI); // 初期はUIエディタ
    initialized_ = true;
    return true;
}
```

---

## 2. GameViewRenderer（ゲームビュー描画）

- ゲーム画面をRenderTextureに描画し、ImGuiウィンドウ内でスケーリング表示
- 仮想FHD（1920x1080）を基準にアスペクト比維持

（インタフェースは従来案と同じ。詳細は省略）

---

## 3. 統一レイアウトと共通パネル

### 3.1 固定レイアウト（全エディタ共通）

```
┌────────────────────────────────────────────┐
│  [メニューバー: Save / Load / Close]      │
├────────────────────────────────────────────┤
│ [L-Pane]     │ [Viewport]    │ [R-Pane]    │
│ ResourceList │ Preview/Canvas│ Properties  │
│ [+] [-] ...  │  Zoom / Grid  │ [+] [-] ... │
├────────────────────────────────────────────┤
│ [Timeline / Context Section]               │
│ Play / Stop / FPS / Loop / Frames ...      │
├────────────────────────────────────────────┤
│ Status Bar                                 │
└────────────────────────────────────────────┘
```

### 3.2 共通パネルインターフェース

```cpp
struct ResourceListPanel {
    std::vector<std::string> items;
    int selectedIndex = -1;
    std::function<void()> onAdd;
    std::function<void()> onDelete;
    std::function<void(int)> onSelect;
    void Render(const char* title);
};

struct ViewportPanel {
    Vector2 camera {0,0};
    float zoom = 1.0f;
    std::function<void()> onDraw;
    std::function<void()> onMouse; // 左ドラッグ=移動, ホイール=ズーム, 拡張: Shift=回転, Ctrl=スケール
    void Render(const char* title);
};

struct PropertyPanel {
    std::function<void()> onRender;
    void Render(const char* title);
};

struct TimelinePanel {
    float fps = 8.0f;
    bool loop = true;
    int currentFrame = 0;
    std::function<void()> onRender; // 再生/停止/フレーム管理など
    void Render(const char* title);
};
```

### 3.3 EditorBase（共通ベース）

```cpp
class EditorBase {
protected:
    ResourceListPanel left_;
    ViewportPanel viewport_;
    PropertyPanel right_;
    TimelinePanel timeline_;
    virtual void BuildLeftPane() = 0;
    virtual void BuildViewport() = 0;
    virtual void BuildRightPane() = 0;
    virtual void BuildTimeline() = 0;
public:
    virtual void Update(float dt) {}
    virtual void Render() {
        BuildLeftPane();
        BuildViewport();
        BuildRightPane();
        BuildTimeline();
    }
    virtual ~EditorBase() = default;
};
```

---

## 4. Workspace（ワークスペース管理）

- レイアウトは固定化したためDockSpace情報は不要。
- 保存対象: 現在のエディタ種別、開いているリソース（パス/ID）、各エディタ固有状態（JSON文字列）

```json
{
  "editorMode": "Motion",
  "openResources": [
    "assets/motions/player_unit_01.json",
    "assets/new/definitions/entities/player_unit_01.json"
  ],
  "editorStates": {
    "MotionEditor": "{\"currentAnim\":\"idle\",\"currentFrame\":3,\"zoom\":1.0}"
  }
}
```

---

## 5. エディタ実装ガイド（共通レイアウト適用例）

### EntityEditor（F2）

- L-Pane: エンティティ一覧＋追加/削除
- Viewport: 選択エンティティのプレビュー（簡易スプライト or パーツ合成）
- R-Pane: プロパティ編集（JSONバッキング）
- Timeline: 行動セット/状態切替等で拡張可

### UIEditor（F1）

- L-Pane: UI要素ツリー
- Viewport: UIレイアウトキャンバス（ドラッグ配置）
- R-Pane: プロパティ/バインディング
- Timeline: UIアニメーションや状態トランジション

### StageEditor（F3）

- L-Pane: オブジェクト/敵/ウェーブリスト
- Viewport: マップ編集（グリッド操作）
- R-Pane: 配置物のプロパティ
- Timeline: 波スケジュール/進行シミュレート

### MotionEditor（F4）

- L-Pane: パーツリスト（追加/削除/リネーム）
- Viewport: パーツ配置プレビュー（位置/回転/スケール編集、zOrderソート描画）
- R-Pane: アニメーション一覧と選択
- Timeline: フレーム管理（再生/停止/FPS/Loop、フレーム追加・削除・複製）
- Timeline拡張: Particleトラックを追加し、フレーム単位で `particleId` とオフセットを設定（例: frame5 に `slashsmall` を再生）
- マウス操作: 左ドラッグ=位置、Shiftドラッグ=回転、Ctrlドラッグ=スケール、ホイール=ズーム
- データ: `assets/motions/<character>.json`（`motion-editor-requirements.md`のCharacterMotionDataに準拠）
- 状態テーブル編集（追加要件）:
  - R-Pane下部に「状態テーブル」セクションを追加
  - 行追加/削除、状態名、対応クリップ、`interruptible`、`onComplete`、遷移（trigger→state）を編集
  - フレームイベント（例: `hitbox_on/off` や `particle_*`）をアニメーションクリップに紐付けて編集
  - 保存先: `assets/characters/<character>/states.json`（HotReload対象）
- 状態遷移テスト（開発用）:
  - Timeline下部に簡易トリガーボタン（Idle/Move/Attack/Hit など）を置き、`RequestStateTransition` を発火。
  - 遷移ログ（from→to, trigger, result/理由）を表示し、StateSystem の割込み判定をデバッグ。

#### MotionEditor (F4) が編集・保存するもの

- `assets/motions/<character>.json`（CharacterMotionData）
- `assets/characters/<character>/states.json`（状態テーブル＋AnimEvent。`particle` イベントはここに保存）
- `assets/effects/*.json` は参照のみ（編集不可、将来の ParticleEditor を想定）

外部参照について:

- `g:/DownLoads/motion-editor-requirements.md` はリポジトリ外部。取得方法またはリポジトリ内への配置計画を別途明記すること（将来の同期漏れ防止）。

---

## 6. HotReloadSystem（ホットリロード）

- JSON定義の変更を監視し、対応するエディタ/レジストリへ反映
- エディタ種別ごとのリロードフックを用意（UI/Entity/Stage/Motion）

---

## 7. メインループ統合

- 開発者モードON中はゲームロジック更新を停止/スロットルし、エディタ更新を優先
- レンダリング: RenderTextureにゲーム画面→ImGuiでエディタUI表示→通常描画と切替

---

## 8. 実装チェックリスト

### DevModeManager

- [ ] F1〜F4切替（UI/Entity/Stage/Motion）
- [ ] 開発者モードトグル
- [ ] ホットリロード統合
- [ ] ワークスペース連携

### GameViewRenderer

- [ ] RenderTexture管理
- [ ] ImGuiウィンドウ内表示
- [ ] アスペクト比維持

### Workspace

- [ ] エディタ種別/開いているリソース/各エディタ状態を保存
- [ ] JSON保存/読み込み

### 各エディタ

- [ ] UIEditor（F1）
- [ ] EntityEditor（F2）
- [ ] StageEditor（F3）
- [ ] MotionEditor（F4：パーツ/フレーム/タイムライン）

### HotReloadSystem

- [ ] ファイル変更監視
- [ ] コールバック実行
- [ ] エラーハンドリング

---

---

## 9. DevModeデバッグUI拡張（F1パネル）

- レイアウト: 左にメトリクス、右にログ/プレビュー。
- Performance:
  - FPS(現/目標)、Particles: used/max、Entities: alive、DrawCalls。
  - アラート閾値: FPS < 50, Particles 使用率 > 80% でハイライト。
- CombatLog:
  - HitEvent / Death / DOT tick をリングバッファ（最新100件）。`type` と `entity` でフィルタ。
  - クリックで詳細（damage, crit, source/target lane, frame）。
- Systems:
  - LaneMovement / HitEmitter / Collision / ParticleSystem の ON/OFF トグル。
  - 直近フレームの処理時間(ms)を表示。
- HotReload:
  - 監視ファイルの最終更新とリロード結果を一覧（例: `effects.json: reloaded OK / 12:03:12`）。
  - エラー時はメッセージを赤表示、再試行ボタン。
- Validation / Errors:
  - SchemaValidatorエラー、HotReload失敗、ロード失敗を時系列で表示（ファイル名・行・メッセージ）。
  - 「再検証」ボタンで対象ファイルを再チェック。成功時は緑で履歴に残す。
- Particle Preview:
  - `effects.json` のプリセットをプルダウン選択→即再生。
  - 位置オフセットとスケールを入力して確認可能。
- State遷移可視化（Motion連携）:
  - 現在状態をハイライトした矢印グラフを描画。
  - 直近の遷移履歴（state-from → state-to, trigger, frame, reason）を表形式で表示。

---

## 10. Workspace自動バックアップ

- 間隔: デフォルト10分。設定で 5/10/30 分から選択。
- 世代: 直近3スナップショットを保持（例: v1=10分前, v2=5分前, v3=最新）。
- 保存形式: `workspace_name.autosave-YYYYMMDD-HHMM.json` として別フォルダに保存。
- UI: Workspaceセレクタに「Restore」ボタンを追加し、任意スナップショットへ復元可能（プレビュー付き）。
- 手動保存は従来通り（自動保存と衝突しないようファイル名を分離）。
- オフ/オン切替と「次回だけスキップ」オプションを用意。

---

## 11. レイアウトのリサイズ対応

- 基本レイアウト: 左ペイン / Viewport / 右ペイン＋下段Timeline。
- 自動調整: ウィンドウ幅に応じて left/right を割合でスケーリング（デフォルト 20% / 50% / 30%）。`lockRatio=true` で比率維持。
- カスタマイズ: ユーザーが一時的にドラッグで幅変更可能。セッション終了時にレイアウト設定を保存し、再起動時に復元。
- 最小サイズ目安: left/right 幅は最低 180px、Timeline高さは最低 140px を下回らないようクランプ（UI崩れ防止）。

---

## 12. UIエディタ→ゲームUI反映（境界メモ）

- 反映フロー: エディタ保存 → スキーマ検証 → ドラフト保存 → ゲーム側HotReload（SchemaValidator→UILoader）→ 適用/部分適用。失敗時は前回キャッシュ保持＋Devトースト通知。  
- 監視パス: `assets/new/definitions/ui/*.json`。  
- エラーポリシー: 部分失敗は partial で通知、致命時は適用しない。  
- テスト最小セット: スキーマ/ローダ回帰（ui_layout）、解像度再初期化、設定適用失敗時のロールバック、通知トーストの重複抑制確認。

## 13. 参考資料

- [コアアーキテクチャ設計](core-architecture-design.md)
- [設計原則と制約](design-principles-and-constraints.md)
- [ライブラリ注意点ガイド](libs_guide.md)
- `g:/DownLoads/motion-editor-requirements.md`（モーションエディタ詳細要件）

---

**作成日**: 2025-12-06  
**バージョン**: 1.1  
**対象**: 新アーキテクチャ開発者向け
