# 新アーキテクチャ設計方針と制約
>
> 本書は「詳細版」です。概要のみ確認したい場合は `design-principles.md` を参照してください。

> **重要**: このドキュメントは新アーキテクチャ（`include/new/`, `src/new/`）の設計原則と制約を定義します。

## 基本方針

### 1. 疎結合アーキテクチャ

#### 1.1 システム間の結合度を最小化

- **原則**: システム間はインターフェースを通じてのみ通信
- **禁止事項**:
  - システム間の直接的な依存関係
  - グローバル変数による状態共有
  - Singletonパターンによるグローバルアクセス

```cpp
// ❌ 禁止: グローバル変数
extern entt::registry g_registry;
extern ResourceManager* g_resourceManager;

// ✅ 推奨: 依存性注入（DI）
class MovementSystem {
private:
    entt::registry& registry_;
    ResourceManager& resourceManager_;
    
public:
    MovementSystem(entt::registry& registry, ResourceManager& resources)
        : registry_(registry), resourceManager_(resources) {}
};
```

#### 1.2 インターフェースによる抽象化

```cpp
// ✅ 推奨: インターフェース定義
class IResourceManager {
public:
    virtual ~IResourceManager() = default;
    virtual Texture2D GetTexture(const std::string& path) = 0;
    virtual Font GetFont(const std::string& path, int size) = 0;
};

// 実装クラス
class ResourceManager : public IResourceManager {
    // 実装...
};

// システムはインターフェースに依存
class RenderSystem {
    IResourceManager& resourceManager_;
public:
    RenderSystem(IResourceManager& resources) : resourceManager_(resources) {}
};
```

### 2. グローバル変数の禁止

#### 2.1 グローバル状態の管理

- **原則**: すべての状態は`GameContext`を通じて管理
- **例外**: ライブラリ固有のグローバル状態（Raylibのウィンドウ、EnTTのレジストリ）は最小限に

```cpp
// ❌ 禁止: グローバル変数
static entt::registry g_registry;
static bool g_gameRunning = false;

// ✅ 推奨: GameContextによる集約管理
class GameContext {
private:
    entt::registry registry_;
    std::unique_ptr<IResourceManager> resourceManager_;
    std::unique_ptr<InputManager> inputManager_;
    
public:
    entt::registry& GetRegistry() { return registry_; }
    IResourceManager& GetResourceManager() { return *resourceManager_; }
    InputManager& GetInputManager() { return *inputManager_; }
};
```

### 3. ライブラリリソース管理の注意点

#### 3.1 Raylibリソース管理

**重要**: Raylibのリソースは明示的に管理する必要があります。

```cpp
// ✅ 推奨: RAIIパターンでリソース管理
class TextureHandle {
private:
    Texture2D texture_;
    bool valid_ = false;
    
public:
    TextureHandle(const std::string& path) {
        texture_ = LoadTexture(path.c_str());
        valid_ = IsTextureReady(texture_);
    }
    
    ~TextureHandle() {
        if (valid_) {
            UnloadTexture(texture_);
        }
    }
    
    Texture2D Get() const { return texture_; }
    bool IsValid() const { return valid_; }
    
    // コピー禁止、ムーブ許可
    TextureHandle(const TextureHandle&) = delete;
    TextureHandle& operator=(const TextureHandle&) = delete;
    TextureHandle(TextureHandle&& other) noexcept
        : texture_(other.texture_), valid_(other.valid_) {
        other.valid_ = false;
    }
};

// 使用例
class ResourceManager {
private:
    std::unordered_map<std::string, std::unique_ptr<TextureHandle>> textures_;
    
public:
    Texture2D GetTexture(const std::string& path) {
        auto it = textures_.find(path);
        if (it == textures_.end()) {
            auto handle = std::make_unique<TextureHandle>(path);
            if (handle->IsValid()) {
                textures_[path] = std::move(handle);
                return textures_[path]->Get();
            }
            return Texture2D{}; // 無効なテクスチャ
        }
        return it->second->Get();
    }
};
```

**注意事項**:

- `CloseWindow()`はリソースを自動解放しない
- テクスチャ、フォント、モデル、シェーダーは明示的に`Unload*()`を呼ぶ
- `UnloadModel()`は内部のメッシュも一緒に削除するため、メッシュを個別に`UnloadMesh()`しない

#### 3.2 EnTTレジストリ管理

**重要**: EnTTのレジストリはライフサイクル管理が必要です。

```cpp
// ✅ 推奨: GameContextでレジストリを管理
class GameContext {
private:
    entt::registry registry_;
    
public:
    entt::registry& GetRegistry() { return registry_; }
    const entt::registry& GetRegistry() const { return registry_; }
    
    // エンティティの一括削除など
    void Clear() {
        registry_.clear();
    }
};

// システムはレジストリへの参照を受け取る
class MovementSystem {
private:
    entt::registry& registry_;
    
public:
    MovementSystem(entt::registry& registry) : registry_(registry) {}
    
    void Update(float deltaTime) {
        auto view = registry_.view<Position, Velocity>();
        for (auto entity : view) {
            auto& pos = view.get<Position>(entity);
            auto& vel = view.get<Velocity>(entity);
            pos.x += vel.dx * deltaTime;
            pos.y += vel.dy * deltaTime;
        }
    }
};
```

**注意事項**:

- EnTT v3.12.0以降、ネストされたグループは非対応
- `view`を使用してエンティティを取得
- コンポーネントはPOD型（データのみ）として定義

### 4. 言語とエンコーディング

#### 4.1 日本語GUI、英語コンソール出力

```cpp
// ✅ GUI: 日本語を使用
ImGui::Text("エンティティ一覧");
ImGui::Button("保存");

// ✅ コンソール出力: 英語を使用
std::cout << "Entity created: " << entity << std::endl;
TRACELOG(LOG_INFO, "Texture loaded: %s", path.c_str());
```

#### 4.2 フォント設定

**フォントファイル**: `assets/fonts/NotoSansJP-Medium.ttf`

**読み込み範囲**:

- ひらがな（U+3040-U+309F）
- カタカナ（U+30A0-U+30FF）
- ASCII（U+0020-U+007F）
- JIS第一水準漢字（U+4E00-U+9FAF）

```cpp
// ✅ フォント読み込み実装例
Font LoadJapaneseFont(const std::string& path, int fontSize) {
    // 読み込むコードポイント範囲を定義
    int codepoints[] = {
        // ASCII
        0x0020, 0x007F,
        // ひらがな
        0x3040, 0x309F,
        // カタカナ
        0x30A0, 0x30FF,
        // JIS第一水準漢字（主要な範囲）
        0x4E00, 0x9FAF,
    };
    
    int codepointCount = sizeof(codepoints) / sizeof(codepoints[0]);
    
    Font font = LoadFontEx(path.c_str(), fontSize, codepoints, codepointCount);
    
    // すべてのライブラリでデフォルトフォントに設定
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    
    return font;
}

// 初期化時にフォントを設定
void InitializeFonts() {
    Font japaneseFont = LoadJapaneseFont("assets/fonts/NotoSansJP-Medium.ttf", 20);
    
    // Raylibデフォルトフォント設定
    // (Raylibはデフォルトフォントを変更できないため、DrawTextExを使用)
    
    // ImGuiフォント設定
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("assets/fonts/NotoSansJP-Medium.ttf", 20.0f, NULL, 
                                  io.Fonts->GetGlyphRangesJapanese());
    io.Fonts->Build();
    rlImGuiReloadFonts();
}
```

**注意**: ファイルサイズが大きくなるため、後で最適化予定。現時点では上記範囲で読み込む。

#### 4.3 UTF-8 BOMなしで保存

- **原則**: すべてのソースコードはUTF-8（BOMなし）で保存
- **CMake設定**: `/utf-8`（MSVC）または`-finput-charset=UTF-8`（GCC/Clang）が設定済み

```cpp
// ✅ 正しい: UTF-8 BOMなし
// ファイル先頭にBOM（0xEF 0xBB 0xBF）を含めない

// 日本語コメントもUTF-8で記述可能
// これは日本語のコメントです
```

### 5. データ駆動設計

#### 5.1 JSON定義による拡張

すべてのゲームコンテンツはJSONファイルで定義します。

```json
// entities.json の例
{
  "entities": [
    {
      "id": "enemy_basic",
      "name": "基本敵",
      "health": 100,
      "speed": 50.0,
      "sprite": "assets/sprites/enemy_basic.png"
    }
  ]
}
```

#### 5.2 JSON解析のエラーハンドリング

```cpp
// ✅ 必須: try-catchでJSON解析を囲む
json LoadJsonFile(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << path << std::endl;
            return json();
        }
        
        json j;
        file >> j;
        return j;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error in " << path << ": " << e.what() << std::endl;
        return json();
    } catch (const std::exception& e) {
        std::cerr << "Error reading " << path << ": " << e.what() << std::endl;
        return json();
    }
}
```

### 6. ECS設計原則

#### 6.1 コンポーネントはPOD型

```cpp
// ✅ 正しい: POD型コンポーネント
struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

struct Velocity {
    float dx = 0.0f;
    float dy = 0.0f;
};

// ❌ 間違い: ロジックを含むコンポーネント
struct BadComponent {
    float x, y;
    void Update() {}  // NG - ロジックはSystemに実装
};
```

#### 6.2 システムは独立して動作

```cpp
// ✅ 推奨: システムは独立
class MovementSystem {
public:
    void Update(entt::registry& registry, float deltaTime) {
        auto view = registry.view<Position, Velocity>();
        for (auto entity : view) {
            auto& pos = view.get<Position>(entity);
            auto& vel = view.get<Velocity>(entity);
            pos.x += vel.dx * deltaTime;
            pos.y += vel.dy * deltaTime;
        }
    }
};

class RenderSystem {
public:
    void Render(entt::registry& registry, IResourceManager& resources) {
        auto view = registry.view<Position, Sprite>();
        for (auto entity : view) {
            auto& pos = view.get<Position>(entity);
            auto& sprite = view.get<Sprite>(entity);
            Texture2D tex = resources.GetTexture(sprite.path);
            DrawTexture(tex, pos.x, pos.y, WHITE);
        }
    }
};
```

### 7. 仮想FHDレンダリング

#### 7.1 仮想解像度の管理

```cpp
// ✅ 仮想FHD（1920x1080）を基準にレンダリング
constexpr int VIRTUAL_WIDTH = 1920;
constexpr int VIRTUAL_HEIGHT = 1080;

class RenderSystem {
private:
    RenderTexture2D virtualRenderTarget_;
    float scaleX_ = 1.0f;
    float scaleY_ = 1.0f;
    
public:
    void Initialize() {
        virtualRenderTarget_ = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
    }
    
    void BeginRender() {
        BeginTextureMode(virtualRenderTarget_);
        ClearBackground(BLACK);
    }
    
    void EndRender() {
        EndTextureMode();
        
        // 実画面にスケーリングして描画
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        
        scaleX_ = static_cast<float>(screenWidth) / VIRTUAL_WIDTH;
        scaleY_ = static_cast<float>(screenHeight) / VIRTUAL_HEIGHT;
        
        DrawTexturePro(
            virtualRenderTarget_.texture,
            Rectangle{0, 0, VIRTUAL_WIDTH, -VIRTUAL_HEIGHT},
            Rectangle{0, 0, screenWidth, screenHeight},
            Vector2{0, 0},
            0.0f,
            WHITE
        );
    }
    
    // 入力座標を仮想座標に変換
    Vector2 ScreenToVirtual(Vector2 screenPos) {
        return Vector2{
            screenPos.x / scaleX_,
            screenPos.y / scaleY_
        };
    }
};
```

### 8. 依存性注入パターン

#### 8.1 GameContextによる依存管理

```cpp
class GameContext {
private:
    entt::registry registry_;
    std::unique_ptr<IResourceManager> resourceManager_;
    std::unique_ptr<InputManager> inputManager_;
    std::unique_ptr<RenderSystem> renderSystem_;
    
public:
    GameContext() {
        resourceManager_ = std::make_unique<ResourceManager>();
        inputManager_ = std::make_unique<InputManager>();
        renderSystem_ = std::make_unique<RenderSystem>(*resourceManager_);
    }
    
    entt::registry& GetRegistry() { return registry_; }
    IResourceManager& GetResourceManager() { return *resourceManager_; }
    InputManager& GetInputManager() { return *inputManager_; }
    RenderSystem& GetRenderSystem() { return *renderSystem_; }
};
```

## チェックリスト

新アーキテクチャ実装時の確認事項:

- [ ] グローバル変数を使用していない
- [ ] Singletonパターンを使用していない
- [ ] システム間はインターフェースを通じて通信している
- [ ] Raylibリソースは明示的に管理されている（RAIIパターンなど）
- [ ] EnTTレジストリはGameContextで管理されている
- [ ] JSON解析はtry-catchで囲まれている
- [ ] コンポーネントはPOD型として定義されている
- [ ] GUIは日本語、コンソール出力は英語
- [ ] フォントは指定範囲で読み込まれている
- [ ] ファイルはUTF-8 BOMなしで保存されている
- [ ] 仮想FHDレンダリングが実装されている
- [ ] 入力座標の仮想座標変換が実装されている

---

## スコープ宣言 / ドキュメント整理メモ

- L10n: ローカライズは現状スコープ外（日本語UI固定）。必要時に文言ID化/フォント/切替を別設計で追加。  
- ネットワーク: オフライン専用。マルチプレイ/オンライン同期は未対応（検討時に別設計）。  
- ドキュメント整理: `design-principles*.md` は概要/詳細の役割を分け、必要に応じ統合。`libs_guide.md` と `gamedev_libs_guide.md` は将来 `libs-overview.md` へ集約予定。`linea-td-detailed-design.md` は `line-td-detailed-design.md` へリネーム検討。

---

**作成日**: 2025-12-06  
**バージョン**: 1.0  
**対象**: 新アーキテクチャ（`include/new/`, `src/new/`）開発者向け
