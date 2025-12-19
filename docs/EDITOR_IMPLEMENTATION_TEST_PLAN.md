# エディタ実装テスト要綱

## 概要

エディタ実装3週間計画（Week 1-3）の実装完了に伴うテスト計画です。
SimulationContext、PreviewWindow、HotReload機能の動作確認を行います。

---

## テスト環境

- **OS**: Windows 10/11
- **ビルドシステム**: CMake + Ninja
- **コンパイラ**: MSVC
- **依存ライブラリ**: EnTT, Raylib, ImGui, nlohmann/json

---

## Week 1: SimulationContext テスト

### 1.1 基本機能テスト

#### テストケース: SC-001 エンティティ生成

**目的**: `SpawnEntity` が正常にエンティティを生成できることを確認

**手順**:

1. `SimulationContext` を初期化（`GameContext` と `DefinitionRegistry` を設定）
2. 有効な定義IDで `SpawnEntity("test_entity", Vector2{100, 200}, Team::Player)` を呼び出し
3. 戻り値が `entt::null` でないことを確認
4. `GetEntityCount()` が 1 を返すことを確認
5. 生成されたエンティティに `Transform`, `Team`, `Stats`, `EntityDefId` コンポーネントが存在することを確認

**期待結果**: エンティティが正常に生成され、必要なコンポーネントが付与される

#### テストケース: SC-002 無効な定義ID

**目的**: 存在しない定義IDでエンティティ生成を試みた場合のエラーハンドリング

**手順**:

1. `SpawnEntity("invalid_id", Vector2{0, 0}, Team::Player)` を呼び出し
2. 戻り値が `entt::null` であることを確認
3. エラーログが出力されることを確認

**期待結果**: `entt::null` が返され、エラーログが出力される

#### テストケース: SC-003 エンティティ削除

**目的**: `DestroyEntity` が正常にエンティティを削除できることを確認

**手順**:

1. エンティティを生成
2. `GetEntityCount()` でカウントを確認（1であることを確認）
3. `DestroyEntity(entity)` を呼び出し
4. `GetEntityCount()` が 0 を返すことを確認
5. `registry_.valid(entity)` が `false` を返すことを確認

**期待結果**: エンティティが正常に削除される

#### テストケース: SC-004 エンティティ検索

**目的**: `FindEntitiesByDefinition` が正常に動作することを確認

**手順**:

1. 同じ定義IDで3つのエンティティを生成
2. 別の定義IDで1つのエンティティを生成
3. `FindEntitiesByDefinition("test_entity")` を呼び出し
4. 結果が3つのエンティティを含むことを確認

**期待結果**: 指定した定義IDのエンティティのみが検索される

#### テストケース: SC-005 位置設定・取得

**目的**: `SetEntityPosition` と `GetEntityPosition` が正常に動作することを確認

**手順**:

1. エンティティを生成
2. `SetEntityPosition(entity, Vector2{300, 400})` を呼び出し
3. `GetEntityPosition(entity)` が `Vector2{300, 400}` を返すことを確認
4. `Transform` コンポーネントの `x`, `y` が更新されていることを確認

**期待結果**: 位置が正常に設定・取得される

#### テストケース: SC-006 Clear機能

**目的**: `Clear` が全エンティティを削除することを確認

**手順**:

1. 複数のエンティティを生成
2. `GetEntityCount()` でカウントを確認（0より大きいことを確認）
3. `Clear()` を呼び出し
4. `GetEntityCount()` が 0 を返すことを確認

**期待結果**: 全エンティティが削除される

### 1.2 CharacterFactory テスト

#### テストケース: CF-001 GridSheetProvider生成

**目的**: GridSheet形式のスプライトシートからProviderが生成されることを確認

**手順**:

1. `EntityDef` に `sprite_sheet` パスを設定
2. `CharacterFactory::CreateProvider` を呼び出し
3. `GridSheetProvider` インスタンスが返されることを確認
4. テクスチャが正常にロードされていることを確認

**期待結果**: GridSheetProviderが正常に生成される

#### テストケース: CF-002 AsepriteJsonAtlasProvider生成

**目的**: Aseprite形式のアトラスからProviderが生成されることを確認

**手順**:

1. `EntityDef` に `atlas_texture` と `sprite_actions` を設定
2. `CharacterFactory::CreateProvider` を呼び出し
3. `AsepriteJsonAtlasProvider` インスタンスが返されることを確認
4. JSONファイルが正常にパースされていることを確認

**期待結果**: AsepriteJsonAtlasProviderが正常に生成される

#### テストケース: CF-003 Providerキャッシュ

**目的**: 同じテクスチャパスでProviderがキャッシュされることを確認

**手順**:

1. 同じ `sprite_sheet` パスで2回 `CreateProvider` を呼び出し
2. 2回目の呼び出しがキャッシュから返されることを確認（ログまたはパフォーマンス測定）

**期待結果**: 2回目はキャッシュから返される

---

## Week 2: PreviewWindow テスト

### 2.1 基本機能テスト

#### テストケース: PW-001 ウィンドウ表示

**目的**: PreviewWindowが正常に表示されることを確認

**手順**:

1. `EditorApp` を起動
2. PreviewWindowが表示されることを確認
3. ウィンドウタイトルが "Preview" であることを確認

**期待結果**: PreviewWindowが正常に表示される

#### テストケース: PW-002 エンティティロード

**目的**: `LoadEntity` が正常にエンティティをロードできることを確認

**手順**:

1. PreviewWindowを開く
2. `LoadEntity("test_entity")` を呼び出し
3. プレビューエリアにエンティティが表示されることを確認
4. アニメーションが再生されることを確認

**期待結果**: エンティティが正常にロードされ、表示される

#### テストケース: PW-003 再生/停止制御

**目的**: アニメーションの再生/停止が正常に動作することを確認

**手順**:

1. エンティティをロード
2. 再生ボタンをクリック → アニメーションが再生されることを確認
3. 停止ボタンをクリック → アニメーションが停止することを確認

**期待結果**: 再生/停止が正常に動作する

#### テストケース: PW-004 速度制御

**目的**: アニメーション速度の変更が正常に動作することを確認

**手順**:

1. エンティティをロード
2. 速度スライダーを変更
3. アニメーション速度が変更されることを確認

**期待結果**: 速度制御が正常に動作する

#### テストケース: PW-005 PropertyPanel連携

**目的**: PropertyPanelにエンティティ情報が表示されることを確認

**手順**:

1. PreviewWindowでエンティティをロード
2. PropertyPanelを開く
3. エンティティのコンポーネント情報が表示されることを確認

**期待結果**: PropertyPanelに情報が表示される

---

## Week 3: HotReload テスト

### 3.1 DefinitionRegistry Signal テスト

#### テストケース: DR-001 Entity定義リロードSignal

**目的**: `OnEntityDefinitionReloaded` Signalが正常に発行されることを確認

**手順**:

1. `DefinitionRegistry` の `OnEntityDefinitionReloaded` にコールバックを登録
2. `OnFileChanged("entities_debug.json")` を呼び出し
3. コールバックが呼び出されることを確認
4. 引数が正しい定義IDであることを確認

**期待結果**: Signalが正常に発行される

#### テストケース: DR-002 Skill定義リロードSignal

**目的**: `OnSkillDefinitionReloaded` Signalが正常に発行されることを確認

**手順**:

1. `OnSkillDefinitionReloaded` にコールバックを登録
2. `OnFileChanged("skills_debug.json")` を呼び出し
3. コールバックが呼び出されることを確認

**期待結果**: Signalが正常に発行される

### 3.2 SimulationContext ReloadEntity テスト

#### テストケース: SC-007 ReloadEntity (PreserveState)

**目的**: `ReloadEntity` が状態を保持してリロードすることを確認

**手順**:

1. エンティティを生成し、位置を `(100, 200)` に設定
2. 定義ファイルを変更（例: HP値を変更）
3. `ReloadEntity(entity, ReloadPolicy::PreserveState)` を呼び出し
4. 位置が `(100, 200)` のままであることを確認
5. HP値が新しい定義に更新されることを確認

**期待結果**: 位置は保持され、定義値は更新される

#### テストケース: SC-008 ReloadEntity (ResetToDefault)

**目的**: `ReloadEntity` がデフォルト値にリセットすることを確認

**手順**:

1. エンティティを生成し、位置を `(100, 200)` に設定
2. `ReloadEntity(entity, ReloadPolicy::ResetToDefault)` を呼び出し
3. 位置がデフォルト値（定義の初期位置）にリセットされることを確認

**期待結果**: 位置がデフォルト値にリセットされる

#### テストケース: SC-009 ReloadAllInstances

**目的**: `ReloadAllInstances` が指定定義IDの全インスタンスをリロードすることを確認

**手順**:

1. 同じ定義IDで3つのエンティティを生成
2. 定義ファイルを変更
3. `ReloadAllInstances("test_entity", ReloadPolicy::PreserveState)` を呼び出し
4. 3つのエンティティすべてが更新されることを確認

**期待結果**: 全インスタンスがリロードされる

### 3.3 PreviewWindow HotReload統合テスト

#### テストケース: PW-006 自動リロード

**目的**: 定義ファイル変更時にPreviewWindowが自動的にリロードすることを確認

**手順**:

1. PreviewWindowでエンティティをロード
2. 定義ファイル（JSON）を外部エディタで編集・保存
3. `FileWatcher` が変更を検知し、`DefinitionRegistry::OnFileChanged` が呼び出されることを確認
4. PreviewWindowのエンティティが自動的にリロードされることを確認

**期待結果**: 自動リロードが正常に動作する

### 3.4 TDGameScene HotReload統合テスト

#### テストケース: TS-001 ゲーム中リロード

**目的**: ゲーム実行中に定義ファイルを変更した場合、該当エンティティがリロードされることを確認

**手順**:

1. TDGameSceneを起動し、エンティティをスポーン
2. 定義ファイルを変更（例: 攻撃力を変更）
3. `FileWatcher` が変更を検知
4. `TDGameScene::OnEntityDefinitionReloaded` が呼び出されることを確認
5. 該当エンティティが `ReloadEntity` でリロードされることを確認
6. ゲームが正常に継続することを確認

**期待結果**: ゲーム中にリロードが正常に動作する

---

## 統合テスト

### IT-001 エディタ→ゲーム連携

**目的**: エディタで定義を編集し、ゲームで即座に反映されることを確認

**手順**:

1. エディタでエンティティ定義を編集
2. ファイルを保存
3. ゲームを起動し、該当エンティティをスポーン
4. 変更が反映されていることを確認

**期待結果**: エディタの変更がゲームに即座に反映される

### IT-002 複数ウィンドウ同時動作

**目的**: PreviewWindowとPropertyPanelが同時に正常に動作することを確認

**手順**:

1. PreviewWindowとPropertyPanelを同時に開く
2. エンティティをロード
3. 両ウィンドウが正常に更新されることを確認

**期待結果**: 複数ウィンドウが正常に動作する

---

## パフォーマンステスト

### PT-001 大量エンティティ生成

**目的**: 100個のエンティティを生成した場合のパフォーマンスを確認

**手順**:

1. 100個のエンティティを生成
2. 生成時間を測定
3. メモリ使用量を確認

**期待結果**: 60FPSを維持できること

### PT-002 HotReloadパフォーマンス

**目的**: リロード処理のパフォーマンスを確認

**手順**:

1. 50個のエンティティを生成
2. 定義ファイルを変更
3. リロード時間を測定

**期待結果**: リロードが1秒以内に完了すること

---

## エラーハンドリングテスト

### EH-001 無効なJSONファイル

**目的**: 不正なJSONファイルをロードした場合のエラーハンドリング

**手順**:

1. 不正なJSONファイルをロード
2. エラーログが出力されることを確認
3. アプリケーションがクラッシュしないことを確認

**期待結果**: エラーが適切に処理される

### EH-002 存在しないテクスチャ

**目的**: 存在しないテクスチャパスを指定した場合のエラーハンドリング

**手順**:

1. 存在しないテクスチャパスを指定したエンティティを生成
2. エラーログが出力されることを確認
3. エンティティは生成されるが、スプライトが表示されないことを確認

**期待結果**: エラーが適切に処理される

---

## テスト実行手順

### 1. ユニットテスト実行

cd build/ninja
ctest --output-on-failure### 2. 手動テスト実行

1. ゲームアプリケーションを起動
2. エディタアプリケーションを起動
3. 各テストケースを手動で実行

### 3. テスト結果記録

- テスト結果を `docs/test_results/` に記録
- 失敗したテストケースはIssueとして登録

---

## テスト完了基準

- [ ] Week 1 テストケースすべて合格
- [ ] Week 2 テストケースすべて合格
- [ ] Week 3 テストケースすべて合格
- [ ] 統合テストすべて合格
- [ ] パフォーマンステスト合格
- [ ] エラーハンドリングテスト合格
- [ ] メモリリークなし（Valgrind/Application Verifierで確認）
- [ ] 60FPS維持（パフォーマンステスト）

---

## 既知の問題

現在、以下の問題が確認されています：

1. **Provider所有権管理**: `CharacterFactory` で生成されたProviderの所有権管理が未実装（TODO）
2. **RenderCommand実装**: `SimulationContext::GetRenderCommands` が未実装（Week 2/3で実装予定）

---

## 更新履歴

- 2025-01-XX: 初版作成

```

## 引継ぎ資料

```markdown:docs/EDITOR_IMPLEMENTATION_HANDOVER.md
# エディタ実装引継ぎ資料

## 概要

エディタ実装3週間計画（Week 1-3）の実装完了に伴う引継ぎ資料です。
SimulationContext、PreviewWindow、HotReload機能の実装内容と使用方法を記載します。

**実装完了日**: 2025-01-XX  
**実装者**: AI Assistant  
**レビュー状況**: 未レビュー

---

## 実装完了内容

### Week 1: SimulationContext 実装 ✅

#### 実装ファイル

**新規作成**:
- `shared/include/Core/Signal.h` - 型安全なイベントシグナル
- `shared/include/Shared/Simulation/SimulationContext.h` - シミュレーションコンテキスト
- `shared/src/Shared/Simulation/SimulationContext.cpp` - 実装
- `shared/include/Shared/Simulation/Factories/CharacterFactory.h` - エンティティファクトリ
- `shared/src/Shared/Simulation/Factories/CharacterFactory.cpp` - 実装
- `shared/include/Shared/Simulation/Systems/*.h` - システム前方宣言（4ファイル）

**変更**:
- `shared/include/Core/GameContext.h` - SimulationContext統合
- `shared/src/Core/GameContext.cpp` - SimulationContext初期化
- `game/include/Game/Managers/EntityFactory.h` - usingエイリアス追加（互換性維持）
- `game/src/Game/Scenes/TDGameScene.cpp` - SimulationContext経由に変更
- `game/src/Game/Application/GameApp.cpp` - SimulationContext統合

#### 主要機能

1. **SimulationContext**
   - エンティティ生成・削除・検索
   - 位置設定・取得
   - 更新ループ（現状は空実装、Week 2でSystems統合予定）
   - リロード機能（Week 3で実装）

2. **CharacterFactory**
   - `EntityFactory` から移動・リネーム
   - GridSheetProvider / AsepriteJsonAtlasProvider の自動選択
   - Providerキャッシュ機能

### Week 2: PreviewWindow 実装 ✅

#### 実装ファイル

**新規作成**:
- `editor/include/Editor/Windows/PreviewWindow.h` - プレビューウィンドウ
- `editor/src/Editor/Windows/PreviewWindow.cpp` - 実装
- `editor/include/Editor/Windows/PropertyPanel.h` - プロパティパネル
- `editor/src/Editor/Windows/PropertyPanel.cpp` - 実装

**変更**:
- `editor/include/Editor/Application/EditorApp.h` - PreviewWindow/PropertyPanel追加
- `editor/src/Editor/Application/EditorApp.cpp` - ウィンドウ統合
- `editor/CMakeLists.txt` - ソースファイル追加

#### 主要機能

1. **PreviewWindow**
   - エンティティプレビュー表示
   - 再生/停止/速度制御UI
   - RenderTexture2Dによる描画
   - グリッド表示オプション

2. **PropertyPanel**
   - エンティティコンポーネント情報表示
   - リアルタイム更新

### Week 3: HotReload 統合 ✅

#### 実装ファイル

**変更**:
- `shared/include/Data/DefinitionRegistry.h` - Signal型イベント追加
- `shared/src/Data/DefinitionRegistry.cpp` - OnFileChanged実装
- `shared/include/Shared/Simulation/SimulationContext.h` - ReloadEntity追加
- `shared/src/Shared/Simulation/SimulationContext.cpp` - ReloadEntity実装
- `editor/src/Editor/Windows/PreviewWindow.cpp` - Signal購読追加
- `game/src/Game/Scenes/TDGameScene.cpp` - Signal購読追加

#### 主要機能

1. **DefinitionRegistry**
   - `OnEntityDefinitionReloaded` Signal
   - `OnSkillDefinitionReloaded` Signal
   - `OnAbilityDefinitionReloaded` Signal
   - `OnFileChanged` メソッド（ファイル変更検知）

2. **SimulationContext**
   - `ReloadEntity` メソッド（3つのポリシー対応）
   - `ReloadAllInstances` メソッド

3. **統合**
   - PreviewWindow: 定義変更時に自動リロード
   - TDGameScene: ゲーム中に定義変更を検知してリロード

---

## アーキテクチャ概要

### レイヤー構造

```

Application Layer
├── Game (game/)
│   └── TDGameScene - SimulationContext経由でエンティティ管理
└── Editor (editor/)
    └── PreviewWindow - SimulationContext経由でプレビュー

Domain Layer
└── Simulation (shared/Simulation/)
    ├── SimulationContext - エンティティシミュレーション層
    └── CharacterFactory - エンティティ生成

Core Layer
└── Signal (shared/Core/)
    └── Signal<T> - 型安全なイベントシステム

```

### データフロー

```

定義ファイル変更
    ↓
FileWatcher検知
    ↓
DefinitionRegistry::OnFileChanged
    ↓
Signal発行 (OnEntityDefinitionReloaded)
    ↓
PreviewWindow / TDGameScene 購読
    ↓
SimulationContext::ReloadEntity
    ↓
エンティティ更新

```

---

## 使用方法

### SimulationContext の基本使用

```cpp
// 初期化
Shared::Core::GameContext context;
Shared::Data::DefinitionRegistry definitions;
context.Initialize("assets/config.json");
// ... definitions にデータをロード ...

// SimulationContext作成
Shared::Simulation::SimulationContext simulation(&context, &definitions);

// エンティティ生成
entt::entity entity = simulation.SpawnEntity(
    "test_entity",
    Vector2{100, 200},
    Game::Components::Team::Type::Player
);

// 位置設定
simulation.SetEntityPosition(entity, Vector2{300, 400});

// 更新
simulation.Update(deltaTime);

// 削除
simulation.DestroyEntity(entity);
```

### PreviewWindow の使用

```cpp
// EditorApp内で初期化
PreviewWindow preview;
preview.Initialize(context, definitions);

// エンティティロード
preview.LoadEntity("test_entity");

// 更新ループ
preview.OnUpdate(deltaTime);
preview.OnDrawUI();
```

### HotReload の使用

```cpp
// DefinitionRegistryのSignalに購読
definitions.OnEntityDefinitionReloaded.Connect([](const std::string& entityId) {
    std::cout << "Entity reloaded: " << entityId << std::endl;
    // リロード処理
});

// ファイル変更検知（FileWatcher経由で自動呼び出し）
definitions.OnFileChanged("assets/definitions/entities_debug.json");

// 手動リロード
simulation.ReloadEntity(entity, SimulationContext::ReloadPolicy::PreserveState);
simulation.ReloadAllInstances("test_entity", SimulationContext::ReloadPolicy::ResetToDefault);
```

---

## 重要な注意事項

### 1. Provider所有権管理

**現状**: `CharacterFactory` で生成されたProviderの所有権管理が未実装（TODO）

**問題**: 現在、Providerは `unique_ptr` で生成されるが、エンティティの `Sprite` コンポーネントには生ポインタで保存されている。Providerが破棄されると、エンティティのスプライトが無効になる。

**対応策（将来実装）**:

- `SimulationContext` または専用の `ProviderManager` でProviderの所有権を管理
- `Sprite` コンポーネントには `shared_ptr` またはID参照を使用

### 2. RenderCommand実装

**現状**: `SimulationContext::GetRenderCommands` が空実装（TODO）

**目的**: Week 2/3で実装予定だったが、現状は `NewRenderingSystem` が直接 `registry` を参照しているため未実装。

**対応策（将来実装）**:

- `NewRenderingSystem` を `SimulationContext` 経由に変更
- `GetRenderCommands` で描画コマンドを生成

### 3. Systems統合

**現状**: `SimulationContext::Update` が空実装

**目的**: Week 2でSystemsを `shared/Simulation/Systems/` に移動予定だったが、現状は `game/Systems/` に残っている。

**対応策（将来実装）**:

- `AnimationSystem`, `MovementSystem`, `AttackSystem`, `SkillSystem` を `shared/` に移動
- `SimulationContext::Update` でSystemsを呼び出し

### 4. 名前空間の整理

**現状**: `CharacterFactory` 内で `Game::Graphics::GridSheetProvider` などを直接使用

**問題**: `shared/` レイヤーから `game/` レイヤーを参照している（循環参照の可能性）

**対応策（将来実装）**:

- Providerインターフェースを `shared/` に移動
- 実装は `game/` に残し、ファクトリパターンで生成

---

## ビルド・実行方法

### ビルド

```bash
# CMake設定
cmake --preset ninja

# ビルド（ゲーム）
cmake --build --preset ninja-release --target SimpleTDCGame_NewArch

# ビルド（エディタ）
cmake --build --preset ninja-release --target SimpleTDCEditor
```

### 実行

```bash
# ゲーム実行
./build/ninja/bin/SimpleTDCGame_NewArch.exe

# エディタ実行
./build/ninja/bin/SimpleTDCEditor.exe
```

---

## テスト方法

詳細は `docs/EDITOR_IMPLEMENTATION_TEST_PLAN.md` を参照。

### クイックテスト

```bash
# ユニットテスト実行
cd build/ninja
ctest --output-on-failure

# 手動テスト
# 1. エディタを起動
# 2. PreviewWindowでエンティティをロード
# 3. 定義ファイルを編集・保存
# 4. 自動リロードを確認
```

---

## 既知の問題・制限事項

### 1. Provider所有権管理未実装

- **影響**: 長時間実行時にメモリリークの可能性
- **優先度**: 中
- **対応**: Week 4以降で実装予定

### 2. RenderCommand未実装

- **影響**: `SimulationContext` から描画コマンドを取得できない
- **優先度**: 低（現状は `NewRenderingSystem` が直接参照）
- **対応**: 将来のリファクタリング時に実装

### 3. Systems未統合

- **影響**: `SimulationContext::Update` が空実装
- **優先度**: 中
- **対応**: Week 4以降で実装予定

### 4. エラーハンドリング不足

- **影響**: 不正な定義ファイルでクラッシュする可能性
- **優先度**: 高
- **対応**: テスト実施時に修正

---

## 次のステップ

### 短期（1-2週間）

1. **テスト実施**
   - `EDITOR_IMPLEMENTATION_TEST_PLAN.md` に従ってテスト実行
   - バグ修正

2. **Provider所有権管理実装**
   - `ProviderManager` クラス作成
   - `SimulationContext` に統合

3. **エラーハンドリング強化**
   - try-catch追加
   - エラーログ改善

### 中期（1ヶ月）

1. **Systems統合**
   - Systemsを `shared/` に移動
   - `SimulationContext::Update` 実装

2. **RenderCommand実装**
   - `GetRenderCommands` 実装
   - `NewRenderingSystem` を `SimulationContext` 経由に変更

3. **パフォーマンス最適化**
   - Providerキャッシュ最適化
   - メモリプール実装

### 長期（3ヶ月）

1. **エディタ機能拡張**
   - アニメーション編集機能
   - スキル編集機能
   - ステージ編集機能

2. **ドキュメント整備**
   - APIリファレンス作成
   - チュートリアル作成

---

## 関連ドキュメント

- [エディタ実装3週間計画](../plans/エディタ実装3週間計画_545fea86.plan.md)
- [テスト要綱](EDITOR_IMPLEMENTATION_TEST_PLAN.md)
- [CHARACTER_RENDERING_DESIGN.md](CHARACTER_RENDERING_DESIGN.md)
- [IMPLEMENTATION_ROADMAP.md](IMPLEMENTATION_ROADMAP.md)

---

## 連絡先・サポート

質問や問題がある場合は、GitHub Issuesに登録してください。

---

## 更新履歴

- 2025-01-XX: 初版作成（実装完了時）

```

2つのドキュメントを作成しました。

1. **テスト要綱** (`docs/EDITOR_IMPLEMENTATION_TEST_PLAN.md`)
   - Week 1-3のテストケース
   - 統合テスト、パフォーマンステスト、エラーハンドリングテスト
   - テスト実行手順と完了基準

2. **引継ぎ資料** (`docs/EDITOR_IMPLEMENTATION_HANDOVER.md`)
   - 実装完了内容の概要
   - アーキテクチャと使用方法
   - 注意事項、既知の問題、次のステップ

これらのドキュメントを `docs/` ディレクトリに保存してください。必要に応じて内容を調整してください。

## 概要

エディタ実装3週間計画（Week 1-3）の実装完了に伴うテスト計画です。
SimulationContext、PreviewWindow、HotReload機能の動作確認を行います。

---

## テスト環境

- **OS**: Windows 10/11
- **ビルドシステム**: CMake + Ninja
- **コンパイラ**: MSVC
- **依存ライブラリ**: EnTT, Raylib, ImGui, nlohmann/json

---

## Week 1: SimulationContext テスト

### 1.1 基本機能テスト

#### テストケース: SC-001 エンティティ生成
**目的**: `SpawnEntity` が正常にエンティティを生成できることを確認

**手順**:
1. `SimulationContext` を初期化（`GameContext` と `DefinitionRegistry` を設定）
2. 有効な定義IDで `SpawnEntity("test_entity", Vector2{100, 200}, Team::Player)` を呼び出し
3. 戻り値が `entt::null` でないことを確認
4. `GetEntityCount()` が 1 を返すことを確認
5. 生成されたエンティティに `Transform`, `Team`, `Stats`, `EntityDefId` コンポーネントが存在することを確認

**期待結果**: エンティティが正常に生成され、必要なコンポーネントが付与される

#### テストケース: SC-002 無効な定義ID
**目的**: 存在しない定義IDでエンティティ生成を試みた場合のエラーハンドリング

**手順**:
1. `SpawnEntity("invalid_id", Vector2{0, 0}, Team::Player)` を呼び出し
2. 戻り値が `entt::null` であることを確認
3. エラーログが出力されることを確認

**期待結果**: `entt::null` が返され、エラーログが出力される

#### テストケース: SC-003 エンティティ削除
**目的**: `DestroyEntity` が正常にエンティティを削除できることを確認

**手順**:
1. エンティティを生成
2. `GetEntityCount()` でカウントを確認（1であることを確認）
3. `DestroyEntity(entity)` を呼び出し
4. `GetEntityCount()` が 0 を返すことを確認
5. `registry_.valid(entity)` が `false` を返すことを確認

**期待結果**: エンティティが正常に削除される

#### テストケース: SC-004 エンティティ検索
**目的**: `FindEntitiesByDefinition` が正常に動作することを確認

**手順**:
1. 同じ定義IDで3つのエンティティを生成
2. 別の定義IDで1つのエンティティを生成
3. `FindEntitiesByDefinition("test_entity")` を呼び出し
4. 結果が3つのエンティティを含むことを確認

**期待結果**: 指定した定義IDのエンティティのみが検索される

#### テストケース: SC-005 位置設定・取得
**目的**: `SetEntityPosition` と `GetEntityPosition` が正常に動作することを確認

**手順**:
1. エンティティを生成
2. `SetEntityPosition(entity, Vector2{300, 400})` を呼び出し
3. `GetEntityPosition(entity)` が `Vector2{300, 400}` を返すことを確認
4. `Transform` コンポーネントの `x`, `y` が更新されていることを確認

**期待結果**: 位置が正常に設定・取得される

#### テストケース: SC-006 Clear機能
**目的**: `Clear` が全エンティティを削除することを確認

**手順**:
1. 複数のエンティティを生成
2. `GetEntityCount()` でカウントを確認（0より大きいことを確認）
3. `Clear()` を呼び出し
4. `GetEntityCount()` が 0 を返すことを確認

**期待結果**: 全エンティティが削除される

### 1.2 CharacterFactory テスト

#### テストケース: CF-001 GridSheetProvider生成
**目的**: GridSheet形式のスプライトシートからProviderが生成されることを確認

**手順**:
1. `EntityDef` に `sprite_sheet` パスを設定
2. `CharacterFactory::CreateProvider` を呼び出し
3. `GridSheetProvider` インスタンスが返されることを確認
4. テクスチャが正常にロードされていることを確認

**期待結果**: GridSheetProviderが正常に生成される

#### テストケース: CF-002 AsepriteJsonAtlasProvider生成
**目的**: Aseprite形式のアトラスからProviderが生成されることを確認

**手順**:
1. `EntityDef` に `atlas_texture` と `sprite_actions` を設定
2. `CharacterFactory::CreateProvider` を呼び出し
3. `AsepriteJsonAtlasProvider` インスタンスが返されることを確認
4. JSONファイルが正常にパースされていることを確認

**期待結果**: AsepriteJsonAtlasProviderが正常に生成される

#### テストケース: CF-003 Providerキャッシュ
**目的**: 同じテクスチャパスでProviderがキャッシュされることを確認

**手順**:
1. 同じ `sprite_sheet` パスで2回 `CreateProvider` を呼び出し
2. 2回目の呼び出しがキャッシュから返されることを確認（ログまたはパフォーマンス測定）

**期待結果**: 2回目はキャッシュから返される

---

## Week 2: PreviewWindow テスト

### 2.1 基本機能テスト

#### テストケース: PW-001 ウィンドウ表示
**目的**: PreviewWindowが正常に表示されることを確認

**手順**:
1. `EditorApp` を起動
2. PreviewWindowが表示されることを確認
3. ウィンドウタイトルが "Preview" であることを確認

**期待結果**: PreviewWindowが正常に表示される

#### テストケース: PW-002 エンティティロード
**目的**: `LoadEntity` が正常にエンティティをロードできることを確認

**手順**:
1. PreviewWindowを開く
2. `LoadEntity("test_entity")` を呼び出し
3. プレビューエリアにエンティティが表示されることを確認
4. アニメーションが再生されることを確認

**期待結果**: エンティティが正常にロードされ、表示される

#### テストケース: PW-003 再生/停止制御
**目的**: アニメーションの再生/停止が正常に動作することを確認

**手順**:
1. エンティティをロード
2. 再生ボタンをクリック → アニメーションが再生されることを確認
3. 停止ボタンをクリック → アニメーションが停止することを確認

**期待結果**: 再生/停止が正常に動作する

#### テストケース: PW-004 速度制御
**目的**: アニメーション速度の変更が正常に動作することを確認

**手順**:
1. エンティティをロード
2. 速度スライダーを変更
3. アニメーション速度が変更されることを確認

**期待結果**: 速度制御が正常に動作する

#### テストケース: PW-005 PropertyPanel連携
**目的**: PropertyPanelにエンティティ情報が表示されることを確認

**手順**:
1. PreviewWindowでエンティティをロード
2. PropertyPanelを開く
3. エンティティのコンポーネント情報が表示されることを確認

**期待結果**: PropertyPanelに情報が表示される

---

## Week 3: HotReload テスト

### 3.1 DefinitionRegistry Signal テスト

#### テストケース: DR-001 Entity定義リロードSignal
**目的**: `OnEntityDefinitionReloaded` Signalが正常に発行されることを確認

**手順**:
1. `DefinitionRegistry` の `OnEntityDefinitionReloaded` にコールバックを登録
2. `OnFileChanged("entities_debug.json")` を呼び出し
3. コールバックが呼び出されることを確認
4. 引数が正しい定義IDであることを確認

**期待結果**: Signalが正常に発行される

#### テストケース: DR-002 Skill定義リロードSignal
**目的**: `OnSkillDefinitionReloaded` Signalが正常に発行されることを確認

**手順**:
1. `OnSkillDefinitionReloaded` にコールバックを登録
2. `OnFileChanged("skills_debug.json")` を呼び出し
3. コールバックが呼び出されることを確認

**期待結果**: Signalが正常に発行される

### 3.2 SimulationContext ReloadEntity テスト

#### テストケース: SC-007 ReloadEntity (PreserveState)
**目的**: `ReloadEntity` が状態を保持してリロードすることを確認

**手順**:
1. エンティティを生成し、位置を `(100, 200)` に設定
2. 定義ファイルを変更（例: HP値を変更）
3. `ReloadEntity(entity, ReloadPolicy::PreserveState)` を呼び出し
4. 位置が `(100, 200)` のままであることを確認
5. HP値が新しい定義に更新されることを確認

**期待結果**: 位置は保持され、定義値は更新される

#### テストケース: SC-008 ReloadEntity (ResetToDefault)
**目的**: `ReloadEntity` がデフォルト値にリセットすることを確認

**手順**:
1. エンティティを生成し、位置を `(100, 200)` に設定
2. `ReloadEntity(entity, ReloadPolicy::ResetToDefault)` を呼び出し
3. 位置がデフォルト値（定義の初期位置）にリセットされることを確認

**期待結果**: 位置がデフォルト値にリセットされる

#### テストケース: SC-009 ReloadAllInstances
**目的**: `ReloadAllInstances` が指定定義IDの全インスタンスをリロードすることを確認

**手順**:
1. 同じ定義IDで3つのエンティティを生成
2. 定義ファイルを変更
3. `ReloadAllInstances("test_entity", ReloadPolicy::PreserveState)` を呼び出し
4. 3つのエンティティすべてが更新されることを確認

**期待結果**: 全インスタンスがリロードされる

### 3.3 PreviewWindow HotReload統合テスト

#### テストケース: PW-006 自動リロード
**目的**: 定義ファイル変更時にPreviewWindowが自動的にリロードすることを確認

**手順**:
1. PreviewWindowでエンティティをロード
2. 定義ファイル（JSON）を外部エディタで編集・保存
3. `FileWatcher` が変更を検知し、`DefinitionRegistry::OnFileChanged` が呼び出されることを確認
4. PreviewWindowのエンティティが自動的にリロードされることを確認

**期待結果**: 自動リロードが正常に動作する

### 3.4 TDGameScene HotReload統合テスト

#### テストケース: TS-001 ゲーム中リロード
**目的**: ゲーム実行中に定義ファイルを変更した場合、該当エンティティがリロードされることを確認

**手順**:
1. TDGameSceneを起動し、エンティティをスポーン
2. 定義ファイルを変更（例: 攻撃力を変更）
3. `FileWatcher` が変更を検知
4. `TDGameScene::OnEntityDefinitionReloaded` が呼び出されることを確認
5. 該当エンティティが `ReloadEntity` でリロードされることを確認
6. ゲームが正常に継続することを確認

**期待結果**: ゲーム中にリロードが正常に動作する

---

## 統合テスト

### IT-001 エディタ→ゲーム連携
**目的**: エディタで定義を編集し、ゲームで即座に反映されることを確認

**手順**:
1. エディタでエンティティ定義を編集
2. ファイルを保存
3. ゲームを起動し、該当エンティティをスポーン
4. 変更が反映されていることを確認

**期待結果**: エディタの変更がゲームに即座に反映される

### IT-002 複数ウィンドウ同時動作
**目的**: PreviewWindowとPropertyPanelが同時に正常に動作することを確認

**手順**:
1. PreviewWindowとPropertyPanelを同時に開く
2. エンティティをロード
3. 両ウィンドウが正常に更新されることを確認

**期待結果**: 複数ウィンドウが正常に動作する

---

## パフォーマンステスト

### PT-001 大量エンティティ生成
**目的**: 100個のエンティティを生成した場合のパフォーマンスを確認

**手順**:
1. 100個のエンティティを生成
2. 生成時間を測定
3. メモリ使用量を確認

**期待結果**: 60FPSを維持できること

### PT-002 HotReloadパフォーマンス
**目的**: リロード処理のパフォーマンスを確認

**手順**:
1. 50個のエンティティを生成
2. 定義ファイルを変更
3. リロード時間を測定

**期待結果**: リロードが1秒以内に完了すること

---

## エラーハンドリングテスト

### EH-001 無効なJSONファイル
**目的**: 不正なJSONファイルをロードした場合のエラーハンドリング

**手順**:
1. 不正なJSONファイルをロード
2. エラーログが出力されることを確認
3. アプリケーションがクラッシュしないことを確認

**期待結果**: エラーが適切に処理される

### EH-002 存在しないテクスチャ
**目的**: 存在しないテクスチャパスを指定した場合のエラーハンドリング

**手順**:
1. 存在しないテクスチャパスを指定したエンティティを生成
2. エラーログが出力されることを確認
3. エンティティは生成されるが、スプライトが表示されないことを確認

**期待結果**: エラーが適切に処理される

---

## テスト実行手順

### 1. ユニットテスト実行
cd build/ninja
ctest --output-on-failure### 2. 手動テスト実行
1. ゲームアプリケーションを起動
2. エディタアプリケーションを起動
3. 各テストケースを手動で実行

### 3. テスト結果記録
- テスト結果を `docs/test_results/` に記録
- 失敗したテストケースはIssueとして登録

---

## テスト完了基準

- [ ] Week 1 テストケースすべて合格
- [ ] Week 2 テストケースすべて合格
- [ ] Week 3 テストケースすべて合格
- [ ] 統合テストすべて合格
- [ ] パフォーマンステスト合格
- [ ] エラーハンドリングテスト合格
- [ ] メモリリークなし（Valgrind/Application Verifierで確認）
- [ ] 60FPS維持（パフォーマンステスト）

---

## 既知の問題

現在、以下の問題が確認されています：

1. **Provider所有権管理**: `CharacterFactory` で生成されたProviderの所有権管理が未実装（TODO）
2. **RenderCommand実装**: `SimulationContext::GetRenderCommands` が未実装（Week 2/3で実装予定）

---

## 更新履歴

- 2025-01-XX: 初版作成
