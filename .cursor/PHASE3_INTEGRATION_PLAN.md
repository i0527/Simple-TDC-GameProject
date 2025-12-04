## Phase 3: 既存実装統合計画

### 概要

このドキュメントは Phase 3「既存実装統合（TD/Roguelike）」の詳細な統合計画を記述します。

### 既存資産リスト

#### Roguelike

**コンポーネント** (`include/Domain/Roguelike/Components/`)
```
GridComponents.h        - グリッド座標、タイル、マップデータ
RoguelikeComponents.h   - プレイヤーエンティティ定義
TurnComponents.h        - ターンシステムコンポーネント
CombatComponents.h      - 戦闘関連コンポーネント
ItemComponents.h        - アイテムシステムコンポーネント
HungerComponents.h      - 満腹度システムコンポーネント
```

**システム** (`include/Domain/Roguelike/Systems/`)
```
ActionSystem.h          - 行動実行（移動、攻撃、待機）
AISystem.h              - 敵AI
CombatSystem.h          - 戦闘処理
FOVSystem.h             - 視野（Field of View）
HungerSystem.h          - 満腹度管理
InputSystem.h           - プレイヤー入力処理
ItemSystem.h            - アイテム管理
LevelUpSystem.h         - レベルアップ
```

**マネージャー** (`include/Domain/Roguelike/Managers/`)
```
TurnManager.h           - ターン管理
```

**互換性**
```
RoguelikeCompatibility.h - 既存コード互換性エイリアス
```

#### TD

**コンポーネント** (`include/Domain/TD/Components/`)
```
TDComponents.h          - TD固有コンポーネント（Lane, Stats等）
```

**システム** (`include/Domain/TD/Systems/`)
```
TDSystems.h             - 全TD関連システム（移動、戦闘、Wave等）
```

**マネージャー** (`include/Domain/TD/Managers/`)
```
WaveManager.h           - Wave管理
SpawnManager.h          - 敵出現管理
GameStateManager.h      - ゲーム状態管理
```

**互換性**
```
TDCompatibility.h       - 既存コード互換性エイリアス
```

### 統合手順

#### Phase 3.1: Roguelike システム統合

**目標**: RoguelikeGameScene で ActionSystem, AISystem, CombatSystem などを動作させる

**作業内容**:

1. `RoguelikeGameScene::RegisterRoguelikeSystems()` を拡張
   - ActionSystem の static 関数登録
   - AISystem の static 関数登録
   - CombatSystem の static 関数登録
   - FOVSystem の static 関数登録
   - その他必要なシステムを SystemRunner に登録

2. `RoguelikeGameScene::Update()` で SystemRunner を実行

3. 初期エンティティの生成
   - プレイヤーエンティティ（GridPosition, HP, Stats等を持つ）
   - 敵エンティティ

**参考実装パターン**:
```cpp
// UnifiedGame.cpp の RegisterTDSystems() を参考に
systemRunner_->Register(Core::SystemPhase::Update, "ActionSystem",
    [](Core::World& world, Core::GameContext& ctx, float dt) {
        // Roguelike::Systems::ActionSystem::Execute(...)
    }, priority);
```

#### Phase 3.2: ダンジョン生成統合

**目標**: RoguelikeGameScene で ダンジョンを自動生成

**作業内容**:

1. ダンジョン生成ライブラリの調査
   - `include/Domain/Roguelike/` に DungeonGenerator があるか確認
   - なければ BSPアルゴリズムの実装

2. `RoguelikeGameScene::GenerateDungeon()` 実装
   - MapData の初期化
   - ダンジョン生成アルゴリズムの実行
   - プレイヤー、敵、アイテムの初期配置

3. マップデータの GameContext 登録
   - `context_->Register<MapData>(mapData)` で保存
   - システムから アクセス可能にする

#### Phase 3.3: Roguelike 描画システム

**新規ファイル**: 
- `include/Application/Renderers/RoguelikeRenderer.h`
- `src/Application/Renderers/RoguelikeRenderer.cpp`

**実装内容**:

1. グリッド表示
2. タイル描画（床、壁、階段）
3. エンティティ描画（プレイヤー、敵）
4. UI描画（HP、ターン数、ステータス）
5. FOV表示

#### Phase 3.4: TD システム統合

**目標**: TDGameScene で TDSystems を動作させる

**作業内容**:

1. `TDGameScene::RegisterTDSystems()` 拡張
   - TDSystems.h の全システム登録
   - WaveManager, SpawnManager, GameStateManager の初期化

2. ステージ読み込み
   - JSON ステージ定義から レーン情報、敵波情報を読み込み
   - 敵配置情報を初期化

3. ゲーム終了条件
   - プレイヤー拠点 HP = 0（敗北）
   - 全Wave完了（勝利）

#### Phase 3.5: TD 描画システム

**新規ファイル**:
- `include/Application/Renderers/TDRenderer.h`
- `src/Application/Renderers/TDRenderer.cpp`

**実装内容**:

1. ゲームボード描画
2. レーン描画
3. ユニット描画（タワー、敵）
4. UI描画（HP、Wave、リソース）

#### Phase 3.6-7: 統合テスト

**テスト項目**:

1. ビルド確認
   - 全新規ファイル追加後に Release ビルド成功確認

2. 実行確認
   - ゲーム起動
   - Roguelike シーンでダンジョン表示確認
   - TD シーンでゲームボード表示確認

3. 機能確認
   - Roguelike: 移動、敵との遭遇、ターン制動作
   - TD: 敵出現、タワー配置、Wave切り替え

4. パフォーマンス
   - FPS 計測（60 FPS 維持確認）
   - メモリ使用量確認

### CMakeLists.txt 更新予定

新規ファイルを追加する場合のみ、以下を UNIFIED_SOURCES に追加：

```cmake
src/Application/Renderers/RoguelikeRenderer.cpp
src/Application/Renderers/TDRenderer.cpp
```

### 推奨実装順序

```
1. Phase 3.1: Roguelike システム統合     (3時間)
2. Phase 3.2: ダンジョン生成              (3時間)
3. Phase 3.3: Roguelike 描画              (4時間)
4. Phase 3.4: TD システム統合             (2時間)
5. Phase 3.5: TD 描画                     (3時間)
6. Phase 3.6-7: テスト・バグ修正          (2時間)

合計: 約 16時間
```

### 既存実装の特徴

- ✅ **POD型コンポーネント**: 全てのコンポーネントはシンプルなデータ構造
- ✅ **静的システム関数**: システムは static 関数として実装
- ✅ **SystemRunner 統合可能**: UnifiedGame の パターンに従える
- ✅ **互換性エイリアス**: 既存コードとの互換性確保

### 参考リンク

- `.cursor/UNIFIED_PLATFORM_TODO.md` - マスタープラン
- `docs/ROGUELIKE_SYSTEM_DESIGN.md` - Roguelike 設計
- `docs/CHARACTER_SYSTEM_DESIGN.md` - キャラクターシステム設計
- `include/Domain/Roguelike/` - Roguelike 実装
- `include/Domain/TD/` - TD 実装

---

**作成日**: 2025-12-04

