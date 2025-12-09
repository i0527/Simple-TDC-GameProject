# 技術要件仕様書

**プロジェクト**: Simple TDC Game - New Architecture  
**バージョン**: 0.2.0  
**最終更新**: 2025-12-08

---

## 📋 概要

このドキュメントは、Simple TDC Game プロジェクトの技術要件を定義します。

---

## 🖥️ 解像度仕様

### 固定解像度

- **解像度**: 1920x1080 (Full HD)
- **アスペクト比**: 16:9
- **ウィンドウモード**: デフォルトでウィンドウモード、フルスクリーン切り替え可能

### 実装

```cpp
// game/src/Game/Application/GameApp.cpp
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple TDC Game");
SetWindowState(FLAG_WINDOW_RESIZABLE);  // リサイズ可能だが内部は固定
```

### レンダリング

- すべてのUI要素は1920x1080基準で配置
- ウィンドウサイズが変更された場合はスケーリングで対応（アスペクト比維持）

---

## 🔤 フォント仕様

### 使用フォント

- **ファイル**: `assets/fonts/NotoSansJP-Medium.ttf`
- **フォントファミリー**: Noto Sans JP
- **ウェイト**: Medium (500)
- **ライセンス**: SIL Open Font License 1.1

### 対応文字範囲

#### 1. ひらがな

- **Unicode範囲**: U+3040 - U+309F
- **文字数**: 約93文字
- **例**: あいうえお、がぎぐげご、ぱぴぷぺぽ

#### 2. カタカナ

- **Unicode範囲**: U+30A0 - U+30FF
- **文字数**: 約96文字
- **例**: アイウエオ、ガギグゲゴ、パピプペポ

#### 3. ASCII

- **Unicode範囲**: U+0020 - U+007E
- **文字数**: 95文字
- **含む**: 英数字、基本記号

#### 4. JIS第一水準漢字

- **Unicode範囲**: U+4E00 - U+9FFF (CJK統合漢字)
- **文字数**: 約2,965文字
- **用途**: 一般的な日本語表記

#### 5. JIS第二水準漢字

- **Unicode範囲**: U+4E00 - U+9FFF (追加漢字)
- **文字数**: 約3,390文字
- **用途**: 専門用語、人名、地名

### Raylib でのフォントロード

```cpp
// Raylib用フォントロード
Font LoadJapaneseFont(const char* filePath, int fontSize) {
    // フォントファイルのロード
    int fileSize = 0;
    unsigned char* fileData = LoadFileData(filePath, &fileSize);
    
    if (fileData == nullptr) {
        std::cerr << "Failed to load font: " << filePath << std::endl;
        return GetFontDefault();
    }
    
    // Unicode範囲の定義
    // ひらがな: U+3040-U+309F
    // カタカナ: U+30A0-U+30FF
    // ASCII: U+0020-U+007E
    // JIS第一・第二水準: U+4E00-U+9FFF
    
    int codepointsCount = 0;
    int* codepoints = nullptr;
    
    // 必要な文字範囲を配列に格納
    std::vector<int> charRanges;
    
    // ASCII
    for (int i = 0x0020; i <= 0x007E; i++) charRanges.push_back(i);
    
    // ひらがな
    for (int i = 0x3040; i <= 0x309F; i++) charRanges.push_back(i);
    
    // カタカナ
    for (int i = 0x30A0; i <= 0x30FF; i++) charRanges.push_back(i);
    
    // JIS第一・第二水準（CJK統合漢字）
    for (int i = 0x4E00; i <= 0x9FFF; i++) charRanges.push_back(i);
    
    codepointsCount = charRanges.size();
    codepoints = charRanges.data();
    
    // フォントのロード
    Font font = LoadFontFromMemory(
        ".ttf",
        fileData,
        fileSize,
        fontSize,
        codepoints,
        codepointsCount
    );
    
    UnloadFileData(fileData);
    
    // テクスチャフィルタリング設定（滑らか表示）
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    
    return font;
}
```

### ImGui でのフォントロード

```cpp
// ImGui用フォントロード
ImFont* LoadImGuiJapaneseFont(const char* filePath, float fontSize) {
    ImGuiIO& io = ImGui::GetIO();
    
    // フォント設定
    ImFontConfig config;
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.PixelSnapH = true;
    
    // 日本語グリフ範囲の取得
    const ImWchar* glyphRanges = io.Fonts->GetGlyphRangesJapanese();
    
    // フォントの追加
    ImFont* font = io.Fonts->AddFontFromFileTTF(
        filePath,
        fontSize,
        &config,
        glyphRanges
    );
    
    if (font == nullptr) {
        std::cerr << "Failed to load ImGui font: " << filePath << std::endl;
        return io.Fonts->AddFontDefault();
    }
    
    // フォントアトラスのビルド
    io.Fonts->Build();
    
    return font;
}
```

### デフォルトフォント設定

#### Game Layer

```cpp
// game/src/Game/Application/GameApp.cpp

void GameApp::Initialize() {
    // Raylib初期化
    InitWindow(1920, 1080, "Simple TDC Game");
    SetTargetFPS(60);
    
    // 日本語フォントのロード
    defaultFont_ = LoadJapaneseFont("assets/fonts/NotoSansJP-Medium.ttf", 32);
    
    // フォントが正しくロードされたか確認
    if (defaultFont_.texture.id == 0) {
        std::cerr << "Warning: Using default font instead of NotoSansJP" << std::endl;
        defaultFont_ = GetFontDefault();
    }
}

void GameApp::Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // 日本語フォントでテキスト描画
    DrawTextEx(
        defaultFont_,
        "タワーディフェンスゲーム",
        {100, 100},
        32,
        2.0f,
        BLACK
    );
    
    EndDrawing();
}

void GameApp::Shutdown() {
    // フォントのアンロード
    UnloadFont(defaultFont_);
    CloseWindow();
}
```

#### Editor Layer

```cpp
// editor/src/Editor/Application/EditorApp.cpp

void EditorApp::Initialize() {
    // Raylib初期化
    InitWindow(1920, 1080, "Simple TDC Editor");
    SetTargetFPS(60);
    
    // ImGui初期化
    rlImGuiSetup(true);
    
    // 日本語フォントのロード
    ImGuiIO& io = ImGui::GetIO();
    japaneseFont_ = LoadImGuiJapaneseFont(
        "assets/fonts/NotoSansJP-Medium.ttf",
        18.0f
    );
    
    // デフォルトフォントとして設定
    io.FontDefault = japaneseFont_;
}

void EditorApp::Render() {
    BeginDrawing();
    ClearBackground(DARKGRAY);
    
    // ImGui描画開始
    rlImGuiBegin();
    
    // メインメニューバー（日本語）
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("ファイル")) {
            if (ImGui::MenuItem("開く")) { /* ... */ }
            if (ImGui::MenuItem("保存")) { /* ... */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("編集")) {
            if (ImGui::MenuItem("エンティティ")) { /* ... */ }
            if (ImGui::MenuItem("スキル")) { /* ... */ }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    
    rlImGuiEnd();
    EndDrawing();
}
```

---

## 📦 依存ライブラリ管理

### FetchContent による自動ダウンロード

すべての外部ライブラリは CMake の `FetchContent` モジュールを使用して自動取得します。

#### ルート CMakeLists.txt

```cmake
include(FetchContent)

# EnTT (ECS)
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.12.2
)

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

# Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_MakeAvailable(entt json raylib)
```

#### Editor CMakeLists.txt

```cmake
# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9  # v1.90.x はrlImGuiと互換性問題あり
)

# rlImGui
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG main
)

FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(rlimgui)
if(NOT rlimgui_POPULATED)
    FetchContent_Populate(rlimgui)
endif()

# ImGui ライブラリ作成
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui_lib PUBLIC ${imgui_SOURCE_DIR})

# rlImGui ライブラリ作成
add_library(rlimgui_lib STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(rlimgui_lib PUBLIC
    ${rlimgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
)
target_link_libraries(rlimgui_lib PUBLIC raylib imgui_lib)
```

### バージョン互換性

| ライブラリ | バージョン | 互換性ノート |
|-----------|----------|-------------|
| EnTT | v3.12.2 | 安定版、C++17対応 |
| nlohmann/json | v3.11.3 | 最新安定版 |
| Raylib | 5.5 | 最新版、OpenGL 3.3 |
| ImGui | v1.89.9 | **v1.90.xはrlImGuiと互換性問題あり** |
| rlImGui | main branch | ImGui v1.89.x系と互換 |

**重要**: ImGui v1.90.0 以降では `GetPlatformIO()` が削除されたため、rlImGui の main ブランチ（旧版）との互換性がありません。現時点では ImGui v1.89.9 を使用してください。

---

## 🌐 UI 言語仕様

### 全UI日本語化

- すべてのメニュー、ボタン、ラベルは日本語で表記
- エラーメッセージも日本語（開発ログは英語可）
- 設定ファイル（JSON）のキーは英語、値は日本語可

### 例

#### ゲーム内UI

```cpp
// タイトル画面
DrawTextEx(font, "新規ゲーム", {x, y}, 32, 2.0f, BLACK);
DrawTextEx(font, "続きから", {x, y + 50}, 32, 2.0f, BLACK);
DrawTextEx(font, "設定", {x, y + 100}, 32, 2.0f, BLACK);

// ゲーム画面
DrawTextEx(font, "体力: 100", {10, 10}, 24, 1.5f, RED);
DrawTextEx(font, "コスト: 50 / 100", {10, 40}, 24, 1.5f, BLUE);
DrawTextEx(font, "Wave 3 / 10", {10, 70}, 24, 1.5f, GREEN);
```

#### エディタUI

```cpp
// メニューバー
if (ImGui::BeginMenu("ファイル")) {
    if (ImGui::MenuItem("新規作成", "Ctrl+N")) { /* ... */ }
    if (ImGui::MenuItem("開く", "Ctrl+O")) { /* ... */ }
    if (ImGui::MenuItem("保存", "Ctrl+S")) { /* ... */ }
    ImGui::Separator();
    if (ImGui::MenuItem("終了", "Alt+F4")) { /* ... */ }
    ImGui::EndMenu();
}

// エンティティエディタ
ImGui::Begin("エンティティエディタ");
ImGui::Text("名前:");
ImGui::InputText("##entity_name", buffer, 256);
ImGui::Text("体力:");
ImGui::InputInt("##entity_hp", &hp);
ImGui::Text("攻撃力:");
ImGui::InputInt("##entity_attack", &attack);
if (ImGui::Button("保存")) { /* ... */ }
ImGui::SameLine();
if (ImGui::Button("キャンセル")) { /* ... */ }
ImGui::End();
```

---

## 🔧 ビルド要件

### 最小要件

- **CMake**: 3.19 以上
- **C++ コンパイラ**: C++17 対応
  - Windows: MSVC 2019 以上
  - Linux: GCC 9.0 以上 / Clang 10.0 以上
  - macOS: Clang 12.0 以上（Xcode 12.5以上）
- **Git**: FetchContent での依存関係取得に必要

### 推奨環境

- **OS**: Windows 10/11 (64-bit)
- **RAM**: 8GB 以上
- **GPU**: OpenGL 3.3 対応
- **ディスク**: 500MB 以上の空き容量

---

## 📝 実装チェックリスト

### Phase 1: 基盤構築

- [x] Shared Layer 実装
- [x] Game Layer 基盤実装
- [x] Editor Layer 基盤実装（ビルド調整中）
- [x] FHD解像度固定（1920x1080）
- [ ] NotoSansJP フォント統合
- [ ] 全UI日本語化

### 今後の実装予定

- [ ] フォントローダークラス実装
- [ ] Raylib用日本語フォント設定
- [ ] ImGui用日本語フォント設定
- [ ] UI文字列の日本語化
- [ ] エディタビルド修正（ImGui v1.89.9対応）

---

## 📚 参考資料

### フォント

- [Noto Sans JP - Google Fonts](https://fonts.google.com/noto/specimen/Noto+Sans+JP)
- [SIL Open Font License](https://scripts.sil.org/OFL)

### ライブラリ

- [Raylib Documentation](https://www.raylib.com/)
- [Dear ImGui Documentation](https://github.com/ocornut/imgui)
- [EnTT Documentation](https://github.com/skypjack/entt)
- [nlohmann/json Documentation](https://json.nlohmann.me/)

---

**最終更新**: 2025-12-08
