# rlImGui 関数リファレンス チートシート

rlImGui (Raylib + ImGui統合) の主要なAPIをまとめたクイックリファレンスです。

## 目次

- [基本設定](#基本設定)
- [初期化・終了](#初期化終了)
- [フレーム管理](#フレーム管理)
- [高度な初期化](#高度な初期化)
- [画像表示](#画像表示)
- [画像ボタン](#画像ボタン)
- [カラー変換](#カラー変換)
- [よく使うパターン](#よく使うパターン)

---

## 基本設定

```cpp
#include "rlImGui.h"
#include "raylib.h"

// Font Awesomeアイコンを使用する場合（デフォルトで有効）
#include "extras/IconsFontAwesome6.h"

// Font Awesomeを無効にする場合
#define NO_FONT_AWESOME
#include "rlImGui.h"
```

---

## 初期化・終了

### 基本的な初期化

```cpp
// 初期化（ダークテーマ、デフォルト）
rlImGuiSetup(true);

// ライトテーマで初期化
rlImGuiSetup(false);

// 終了処理
rlImGuiShutdown();
```

### 基本的な使用パターン

```cpp
#include "rlImGui.h"
#include "raylib.h"

int main() {
    // ウィンドウ初期化
    InitWindow(800, 600, "rlImGui Example");
    
    // rlImGui初期化
    rlImGuiSetup(true);
    
    // メインループ
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // ImGuiフレーム開始
        rlImGuiBegin();
        
        // ImGuiコード
        ImGui::ShowDemoWindow();
        
        // ImGuiフレーム終了
        rlImGuiEnd();
        
        EndDrawing();
    }
    
    // クリーンアップ
    rlImGuiShutdown();
    CloseWindow();
    
    return 0;
}
```

---

## フレーム管理

### 基本的なフレーム管理

```cpp
// フレーム開始（自動的にGetFrameTime()を使用）
rlImGuiBegin();

// ImGuiコード
ImGui::Begin("Window");
ImGui::Text("Hello, world!");
ImGui::End();

// フレーム終了
rlImGuiEnd();
```

### カスタムデルタタイム

```cpp
// カスタムデルタタイムを指定
float customDeltaTime = 0.016f;  // 60 FPS
rlImGuiBeginDelta(customDeltaTime);

// 負の値を指定すると自動的にGetFrameTime()を使用
rlImGuiBeginDelta(-1.0f);  // 自動

// ImGuiコード
// ...

rlImGuiEnd();
```

---

## 高度な初期化

カスタムフォントや設定を追加する場合に使用します。

```cpp
// カスタム初期化の開始
rlImGuiBeginInitImGui();

// カスタム設定を追加
ImGuiIO& io = ImGui::GetIO();
io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

// カスタムフォントの追加
ImFont* customFont = io.Fonts->AddFontFromFileTTF("path/to/font.ttf", 16.0f);

// 初期化の完了（フォントのレンダリングを含む）
rlImGuiEndInitImGui();
```

---

## 画像表示

### 基本的な画像表示

```cpp
Texture2D texture = LoadTexture("image.png");

// テクスチャ全体を表示（カーソル位置、元のサイズ）
rlImGuiImage(&texture);

// 指定サイズで表示
rlImGuiImageSize(&texture, 200, 150);

// Vector2でサイズ指定
rlImGuiImageSizeV(&texture, Vector2{200, 150});

// テクスチャの一部を表示
Rectangle sourceRect = {0, 0, 64, 64};  // x, y, width, height
rlImGuiImageRect(&texture, 200, 200, sourceRect);

// 負の幅・高さで反転
Rectangle flippedRect = {0, 0, -64, -64};  // 上下左右反転
rlImGuiImageRect(&texture, 200, 200, flippedRect);
```

### レンダーテクスチャの表示

```cpp
RenderTexture2D renderTexture = LoadRenderTexture(800, 600);

// レンダーテクスチャを表示（Y軸自動反転）
rlImGuiImageRenderTexture(&renderTexture);

// コンテンツ領域にフィット（中央揃え）
rlImGuiImageRenderTextureFit(&renderTexture, true);

// 左揃え
rlImGuiImageRenderTextureFit(&renderTexture, false);
```

---

## 画像ボタン

```cpp
Texture2D texture = LoadTexture("button.png");

// 画像ボタン（テクスチャ全体、カーソル位置）
if (rlImGuiImageButton("ButtonID", &texture)) {
    // ボタンがクリックされた
    std::cout << "Button clicked!" << std::endl;
}

// サイズ指定の画像ボタン
Vector2 buttonSize = {100, 50};
if (rlImGuiImageButtonSize("ButtonID", &texture, buttonSize)) {
    // ボタンがクリックされた
}
```

---

## カラー変換

```cpp
#include "rlImGuiColors.h"

// Raylib ColorからImGui ImVec4へ変換
Color raylibColor = RED;
ImVec4 imguiColor = rlImGuiColors::Convert(raylibColor);
ImGui::PushStyleColor(ImGuiCol_Button, imguiColor);

// ImGui ImVec4からRaylib Colorへ変換
ImVec4 imguiColor2 = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
Color raylibColor2 = rlImGuiColors::Convert(imguiColor2);
DrawCircle(100, 100, 50, raylibColor2);
```

---

## よく使うパターン

### 基本的なウィンドウ

```cpp
void ShowMainWindow() {
    rlImGuiBegin();
    
    bool open = true;
    if (ImGui::Begin("Main Window", &open)) {
        ImGui::Text("Hello from rlImGui!");
        
        if (ImGui::Button("Click me")) {
            // ボタンクリック処理
        }
    }
    ImGui::End();
    
    rlImGuiEnd();
}
```

### レンダーテクスチャをImGuiウィンドウに表示

```cpp
RenderTexture2D gameTexture = LoadRenderTexture(800, 600);

void ShowGameView() {
    rlImGuiBegin();
    
    if (ImGui::Begin("Game View")) {
        // レンダーテクスチャをウィンドウ内に表示
        rlImGuiImageRenderTextureFit(&gameTexture, true);
    }
    ImGui::End();
    
    rlImGuiEnd();
}
```

### 画像ギャラリー

```cpp
std::vector<Texture2D> textures;

void ShowImageGallery() {
    rlImGuiBegin();
    
    if (ImGui::Begin("Image Gallery")) {
        for (size_t i = 0; i < textures.size(); ++i) {
            ImGui::PushID((int)i);
            
            // 画像ボタン
            if (rlImGuiImageButtonSize("Image", &textures[i], Vector2{100, 100})) {
                // 画像がクリックされた
                std::cout << "Image " << i << " clicked" << std::endl;
            }
            
            // 同じ行に複数の画像を配置
            if ((i + 1) % 4 != 0) {
                ImGui::SameLine();
            }
            
            ImGui::PopID();
        }
    }
    ImGui::End();
    
    rlImGuiEnd();
}
```

### カスタムフォントの使用

```cpp
void SetupCustomFont() {
    rlImGuiBeginInitImGui();
    
    ImGuiIO& io = ImGui::GetIO();
    
    // カスタムフォントの追加
    ImFont* font = io.Fonts->AddFontFromFileTTF("assets/fonts/NotoSansJP-Medium.ttf", 16.0f);
    
    // Font Awesomeアイコンの追加（オプション）
    #ifndef NO_FONT_AWESOME
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = FONT_AWESOME_ICON_SIZE;
    io.Fonts->AddFontFromMemoryCompressedTTF(
        fa_solid_900_compressed_data,
        fa_solid_900_compressed_size,
        FONT_AWESOME_ICON_SIZE,
        &icons_config,
        icons_ranges
    );
    #endif
    
    rlImGuiEndInitImGui();
}
```

### デバッグ情報の表示

```cpp
void ShowDebugInfo() {
    rlImGuiBegin();
    
    if (ImGui::Begin("Debug Info")) {
        ImGui::Text("FPS: %.1f", GetFPS());
        ImGui::Text("Frame Time: %.3f ms", GetFrameTime() * 1000.0f);
        
        Vector2 mousePos = GetMousePosition();
        ImGui::Text("Mouse: (%.0f, %.0f)", mousePos.x, mousePos.y);
        
        // RaylibカラーをImGuiで表示
        Color bgColor = GetBackgroundColor();
        ImVec4 imguiColor = rlImGuiColors::Convert(bgColor);
        ImGui::ColorEdit4("Background Color", (float*)&imguiColor);
    }
    ImGui::End();
    
    rlImGuiEnd();
}
```

### 設定ウィンドウ

```cpp
struct GameSettings {
    float volume = 0.5f;
    bool fullscreen = false;
    Color bgColor = RAYWHITE;
};

void ShowSettingsWindow(GameSettings& settings) {
    rlImGuiBegin();
    
    if (ImGui::Begin("Settings")) {
        ImGui::SliderFloat("Volume", &settings.volume, 0.0f, 1.0f);
        ImGui::Checkbox("Fullscreen", &settings.fullscreen);
        
        // カラー選択
        ImVec4 color = rlImGuiColors::Convert(settings.bgColor);
        if (ImGui::ColorEdit4("Background Color", (float*)&color)) {
            settings.bgColor = rlImGuiColors::Convert(color);
        }
        
        if (ImGui::Button("Apply")) {
            // 設定を適用
            SetMasterVolume(settings.volume);
            if (settings.fullscreen) {
                ToggleFullscreen();
            }
        }
    }
    ImGui::End();
    
    rlImGuiEnd();
}
```

### スプライトシートの表示

```cpp
Texture2D spriteSheet = LoadTexture("sprites.png");

void ShowSpriteSheet() {
    rlImGuiBegin();
    
    if (ImGui::Begin("Sprite Sheet")) {
        // スプライトシートから個別のスプライトを表示
        int spriteWidth = 32;
        int spriteHeight = 32;
        
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                Rectangle sourceRect = {
                    (float)(x * spriteWidth),
                    (float)(y * spriteHeight),
                    (float)spriteWidth,
                    (float)spriteHeight
                };
                
                rlImGuiImageRect(&spriteSheet, spriteWidth, spriteHeight, sourceRect);
                
                if (x < 3) {
                    ImGui::SameLine();
                }
            }
        }
    }
    ImGui::End();
    
    rlImGuiEnd();
}
```

---

## 検索インデックス

### カテゴリ別検索

- **初期化**: `rlImGuiSetup()`, `rlImGuiBeginInitImGui()`, `rlImGuiEndInitImGui()`
- **終了**: `rlImGuiShutdown()`
- **フレーム管理**: `rlImGuiBegin()`, `rlImGuiEnd()`, `rlImGuiBeginDelta()`
- **画像表示**: `rlImGuiImage()`, `rlImGuiImageSize()`, `rlImGuiImageSizeV()`, `rlImGuiImageRect()`
- **レンダーテクスチャ**: `rlImGuiImageRenderTexture()`, `rlImGuiImageRenderTextureFit()`
- **画像ボタン**: `rlImGuiImageButton()`, `rlImGuiImageButtonSize()`
- **カラー変換**: `rlImGuiColors::Convert()`

### よく使う関数

- **初期化**: `rlImGuiSetup(true)` （ダークテーマ）
- **フレーム開始**: `rlImGuiBegin()`
- **フレーム終了**: `rlImGuiEnd()`
- **終了**: `rlImGuiShutdown()`
- **画像表示**: `rlImGuiImage(&texture)`
- **画像ボタン**: `rlImGuiImageButton("ID", &texture)`

---

## 注意事項

### High DPIディスプレイ

- `FLAG_WINDOW_HIGHDPI`を使用する場合、フォントがぼやける可能性があります
- rlImGuiは自動的にフォントをスケールしますが、カスタムフォントの場合は手動でスケールが必要な場合があります
- より良い方法は、`FLAG_WINDOW_HIGHDPI`を使わず、`GetWindowDPIScale()`で入力値をスケールすることです

### Font Awesome

- デフォルトでFont Awesome 6が有効になっています
- 無効にするには`#define NO_FONT_AWESOME`を定義してください
- アイコンは`ICON_FA_*`マクロで使用できます

### 低レベルAPI

- より細かい制御が必要な場合は、`imgui_impl_raylib.h`の低レベルAPIを使用できます
- 低レベルAPIは自動的なコンテキスト管理を行いません

---

**注意**: このチートシートは rlImGui の主要な機能をまとめています。詳細なパラメータや使用例については、[rlImGui公式リポジトリ](https://github.com/raylib-extras/rlImGui)を参照してください。
