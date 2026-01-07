# Cat Tower Defense - Simple Rapid Development Architecture

**最終更新**: 2026-01-07  
**目標**: シンプル × 迅速開発  
**優先度**: シンプルさ > パフォーマンス > スケーラビリティ  
**アーキテクチャ**: モジュールシステム + API層分離

---

## 🎯 設計思想

### 原則 1: シンプルファースト
- 複雑な抽象化を避ける
- 直感的に理解できるコード
- API層とモジュールシステムで責務を分離

### 原則 2: 迅速開発
- 1 日で「動く」機能を実装
- 後付けよりも「先に動かす」
- JSON で数値調整可能に

### 原則 3: モジュール化
- IModuleインターフェースによる統一的なモジュール管理
- 優先順位に基づいた実行順序
- 共有コンテキストによる情報共有

---

## 🏗️ アーキテクチャ全体図

```
┌─────────────────────────────────────────────────┐
│              main.cpp (エントリ)                 │
│  GameSystem::Initialize() → GameSystem::Run()  │
└────────────────────┬────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│            GameSystem (ゲームシステム)           │
│  ├─ BaseSystemAPI (Raylib統合)                 │
│  ├─ GameModuleAPI (EnTTラッパー)               │
│  ├─ ModuleSystem (モジュール管理)               │
│  ├─ SharedContext (共有コンテキスト)            │
│  └─ ステート管理 (GameState enum)               │
└────────────────────┬────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│          ModuleSystem (モジュール管理)             │
│  ├─ IModuleインターフェース                     │
│  ├─ モジュール登録・管理                        │
│  └─ 優先順位に基づいた実行順序                  │
└────────────────────┬────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│          BaseSystemAPI (システムAPI層)            │
│  ├─ Raylibウィンドウ・オーディオ管理            │
│  ├─ リソース管理（テクスチャ・フォント・音声）  │
│  ├─ 描画管理（RenderTexture・描画プリミティブ） │
│  ├─ 入力管理（キーボード・マウス・ゲームパッド）│
│  └─ 解像度管理（FHD固定、スケーリング対応）     │
└────────────────────┬────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│        GameModuleAPI (ゲームモジュールAPI)        │
│  ├─ EnTTレジストリのラッパー                   │
│  ├─ エンティティ操作（Create, Destroy, etc.）  │
│  ├─ コンポーネント操作（Add, Get, Remove）     │
│  └─ ビュー作成（View, View with exclude）      │
└────────────────────┬────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│        独立シーン層 (TitleScreen, etc)            │
│  ├─ メニュー表示・操作                          │
│  ├─ 遷移リクエスト管理                          │
│  └─ 終了リクエスト管理                          │
└────────────────────┬────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│      オーバーレイスタック層 (OverlayManager)      │
│  ├─ スタック管理（LIFO）                        │
│  ├─ 最上層のみ Update                           │
│  └─ 全層を Render                              │
└────────────────────┬────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│      UI コンポーネント層 (Card, List, Tile)      │
│  ├─ Card（カード型）                           │
│  ├─ List（リスト型）                            │
│  ├─ Tile（タイル型）                            │
│  ├─ Panel（パネル）                             │
│  └─ Button（ボタン）                            │
└────────────────────┬────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│      描画バックエンド (ImGui)                    │
│  └─ 現在: ImGui直接呼び出し                     │
│  └─ 将来: IUIRenderer経由で切り替え可能         │
└─────────────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────┐
│     ResourceInitializer (リソース初期化)          │
│  ├─ リソーススキャン・読み込み                  │
│  ├─ 進捗管理                                    │
│  └─ 初期化画面・エラー画面の描画                │
└─────────────────────────────────────────────────┘
```

---

## 💾 ディレクトリ構造

### 実装されている構造

```
game/
├── main/
│   └── main.cpp              # エントリーポイント
├── core/
│   ├── api/                  # システムAPI層
│   │   ├── BaseSystemAPI.hpp/cpp    # Raylib統合API
│   │   └── GameModuleAPI.hpp/cpp    # EnTTラッパー
│   ├── config/               # 設定・型定義
│   │   ├── GameConfig.hpp           # 解像度・FPS設定
│   │   ├── GameState.hpp            # ゲームステートenum
│   │   └── SharedContext.hpp        # 共有コンテキスト
│   ├── ecs/                  # ECS関連（モジュール定義）
│   │   ├── defineComponents.hpp     # コンポーネント定義（将来実装予定）
│   │   ├── defineModules.hpp        # モジュール定義（将来実装予定）
│   │   └── IModule.hpp              # モジュールインターフェース
│   ├── init/                 # 初期化関連
│   │   └── ResourceInitializer.hpp/cpp  # リソース初期化
│   ├── states/               # ステート/シーン
│   │   ├── IScene.hpp              # シーン基底インターフェース
│   │   ├── TitleScreen.hpp/cpp     # タイトル画面
│   │   └── overlays/               # オーバーレイシーン
│   │       ├── IOverlay.hpp
│   │       ├── StageSelectOverlay.hpp/cpp
│   │       ├── FormationOverlay.hpp/cpp
│   │       ├── EnhancementOverlay.hpp/cpp
│   │       ├── CodexOverlay.hpp/cpp
│   │       ├── SettingsOverlay.hpp/cpp
│   │       └── GachaOverlay.hpp/cpp
│   ├── ui/                   # UI コンポーネント層
│   │   ├── IUIComponent.hpp
│   │   └── components/
│   │       ├── Card.hpp/cpp
│   │       ├── List.hpp/cpp
│   │       ├── Tile.hpp/cpp
│   │       ├── Panel.hpp/cpp
│   │       └── Button.hpp/cpp
│   └── system/               # コアシステム
│       ├── GameSystem.hpp/cpp       # ゲームシステム統合クラス
│       ├── ModuleSystem.hpp/cpp     # モジュール管理システム
│       └── OverlayManager.hpp/cpp   # オーバーレイ管理システム
└── utils/
    └── Log.h                 # ログユーティリティ
```

### 実装済みECSコンポーネント

```
game/
├── core/
│   ├── ecs/
│   │   ├── Components/      # ECSコンポーネント（POD）✅ 実装済み
│   │   │   ├── Position.hpp
│   │   │   ├── Health.hpp
│   │   │   ├── Stats.hpp
│   │   │   ├── Movement.hpp
│   │   │   ├── Combat.hpp
│   │   │   ├── Sprite.hpp
│   │   │   ├── Animation.hpp
│   │   │   └── CharacterId.hpp
│   │   ├── defineComponents.hpp  # コンポーネント一元管理
│   │   └── modules/         # ゲームモジュール実装（将来実装予定）
│   │       ├── MovementModule.hpp/cpp
│   │       ├── CombatModule.hpp/cpp
│   │       ├── RenderModule.hpp/cpp
│   │       └── ...
│   ├── entities/            # ✅ 実装済み
│   │   ├── Character.hpp/cpp
│   │   ├── CharacterManager.hpp/cpp
│   │   └── animation/
│   │       └── AnimationData.hpp
│   └── states/
│       ├── HomeScreen.hpp/cpp  # ✅ 実装済み
│       ├── TitleScreen.hpp/cpp
│       └── ...
```

---

## 🎯 コアシステム

### 1. GameSystem（ゲームシステム統合クラス）

**責務**:
- アプリケーション全体の初期化・終了管理
- メインループの管理（フレーム制御）
- BaseSystemAPIとGameModuleAPIの所有・管理
- SharedContextの所有・管理
- ステート管理（enum + 遷移制御）
- 各ステートクラスの所有・管理
- 安全なステート遷移（二重初期化/解放防止）
- モジュール登録の呼び出し（RegisterModules）

**実装例**:

```cpp
// game/core/system/GameSystem.hpp
class GameSystem {
public:
    GameSystem();
    int Initialize();
    int Run();
    void Shutdown();

private:
    std::unique_ptr<BaseSystemAPI> systemAPI_;
    std::unique_ptr<GameModuleAPI> gameAPI_;
    std::unique_ptr<ModuleSystem> moduleSystem_;
    std::unique_ptr<ResourceInitializer> resourceInitializer_;
    std::unique_ptr<TitleScreen> titleScreen_;
    SharedContext sharedContext_;
    GameState currentState_;
    bool requestShutdown_;

    void transitionTo(GameState newState);
    void RegisterModules();
};
```

**メインループ**:

```cpp
int GameSystem::Run() {
    while (!systemAPI_->WindowShouldClose() && !requestShutdown_) {
        float deltaTime = systemAPI_->GetFrameTime();
        sharedContext_.deltaTime = deltaTime;

        systemAPI_->UpdateInput();

        // ステートに応じた更新
        switch (currentState_) {
            case GameState::Initializing:
                // リソース初期化処理
                break;
            case GameState::Title:
                titleScreen_->Update(deltaTime);
                moduleSystem_->Update(sharedContext_, deltaTime);
                break;
        }

        // 描画処理
        systemAPI_->BeginRender();
        // ステートに応じた描画
        systemAPI_->EndRender();
        systemAPI_->EndFrame();
    }
    return 0;
}
```

### 2. ModuleSystem（モジュール管理システム）

**責務**:
- モジュールの登録・管理（所有権を持つ）
- モジュールのライフサイクル管理（Initialize, Update, Render, Shutdown）
- 優先順位に基づいた実行順序の管理

**実装例**:

```cpp
// game/core/system/ModuleSystem.hpp
class ModuleSystem {
public:
    explicit ModuleSystem(GameModuleAPI* gameAPI);
    
    template<typename ModuleType>
    void RegisterModule();
    
    bool Initialize(SharedContext& ctx);
    void Update(SharedContext& ctx, float dt);
    void Render(SharedContext& ctx);
    void Shutdown(SharedContext& ctx);

private:
    GameModuleAPI* gameAPI_;
    std::vector<std::unique_ptr<IModule>> modules_;
    void SortModulesByPriority();
};
```

**モジュール登録**:

```cpp
// GameSystem::RegisterModules()内
moduleSystem_->RegisterModule<MovementModule>();
moduleSystem_->RegisterModule<CombatModule>();
moduleSystem_->RegisterModule<RenderModule>();
```

### 3. IModule（モジュールインターフェース）

**責務**:
- すべてのゲームモジュールが実装する必要があるインターフェース
- ライフサイクル（初期化・更新・描画・終了）と優先順位管理を提供

**実装例**:

```cpp
// game/core/ecs/IModule.hpp
class IModule {
public:
    virtual ~IModule() = default;
    
    virtual bool Initialize(SharedContext& ctx) = 0;
    virtual void Update(SharedContext& ctx, float dt) = 0;
    virtual void Render(SharedContext& ctx) = 0;
    virtual void Shutdown(SharedContext& ctx) = 0;
    
    virtual const char* GetName() const = 0;
    virtual int GetUpdatePriority() const { return 0; }
    virtual int GetRenderPriority() const { return 0; }
};
```

---

## 🔧 API層

### 1. BaseSystemAPI（システムAPI層）

**責務**:
- Raylibウィンドウ・オーディオデバイスの初期化/終了
- リソース管理（テクスチャ、フォント、サウンド、ミュージック）
- 描画管理（RenderTexture、描画フレーム制御、描画プリミティブ）
- 入力管理（キーボード、マウス、ゲームパッド、タッチ）
- 解像度管理（FHD固定、スケーリング対応）
- オーディオ管理（ボリューム制御、再生制御）

**主要メソッド**:

```cpp
// 初期化・終了
bool Initialize(Resolution initialResolution);
void Shutdown();

// リソース管理
void* GetTexture(const std::string& name);
void* GetFont(const std::string& name);
void* GetSound(const std::string& name);
void* GetMusic(const std::string& name);

// 描画管理
void BeginRender();
void EndRender();
void EndFrame();
void DrawTextDefault(const std::string& text, float x, float y, float fontSize, Color color);
void DrawRectangle(float x, float y, float width, float height, Color color);
// ... その他多数の描画メソッド

// 入力管理
void UpdateInput();
bool IsKeyPressed(int key);
bool IsMouseButtonPressed(int button);
Vector2 GetMousePosition();

// オーディオ管理
bool PlaySound(const std::string& name);
bool PlayMusic(const std::string& name);
void SetMasterVolume(float volume);
```

**特徴**:
- 内部解像度は常に1920x1080（FHD）固定
- ウィンドウサイズに応じて自動スケーリング
- リソースはshared_ptrで管理（自動解放）
- 日本語フォント対応（NotoSansJP-Medium.ttf）

### 2. GameModuleAPI（ゲームモジュールAPI）

**責務**:
- EnTTレジストリのラッパー
- エンティティ操作（Create, Destroy, Valid, Clear）
- コンポーネント操作（Add, Get, Try, Remove, Has）
- ビュー作成（View, View with exclude）
- コンテキスト変数管理（Ctx）

**実装例**:

```cpp
// game/core/api/GameModuleAPI.hpp
class GameModuleAPI {
public:
    entt::registry& Registry() { return registry_; }
    
    // エンティティ操作
    entt::entity Create();
    void Destroy(entt::entity e);
    bool Valid(entt::entity e) const;
    size_t Count() const;
    void Clear();
    
    // コンポーネント操作
    template<typename T, typename... Args>
    T& Add(entt::entity e, Args&&... args);
    
    template<typename T>
    T& Get(entt::entity e);
    
    template<typename T>
    T* Try(entt::entity e);
    
    template<typename T>
    void Remove(entt::entity e);
    
    template<typename T>
    bool Has(entt::entity e) const;
    
    // ビュー作成
    template<typename... T>
    auto View();
    
    template<typename... T, typename... Exclude>
    auto View(entt::exclude_t<Exclude...>);
    
    // コンテキスト変数
    template<typename T, typename... Args>
    T& Ctx(Args&&... args);
    
private:
    entt::registry registry_;
};
```

**使用例**:

```cpp
// エンティティ作成
auto entity = gameAPI->Create();

// コンポーネント追加
gameAPI->Add<Position>(entity, 100.0f, 200.0f);
gameAPI->Add<Health>(entity, 100);

// ビュー作成
auto view = gameAPI->View<Position, Velocity>();
for (auto e : view) {
    auto& pos = view.get<Position>(e);
    auto& vel = view.get<Velocity>(e);
    // 処理
}
```

---

## 📊 ステート管理

### GameState（ゲームステートenum）

```cpp
// game/core/config/GameState.hpp
enum class GameState {
    Initializing,  // リソース初期化中
    Title,         // タイトル画面
    Home,          // ホーム画面
    Game,          // ゲーム画面
};

enum class OverlayState {
    None = 0,
    StageSelect = 1,
    Formation = 2,
    Enhancement = 3,
    Codex = 4,
    Settings = 5,
    Gacha = 6,
};
```

### SharedContext（共有コンテキスト）

```cpp
// game/core/config/SharedContext.hpp
struct SharedContext {
    BaseSystemAPI* systemAPI = nullptr;
    GameModuleAPI* gameAPI = nullptr;
    GameSystem* gameSystem = nullptr;  // オーバーレイから遷移リクエスト用
    float deltaTime = 0.0f;
    bool isPaused = false;
    bool requestShutdown = false;
};
```

**特徴**:
- GameSystemが所有し、すべてのモジュールに共有
- モジュールはこのコンテキストを通じてシステムAPIにアクセス
- 所有権は持たない（参照のみ）
- `gameSystem`ポインタでオーバーレイからステート遷移をリクエスト可能

---

## 🎨 オーバーレイシステム

### 設計方針

オーバーレイは**UI層固有の概念**として、独立したスタック管理システムで実装されています。

**特徴**:
- LIFOスタック管理（`OverlayManager`）
- 最上層のオーバーレイのみUpdate、すべてのオーバーレイをRender
- 背景シーンは表示したまま（半透明背景でオーバーレイを表示）
- ESCキーで閉じる機能

### OverlayManager

```cpp
// game/core/system/OverlayManager.hpp
class OverlayManager {
public:
    bool PushOverlay(OverlayState state, BaseSystemAPI* systemAPI);
    void PopOverlay();
    void PopAllOverlays();
    void Update(SharedContext& ctx, float deltaTime);
    void Render(SharedContext& ctx);
    // ...
};
```

**使用例**:
```cpp
// オーバーレイを開く
overlayManager_->PushOverlay(OverlayState::StageSelect, systemAPI_.get());

// オーバーレイを閉じる
overlayManager_->PopOverlay();
```

### IOverlayインターフェース

すべてのオーバーレイは`IOverlay`を実装します：

```cpp
class IOverlay {
public:
    virtual bool Initialize(BaseSystemAPI* systemAPI) = 0;
    virtual void Update(SharedContext& ctx, float deltaTime) = 0;
    virtual void Render(SharedContext& ctx) = 0;
    virtual void Shutdown() = 0;
    virtual OverlayState GetState() const = 0;
    virtual bool RequestClose() const = 0;
    virtual bool RequestTransition(GameState& nextState) const = 0;
};
```

**実装済みオーバーレイ**:
- ✅ `StageSelectOverlay` - ステージ選択（Tileコンポーネント使用）
- ✅ `FormationOverlay` - 編成（Listコンポーネント使用）
- ✅ `EnhancementOverlay` - 強化（Listコンポーネント使用）
- ✅ `CodexOverlay` - 図鑑（Listコンポーネント使用）
- ✅ `SettingsOverlay` - 設定（Panel/Buttonコンポーネント使用）
- ✅ `GachaOverlay` - ガチャ（Cardコンポーネント使用）
- ✅ `LicenseOverlay` - ライセンス情報（スクロール機能付き）

---

## 🎨 UIコンポーネント層

### 設計方針

UIコンポーネント層は、カード、リスト、タイルなどのUI要素を統一的に扱うための抽象化層です。

**階層化アプローチ**:
```
[オーバーレイ層] ← ImGui直接呼び出し（現在）
      ↓
[UI コンポーネント層] ← Card, List, Tile など統一インターフェース
      ↓
[描画バックエンド] ← ImGui / Canvas / Custom など切り替え可能（将来）
```

### IUIComponentインターフェース

すべてのUIコンポーネントは`IUIComponent`を実装します：

```cpp
class IUIComponent {
public:
    virtual bool Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void Shutdown() = 0;
    
    // レイアウト
    virtual void SetPosition(float x, float y) = 0;
    virtual void SetSize(float width, float height) = 0;
    virtual Rect GetBounds() const = 0;
    
    // 表示制御
    virtual void SetVisible(bool visible) = 0;
    virtual void SetEnabled(bool enabled) = 0;
    
    // イベント
    virtual bool OnMouseClick(float x, float y) = 0;
    virtual bool OnMouseHover(float x, float y) = 0;
    virtual bool OnKey(int key) = 0;
    
    // 子要素管理
    virtual void AddChild(std::shared_ptr<IUIComponent> child) = 0;
    virtual void RemoveChild(std::shared_ptr<IUIComponent> child) = 0;
};
```

### 実装済みUIコンポーネント

1. **Card** - カード型コンポーネント
   - タイトル、説明、画像、メタデータを表示
   - クリックコールバック対応

2. **List** - リスト型コンポーネント
   - アイテム一覧を表示
   - 選択機能、スクロール機能

3. **Tile** - タイル型コンポーネント
   - グリッド配置でタイルを表示
   - 選択機能

4. **Panel** - パネルコンポーネント
   - 子要素のレイアウト管理

5. **Button** - ボタンコンポーネント
   - クリックコールバック対応

**注意**: 現在の実装では、すべてのUIコンポーネントは直接ImGuiを呼び出して描画しています。  
将来の描画バックエンド抽象化（フェーズ4）では、`IUIRenderer`インターフェース経由で描画するように変更します。

詳細は `docs/UI_OVERLAY_SYSTEM.md` を参照してください。

### TitleScreen（タイトル画面）

**責務**:
- タイトル画面の更新・描画
- メニュー操作（キーボード/マウス）
- メニューアクション実行
- 遷移リクエストの通知

**実装例**:

```cpp
// game/core/states/TitleScreen.hpp
class TitleScreen {
public:
    bool Initialize(BaseSystemAPI* systemAPI);
    void Update(float deltaTime);
    void Render();
    void Shutdown();
    
    bool RequestTransition(GameState& nextState);
    bool RequestQuit();

private:
    BaseSystemAPI* systemAPI_;
    TitleState titleState_;
    bool isInitialized_;
    bool hasTransitionRequest_;
    GameState requestedNextState_;
};
```

---

## 🔧 リソース初期化

### ResourceInitializer（リソース初期化専用クラス）

**責務**:
- リソーススキャン・読み込み処理
- 進捗管理
- 初期化画面・エラー画面の描画
- 初期化完了/失敗の状態通知

**実装例**:

```cpp
// game/core/init/ResourceInitializer.hpp
class ResourceInitializer {
public:
    bool Initialize(BaseSystemAPI* systemAPI);
    bool Update(float deltaTime);
    void Render();
    bool IsCompleted() const;
    bool HasFailed() const;
    bool ShouldShutdown() const;
    void Reset();

private:
    BaseSystemAPI* systemAPI_;
    InitState initState_;
    bool isInitialized_;
};
```

**処理フロー**:
1. リソースファイルのスキャン（ScanResourceFiles）
2. 1つずつリソースを読み込み（LoadNextResource）
3. 進捗表示（プログレスバー）
4. 完了時にIsCompleted()がtrueを返す
5. エラー時はエラー画面を表示（5秒後に終了）

---

## 📄 設定

### GameConfig（ゲーム設定）

```cpp
// game/core/config/GameConfig.hpp
enum class Resolution {
    FHD,  // フルHD: 1920x1080
    HD,   // HD: 1280x720
    SD    // SD: 854x480
};

constexpr int TARGET_FPS = 60;
```

**特徴**:
- 内部解像度は常にFHD（1920x1080）固定
- ウィンドウサイズに応じて自動スケーリング
- ターゲットFPSは60

---

## 🎮 エントリーポイント

### main.cpp

```cpp
// game/main/main.cpp
#include "core/system/GameSystem.hpp"
#include "utils/Log.h"

int main() {
    game::core::GameSystem system;

    // ゲームの初期化
    int initResult = system.Initialize();
    if (initResult != 0) {
        return initResult;
    }

    // メインループ実行（初期化シーン→タイトル画面）
    int runResult = system.Run();

    // ゲームのシャットダウン
    system.Shutdown();

    return runResult;
}
```

**ポイント**: 30行以下。余計なコードなし。

---

## ✅ 実装済み機能

### ECSコンポーネント（実装済み）

```cpp
// game/core/ecs/Components/Position.hpp ✅ 実装済み
struct Position {
    float x = 0.0f;
    float y = 0.0f;
    Position() = default;
    Position(float x, float y) : x(x), y(y) {}
    Vector2 ToVector2() const;
};

// game/core/ecs/Components/Health.hpp ✅ 実装済み
struct Health {
    int current = 100;
    int max = 100;
    bool IsAlive() const;
    float GetPercentage() const;
};

// その他の実装済みコンポーネント:
// - Stats.hpp: ステータス情報
// - Movement.hpp: 移動情報
// - Combat.hpp: 戦闘情報
// - Sprite.hpp: スプライト情報
// - Animation.hpp: アニメーション情報
// - CharacterId.hpp: キャラクターID
```

### キャラクターシステム（実装済み）

- ✅ `Character.hpp/cpp`: キャラクター構造体（攻撃タイプ、エフェクトタイプ、スキル、装備対応）
- ✅ `CharacterManager.hpp/cpp`: キャラクターマスターデータ管理（JSON/ハードコード対応）
- ✅ `AnimationData.hpp`: アニメーションメタデータ

### ホームスクリーン（実装済み）

- ✅ `HomeScreen.hpp/cpp`: ホーム画面実装
- ✅ `TabBarManager.hpp/cpp`: タブナビゲーション
- ✅ `ResourceHeader.hpp/cpp`: リソース表示ヘッダー
- ✅ `ContentContainer.hpp/cpp`: コンテンツコンテナ（既存オーバーレイ統合）

## 🚀 将来実装予定の機能

### ゲームモジュール（将来実装予定）

### ゲームモジュール

```cpp
// game/core/ecs/modules/MovementModule.hpp（将来実装予定）
class MovementModule : public IModule {
public:
    bool Initialize(SharedContext& ctx) override;
    void Update(SharedContext& ctx, float dt) override;
    void Render(SharedContext& ctx) override;
    void Shutdown(SharedContext& ctx) override;
    
    const char* GetName() const override { return "MovementModule"; }
    int GetUpdatePriority() const override { return 100; }
};
```

### その他のシーン

- ✅ HomeScreen（ホーム画面） - 実装済み
- GameScene（ゲーム画面） - 将来実装予定
- StageSelectScene（ステージ選択画面） - オーバーレイとして実装済み
- ConfigScene（設定画面） - オーバーレイとして実装済み

---

## 📋 使用技術スタック

| 層 | ツール | 用途 | 複雑度 |
|-----|--------|------|--------|
| Graphics | Raylib 5.x | 2D 描画 | ⭐ |
| ECS | EnTT | エンティティ管理 | ⭐⭐ |
| Data | nlohmann/json | JSON 読み込み | ⭐ |
| UI | rlImGui | デバッグ UI | ⭐ |
| Logging | spdlog | ログ出力 | ⭐ |
| Build | CMake | ビルド | ⭐⭐ |

**計**: 6 個のシンプルな依存関係

---

## 🎯 このアーキテクチャの利点

✅ **シンプル**: API層とモジュールシステムで責務が明確  
✅ **迅速**: モジュール追加が容易（IModuleを実装するだけ）  
✅ **保守性**: 各モジュールが独立して動作  
✅ **テスト容易**: 各モジュールを独立テスト可能  
✅ **拡張容易**: 新しいモジュールを追加するだけで機能拡張  
✅ **型安全**: EnTTによる型安全なECS操作  

---

## 📌 まとめ

1. **GameSystem**: アプリケーション全体を管理
2. **ModuleSystem**: モジュールの登録・管理・実行
3. **BaseSystemAPI**: Raylib統合API（リソース・描画・入力・オーディオ）
4. **GameModuleAPI**: EnTTラッパー（エンティティ・コンポーネント操作）
5. **SharedContext**: モジュール間の情報共有
6. **IModule**: モジュールインターフェース（統一的なライフサイクル管理）

このシンプル設計で、迅速かつ保守しやすい開発が可能です！ 🚀
