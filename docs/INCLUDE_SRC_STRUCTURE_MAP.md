# Include/Src 構造整理マップ

**更新日**: 2025-12-06  
**ステータス**: 📊 現状分析

---

## 1. ディレクトリ構造の概要

```
プロジェクトルート
│
├── include/          ← ヘッダーファイル（インターフェース定義）
│   ├── Application/  ← アプリケーション層
│   ├── Core/         ← コア層（ECS、DI、システム）
│   ├── Data/         ← データ層（定義、ローダー、シリアライザー）
│   ├── Domain/       ← ドメイン層（TD、Roguelike）
│   ├── Game/         ← ゲーム層（システム、エディタ）
│   ├── TD/           ← 互換性パッケージ（非推奨）
│   ├── Roguelike/    ← 旧構造（未使用）
│   ├── Scenes/       ← 旧構造（未使用）
│   └── ComponentsNew.h  ← 統合コンポーネント定義
│
└── src/              ← ソースファイル（実装）
    ├── Application/  ← アプリケーション層実装
    ├── Core/         ← コア層実装
    ├── Data/         ← データ層実装
    ├── Game/         ← ゲーム層実装
    ├── FileUtils.cpp ← ユーティリティ
    ├── main_unified.cpp ← エントリーポイント
    └── Scenes/       ← 旧構造（未使用）
```

---

## 2. レイヤー別の対応関係

### 2.1 Application Layer（アプリケーション層）

| ヘッダー | 説明 | ソース | 状態 |
|---------|------|--------|------|
| `Application/GameMode.h` | ゲームモード定義 | - | ✅ |
| `Application/IScene.h` | シーンインターフェース | - | ✅ |
| `Application/HomeScene.h` | ホームシーン | `HomeScene.cpp` | ✅ |
| `Application/TDGameScene.h` | TDゲームシーン | `TDGameScene.cpp` | ✅ |
| `Application/RoguelikeGameScene.h` | ローグライクシーン | `RoguelikeGameScene.cpp` | ✅ |
| `Application/UnifiedGame.h` | 統合ゲーム | `UnifiedGame.cpp` | ✅ |

**役割**:

- ゲームの統合制御
- シーン管理
- ゲームモード切り替え

---

### 2.2 Game Layer（ゲーム層）

#### Game/Components

| ヘッダー | 説明 | 状態 |
|---------|------|------|
| `Game/Components/GameComponents.h` | ゲーム共通コンポーネント | ✅ |

#### Game/Systems

| ヘッダー | ソース | 説明 | 状態 |
|---------|--------|------|------|
| `Game/Systems/AnimationSystem.h` | `AnimationSystem.cpp` | アニメーション制御 | ✅ |
| `Game/Systems/InputSystem.h` | `InputSystem.cpp` | 入力処理（ゲーム共通） | ✅ |
| `Game/Systems/MovementSystem.h` | `MovementSystem.cpp` | 移動ロジック | ✅ |
| `Game/Systems/RenderSystem.h` | `RenderSystem.cpp` | 描画ロジック | ✅ |

#### Game/DevMode（開発者モード）

| ヘッダー | ソース | 説明 | 状態 |
|---------|--------|------|------|
| `Game/DevMode/DevModeManager.h` | `DevModeManager.cpp` | 開発者モード統合 | ✅ |
| `Game/DevMode/GameViewRenderer.h` | `GameViewRenderer.cpp` | ゲームビュー描画 | ✅ |
| `Game/DevMode/WindowManager.h` | `WindowManager.cpp` | ウィンドウ管理 | ✅ |
| `Game/DevMode/Workspace.h` | `Workspace.cpp` | ワークスペース設定 | ✅ |
| `Game/DevMode/CharacterEditor.h` | `CharacterEditor.cpp` | キャラクターエディタ | ✅ |
| `Game/DevMode/Editors.h` | `Editors.cpp` | 各種エディタ | ✅ |
| `Game/DevMode/DebugTools.h` | `DebugTools.cpp` | デバッグツール | ✅ |

#### Game/Editor（旧エディタ）

| ヘッダー | ソース | 説明 | 状態 |
|---------|--------|------|------|
| `Game/Editor/EditorManager.h` | `EditorManager.cpp` | 旧エディタ | ⚠️ (DevModeに統合) |

**役割**:

- ゲーム共通システム（アニメーション、入力、移動、描画）
- 開発者モード（内部エディタ）
- ホットリロード対応

---

### 2.3 Core Layer（コア層）

#### Core/Components

| ヘッダー | 説明 | 状態 |
|---------|------|------|
| `Core/Components/CoreComponents.h` | コア共通コンポーネント | ✅ |

#### Core/Systems

| ヘッダー | 説明 | 状態 |
|---------|------|------|
| `Core/Systems/ISystem.h` | システムインターフェース | ✅ |
| `Core/Systems/SystemManager.h` | システム管理 | ✅ |

#### Core/ECS管理

| ヘッダー | 説明 | 状態 |
|---------|------|------|
| `Core/World.h` | ECSレジストリ（EnTT） | ✅ |
| `Core/GameContext.h` | DI コンテナ | ✅ |
| `Core/SystemRunner.h` | システム実行管理 | ✅ |

#### Core/リソース管理

| ヘッダー | 説明 | 状態 |
|---------|------|------|
| `Core/EffectManager.h` | エフェクト管理 | ✅ |
| `Core/SoundManager.h` | サウンド管理 | ✅ |
| `Core/GameRenderer.h` | ゲーム描画管理 | ✅ |
| `Core/FallbackRenderer.h` | フォールバック描画 | ✅ |

#### Core/ノードグラフ

| ヘッダー | 説明 | 状態 |
|---------|------|------|
| `Core/NodeGraph/Node.h` | ノード基底 | ✅ |
| `Core/NodeGraph/NodeGraph.h` | グラフ管理 | ✅ |
| `Core/NodeGraph/NodeRegistry.h` | ノード登録 | ✅ |
| `Core/NodeGraph/NodeExecutor.h` | ノード実行 | ✅ |
| `Core/NodeGraph/NodeTypes/*.h` | ノード型（Wave, Spawn, Logic） | ✅ |

#### Core/その他

| ヘッダー | ソース | 説明 | 状態 |
|---------|--------|------|------|
| `Core/HotReloadSystem.h` | `HotReloadSystem.cpp` | ホットリロード | ✅ |
| `Core/EntityFactory.h` | - | エンティティ生成 | ✅ |
| `Core/Platform.h` | - | Raylib ラッパー | ✅ |

**役割**:

- ECS システム（EnTT）
- 依存性注入（GameContext）
- リソース管理
- ノードグラフシステム
- Raylib ラッパー

---

### 2.4 Domain Layer（ドメイン層）

#### Domain/TD

| ヘッダー | 説明 | 状態 |
|---------|------|------|
| `Domain/TD/Components/TDComponents.h` | TDコンポーネント | ✅ |
| `Domain/TD/Systems/TDSystems.h` | TDシステム | ✅ |
| `Domain/TD/Managers/GameStateManager.h` | ゲーム状態 | ✅ |
| `Domain/TD/Managers/SpawnManager.h` | 敵生成 | ✅ |
| `Domain/TD/Managers/WaveManager.h` | ウェーブ管理 | ✅ |
| `Domain/TD/TDCompatibility.h` | 互換性エイリアス | ✅ |

#### Domain/Roguelike

| ヘッダー | 説明 | 状態 |
|---------|------|------|
| `Domain/Roguelike/Components/*.h` | ローグライクコンポーネント | ✅ |
| `Domain/Roguelike/Systems/*.h` | ローグライクシステム | ✅ |
| `Domain/Roguelike/Managers/TurnManager.h` | ターン管理 | ✅ |

**役割**:

- ドメイン固有の機能（TD, Roguelike）
- 互換性レイヤー（旧インターフェース対応）

---

### 2.5 Data Layer（データ層）

#### Data/Definitions

| ヘッダー | 説明 | 状態 |
|---------|------|------|
| `Data/Definitions/CommonTypes.h` | 共通型定義 | ✅ |
| `Data/Definitions/CharacterDef.h` | キャラクター定義 | ✅ |
| `Data/Definitions/StageDef.h` | ステージ定義 | ✅ |
| `Data/Definitions/MapDef.h` | マップ定義 | ✅ |
| `Data/Definitions/AnimationDef.h` | アニメーション定義 | ✅ |
| `Data/Definitions/UILayoutDef.h` | UIレイアウト定義 | ✅ |
| `Data/Definitions/StatusEffectDef.h` | ステータス効果定義 | ✅ |

#### Data/Loaders

| ヘッダー | ソース | 説明 | 状態 |
|---------|--------|------|------|
| `Data/Loaders/DataLoaderBase.h` | - | ローダー基底 | ✅ |
| `Data/Loaders/CharacterLoader.h` | - | キャラクターローダー | ✅ |
| `Data/Loaders/StageLoader.h` | - | ステージローダー | ✅ |
| `Data/Loaders/MapLoader.h` | - | マップローダー | ✅ |
| `Data/Loaders/UILoader.h` | - | UIローダー | ✅ |
| `Data/Loaders/DefinitionLoader.h` | - | 統合ローダー | ✅ |

#### Data/Serializers

| ヘッダー | ソース | 説明 | 状態 |
|---------|--------|------|------|
| `Data/Serializers/CharacterSerializer.h` | `CharacterSerializer.cpp` | キャラクター保存 | ✅ |
| `Data/Serializers/StageSerializer.h` | `StageSerializer.cpp` | ステージ保存 | ✅ |
| `Data/Serializers/UISerializer.h` | `UISerializer.cpp` | UI保存 | ✅ |

#### Data/Registry

| ヘッダー | 説明 | 状態 |
|---------|------|------|
| `Data/Registry.h` | 定義レジストリ | ✅ |

**役割**:

- JSON 定義の型化
- ローダー（JSON → 型変換）
- シリアライザー（型 → JSON）
- レジストリ（定義管理）

---

## 3. 問題のあるディレクトリ構造

### 🚨 include/TD/（非推奨）

```
include/TD/
  ├── Components/TDComponents.h    → ⚠️ include/Domain/TD/Components/TDComponents.h へリダイレクト
  ├── Managers/GameStateManager.h  → ⚠️ include/Domain/TD/Managers/ へリダイレクト
  ├── Systems/TDSystems.h          → ⚠️ include/Domain/TD/Systems/ へリダイレクト
  └── UI/GameUI.h                  → ⚠️ include/Domain/TD/UI/ へリダイレクト
```

**現状**: 互換性エイリアスで Domain/TD へリダイレクト  
**推奨**: `include/TD/` ディレクトリは削除可能（マイグレーション完了後）

### 🚨 include/Roguelike/（未使用）

```
include/Roguelike/
  ├── Components/    → 未実装
  ├── Generators/    → 未実装
  ├── Managers/      → 未実装
  ├── Rendering/     → 未実装
  └── Systems/       → 未実装
```

**現状**: 旧構造、実装なし  
**推奨**: 削除、`include/Domain/Roguelike/` に統一

### 🚨 include/Scenes/（未使用）

```
include/Scenes/     → 旧構造、空ディレクトリ
```

**推奨**: 削除

### 🚨 src/Scenes/（未使用）

```
src/Scenes/         → 旧構造、空ディレクトリ
```

**推奨**: 削除

---

## 4. インクルード依存関係（推奨される流れ）

```
main_unified.cpp
  ↓
Application/UnifiedGame.h/cpp
  ├─→ Application/HomeScene.h
  ├─→ Application/TDGameScene.h
  └─→ Application/RoguelikeGameScene.h
       ↓
    Game/DevMode/DevModeManager.h
    Game/Systems/*.h
       ↓
    Core/World.h
    Core/GameContext.h
    Core/SystemRunner.h
       ↓
    Domain/TD/*
    Domain/Roguelike/*
       ↓
    Data/Registry.h
    Data/Loaders/*
       ↓
    Core/Components/CoreComponents.h
    Core/Platform.h（Raylib）
       ↓
    External Libraries
    ├─ entt/entt.hpp
    ├─ nlohmann/json.hpp
    └─ raylib.h
```

---

## 5. ファイルマッピング表

### ✅ 実装済み（対応ペア）

| Category | Header | Source | 説明 |
|----------|--------|--------|------|
| **Application** | UnifiedGame.h | UnifiedGame.cpp | ✅ |
| | HomeScene.h | HomeScene.cpp | ✅ |
| | TDGameScene.h | TDGameScene.cpp | ✅ |
| | RoguelikeGameScene.h | RoguelikeGameScene.cpp | ✅ |
| **Game/Systems** | AnimationSystem.h | AnimationSystem.cpp | ✅ |
| | InputSystem.h | InputSystem.cpp | ✅ |
| | MovementSystem.h | MovementSystem.cpp | ✅ |
| | RenderSystem.h | RenderSystem.cpp | ✅ |
| **Game/DevMode** | DevModeManager.h | DevModeManager.cpp | ✅ |
| | GameViewRenderer.h | GameViewRenderer.cpp | ✅ |
| | WindowManager.h | WindowManager.cpp | ✅ |
| | Workspace.h | Workspace.cpp | ✅ |
| | CharacterEditor.h | CharacterEditor.cpp | ✅ |
| | Editors.h | Editors.cpp | ✅ |
| | DebugTools.h | DebugTools.cpp | ✅ |
| **Core** | HotReloadSystem.h | HotReloadSystem.cpp | ✅ |
| **Data/Serializers** | CharacterSerializer.h | CharacterSerializer.cpp | ✅ |
| | StageSerializer.h | StageSerializer.cpp | ✅ |
| | UISerializer.h | UISerializer.cpp | ✅ |

### ⚠️ ヘッダーのみ（実装なし）

| Header | 用途 | 状態 |
|--------|------|------|
| Core/World.h | ECS レジストリ | ヘッダーのみ（template） |
| Core/GameContext.h | DI コンテナ | ヘッダーのみ（template） |
| Core/EntityFactory.h | エンティティ生成 | ヘッダーのみ |
| Data/*.h (Definitions) | 定義型 | ヘッダーのみ |
| Game/Components/GameComponents.h | コンポーネント定義 | ヘッダーのみ |
| Core/Components/CoreComponents.h | コア コンポーネント | ヘッダーのみ |
| Domain/*/Components/*.h | ドメインコンポーネント | ヘッダーのみ |
| Domain/*/Systems/*.h | ドメインシステム | ヘッダーのみ（静的関数） |

---

## 6. クリーンアップ提案

### 削除対象

```
❌ include/TD/                  → Domain/TD へ統一（互換性レイヤー使用後）
❌ include/Roguelike/           → Domain/Roguelike へ統一
❌ include/Scenes/             → Application/ へ統一
❌ src/Scenes/                 → 削除
❌ Game/Editor/EditorManager.* → DevMode に統合済み
```

### リネーム/移動対象

```
⚠️ src/FileUtils.cpp           → Core/Utils/ へ移動検討
⚠️ include/ComponentsNew.h      → include/Game/Components/ComponentsNew.h へ移動検討
```

---

## 7. 推奨アクション（優先度順）

### 高優先度（即座）

1. ✅ **インクルード順序の標準化** - **完了**
2. ✅ **相対パス削除** - **完了**
3. ✅ **ドキュメント作成** - **完了**

### 中優先度（1週間以内）

1. 🔄 `include/TD/` と `include/Roguelike/` の削除（マイグレーション完了後）
2. 🔄 `include/Scenes/` と `src/Scenes/` の削除
3. 🔄 `src/FileUtils.cpp` の適切な配置

### 低優先度（1ヶ月以内）

1. 🔄 PCH（stdafx.h）の CMake 統合
2. 🔄 インクルード依存関係の可視化ツール導入
3. 🔄 `include/ComponentsNew.h` の適切な位置付け

---

## 8. まとめ

### 現状評価

| 項目 | 評価 | 理由 |
|------|------|------|
| **構造の一貫性** | 🟡 中程度 | Application/Game/Core/Domain/Data の階層化は完成 |
| **ファイル整理** | 🟡 中程度 | 旧構造（TD/, Roguelike/, Scenes/）が残存 |
| **インクルード最適化** | 🟢 良好 | 相対パス削除、順序統一完了 |
| **ドキュメント** | 🟢 良好 | ガイドラインとマップが整備 |

### 次のステップ

1. **旧ディレクトリ削除** - `include/TD/`, `include/Roguelike/`, `include/Scenes/`
2. **マイグレーション完了** - コードから旧インターフェース削除
3. **CMake 最適化** - PCH の有効化
4. **可視化ツール** - インクルード依存関係の自動分析

---

**作成者**: AI Assistant  
**最終更新**: 2025-12-06  
**ステータス**: 📊 分析完了
