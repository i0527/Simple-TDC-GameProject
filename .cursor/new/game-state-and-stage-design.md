# ゲーム状態管理・ステージシステム設計

> 目的: プレイ前→プレイ中→ポーズ→クリア/ゲームオーバーまでの状態管理と、ステージ定義（ウェーブ、ボス、リソース、難易度）を統合する。

## ゴール

- ゲーム全体の状態遷移を明文化し、UI/入力/サウンドを同期。
- ステージ定義を JSON で管理し、ウェーブや難易度を HotReload 可能にする。
- TD固有のライフ/コスト/到達判定と結びつける。

## コアループと勝敗条件

- フロー: ステージ選択 → ロード → Ready演出 → Playing(速度 0.5/1/2x) → Victory/Defeat → リザルト → リトライ/戻る。
- 勝利条件: すべてのウェーブを完了し、敵が全滅。
- 敗北条件: ライフ0 または 制限時間切れ（ステージ定義でオプション）。
- 報酬: Victory 時にステージ報酬テーブルを参照（ゴールド/経験値/アイテム）。敗北時はリトライ提示のみ。

## 状態遷移

```
Boot → MainMenu → StageLoading → StageReady
    → Playing ↔ Paused
    → Victory | Defeat
```

- `Boot`: アセット/定義ロード、Config。
- `MainMenu`: ステージ選択、設定。
- `StageLoading`: ステージJSONロード、Wave/Cost/Lane 初期化。
- `StageReady`: 開始前演出、カウントダウン、UI初期化。
- `Playing`: 通常進行。速度変更(0.5x/1x/2x)を許可。
- `Paused`: 入力/進行停止、UIのみ。
- `Victory`: クリア演出、リザルト表示。
- `Defeat`: ライフ0または強制失敗で遷移。

## コンポーネント

- `GameStateComponent`
  - `state`（enum）
  - `timeScale`（0.0, 0.5, 1.0, 2.0）
  - `pendingTransition`（次状態）
- `StageDefinitionComponent`
  - `stageId`, `waves`, `lanes`, `cost`, `life`
  - `bossInfo`（waveId, name, announceTime）
  - `difficulty`（`baseHpMul`, `atkMul`, `rewardMul`）
- `LifeComponent`（拠点）
  - `life`, `maxLife`
- `ResultComponent`
  - `clearTime`, `remainingLife`, `score`, `rewards`

## システム

### GameStateSystem

- 入力: `GameStateComponent`, ユーザー入力（ポーズ/速度変更）
- 処理:
  - `state` に応じて Update を分岐。
  - `Paused` ではロジック更新をスキップ（UIは許可）。
  - `timeScale` を全システムに伝播（Physics/Animation/Particle/Combat の deltaTime に乗算）。
- 遷移条件:
  - Victory: `WaveScheduler` 完了 + 敵全滅。
  - Defeat: `LifeComponent.life <= 0` または 制限時間切れ（任意）。

### StageLoaderSystem

- ステージ JSON をロードして `StageDefinitionComponent` をセット。
- `lanes`/`waves`/`cost` をそれぞれのレジストリへ転送。
- `life` 初期化、UI に初期値を通知。

### StageReadySystem

- カウントダウン（3..2..1..Start）演出。
- `GameState` を `Playing` に変更。

### LifeSystem

- 敵到達時に `life` を減算。0 以下で `Defeat`。
- UI へ `LifeChanged` イベント送信。

### ResultSystem

- Victory/Defeat 遷移時に統計を集計（クリア時間、残ライフ、使用コスト量など）。
- リザルト UI へデータを送る。

## ステージJSON（例）

```json
{
  "id": "stage_01",
  "displayName": "草原の侵攻",
  "life": 20,
  "difficulty": { "atkMul": 1.0, "hpMul": 1.0, "rewardMul": 1.0 },
  "lanes": [...],
  "cost": { "max": 50, "start": 10, "regenPerSec": 2.5 },
  "waves": [...],
  "boss": { "waveId": 2, "announceTime": 3.0, "name": "スライムロード" }
}
```

- 難易度バリエーション: `stage_01_easy/normal/hard` などで倍率のみ変更。

## UI連携

- `StateChanged(state)`
- `TimeScaleChanged(scale)`
- `LifeChanged(current, max)`
- `BossIncoming(waveId, name)`
- `ResultReady(victory, stats)`
- ポーズメニュー: 再開/リトライ/ステージ選択へ。

## コアループ詳細と勝敗・報酬

- 進行フロー: タイトル → ステージ選択 → ロード → Ready演出 → Playing（0.5x/1x/2x）↔ ポーズ → Victory/Defeat → リザルト → リトライ/戻る。
- 勝利条件: 全ウェーブ消化＋敵殲滅。敗北条件: ライフ0 または タイマー敗北（ステージ定義で有効化）。
- リトライ: 現行ステージを即時再ロード。報酬はVictory時のみ適用、Defeat時は再挑戦導線のみ。
- 報酬フロー: Victory → ResultSystem で統計集計 → 報酬テーブル適用（ゴールド/経験値/アイテム）→ セーブ反映。

## 経済・バランス指標（目安）

| 指標 | 目安 |
| --- | --- |
| コスト開始値/最大値 | 10 / 50 |
| コスト回復 | 2.5/秒（初期）、難度で±20% |
| 敵HP帯 | 序盤 50-150, 中盤 200-600, 終盤 800-1500 |
| 敵DPS帯 | 序盤 5-15, 中盤 20-60, 終盤 80-150 |
| ウェーブ難度カーブ | WaveごとにHP/攻撃を1.12〜1.18倍、ボスは+40〜60% |
| 期待クリア時間 | 1面 2-3分, 2面 3-4分, 3面 4-6分 |

## ユニット/敵ロール定義

- 前衛: 高耐久/短射程、足止めと受け。  
- 後衛: 中〜長射程/中DPS、耐久低。  
- サポート: バフ/デバフ/回復、CC提供。  
- 範囲/CC: スロー/スタン/ノックバックで群れ対応。  
- ボス: 高HP/高DPS/特殊ギミック、耐CCやフェーズ持ち。

## コンテンツ例（概要）

- ユニット/敵/アビリティのメタ:  
  - ユニット: 前衛「ガードA」(高耐久, 近接), 後衛「レンジャーB」(中射程, 単体DPS), サポート「ヒーラーC」(回復, 低耐久)。  
  - 敵: 雑魚「スライム」(低HP低DPS), エリート「ゴブリン」(中HP中DPS), ボス「スライムロード」(高HP, 範囲攻撃)。  
  - アビリティ: 範囲火球(DoT), 単体高火力, 回復バースト。
- 代表ステージ例（期待クリア時間は上表参照）:

| ステージ | 推奨HP倍率/攻撃倍率 | Wave数 | ボス | 備考 |
| --- | --- | --- | --- | --- |
| 1面 | 1.0 / 1.0 | 3 | 無 | 入門、チュートリアル |
| 2面 | 1.15 / 1.1 | 4 | Wave4 | 雑魚ラッシュ後に中ボス |
| 3面 | 1.25 / 1.2 | 5 | Wave5 | 耐久＋範囲DPS要求 |

## UX/フロー（ポーズ/速度/チュートリアル/遷移）

- ポーズ: Esc/Start で即時停止、UIのみ有効。  
- 速度変更: 0.5x/1x/2x をUIトグルとホットキーで提供。  
- チュートリアル導線: 1面開始時に操作オーバーレイ、ポーズ中にヘルプパネル表示。  
- 画面遷移: タイトル → ステージ選択 → ゲーム（Ready→Playing→Victory/Defeat）→ リザルト → リトライ/戻る。

## 入力/操作ポリシー（抜粋）

- アクセシビリティ: 色覚配色プリセット、フォントサイズスライダ、マスター/SE/BGM音量スライダ。  
- 入力割り当て: マウス（左クリック配置/選択、右クリックキャンセル）、ホイールズーム/パン（仮想座標で補正）、キーボードショートカット（速度変更/ポーズ/リトライ）。  
- 仮想座標対応: 1920x1080基準で入力を正規化し、UI/選択ヒットを補正。

## 演出・フィードバック / テレメトリ

- 通知: ボス出現、ウェーブ開始/終了をUIトースト＋サウンドで告知。  
- フィードバック: ヒット/被弾/死亡/クリア失敗時にエフェクト＋SE＋UIトースト（色コード: ダメージ=赤、被弾=オレンジ、死亡=紫、敗北=灰）。  
- テレメトリイベント: `boss_spawned`, `wave_started`, `wave_finished`, `player_hit`, `player_down`, `defeat`, `victory` を発火（DevModeコンソールとログに送る）。

## データフロー/保存

- セーブデータ: 進行度（ステージクリア履歴、難度、報酬獲得）、設定（入力/音量/色覚/フォントサイズ/速度プリセット）。  
- 保存タイミング: Victory時と設定変更時に即時保存。ポーズ中の明示的保存は省略。  
- スキーマ更新時: `schemaVersion` を設けず最新版前提。破壊的変更はローダで検知し、互換変換がない場合はセーフモードで読み取り専用＋再保存を要求（既存セーブを保護）。

## セーブ/ロード方針（簡潔）

- スロット3固定、プリセット構造は `main[3]/sub[5]/abilities[2]`。欠落は空で補完。  
- バックアップはローテーション保持（最新+数世代）。破損時は最新バックアップへロールバックし警告。  
- ロード時は未知キー無視・欠落補完。致命エラーは読み取り専用で扱い、再保存を促す。

## ステージ詳細・リザルト（軽量必須版）

- ステージ詳細: 推奨戦力、敵プレビュー、報酬、難易度解放条件（Easy/Normal/Hard）のみ表示。  
- リザルト導線: Victory/Defeat後に結果表示 → リトライ/ホームへ戻る。追加のミッション詳細や拡張メニューは省略。

## DevMode / テスト

- ステージスキップ: 次ウェーブへジャンプ。
- 即時クリア/失敗トグル（演出確認用）。
- タイムスケール直接入力。
- JSONホットリロード: `StageLoaderSystem` が再ロードし、`WaveScheduler`/`LaneManager`/`CostSystem` に再配信。

## 実装ステップ（チェックリスト）

- [ ] `GameStateComponent` と `GameStateSystem` を追加し、ポーズ/速度変更を管理
- [ ] `StageLoaderSystem` でステージJSONをロードし、`WaveScheduler/LaneManager/Cost` に配信
- [ ] `LifeSystem` で拠点ライフを管理し、到達ダメージと Defeat 判定を行う
- [ ] Victory 判定（全ウェーブ完了＋敵殲滅）と Result 集計を実装
- [ ] UIイベント: 状態・ライフ・ボス予告・結果を通知
- [ ] DevMode: ステージリロード/スキップ/速度変更のデバッグ操作を追加
