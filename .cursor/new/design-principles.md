# 新アーキテクチャ設計原則と制約
>
> 本書は「要約版」です。詳細な解説や補足は `design-principles-and-constraints.md` を参照してください。

## 概要

このドキュメントは、新アーキテクチャ（`include/new/`, `src/new/`）の設計・実装時に遵守すべき原則と制約を定義します。

---

## 1. アーキテクチャ原則

### 1.1 疎結合設計

**原則**: システム間の結合度を極力低く保つ。

#### 実装方針

- **依存性注入（DI）パターンの徹底**
  - Singleton禁止（`.cursorrules` に準拠）
  - すべてのシステムはコンストラクタで依存関係を受け取る
  - `GameContext` を介した依存関係の管理

```cpp
// ❌ 禁止: Singleton使用
class ResourceManager {
    static ResourceManager& GetInstance();
};

// ✅ 推奨: DIパターン
class ResourceManager {
public:
    ResourceManager() = default;
    // ...
};

class RenderSystem {
    ResourceManager& resourceManager_;
public:
    RenderSystem(ResourceManager& rm) : resourceManager_(rm) {}
};
```

- **インターフェースによる抽象化**
  - システム間の直接依存を避け、インターフェースを介した通信
  - イベントシステムによる疎結合な通信

- **モジュール境界の明確化**
  - `include/new/` 配下のレイヤー構造を厳守
  - 上位レイヤーから下位レイヤーへの一方向依存のみ

### 1.2 グローバル変数の禁止

**原則**: グローバル変数は極力使用しない。

#### 実装方針

- **コンテキストオブジェクトによる状態管理**
  - `GameContext` に必要な状態を集約
  - システム間の状態共有は `GameContext` 経由

```cpp
// ❌ 禁止: グローバル変数
Texture2D globalTexture;

// ✅ 推奨: コンテキスト経由
class GameContext {
    ResourceManager resourceManager_;
    // ...
};

class RenderSystem {
    GameContext& context_;
public:
    RenderSystem(GameContext& ctx) : context_(ctx) {}
    void Render() {
        auto& texture = context_.resourceManager.GetTexture("sprite");
        // ...
    }
};
```

- **静的変数の最小化**
  - 定数のみ静的変数として許可
  - 状態を持つ静的変数は禁止

### 1.3 レイヤー構造

```
Application Layer (new/Application/)
    ↓
Game Layer (new/Game/)
    ├── Editor/          # 内部エディタ
    └── Systems/         # ゲームシステム
    ↓
Domain Layer (new/Domain/)
    ├── TD/              # タワーディフェンス
    └── UI/              # UI定義システム
    ↓
Core Layer (new/Core/)
    ├── ECS/             # EnTT統合
    ├── Rendering/       # Raylib統合
    └── Data/            # JSON処理
    ↓
External Libraries
    ├── EnTT
    ├── Raylib
    └── nlohmann/json
```

**依存ルール**:

- 上位レイヤーは下位レイヤーに依存可能
- 下位レイヤーは上位レイヤーに依存禁止
- 同一レイヤー内のモジュール間は最小限の依存のみ

---

## 2. ライブラリ統合の注意点

### 2.1 Raylib リソース管理

**重要**: Raylib のリソースは明示的に管理する必要がある（`.cursor/new/raylib_resource_mgmt.md` 参照）。

#### 実装方針

- **リソースマネージャーの実装**
  - `new/Core/ResourceManager.h` で一元管理
  - RAIIパターンによる自動解放
  - 共有リソースの参照カウント管理

```cpp
// ✅ 推奨: リソースマネージャー経由
class ResourceManager {
    std::unordered_map<std::string, Texture2D> textures_;
public:
    Texture2D& GetTexture(const std::string& path) {
        if (textures_.find(path) == textures_.end()) {
            textures_[path] = LoadTexture(path.c_str());
        }
        return textures_[path];
    }
    
    ~ResourceManager() {
        for (auto& [path, texture] : textures_) {
            UnloadTexture(texture);
        }
    }
};
```

- **リソースライフサイクル**
  - ロード: 初期化フェーズで一括ロード
  - 使用: ゲームループ内で参照
  - アンロード: 終了フェーズで一括アンロード

- **RenderTexture の管理**
  - 仮想FHDレンダリング用の `RenderTexture2D` は `GameViewRenderer` で管理
  - エディタ表示用の `RenderTexture2D` は `EditorManager` で管理

### 2.2 EnTT ECS 統合

**重要**: EnTT v3.12.2 の API 変更に注意（`.cursor/new/gamedev_libs_guide.md` 参照）。

#### 実装方針

- **レジストリの管理**
  - `GameContext` に `entt::registry` を保持
  - システムはレジストリへの参照を受け取る

```cpp
// ✅ 推奨: GameContext経由
class GameContext {
    entt::registry registry_;
public:
    entt::registry& GetRegistry() { return registry_; }
};

class MovementSystem {
    entt::registry& registry_;
public:
    MovementSystem(entt::registry& reg) : registry_(reg) {}
    void Update(float deltaTime) {
        auto view = registry_.view<Transform, Velocity>();
        // ...
    }
};
```

- **コンポーネント設計**
  - POD型のみ（`.cursorrules` に準拠）
  - ロジックはシステムに実装

- **グループの使用制限**
  - ネストされたグループは非対応（v3.12.0+）
  - ビューの組み合わせで代替

### 2.3 nlohmann/json 統合

**重要**: JSON解析は必ず例外処理で囲む（`.cursorrules` に準拠）。

#### 実装方針

```cpp
// ✅ 必須: 例外処理
try {
    std::ifstream file("config.json");
    json config = json::parse(file);
    // ...
} catch (const json::parse_error& e) {
    std::cerr << "JSON parse error: " << e.what() << std::endl;
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

- **JSONスキーマ定義**
  - `new/Data/Schemas/` 配下にスキーマ定義
  - バリデーション関数を実装

- **データローダー**
  - `new/Core/Data/DefinitionLoader.h` で統一的なロード処理
  - エラーハンドリングとログ出力を統一

---

## 3. 国際化と文字エンコーディング

### 3.1 言語使用方針

- **GUI**: 日本語を使用
- **コンソール出力**: 英語を使用

```cpp
// GUI (日本語)
ImGui::Text("エンティティ一覧");

// コンソール (英語)
std::cerr << "Failed to load texture: " << path << std::endl;
```

### 3.2 フォント設定

**フォントファイル**: `assets/fonts/NotoSansJP-Medium.ttf`

**読み込み範囲**:

- ひらがな・カタカナ
- ASCII範囲
- JIS第一水準漢字

**実装方針**:

```cpp
// フォント読み込み範囲の定義
int codepoints[] = {
    // ASCII: 0x0020-0x007E
    0x0020, 0x0021, /* ... */, 0x007E,
    // ひらがな: 0x3040-0x309F
    0x3040, 0x3041, /* ... */, 0x309F,
    // カタカナ: 0x30A0-0x30FF
    0x30A0, 0x30A1, /* ... */, 0x30FF,
    // JIS第一水準: 0x4E00-0x9FAF (範囲指定で最適化可能)
    // 注意: ファイルサイズが大きくなるため、後で最適化予定
};

Font font = LoadFontEx("assets/fonts/NotoSansJP-Medium.ttf", 16, codepoints, sizeof(codepoints) / sizeof(int));
```

**デフォルトフォント設定**:

- Raylib: `SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);`
- ImGui: `ImGuiIO& io = ImGui::GetIO(); io.Fonts->AddFontFromFileTTF("assets/fonts/NotoSansJP-Medium.ttf", 16.0f, NULL, codepoints);`
- raygui: `GuiSetFont(font);`

**注意**: 現時点ではファイルサイズを優先し、後で最適化する。

### 3.3 ファイルエンコーディング

**原則**: すべてのソースコードは UTF-8（BOMなし）で保存。

- CMake設定で UTF-8 を強制（`CMakeLists.txt` に設定済み）
- エディタ設定で UTF-8 BOMなしを指定
- 文字列リテラルは UTF-8 で記述

```cpp
// ✅ 正しい: UTF-8文字列リテラル
const char* text = "エンティティ";

// ❌ 禁止: マルチバイト文字の直接操作
// 必要に応じて std::wstring や UTF-8 ユーティリティを使用
```

---

## 4. データ駆動設計

### 4.1 JSON定義ファイル

**原則**: すべてのゲームコンテンツはJSONファイルで定義。

#### 定義ファイル構造

```
assets/
├── definitions/
│   ├── entities/          # エンティティ定義
│   │   ├── units.json     # ユニット定義
│   │   └── enemies.json   # 敵定義
│   ├── waves/             # ウェーブ定義
│   │   └── stage_01.json
│   ├── abilities/          # スキル・能力定義
│   │   └── skills.json
│   └── ui/                 # UIレイアウト定義
│       └── layouts.json
```

#### JSONスキーマ設計

- **エンティティ定義**: `new/Data/Schemas/EntitySchema.json`
- **ウェーブ定義**: `new/Data/Schemas/WaveSchema.json`
- **UI定義**: `new/Data/Schemas/UISchema.json`

### 4.2 データローダー設計

```cpp
class DefinitionLoader {
    GameContext& context_;
public:
    DefinitionLoader(GameContext& ctx) : context_(ctx) {}
    
    // エンティティ定義のロード
    bool LoadEntities(const std::string& path);
    
    // ウェーブ定義のロード
    bool LoadWaves(const std::string& path);
    
    // UI定義のロード
    bool LoadUI(const std::string& path);
};
```

### 4.3 ホットリロード

- ファイル監視システム（`HotReloadSystem`）で変更を検知
- JSON変更時に自動リロード
- エラーハンドリングとロールバック機能

---

## 5. 内部エディタ設計

### 5.1 エディタ起動

- **F1キー**: 開発者モードのトグル
- **レイアウト**: 固定3ペイン＋タイムライン、F1〜F4ショートカットでエディタ切替（DockSpace廃止）

### 5.2 エディタ機能

- **データエディタ**: JSON定義の編集
  - エンティティエディタ
  - ウェーブエディタ
  - UIレイアウトエディタ
- **デバッグツール**: ゲーム状態の可視化
  - Inspector（エンティティ詳細）
  - Hierarchy（エンティティ一覧）
  - Console（ログ表示）

### 5.3 ゲームビュー統合

- 仮想FHD（1920x1080）の `RenderTexture2D` にゲームを描画
- ImGuiウィンドウ内に `RenderTexture2D` を表示
- スケーリングと入力座標変換を自動処理

---

## 6. 仮想レンダリング設計

### 6.1 仮想解像度

**固定解像度**: 1920x1080 (FHD)

```cpp
class VirtualRenderer {
    RenderTexture2D renderTarget_;
    int virtualWidth_ = 1920;
    int virtualHeight_ = 1080;
    
public:
    void Initialize() {
        renderTarget_ = LoadRenderTexture(virtualWidth_, virtualHeight_);
    }
    
    void BeginRender() {
        BeginTextureMode(renderTarget_);
        ClearBackground(BLACK);
    }
    
    void EndRender() {
        EndTextureMode();
    }
    
    RenderTexture2D& GetRenderTarget() { return renderTarget_; }
};
```

### 6.2 スケーリング

- 実ウィンドウサイズに応じてスケーリング
- アスペクト比を維持
- レターボックス/ピラーボックス対応

### 6.3 入力座標変換

```cpp
Vector2 ScreenToVirtual(Vector2 screenPos, int screenWidth, int screenHeight) {
    float scaleX = static_cast<float>(virtualWidth_) / screenWidth;
    float scaleY = static_cast<float>(virtualHeight_) / screenHeight;
    return {screenPos.x * scaleX, screenPos.y * scaleY};
}
```

---

## 7. 実装チェックリスト

### 7.1 設計時

- [ ] システム間の依存関係を最小化
- [ ] グローバル変数を使用していない
- [ ] DIパターンで依存関係を注入
- [ ] レイヤー構造を遵守

### 7.2 実装時

- [ ] Raylibリソースを明示的に管理
- [ ] EnTT APIを正しく使用（v3.12.2準拠）
- [ ] JSON解析に例外処理を実装
- [ ] フォントを正しく読み込み・設定
- [ ] UTF-8（BOMなし）で保存

### 7.3 テスト時

- [ ] メモリリークがない（Valgrind等で確認）
- [ ] リソースが正しく解放される
- [ ] 日本語GUIが正しく表示される
- [ ] コンソール出力が英語である

---

## 8. 参考ドキュメント

- `.cursorrules` - プロジェクト全体のコーディング規約
- `.cursor/new/raylib_resource_mgmt.md` - Raylibリソース管理詳細
- `.cursor/new/gamedev_libs_guide.md` - ライブラリ統合ガイド
- `.cursor/new/libs_guide.md` - ライブラリ使用ガイド

---

**作成日**: 2025-12-06  
**バージョン**: 1.0  
**対象**: 新アーキテクチャ開発チーム
