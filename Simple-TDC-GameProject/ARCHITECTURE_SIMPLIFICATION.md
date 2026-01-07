# アーキテクチャ簡素化ガイド

## 概要
複雑なプロバイダーシステムからシンプルなTexture2D直接管理への移行

## 削除予定ファイル（deprecated/へ移動）

### 1. 複雑なスプライトプロバイダー系
- `game/managers/AsepriteJsonAtlasProvider.h/cpp`
- `game/managers/GridSheetProvider.h/cpp`
- `game/managers/SeparatedSpriteProvider.h`
- `game/managers/FrameProviderFactory.h`
- `game/managers/IFrameProvider.h`
- `game/managers/FrameRef.h`
- `game/managers/AnimClip.h`
- `game/managers/SpriteRenderer.h`
- `game/managers/SpriteSheetLoader.h/cpp`

### 2. 複雑なコア機能
- `game/core/FileWatcher.h/cpp`
- `game/core/EventSystem.h/cpp`
- `game/core/GameContext.h/cpp`
- `game/core/SettingsManager.h/cpp`
- `game/core/Signal.h`

### 3. 重複するシステム（utils/）
- `game/utils/AnimationSystem.h`
- `game/utils/AttackSystem.h`
- `game/utils/MovementSystem.h`
- `game/utils/SkillSystem.h`
- `game/utils/CharacterFactory.h/cpp`
- `game/utils/SimulationContext.h/cpp`

### 4. 重複するコンポーネント
- `game/ecs/components/CoreComponents.h`（NewCoreComponents.hに統合）

### 5. 重複するレンダリングシステム
- `game/ecs/systems/RenderingSystem.h/cpp`（NewRenderingSystemに統合）

### 6. 複雑なローダー系
- `game/managers/AbilityLoader.h/cpp`
- `game/managers/SkillLoader.h/cpp`
- `game/managers/DataValidator.h/cpp`
- `game/managers/DefinitionRegistry.h/cpp`
- `game/managers/FormationManager.h/cpp`
- `game/managers/UserDataManager.h/cpp`
- `game/managers/BgmService.h`
- `game/managers/EntityFactory.h`

### 7. 複雑なシーン
- `game/scenes/FormationScene.h/cpp`（仮実装に変更済み）
- `game/scenes/LoadingScene.h/cpp`（仮実装に変更済み）
- `game/scenes/StageSelectScene.h/cpp`（仮実装に変更済み）
- `game/scenes/TDGameScene.h/cpp`（GameSceneと統合予定）

## 残すファイル（最小構成）

### 必須ファイル
- `game/main/main.cpp`
- `game/main/GameApp.h/cpp`
- `game/core/FontManager.h/cpp`
- `game/managers/ResourceManager.h/cpp`（簡素化版）
- `game/scenes/IScene.h`
- `game/scenes/SceneManager.h/cpp`
- `game/scenes/TitleScene.h/cpp`
- `game/scenes/HomeScene.h/cpp`
- `game/scenes/GameScene.h/cpp`（簡素化版）

### ECSコンポーネント（統合版）
- `game/ecs/components/NewCoreComponents.h`（統合）

### ECSシステム（必要最小限）
- `game/ecs/systems/MovementSystem.h/cpp`
- `game/ecs/systems/AnimationSystem.h/cpp`
- `game/ecs/systems/AttackSystem.h/cpp`
- `game/ecs/systems/NewRenderingSystem.h/cpp`
- `game/ecs/systems/SkillSystem.h/cpp`
- `game/ecs/systems/DeadSystem.h/cpp`
- `game/ecs/systems/SpawnSystem.h/cpp`

### UI
- `game/ui/SettingsPanel.h/cpp`
- `game/ui/UiComponents.h`

## 移行手順

### フェーズ1: ファイル移動
1. deprecated/ディレクトリを作成
2. 不要なファイルをdeprecated/に移動
3. CMakeLists.txtを明示的なファイルリストに変更

### フェーズ2: 依存関係の解決
1. 残すファイルから削除ファイルへの依存を削除
2. ResourceManagerを簡素化（Texture2D直接管理）
3. GameSceneを簡素化

### フェーズ3: ビルド確認
1. ビルドエラーを修正
2. 最小限のゲームが動作することを確認

## 簡素化後の構造

```
game/
├── main/
│   ├── main.cpp
│   └── GameApp.h/cpp
├── core/
│   └── FontManager.h/cpp
├── managers/
│   └── ResourceManager.h/cpp (簡素化)
├── scenes/
│   ├── IScene.h
│   ├── SceneManager.h/cpp
│   ├── TitleScene.h/cpp
│   ├── HomeScene.h/cpp
│   └── GameScene.h/cpp (簡素化)
├── ecs/
│   ├── components/
│   │   └── Components.h (統合版)
│   └── systems/
│       ├── MovementSystem.h/cpp
│       ├── AnimationSystem.h/cpp
│       ├── AttackSystem.h/cpp
│       ├── RenderingSystem.h/cpp
│       ├── SkillSystem.h/cpp
│       ├── DeadSystem.h/cpp
│       └── SpawnSystem.h/cpp
└── ui/
    ├── SettingsPanel.h/cpp
    └── UiComponents.h

deprecated/ (削除予定ファイル)
└── (移動したファイル)
```

## 注意事項
- deprecated/ディレクトリはGitにコミットしない（.gitignoreに追加）
- 移行は段階的に行い、各フェーズでビルド確認
- 必要に応じてdeprecated/から復元可能
