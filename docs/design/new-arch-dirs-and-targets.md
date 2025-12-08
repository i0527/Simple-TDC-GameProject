# 新アーキテクチャ ディレクトリ構成とビルドターゲット設計

> **重要**: このドキュメントは `include/new/`, `src/new/` 配下の新アーキテクチャのディレクトリ構成とCMakeビルドターゲット設計を定義します。

## 目的

- 新アーキテクチャを既存コードから完全に分離
- 新旧両方のビルドターゲットを並行して維持可能にする
- 段階的移行を可能にする設計
- アセットパスの衝突を回避

---

## 1. ディレクトリ構成

### 1.1 新アーキテクチャのディレクトリ構造

```
Simple-TDC-GameProject/
├── include/
│   ├── new/                    # 新アーキテクチャ（完全分離）
│   │   ├── Application/        # アプリケーション層
│   │   │   ├── NewGame.h       # 新ゲームエントリポイント
│   │   │   └── GameMode.h      # ゲームモード管理
│   │   ├── Core/               # コア層（新設計）
│   │   │   ├── Components/     # コンポーネント定義
│   │   │   │   └── CoreComponents.h
│   │   │   ├── Systems/        # システム基底
│   │   │   │   └── ISystem.h
│   │   │   ├── GameContext.h   # DIコンテナ（新設計）
│   │   │   ├── World.h         # ECS世界（新設計）
│   │   │   ├── GameRenderer.h  # レンダリング（仮想FHD対応）
│   │   │   └── SystemRunner.h  # システム実行管理
│   │   ├── Data/               # データ層（新設計）
│   │   │   ├── Definitions/    # JSON定義構造体
│   │   │   │   ├── EntityDef.h
│   │   │   │   ├── WaveDef.h
│   │   │   │   ├── AbilityDef.h
│   │   │   │   └── UILayoutDef.h
│   │   │   ├── Loaders/        # データローダー
│   │   │   │   ├── EntityLoader.h
│   │   │   │   ├── WaveLoader.h
│   │   │   │   └── DataLoaderBase.h
│   │   │   ├── Validators/     # JSONバリデータ
│   │   │   │   ├── SchemaValidator.h
│   │   │   │   └── EntityValidator.h
│   │   │   └── Registry.h      # 定義レジストリ
│   │   ├── Game/               # ゲーム層（新設計）
│   │   │   ├── Components/     # ゲームコンポーネント
│   │   │   │   └── GameComponents.h
│   │   │   ├── Systems/        # ゲームシステム
│   │   │   │   ├── RenderSystem.h
│   │   │   │   ├── InputSystem.h
│   │   │   │   ├── MovementSystem.h
│   │   │   │   └── AnimationSystem.h
│   │   │   └── Editor/         # 内部エディタ（新設計）
│   │   │       ├── DevModeManager.h
│   │   │       ├── GameViewRenderer.h
│   │   │       ├── WindowManager.h
│   │   │       ├── Workspace.h
│   │   │       ├── Editors/    # 各種エディタ
│   │   │       │   ├── EntityEditor.h
│   │   │       │   ├── WaveEditor.h
│   │   │       │   ├── AbilityEditor.h
│   │   │       │   └── UIEditor.h
│   │   │       └── HotReload/  # ホットリロード
│   │   │           └── HotReloadSystem.h
│   │   └── Domain/             # ドメイン層（新設計）
│   │       ├── TD/             # タワーディフェンス
│   │       │   ├── LineTD/    # 直線型TD
│   │       │   │   ├── Components/
│   │       │   │   ├── Systems/
│   │       │   │   └── Managers/
│   │       │   └── MapTD/      # マップ型TD
│   │       │       ├── Components/
│   │       │       ├── Systems/
│   │       │       └── Managers/
│   │       └── UI/             # UI定義システム
│   │           ├── Components/
│   │           ├── Systems/
│   │           └── UIRenderer.h
│   └── [既存のinclude/]        # 旧アーキテクチャ（保持）
│
├── src/
│   ├── new/                    # 新アーキテクチャ実装（完全分離）
│   │   ├── Application/
│   │   │   └── NewGame.cpp
│   │   ├── Core/
│   │   │   ├── GameContext.cpp
│   │   │   ├── World.cpp
│   │   │   ├── GameRenderer.cpp
│   │   │   └── SystemRunner.cpp
│   │   ├── Data/
│   │   │   ├── Loaders/
│   │   │   └── Validators/
│   │   ├── Game/
│   │   │   ├── Systems/
│   │   │   └── Editor/
│   │   └── Domain/
│   │       ├── TD/
│   │       └── UI/
│   └── [既存のsrc/]            # 旧アーキテクチャ（保持）
│
├── assets/
│   ├── new/                    # 新アーキテクチャ用アセット（オプション）
│   │   ├── definitions/        # JSON定義ファイル
│   │   │   ├── entities/
│   │   │   ├── waves/
│   │   │   ├── abilities/
│   │   │   └── ui/
│   │   ├── textures/
│   │   └── fonts/
│   └── [既存のassets/]         # 旧アーキテクチャ用アセット（保持）
│
└── src/
    └── main_new.cpp            # 新アーキテクチャエントリポイント
```

### 1.2 レイヤー構造の原則

```
Application Layer (new/Application/)
    ↓ 依存
Game Layer (new/Game/)
    ↓ 依存
Domain Layer (new/Domain/)
    ↓ 依存
Core Layer (new/Core/)
    ↓ 依存
Data Layer (new/Data/)
    ↓ 依存
External Libraries (EnTT, Raylib, nlohmann/json)
```

**重要**: 上位レイヤーから下位レイヤーへの一方向依存のみ許可。循環依存は禁止。

---

## 2. CMakeビルドターゲット設計

### 2.1 ターゲット構成

```cmake
# ============================================================================
# ビルドターゲット一覧
# ============================================================================
# 1. SimpleTDCGame (旧アーキテクチャ)
#    - 状態: 保守モード（バグ修正のみ）
#    - 用途: 後方互換性維持、移行完了まで保持
#    - 推奨: 新規開発では使用しない
#
# 2. SimpleTDCGame_NewArch (新アーキテクチャ・推奨)
#    - 状態: アクティブ開発
#    - 用途: 新アーキテクチャの実装・テスト
#    - 推奨: メイン開発ターゲット
#    - エントリポイント: src/main_new.cpp
#    - ソース: include/new/, src/new/
# ============================================================================
```

### 2.2 CMakeLists.txt への追加

```cmake
# ============================================================================
# 新アーキテクチャ: SimpleTDCGame_NewArch
# ============================================================================

# 新アーキテクチャのソースファイル定義
set(NEW_ARCH_SOURCES
    # エントリポイント
    src/main_new.cpp
    
    # Application層
    src/new/Application/NewGame.cpp
    
    # Core層
    src/new/Core/GameContext.cpp
    src/new/Core/World.cpp
    src/new/Core/GameRenderer.cpp
    src/new/Core/SystemRunner.cpp
    
    # Data層
    src/new/Data/Loaders/EntityLoader.cpp
    src/new/Data/Loaders/WaveLoader.cpp
    src/new/Data/Validators/SchemaValidator.cpp
    src/new/Data/Registry.cpp
    
    # Game層
    src/new/Game/Systems/RenderSystem.cpp
    src/new/Game/Systems/InputSystem.cpp
    src/new/Game/Systems/MovementSystem.cpp
    src/new/Game/Systems/AnimationSystem.cpp
    
    # Game層 - Editor
    src/new/Game/Editor/DevModeManager.cpp
    src/new/Game/Editor/GameViewRenderer.cpp
    src/new/Game/Editor/WindowManager.cpp
    src/new/Game/Editor/Workspace.cpp
    src/new/Game/Editor/Editors/EntityEditor.cpp
    src/new/Game/Editor/Editors/WaveEditor.cpp
    src/new/Game/Editor/Editors/AbilityEditor.cpp
    src/new/Game/Editor/Editors/UIEditor.cpp
    src/new/Game/Editor/HotReload/HotReloadSystem.cpp
    
    # Domain層 - TD (直線型)
    src/new/Domain/TD/LineTD/Systems/LineTDRenderSystem.cpp
    src/new/Domain/TD/LineTD/Systems/LineTDSpawnSystem.cpp
    src/new/Domain/TD/LineTD/Managers/WaveManager.cpp
    
    # Domain層 - TD (マップ型)
    src/new/Domain/TD/MapTD/Systems/MapTDRenderSystem.cpp
    src/new/Domain/TD/MapTD/Systems/MapTDPathSystem.cpp
    src/new/Domain/TD/MapTD/Managers/GridManager.cpp
    
    # Domain層 - UI
    src/new/Domain/UI/Systems/UIRenderSystem.cpp
    src/new/Domain/UI/UIRenderer.cpp
)

# 新アーキテクチャ実行ファイル作成
add_executable(SimpleTDCGame_NewArch ${NEW_ARCH_SOURCES})

# インクルードディレクトリ（新アーキテクチャ優先）
target_include_directories(SimpleTDCGame_NewArch
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/new    # 新アーキテクチャ優先
        ${CMAKE_CURRENT_SOURCE_DIR}/include          # 既存（必要時のみ）
        ${raygui_SOURCE_DIR}/src
        ${dearimgui_SOURCE_DIR}
        ${rlimgui_SOURCE_DIR}
)

# リンクライブラリ（既存と同じ）
target_link_libraries(SimpleTDCGame_NewArch
    PRIVATE
        EnTT::EnTT
        nlohmann_json::nlohmann_json
        raylib
        rlimgui_lib
        imgui_lib
)

# アセットコピー（新アーキテクチャ用）
add_custom_command(TARGET SimpleTDCGame_NewArch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:SimpleTDCGame_NewArch>/assets
    COMMENT "Copying assets to build directory..."
)

# 新アーキテクチャ用アセット（オプション）
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets/new)
    add_custom_command(TARGET SimpleTDCGame_NewArch POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/new
            $<TARGET_FILE_DIR:SimpleTDCGame_NewArch>/assets/new
        COMMENT "Copying new architecture assets..."
    )
endif()
```

### 2.3 ビルドコマンド

```bash
# 新アーキテクチャのみビルド
cmake --build build --target SimpleTDCGame_NewArch

# 旧アーキテクチャのみビルド
cmake --build build --target SimpleTDCGame

# 両方ビルド
cmake --build build
```

---

## 3. アセットパス管理

### 3.1 アセットパスの分離方針

**原則**: 新アーキテクチャは既存アセットを参照可能だが、新規アセットは `assets/new/` に配置。

```cpp
// 新アーキテクチャでのアセットパス解決
namespace New::Assets {
    // 既存アセットへのアクセス（後方互換）
    constexpr const char* LEGACY_DEFINITIONS = "assets/definitions/";
    
    // 新アーキテクチャ専用アセット
    constexpr const char* NEW_DEFINITIONS = "assets/new/definitions/";
    
    // アセットパス解決関数
    std::string ResolvePath(const std::string& relativePath, bool useNew = true) {
        if (useNew) {
            std::string newPath = NEW_DEFINITIONS + relativePath;
            if (FileExists(newPath)) {
                return newPath;
            }
        }
        // フォールバック: 既存アセット
        return LEGACY_DEFINITIONS + relativePath;
    }
}
```

### 3.2 JSON定義ファイルの配置

```
assets/
├── definitions/              # 既存（旧アーキテクチャ）
│   ├── characters/
│   ├── stages/
│   └── ui/
│
└── new/                     # 新アーキテクチャ（オプション）
    └── definitions/
        ├── entities/        # エンティティ定義
        ├── waves/           # 波定義
        ├── abilities/       # スキル/能力定義
        └── ui/              # UIレイアウト定義
```

**重要**: `assets/new/` が存在しない場合は、既存の `assets/definitions/` を使用（後方互換）。

---

## 4. インクルードパス設計

### 4.1 新アーキテクチャでのインクルード規則

```cpp
// ✅ 推奨: 新アーキテクチャ内でのインクルード
#include "Core/GameContext.h"           // new/Core/GameContext.h
#include "Game/Systems/RenderSystem.h"  // new/Game/Systems/RenderSystem.h
#include "Domain/TD/LineTD/Systems/LineTDRenderSystem.h"

// ❌ 禁止: 既存アーキテクチャへの直接参照
#include "Application/UnifiedGame.h"    // 旧アーキテクチャ
#include "Core/World.h"                 // 旧Core層

// ✅ 許可: 外部ライブラリは直接インクルード
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include "raylib.h"
```

### 4.2 名前空間設計

```cpp
// 新アーキテクチャの名前空間
namespace New {
    // Application層
    namespace Application {
        class NewGame;
    }
    
    // Core層
    namespace Core {
        class GameContext;
        class World;
        class GameRenderer;
    }
    
    // Data層
    namespace Data {
        namespace Definitions {
            struct EntityDef;
        }
        class EntityLoader;
    }
    
    // Game層
    namespace Game {
        namespace Systems {
            class RenderSystem;
        }
        namespace Editor {
            class DevModeManager;
        }
    }
    
    // Domain層
    namespace Domain {
        namespace TD {
            namespace LineTD {
                class WaveManager;
            }
            namespace MapTD {
                class GridManager;
            }
        }
        namespace UI {
            class UIRenderer;
        }
    }
}
```

---

## 5. 移行戦略

### 5.1 段階的移行の原則

1. **Phase 1: 基盤構築**
   - `include/new/`, `src/new/` ディレクトリ作成
   - CMakeターゲット追加
   - 最小限のエントリポイント実装

2. **Phase 2: コア層移行**
   - `GameContext`, `World`, `GameRenderer` を新設計で実装
   - 既存コードを参考にしつつ、新設計原則に従う

3. **Phase 3: システム移行**
   - ゲームシステム（Render, Input, Movement等）を新設計で実装
   - 内部エディタ統合

4. **Phase 4: ドメイン層移行**
   - TDシステム（直線型→マップ型）を新設計で実装
   - UIシステム実装

5. **Phase 5: パリティ確認と削除**
   - 新アーキテクチャで旧機能のパリティ確認
   - 旧コード削除

### 5.2 並行開発のルール

- **新機能**: 必ず `include/new/`, `src/new/` に実装
- **バグ修正**: 旧アーキテクチャにも適用（移行完了まで）
- **設計変更**: 新アーキテクチャに反映、旧は変更しない

---

## 6. 実装チェックリスト

### 6.1 ディレクトリ作成

- [ ] `include/new/` ディレクトリ作成
- [ ] `src/new/` ディレクトリ作成
- [ ] 各レイヤーのサブディレクトリ作成
- [ ] `assets/new/` ディレクトリ作成（オプション）

### 6.2 CMake設定

- [ ] `NEW_ARCH_SOURCES` 変数定義
- [ ] `SimpleTDCGame_NewArch` ターゲット追加
- [ ] インクルードパス設定（`include/new` 優先）
- [ ] アセットコピー設定

### 6.3 エントリポイント

- [ ] `src/main_new.cpp` 作成
- [ ] 最小限の初期化コード実装
- [ ] ビルド・実行確認

### 6.4 ドキュメント

- [ ] このドキュメントを `.cursor/new/` に保存
- [ ] README.md に新ターゲットの説明追加
- [ ] `.cursorrules` に新アーキテクチャのルール追加

---

## 7. 参考資料

- [設計原則と制約](design-principles-and-constraints.md)
- [新アーキテクチャ移行プラン](new-arch-migration-plan.md)
- [ライブラリ注意点ガイド](libs_guide.md)

---

**作成日**: 2025-12-06  
**バージョン**: 1.0  
**対象**: 新アーキテクチャ開発者向け
