# 新しい機能・ステート追加時の制約

**最終更新**: 2025-01-XX  
**対象**: 新しいゲームステート、モジュール、コンポーネントを追加する際の制約

---

## 📋 目次

1. [新しいゲームステート（GameState）を追加する場合](#新しいゲームステートgamestateを追加する場合)
2. [新しいモジュール（IModule）を追加する場合](#新しいモジュールimoduleを追加する場合)
3. [新しいコンポーネントを追加する場合](#新しいコンポーネントを追加する場合)
4. [全般的な制約](#全般的な制約)

---

## 新しいゲームステート（GameState）を追加する場合

### 必須手順

新しいゲームステートを追加するには、以下のファイルを修正する必要があります：

#### 1. `game/core/config/GameState.hpp` - ステート定義の追加

```cpp
enum class GameState {
    Initializing,  // リソース初期化中
    Title,         // タイトル画面
    Home,          // ← 新しいステートを追加
    Game,          // ← 新しいステートを追加
    // ...
};
```

**制約**:

- enum classを使用（型安全性のため）
- コメントで用途を明記
- 既存のステートを削除しない

#### 2. `game/core/system/GameSystem.hpp` - ステート用メンバ変数の追加

```cpp
private:
    std::unique_ptr<BaseSystemAPI> systemAPI_;
    std::unique_ptr<GameModuleAPI> gameAPI_;
    std::unique_ptr<ModuleSystem> moduleSystem_;
    std::unique_ptr<ResourceInitializer> resourceInitializer_;
    std::unique_ptr<TitleScreen> titleScreen_;
    std::unique_ptr<HomeScreen> homeScreen_;  // ← 新しいステート用のメンバを追加
    // ...
```

**制約**:

- `std::unique_ptr`を使用（所有権を明確化）
- メンバ変数名は`[StateName]Screen`の形式（例: `HomeScreen` → `homeScreen_`）
- プライベートメンバとして定義

#### 3. `game/core/system/GameSystem.cpp` - 複数箇所の修正

##### 3-1. `Initialize()` - ステートオブジェクトの作成

```cpp
int GameSystem::Initialize() {
    // ... 既存のコード ...
    
    // TitleScreenの作成
    titleScreen_ = std::make_unique<TitleScreen>();
    
    // HomeScreenの作成（まだ初期化しない）
    homeScreen_ = std::make_unique<HomeScreen>();  // ← 追加
    
    // ...
}
```

**制約**:

- `Initialize()`内で作成するが、初期化はしない
- 初期化は`initializeState()`で行う

##### 3-2. `Run()` - Update処理の追加

```cpp
int GameSystem::Run() {
    // ...
    switch (currentState_) {
    case GameState::Initializing:
        // ... 既存のコード ...
        break;
    
    case GameState::Title:
        // ... 既存のコード ...
        break;
    
    case GameState::Home:  // ← 新しいケースを追加
        if (homeScreen_) {
            homeScreen_->Update(deltaTime);
            moduleSystem_->Update(sharedContext_, deltaTime);
            
            // 終了リクエストチェック
            if (homeScreen_->RequestQuit()) {
                LOG_INFO("QUIT requested from HomeScreen");
                requestShutdown_ = true;
            }
            
            // 遷移リクエストチェック
            GameState nextState;
            if (homeScreen_->RequestTransition(nextState)) {
                transitionTo(nextState);
            }
        }
        break;
    }
    // ...
}
```

**制約**:

- 各ステートで`Update()`を呼び出す
- `moduleSystem_->Update()`も呼び出す（モジュール更新のため）
- 終了リクエストと遷移リクエストをチェック
- `LOG_INFO`でログ出力

##### 3-3. `Run()` - Render処理の追加

```cpp
int GameSystem::Run() {
    // ...
    switch (currentState_) {
    case GameState::Initializing:
        resourceInitializer_->Render();
        break;
    
    case GameState::Title:
        if (titleScreen_) {
            titleScreen_->Render();
        }
        moduleSystem_->Render(sharedContext_);
        break;
    
    case GameState::Home:  // ← 新しいケースを追加
        if (homeScreen_) {
            homeScreen_->Render();
        }
        moduleSystem_->Render(sharedContext_);
        break;
    }
    // ...
}
```

**制約**:

- 各ステートで`Render()`を呼び出す
- `moduleSystem_->Render()`も呼び出す（モジュール描画のため）

##### 3-4. `cleanupCurrentState()` - クリーンアップ処理の追加

```cpp
void GameSystem::cleanupCurrentState() {
    switch (currentState_) {
    case GameState::Initializing:
        // ResourceInitializerはGameSystemのShutdownでクリーンアップ
        break;
    
    case GameState::Title:
        if (titleScreen_) {
            titleScreen_->Shutdown();
        }
        break;
    
    case GameState::Home:  // ← 新しいケースを追加
        if (homeScreen_) {
            homeScreen_->Shutdown();
        }
        break;
    }
}
```

**制約**:

- 各ステートで`Shutdown()`を呼び出す
- ポインタのnullチェックを行う

##### 3-5. `initializeState()` - 初期化処理の追加

```cpp
bool GameSystem::initializeState(GameState state) {
    switch (state) {
    case GameState::Initializing:
        // ResourceInitializerはInitialize()で既に初期化済み
        return true;
    
    case GameState::Title:
        if (titleScreen_) {
            return titleScreen_->Initialize(systemAPI_.get());
        }
        return false;
    
    case GameState::Home:  // ← 新しいケースを追加
        if (homeScreen_) {
            return homeScreen_->Initialize(systemAPI_.get());
        }
        return false;
    }
    return false;
}
```

**制約**:

- 各ステートで`Initialize()`を呼び出す
- `systemAPI_.get()`を渡す（所有権は渡さない）
- 失敗時は`false`を返す

##### 3-6. `Shutdown()` - シャットダウン処理の追加

```cpp
void GameSystem::Shutdown() {
    LOG_INFO("=== Game Shutdown ===");
    
    // 現在のステートのクリーンアップ
    cleanupCurrentState();
    
    // TitleScreenのクリーンアップ
    if (titleScreen_) {
        titleScreen_->Shutdown();
        titleScreen_.reset();
    }
    
    // HomeScreenのクリーンアップ  // ← 追加
    if (homeScreen_) {
        homeScreen_->Shutdown();
        homeScreen_.reset();
    }
    
    // ...
}
```

**制約**:

- 各ステートオブジェクトの`Shutdown()`を呼び出す
- `reset()`でポインタを解放

#### 4. ステートクラスの実装

新しいステートクラス（例: `HomeScreen`）を作成する場合：

**ファイル配置**:

- `game/core/states/HomeScreen.hpp`
- `game/core/states/HomeScreen.cpp`

**必須メソッド**:

```cpp
class HomeScreen {
public:
    bool Initialize(BaseSystemAPI* systemAPI);
    void Update(float deltaTime);
    void Render();
    void Shutdown();
    bool RequestQuit() const;
    bool RequestTransition(GameState& nextState) const;
};
```

**制約**:

- `Initialize()`: 成功時`true`、失敗時`false`を返す
- `Update()`: 毎フレーム呼び出される
- `Render()`: 毎フレーム呼び出される
- `Shutdown()`: リソースの解放を行う
- `RequestQuit()`: 終了リクエストを返す
- `RequestTransition()`: 遷移リクエストを返す（遷移先を`nextState`に設定）

---

## 新しいモジュール（IModule）を追加する場合

### 必須手順

#### 1. `game/core/ecs/IModule.hpp`を実装

新しいモジュールクラスは`IModule`インターフェースを実装する必要があります：

```cpp
#include "core/ecs/IModule.hpp"
#include "core/config/SharedContext.hpp"

namespace game {
namespace core {

class MyModule : public IModule {
public:
    bool Initialize(SharedContext& ctx) override;
    void Update(SharedContext& ctx, float dt) override;
    void Render(SharedContext& ctx) override;
    void Shutdown(SharedContext& ctx) override;
    const char* GetName() const override;
    int GetUpdatePriority() const override;
    int GetRenderPriority() const override;
    
private:
    // メンバ変数
};

} // namespace core
} // namespace game
```

**制約**:

- すべての純粋仮想関数を実装する必要がある
- `GetName()`: モジュール名を返す（デバッグ用）
- `GetUpdatePriority()`: 更新の優先順位（小さい値から順に実行）
- `GetRenderPriority()`: 描画の優先順位（小さい値から順に実行）

#### 1-1. UI要素とゲームの振る舞いを一塊にする

モジュール内で、UI要素とゲームの振る舞いを一塊にすることができます。`Update()`でゲームロジックを処理し、`Render()`でUI描画を行います。

**実装例**:

```cpp
#include "core/ecs/IModule.hpp"
#include "core/config/SharedContext.hpp"
#include <rlImGui.h>
#include <imgui.h>

namespace game {
namespace core {

class InventoryModule : public IModule {
public:
    bool Initialize(SharedContext& ctx) override {
        ctx_ = &ctx;
        // インベントリの初期化
        itemCount_ = 0;
        return true;
    }
    
    void Update(SharedContext& ctx, float dt) override {
        // ゲームロジック: アイテムの更新処理
        if (ctx.systemAPI->IsKeyPressed(KEY_I)) {
            showInventory_ = !showInventory_;
        }
        
        // アイテムの追加・削除などの処理
        // ...
    }
    
    void Render(SharedContext& ctx) override {
        // UI描画: インベントリウィンドウ
        if (showInventory_) {
            ImGui::Begin("Inventory", &showInventory_);
            
            // アイテムリストの表示
            ImGui::Text("Items: %d", itemCount_);
            
            // アイテムボタンなど
            if (ImGui::Button("Add Item")) {
                itemCount_++;
            }
            
            ImGui::End();
        }
        
        // 常時表示するUI要素（HPバーなど）
        ImGui::Begin("Status", nullptr, 
            ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("HP: %d/%d", currentHP_, maxHP_);
        ImGui::ProgressBar(static_cast<float>(currentHP_) / maxHP_);
        ImGui::End();
    }
    
    void Shutdown(SharedContext& ctx) override {
        // リソースの解放
    }
    
    const char* GetName() const override { return "InventoryModule"; }
    int GetUpdatePriority() const override { return 100; }
    int GetRenderPriority() const override { return 100; }
    
private:
    SharedContext* ctx_;
    bool showInventory_ = false;
    int itemCount_ = 0;
    int currentHP_ = 100;
    int maxHP_ = 100;
};

} // namespace core
} // namespace game
```

**ポイント**:

- **`Update()`**: ゲームロジック（入力処理、状態更新、計算など）を実装
- **`Render()`**: UI描画（ImGuiウィンドウ、ボタン、テキストなど）を実装
- **`SharedContext`**: `ctx.systemAPI`を通じてシステムAPIにアクセス可能
- **ImGui/rlImGui**: 直接`#include <rlImGui.h>`と`#include <imgui.h>`を使用可能
- **状態管理**: モジュールのメンバ変数でUIの表示状態やゲーム状態を管理

**注意事項**:

- `Render()`内でImGuiを使用する場合、`rlImGuiBegin()`と`rlImGuiEnd()`は`GameSystem`側で既に呼ばれているため、直接`ImGui::Begin()`などを使えます
- 複数のモジュールが同じUI要素を描画しないよう注意（優先順位で制御可能）
- UIの表示/非表示はモジュール内で管理するか、`SharedContext`を通じて共有する

#### 2. `game/core/system/GameSystem.cpp` - `RegisterModules()`で登録

```cpp
void GameSystem::RegisterModules() {
    // モジュール登録
    #include "ecs/defineModules.hpp"
    moduleSystem_->RegisterModule<MovementModule>();
    moduleSystem_->RegisterModule<RenderModule>();
    moduleSystem_->RegisterModule<MyModule>();  // ← 新しいモジュールを登録
}
```

**制約**:

- `RegisterModules()`内で`moduleSystem_->RegisterModule<T>()`を呼び出す
- テンプレート引数にモジュール型を指定
- 登録順序は実行順序に影響しない（優先順位で決定される）

#### 3. `game/core/ecs/defineModules.hpp` - モジュール定義の追加（将来的に）

将来的には、モジュール定義を一元管理する予定：

```cpp
#pragma once

// モジュールインクルード
#include "Modules/MovementModule.hpp"
#include "Modules/RenderModule.hpp"
#include "Modules/MyModule.hpp"  // ← 追加

namespace game {
    namespace core {
        namespace ecs {
            // モジュール定義は将来的にここに追加
        }
    }
}
```

**制約**:

- モジュールヘッダーをインクルード
- 名前空間は`game::core::ecs`を使用

---

## 新しいコンポーネントを追加する場合

### 必須手順

#### 1. コンポーネントの定義（POD形式）

```cpp
// game/core/ecs/components/MyComponent.hpp
#pragma once

namespace game {
namespace core {
namespace ecs {
namespace components {

struct MyComponent {
    int value = 0;
    float speed = 1.0f;
    
    MyComponent() = default;
    MyComponent(int v, float s) : value(v), speed(s) {}
};

} // namespace components
} // namespace ecs
} // namespace core
} // namespace game
```

**制約**:

- **POD（Plain Old Data）形式**で定義（データのみ、ロジックなし）
- ヘッダーファイルのみ（`.cpp`ファイルは不要）
- デフォルトコンストラクタを定義
- 必要に応じてパラメータ付きコンストラクタを定義
- メンバ変数は`public`で定義（PODのため）

#### 2. `game/core/ecs/defineComponents.hpp` - コンポーネント定義の追加（将来的に）

将来的には、コンポーネント定義を一元管理する予定：

```cpp
#pragma once

// コンポーネントインクルード
#include "Components/Position.hpp"
#include "Components/Velocity.hpp"
#include "Components/MyComponent.hpp"  // ← 追加

namespace game {
    namespace core {
        namespace ecs {
            // コンポーネント定義は将来的にここに追加
        }
    }
}
```

**制約**:

- コンポーネントヘッダーをインクルード
- 名前空間は`game::core::ecs::components`を使用

---

## 全般的な制約

### 命名規約

- **クラス/構造体**: PascalCase (`GameManager`, `MyComponent`)
- **関数/メソッド**: PascalCase (`UpdatePosition`, `Initialize`)
- **変数**: camelCase、プライベートは末尾にアンダースコア (`playerSpeed`, `registry_`)
- **定数**: UPPER_CASE (`MAX_ENTITIES`)
- **名前空間**: PascalCase (`Components`, `Systems`)

### ディレクトリ構造

- **ヘッダーとソースは同じディレクトリに配置**
- **コンポーネントはヘッダーのみ**（POD、実装ファイル不要）
- **システムはヘッダーとソースを分離**
- **includeパスは相対パスで統一**（`#include "../core/Types.h"`）

詳細は `docs/directory_structure.md` を参照。

### 絶対禁止パターン

#### ❌ グローバル変数

```cpp
// 禁止
entt::registry g_registry;
ResourceManager* g_resource_manager = nullptr;
```

**代わりに**: DI / ServiceContainer 使用

#### ❌ メモリリーク（new/delete）

```cpp
// 禁止
auto* entity = new GameObject();
delete entity;
```

**代わりに**: スマートポインタ（`std::unique_ptr`, `std::shared_ptr`）

#### ❌ 複雑なポリモーフィズム

```cpp
// 禁止
class Entity {
public:
    virtual void update() = 0;
};
```

**代わりに**: Component + Tag で判別（ECSパターン）

#### ❌ 循環参照

```cpp
// 禁止
class A {
    std::shared_ptr<B> m_b;
};
class B {
    std::shared_ptr<A> m_a;  // 循環参照
};
```

**代わりに**: `std::weak_ptr` または参照を使用

### ログ出力

```cpp
#include "../utils/Log.h"

// 情報ログ
LOG_INFO("Enemy spawned: {}", enemy_type);

// 警告ログ
LOG_WARN("Health below threshold: {}", current_health);

// エラーログ
LOG_ERROR("Failed to load texture: {}", file_path);

// デバッグログ
LOG_DEBUG("Position: ({}, {})", pos.x, pos.y);
```

**制約**:

- すべてのログは`LOG_*`マクロを使用
- フォーマットはC++17のfmtスタイル（spdlogでサポート）

### JSONパース

```cpp
try {
    const auto jsonData = nlohmann::json::parse(text);
    // ... use jsonData ...
} catch (const nlohmann::json::parse_error& e) {
    std::cerr << "JSON parse error: " << e.what() << std::endl;
    // fallback
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    // fallback
}
```

**制約**:

- 必ず`try-catch`で囲む
- `parse_error`と`std::exception`を捕捉
- デフォルト値で継続する

---

## ✅ チェックリスト

新しい機能を追加する際の確認事項：

### 新しいステートを追加する場合

- [ ] `GameState.hpp`にenum値を追加
- [ ] `GameSystem.hpp`にメンバ変数を追加
- [ ] `GameSystem.cpp`の`Initialize()`でオブジェクトを作成
- [ ] `GameSystem.cpp`の`Run()`のUpdate用switchにケースを追加
- [ ] `GameSystem.cpp`の`Run()`のRender用switchにケースを追加
- [ ] `GameSystem.cpp`の`cleanupCurrentState()`にケースを追加
- [ ] `GameSystem.cpp`の`initializeState()`にケースを追加
- [ ] `GameSystem.cpp`の`Shutdown()`にクリーンアップを追加
- [ ] ステートクラスを実装（`Initialize`, `Update`, `Render`, `Shutdown`, `RequestQuit`, `RequestTransition`）

### 新しいモジュールを追加する場合

- [ ] `IModule`インターフェースを実装
- [ ] `GetName()`, `GetUpdatePriority()`, `GetRenderPriority()`を実装
- [ ] `GameSystem.cpp`の`RegisterModules()`で登録
- [ ] `defineModules.hpp`に追加（将来的に）
- [ ] UI要素を含む場合: `Render()`でImGuiを使用してUI描画を実装
- [ ] ゲームロジックを含む場合: `Update()`でゲームの振る舞いを実装
- [ ] UIとゲームロジックを一塊にする場合: 両方を同じモジュール内で実装

### 新しいコンポーネントを追加する場合

- [ ] POD形式で定義（データのみ、ロジックなし）
- [ ] ヘッダーファイルのみ（`.cpp`不要）
- [ ] デフォルトコンストラクタを定義
- [ ] `defineComponents.hpp`に追加（将来的に）

---

## 📌 まとめ

1. **新しいステート**: 6箇所の修正が必要（定義、メンバ、Initialize、Update、Render、クリーンアップ）
2. **新しいモジュール**: `IModule`を実装し、`RegisterModules()`で登録
   - **UI要素とゲームの振る舞いを一塊にできる**: `Update()`でゲームロジック、`Render()`でUI描画を実装
   - ImGui/rlImGuiを直接使用可能
3. **新しいコンポーネント**: POD形式で定義、ヘッダーのみ
4. **全般的な制約**: 命名規約、ディレクトリ構造、禁止パターンに従う

この制約に従うことで、一貫性のあるコードベースを維持できます。

---

**このドキュメントは `.cursorrules` と `docs/simple_architecture.md` と整合しています。**
