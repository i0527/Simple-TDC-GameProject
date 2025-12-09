# NotoSansJP-Medium フォント統一設定ガイド（効率化版）

## 概要
このドキュメントでは、Simple TDC Game プロジェクト全体で NotoSansJP-Medium フォントを効率的に管理・使用するための設定方法と実装詳細を説明します。

**v2.0の主な改善点：**
- ✅ フォントを参照で返す（コピーオーバーヘッドを削除）
- ✅ サイズ別フォントキャッシュシステム
- ✅ ヘルパー関数による簡潔なAPI
- ✅ 後方互換性を削除してシンプル化

## 実装内容

### 1. 効率化されたUIManager

#### 主要な変更点

**フォント取得の効率化**
```cpp
// 旧API（コピーが発生）
Font font = uiManager.GetDefaultFont();

// 新API（参照で取得、コピーなし）
const Font& font = uiManager.GetFont();
```

**サイズ別フォントキャッシュ**
```cpp
// 異なるサイズのフォントを効率的に取得
const Font& font16 = uiManager.GetFont(16);  // 初回：ロード＆キャッシュ
const Font& font20 = uiManager.GetFont(20);  // 初回：ロード＆キャッシュ
const Font& font16_2 = uiManager.GetFont(16); // 2回目：キャッシュから即座に取得
```

**ヘルパー関数による簡潔化**
```cpp
// 旧コード（冗長）
Font defaultFont = uiManager.GetDefaultFont();
DrawTextEx(defaultFont, u8"テキスト", {10, 10}, 20, 1, DARKGRAY);

// 新コード（簡潔）
UI::DrawText(u8"テキスト", {10, 10}, 20, DARKGRAY);
```

### 2. 新しいAPI

#### UIManager::GetFont()
```cpp
// ベースフォントを取得（最も効率的）
const Font& baseFont = uiManager.GetFont();

// 特定サイズのフォントを取得（キャッシュ付き）
const Font& font20 = uiManager.GetFont(20);
```

#### UI::DrawText() ヘルパー関数
```cpp
namespace UI {
    inline void DrawText(const char* text, Vector2 position, int fontSize, Color color);
}

// 使用例
UI::DrawText(u8"日本語テキスト", {100, 200}, 24, RED);
```

### 3. フォント管理の仕組み

#### キャッシュシステム
UIManagerは内部で`std::unordered_map<int, Font>`を使用してサイズ別にフォントをキャッシュします：

```
baseFontSize_ = 18
baseFont_ = Font{...}  // ベースフォント（常にメモリ上）

fontCache_ = {
    {16, Font{...}},   // 初回リクエスト時にロード
    {20, Font{...}},   // 初回リクエスト時にロード
    {24, Font{...}},   // 初回リクエスト時にロード
}
```

**メモリ効率:**
- 使用されないサイズのフォントはロードされない
- 一度ロードしたフォントは再利用される
- `Shutdown()`時にすべて自動的にアンロードされる

#### 対象ライブラリ
NotoSansJP-Medium フォントは以下の3つのライブラリで統一的に使用されます：

1. **Raylib** - ゲーム描画エンジン
   - `UI::DrawText()` ヘルパー関数を使用
   
2. **raygui** - ゲーム内HUD・UI
   - `GuiSetFont()` で自動設定
   
3. **Dear ImGui** - デバッグ・開発ツール
   - `io.Fonts->AddFontFromFileTTF()` で自動設定

### 4. 初期化

`Game.cpp` のコンストラクタで初期化：

```cpp
// UIマネージャーの初期化
uiManager_.Initialize("assets/fonts/NotoSansJP-Medium.ttf", 18.0f);
```

### 5. 使用例

#### 基本的な使用方法
```cpp
void MyScene::Render(entt::registry& registry) {
    // システム描画
    Systems::SpriteRenderSystem::Render(registry);
    
    // テキスト描画（簡潔版）
    UI::DrawText(u8"スコア: 1000", {10, 10}, 20, WHITE);
    UI::DrawText(u8"HP: 100", {10, 40}, 16, RED);
}
```

#### 複数サイズのフォント使用
```cpp
void DrawUI() {
    // タイトル（大きいフォント）
    UI::DrawText(u8"タイトル", {10, 10}, 32, YELLOW);
    
    // 説明文（中サイズ）
    UI::DrawText(u8"説明文です", {10, 50}, 20, WHITE);
    
    // 詳細（小さいフォント）
    UI::DrawText(u8"詳細情報", {10, 80}, 14, GRAY);
}
```

#### 直接フォントを取得する場合
```cpp
void AdvancedRender() {
    auto& uiMgr = UI::UIManager::GetInstance();
    
    // ベースフォントを取得（参照）
    const Font& font = uiMgr.GetFont();
    
    // カスタム描画
    DrawTextEx(font, u8"カスタム描画", {100, 100}, 20, 2.0f, BLUE);
    
    // 特定サイズのフォントを取得
    const Font& bigFont = uiMgr.GetFont(48);
    DrawTextEx(bigFont, u8"大きな文字", {100, 150}, 48, 1.0f, RED);
}
```

## パフォーマンス最適化

### 1. フォントコピーの削減
```cpp
// ❌ 悪い例（毎フレームコピーが発生）
void Render() {
    Font font = uiManager.GetFont();  // コピーコンストラクタ呼び出し
    DrawTextEx(font, "text", {0, 0}, 20, 1, WHITE);
}

// ✅ 良い例（参照使用、コピーなし）
void Render() {
    const Font& font = uiManager.GetFont();  // 参照取得
    DrawTextEx(font, "text", {0, 0}, 20, 1, WHITE);
}

// ✅ 最良（ヘルパー関数使用）
void Render() {
    UI::DrawText("text", {0, 0}, 20, WHITE);
}
```

### 2. フォントキャッシュの活用
```cpp
// ✅ フォントサイズを固定して再利用
void RenderHUD() {
    // 毎フレーム同じサイズを使用 → キャッシュから即座に取得
    UI::DrawText(u8"HP: 100", {10, 10}, 20, RED);
    UI::DrawText(u8"Score: 1000", {10, 40}, 20, WHITE);
    UI::DrawText(u8"Level: 5", {10, 70}, 20, YELLOW);
}
```

### 3. 不要なフォントサイズを避ける
```cpp
// ❌ 避けるべき（毎フレーム異なるサイズ → キャッシュが肥大化）
void Render() {
    for (int i = 10; i < 100; ++i) {
        UI::DrawText("text", {0, 0}, i, WHITE);  // 90種類のフォントがロードされる
    }
}

// ✅ 推奨（固定サイズを使用）
void Render() {
    const int sizes[] = {16, 20, 24, 32};  // 必要なサイズのみ
    for (int size : sizes) {
        UI::DrawText("text", {0, 0}, size, WHITE);
    }
}
```

## ベストプラクティス

### 1. ヘルパー関数を優先
```cpp
// 推奨：ヘルパー関数を使用
UI::DrawText(u8"日本語テキスト", {10, 10}, 20, WHITE);

// 必要な場合のみ直接アクセス
const Font& font = uiManager.GetFont(20);
DrawTextEx(font, u8"カスタム描画", {10, 10}, 20, 2.0f, WHITE);
```

### 2. 日本語文字列リテラル
必ず `u8` プレフィックスを使用：
```cpp
UI::DrawText(u8"日本語テキスト", {10, 10}, 20, WHITE);
```

### 3. フォントサイズの標準化
プロジェクト全体で使用するフォントサイズを標準化：
```cpp
namespace FontSizes {
    constexpr int Small = 14;
    constexpr int Medium = 18;
    constexpr int Large = 24;
    constexpr int ExtraLarge = 32;
}

// 使用例
UI::DrawText(u8"タイトル", {10, 10}, FontSizes::Large, YELLOW);
```

## トラブルシューティング

### フォントが読み込まれない
- フォントファイルのパスが正しいか確認
- `assets/fonts/NotoSansJP-Medium.ttf` が存在するか確認
- コンソール出力で "UIManager initialized successfully" を確認

### 文字化けが発生する
- ソースファイルが UTF-8 で保存されているか確認
- 文字列リテラルに `u8` プレフィックスを使用
- 表示したい文字が対応コードポイント範囲に含まれているか確認

### パフォーマンス問題
- フォントを値で受け取っていないか確認（`const Font&` を使用）
- 不要に多くのフォントサイズを使用していないか確認
- ヘルパー関数 `UI::DrawText()` を活用

## API変更まとめ

### 削除されたAPI（後方互換性なし）
- `GetDefaultFont()` → `GetFont()` に統合
- `GetRayguiFont()` → `GetFont()` に統合
- `BeginFrame()` / `EndFrame()` → 不要のため削除

### 新しいAPI
- `const Font& GetFont() noexcept` - ベースフォント取得（参照）
- `const Font& GetFont(int fontSize)` - サイズ指定フォント取得（キャッシュ付き）
- `UI::DrawText(...)` - テキスト描画ヘルパー関数
- `bool IsInitialized() noexcept` - 初期化確認

## まとめ

この効率化により、以下の改善が実現されました：

✅ **パフォーマンス向上** - フォントコピーの削除、キャッシュシステム  
✅ **メモリ効率化** - 必要なサイズのみロード、自動管理  
✅ **コードの簡潔化** - ヘルパー関数による記述量削減  
✅ **保守性向上** - シンプルで明確なAPI、後方互換性削除  

新しいシーンや機能を追加する際は、`UI::DrawText()` ヘルパー関数を使用することで、最も簡潔かつ効率的にテキストを描画できます。
