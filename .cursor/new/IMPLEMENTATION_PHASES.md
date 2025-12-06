# 実装フェーズ定義と優先順位

**作成日**: 2025-12-06  
**バージョン**: 1.0  
**目的**: 新アーキテクチャの段階的実装計画を明確化

---

## フェーズ概要

| フェーズ | 名称 | 所要時間(目安) | 状態 | 完了条件 |
|---------|------|--------------|------|---------|
| Phase 0 | 環境構築 | 1週間 | ✅ 完了 | CMakeターゲット、ディレクトリ作成 |
| Phase 1 | Core基盤 | 2-3週間 | 🟡 進行中 | 最小ゲームループ動作 |
| Phase 2 | データ層 | 2週間 | ⚪ 未着手 | JSON読込＋HotReload動作 |
| Phase 3 | 最小プレイアブル | 3週間 | ⚪ 未着手 | 1ステージ完全プレイ可能 |
| Phase 4 | エディタ統合 | 4週間 | ⚪ 未着手 | F1/F2エディタ動作 |
| Phase 5 | 拡張機能 | 4週間以上 | ⚪ 未着手 | 全機能実装 |

**総所要時間**: 約3.5〜4ヶ月（1人月換算）

---

## Phase 0: 環境構築 ✅ 完了

### 目標
新アーキテクチャのビルド環境を整備し、旧コードと並行開発できる体制を確立

### 成果物
- [x] `include/new/`, `src/new/` ディレクトリ作成
- [x] CMakeターゲット `SimpleTDCGame_NewArch` 追加
- [x] 最小限の`main_new.cpp`
- [x] 設計文書（`.cursor/new/`）の作成

### 確認方法
```bash
cmake --build build --target SimpleTDCGame_NewArch
./build/SimpleTDCGame_NewArch  # 最小ウィンドウが起動
```

---

## Phase 1: Core基盤 🟡 進行中

### 目標
DI、ECS、レンダリング、システム実行の基盤を実装し、最小限のゲームループを動作させる

### 実装タスク

#### 1.1 GameContext (優先度: 最高)
- [x] 基本クラス実装 (`GameContext.h/cpp`)
- [ ] IResourceManager実装
- [ ] IInputManager実装（仮想座標変換対応）
- [ ] 初期化・終了処理

**ファイル**:
- `include/new/Core/GameContext.h`
- `src/new/Core/GameContext.cpp`
- `include/new/Core/IResourceManager.h`
- `src/new/Core/ResourceManager.cpp`

**テスト**:
```cpp
// 最小テスト: GameContextの生成と破棄
GameContext context;
assert(context.Initialize());
context.Shutdown();
```

#### 1.2 GameRenderer (優先度: 高)
- [x] 基本クラス実装 (`GameRenderer.h/cpp`)
- [ ] 仮想FHD (1920x1080) レンダーターゲット
- [ ] スケーリング描画実装
- [ ] 座標変換メソッド

**ファイル**:
- `include/new/Core/GameRenderer.h`
- `src/new/Core/GameRenderer.cpp`

**テスト**:
```cpp
// 仮想座標で四角形を描画し、スケーリング表示
renderer.BeginRender();
DrawRectangle(100, 100, 200, 200, RED); // 仮想座標
renderer.EndRender();
renderer.RenderScaled(); // 実画面にスケーリング
```

#### 1.3 SystemRunner (優先度: 高)
- [ ] ISystem基底クラス実装
- [ ] SystemRunnerクラス実装
- [ ] 優先度ベース実行順序
- [ ] システム有効/無効切替

**ファイル**:
- `include/new/Core/Systems/ISystem.h`
- `include/new/Core/SystemRunner.h`
- `src/new/Core/SystemRunner.cpp`

**テスト**:
```cpp
// 2つのシステムを登録して実行
systemRunner.RegisterSystem(std::make_unique<TestSystem1>());
systemRunner.RegisterSystem(std::make_unique<TestSystem2>());
systemRunner.Update(context, 0.016f);
```

#### 1.4 World (優先度: 中) ※現在保留
- [ ] Worldクラス実装 または GameContextに統合
- [ ] エンティティ名管理
- [ ] コンポーネント操作ヘルパー

**注意**: 現在`World`クラスは実装されていない。設計文書と実装の不一致を解消する必要あり。

**選択肢**:
1. `World`クラスを実装する（設計文書通り）
2. `GameContext`に統合する（現在の実装方針）
3. 後のフェーズで検討

### Phase 1完了条件
- [ ] GameContextが正常に初期化・終了できる
- [ ] 仮想FHD画面に図形を描画し、実画面にスケーリング表示できる
- [ ] 2つ以上のシステムが登録・実行できる
- [ ] メインループが60FPSで安定動作

### 所要時間
- 実装: 2週間
- テスト・デバッグ: 1週間
- **合計: 2-3週間**

---

## Phase 2: データ層 ⚪ 未着手

### 目標
JSON駆動設計の基盤を整備し、データファイルからゲーム定義を読み込めるようにする

### 実装タスク

#### 2.1 DefinitionRegistry (優先度: 最高)
- [ ] DefinitionRegistryクラス実装
- [ ] EntityDef, WaveDef, AbilityDef構造体定義
- [ ] 登録・取得API

**ファイル**:
- `include/new/Data/DefinitionRegistry.h`
- `include/new/Data/Definitions/EntityDef.h`
- `src/new/Data/DefinitionRegistry.cpp`

#### 2.2 データローダー (優先度: 最高)
- [ ] DataLoaderBaseクラス
- [ ] EntityLoader実装
- [ ] WaveLoader実装
- [ ] エラーハンドリング（try-catch必須）

**ファイル**:
- `include/new/Data/Loaders/DataLoaderBase.h`
- `include/new/Data/Loaders/EntityLoader.h`
- `src/new/Data/Loaders/EntityLoader.cpp`

**テスト**:
```cpp
// JSONファイルからエンティティ定義を読み込み
EntityLoader loader;
assert(loader.LoadFromFile("assets/new/definitions/entities/player.json"));
loader.RegisterTo(context.GetDefinitionRegistry());
```

#### 2.3 SchemaValidator (優先度: 中)
- [ ] 基本的なJSON検証（必須フィールド、型チェック）
- [ ] エラーメッセージ生成

**ファイル**:
- `include/new/Data/Validators/SchemaValidator.h`
- `src/new/Data/Validators/SchemaValidator.cpp`

#### 2.4 HotReloadSystem (優先度: 低)
- [ ] ファイル監視（基本版）
- [ ] 変更検出
- [ ] リロードコールバック

**ファイル**:
- `include/new/Game/Editor/HotReload/HotReloadSystem.h`
- `src/new/Game/Editor/HotReload/HotReloadSystem.cpp`

### Phase 2完了条件
- [ ] JSONファイルから最低1種類のエンティティ定義を読み込める
- [ ] 不正なJSONでも例外でクラッシュしない
- [ ] DefinitionRegistryから定義を取得できる
- [ ] (オプション) ファイル変更時にリロードされる

### 所要時間
- 実装: 1.5週間
- テスト: 0.5週間
- **合計: 2週間**

---

## Phase 3: 最小プレイアブル ⚪ 未着手

### 目標
直線型TDの最小プレイアブルバージョンを実装し、1ステージを最後までプレイできる

### 実装タスク

#### 3.1 基本コンポーネント (優先度: 最高)
- [ ] Transform, Sprite, Health, Lane等
- [ ] TD専用コンポーネント（LaneMovement等）

**ファイル**:
- `include/new/Core/Components/CoreComponents.h`
- `include/new/Domain/TD/LineTD/Components/LineTDComponents.h`

#### 3.2 基本システム (優先度: 最高)
- [ ] RenderSystem（スプライト描画）
- [ ] LaneMovementSystem
- [ ] SimpleCombatSystem（攻撃判定）

**ファイル**:
- `include/new/Game/Systems/RenderSystem.h`
- `include/new/Domain/TD/LineTD/Systems/LaneMovementSystem.h`

#### 3.3 スポーン・波管理 (優先度: 高)
- [ ] WaveManagerクラス
- [ ] SpawnSystem
- [ ] Wave定義JSONの読み込み

**ファイル**:
- `include/new/Domain/TD/LineTD/Managers/WaveManager.h`
- `src/new/Domain/TD/LineTD/Managers/WaveManager.cpp`

#### 3.4 最小UI (優先度: 中)
- [ ] コスト表示
- [ ] HP表示
- [ ] 勝敗判定表示

**実装方法**: Raylibの基本描画（ImGuiは次フェーズ）

#### 3.5 テストステージ (優先度: 高)
- [ ] `assets/new/definitions/stages/test_stage.json`
- [ ] 3レーン、2種類の敵、1種類のユニット

### Phase 3完了条件
- [ ] ステージ開始 → 敵出現 → ユニット配置 → 戦闘 → 勝敗判定
- [ ] 最低1ステージをクリア/ゲームオーバーまでプレイ可能
- [ ] 60FPS安定動作
- [ ] クラッシュなし

### 所要時間
- 実装: 2週間
- バランス調整・デバッグ: 1週間
- **合計: 3週間**

---

## Phase 4: エディタ統合 ⚪ 未着手

### 目標
F1/F2エディタを実装し、ゲーム内でエンティティとUIを編集可能にする

### 実装タスク

#### 4.1 DevModeManager (優先度: 最高)
- [ ] F1〜F4ショートカット処理
- [ ] エディタ切替
- [ ] ゲームビュー描画

**ファイル**:
- `include/new/Game/Editor/DevModeManager.h`
- `src/new/Game/Editor/DevModeManager.cpp`

#### 4.2 EditorBase (優先度: 高)
- [ ] 共通パネル（ResourceList, Viewport, Property）
- [ ] 3ペインレイアウト

**ファイル**:
- `include/new/Game/Editor/EditorBase.h`
- `src/new/Game/Editor/EditorBase.cpp`

#### 4.3 EntityEditor (F2) (優先度: 高)
- [ ] エンティティ一覧表示
- [ ] プロパティ編集
- [ ] 生JSON表示/編集

**ファイル**:
- `include/new/Game/Editor/Editors/EntityEditor.h`
- `src/new/Game/Editor/Editors/EntityEditor.cpp`

#### 4.4 UIEditor (F1) (優先度: 中)
- [ ] UIレイアウト一覧
- [ ] プロパティ編集
- [ ] プレビュー

**ファイル**:
- `include/new/Game/Editor/Editors/UIEditor.h`
- `src/new/Game/Editor/Editors/UIEditor.cpp`

#### 4.5 Workspace (優先度: 低)
- [ ] 状態保存/復元

### Phase 4完了条件
- [ ] F2キーでエンティティエディタが開く
- [ ] エンティティのパラメータを編集してホットリロードできる
- [ ] F1キーでUIエディタが開く（基本機能）
- [ ] ゲームとエディタを切り替えてプレイできる

### 所要時間
- 実装: 3週間
- UI調整: 1週間
- **合計: 4週間**

---

## Phase 5: 拡張機能 ⚪ 未着手

### 目標
全エディタ、MapTD、高度なアニメーション等の拡張機能を実装

### 実装タスク

#### 5.1 残りのエディタ
- [ ] StageEditor (F3)
- [ ] MotionEditor (F4)

#### 5.2 MapTD
- [ ] グリッドシステム
- [ ] パス定義
- [ ] タワー配置

#### 5.3 高度なアニメーション
- [ ] AnimationRegistryCache
- [ ] StateSystem
- [ ] AnimationPlaybackSystem

#### 5.4 その他システム
- [ ] パーティクル
- [ ] オーディオ
- [ ] カメラ制御

### Phase 5完了条件
- [ ] 全エディタが動作
- [ ] MapTDがプレイ可能
- [ ] アニメーションシステム完全動作

### 所要時間
- **4週間以上**（並行開発可能）

---

## MVP (Minimum Viable Product) 定義

Phase 3完了時点をMVPとする。

**MVPの条件**:
1. 直線型TDの1ステージが最後までプレイ可能
2. JSON定義からステージを読み込める
3. 60FPS安定動作
4. クラッシュなし

**MVPで提供しない機能**:
- 内部エディタ
- MapTD
- 高度なアニメーション
- パーティクル等の装飾

---

## 実装時の注意事項

### コーディング規約
- [.github/copilot-instructions.md](../.github/copilot-instructions.md) を遵守
- PODコンポーネント、DIパターン、グローバル変数禁止

### テスト方針
各フェーズで以下を実施:
1. ユニットテスト（Core/Data層）
2. 統合テスト（システム連携）
3. 手動プレイテスト

### パフォーマンス目標
- Update: <12ms
- Render: <8ms
- 同時エンティティ: 500体以上

### ドキュメント更新
実装後に以下を更新:
- このファイルの進捗状況
- 設計文書の実装ノート
- コード内のドキュメントコメント

---

## 次のステップ

**現在地**: Phase 1 進行中

**今週のタスク**:
1. GameContext の IResourceManager, IInputManager 実装
2. GameRenderer の座標変換メソッド実装
3. SystemRunner の優先度ソート実装

**来週の目標**:
- Phase 1 完了
- Phase 2 開始

---

**更新履歴**:
- 2025-12-06: 初版作成
