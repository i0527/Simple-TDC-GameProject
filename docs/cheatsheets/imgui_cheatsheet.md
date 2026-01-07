# Dear ImGui 関数リファレンス チートシート

Dear ImGuiの主要なAPIをまとめたクイックリファレンスです。

## 目次

- [基本設定](#基本設定)
- [コンテキスト管理](#コンテキスト管理)
- [フレーム管理](#フレーム管理)
- [ウィンドウ](#ウィンドウ)
  - [ウィンドウの作成](#ウィンドウの作成)
  - [ウィンドウの操作](#ウィンドウの操作)
  - [子ウィンドウ](#子ウィンドウ)
- [ウィジェット](#ウィジェット)
  - [テキスト](#テキスト)
  - [ボタン](#ボタン)
  - [入力](#入力)
  - [スライダー・ドラッグ](#スライダードラッグ)
  - [チェックボックス・ラジオボタン](#チェックボックスラジオボタン)
  - [コンボボックス・リストボックス](#コンボボックスリストボックス)
  - [カラーピッカー](#カラーピッカー)
  - [ツリーノード](#ツリーノード)
  - [セレクタブル](#セレクタブル)
  - [プログレスバー](#プログレスバー)
  - [画像](#画像)
- [テーブル](#テーブル)
- [メニュー・ポップアップ](#メニューポップアップ)
- [ドラッグ&ドロップ](#ドラッグドロップ)
- [スタイル](#スタイル)
- [描画API](#描画api)
- [ユーティリティ](#ユーティリティ)
- [よく使うパターン](#よく使うパターン)

---

## 基本設定

```cpp
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// バージョンチェック
IMGUI_CHECKVERSION();

// コンテキスト作成
ImGui::CreateContext();
ImGuiIO& io = ImGui::GetIO();
```

---

## コンテキスト管理

```cpp
// コンテキストの作成・削除
ImGuiContext* ctx = ImGui::CreateContext(ImFontAtlas* shared_font_atlas = NULL);
ImGui::DestroyContext(ImGuiContext* ctx = NULL);

// コンテキストの取得・設定
ImGuiContext* ctx = ImGui::GetCurrentContext();
ImGui::SetCurrentContext(ctx);

// IO/スタイルの取得
ImGuiIO& io = ImGui::GetIO();
ImGuiStyle& style = ImGui::GetStyle();
ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
```

---

## フレーム管理

```cpp
// フレームの開始・終了
ImGui::NewFrame();        // フレーム開始
// ... UIコード ...
ImGui::EndFrame();        // フレーム終了（オプション）
ImGui::Render();          // レンダリング（EndFrame()も呼ばれる）
ImDrawData* draw_data = ImGui::GetDrawData();  // 描画データ取得
```

---

## ウィンドウ

### ウィンドウの作成

```cpp
// 基本ウィンドウ
bool open = true;
if(ImGui::Begin("Window Title", &open)) {
    // ウィンドウコンテンツ
}
ImGui::End();

// フラグ付きウィンドウ
if(ImGui::Begin("Window", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
    // ...
}
ImGui::End();
```

### ウィンドウの操作

```cpp
// ウィンドウ状態の確認
bool appearing = ImGui::IsWindowAppearing();
bool collapsed = ImGui::IsWindowCollapsed();
bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_None);
bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_None);

// ウィンドウ情報の取得
ImVec2 pos = ImGui::GetWindowPos();
ImVec2 size = ImGui::GetWindowSize();
float width = ImGui::GetWindowWidth();
float height = ImGui::GetWindowHeight();
ImDrawList* draw_list = ImGui::GetWindowDrawList();
float dpi_scale = ImGui::GetWindowDpiScale();

// 次のウィンドウの設定（Begin()の前に呼ぶ）
ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
ImGui::SetNextWindowCollapsed(false, ImGuiCond_FirstUseEver);
ImGui::SetNextWindowFocus();
ImGui::SetNextWindowBgAlpha(0.5f);

// 現在のウィンドウの設定（Begin()の後に呼ぶ）
ImGui::SetWindowPos(ImVec2(100, 100), ImGuiCond_Once);
ImGui::SetWindowSize(ImVec2(400, 300), ImGuiCond_Once);
ImGui::SetWindowCollapsed(false, ImGuiCond_Once);
ImGui::SetWindowFocus();

// 名前付きウィンドウの設定
ImGui::SetWindowPos("Window Name", ImVec2(100, 100));
ImGui::SetWindowSize("Window Name", ImVec2(400, 300));
ImGui::SetWindowCollapsed("Window Name", false);
ImGui::SetWindowFocus("Window Name");
```

### 子ウィンドウ

```cpp
// 子ウィンドウの作成
if(ImGui::BeginChild("Child", ImVec2(0, 200), ImGuiChildFlags_Border)) {
    // 子ウィンドウコンテンツ
}
ImGui::EndChild();

// ID指定の子ウィンドウ
ImGuiID child_id = ImGui::GetID("Child");
if(ImGui::BeginChild(child_id, ImVec2(0, 200))) {
    // ...
}
ImGui::EndChild();
```

### スクロール

```cpp
// スクロール位置の取得・設定
float scroll_x = ImGui::GetScrollX();
float scroll_y = ImGui::GetScrollY();
ImGui::SetScrollX(float scroll_x);
ImGui::SetScrollY(float scroll_y);

// スクロール最大値
float max_x = ImGui::GetScrollMaxX();
float max_y = ImGui::GetScrollMaxY();

// スクロールの設定（次のウィンドウ）
ImGui::SetNextWindowScroll(ImVec2(0, 100));
```

---

## ウィジェット

### テキスト

```cpp
// テキスト表示
ImGui::Text("Hello, world!");
ImGui::Text("Value: %d", value);
ImGui::Text("Position: (%.2f, %.2f)", pos.x, pos.y);

// 色付きテキスト
ImGui::TextColored(ImVec4(1, 0, 0, 1), "Red text");
ImGui::TextDisabled("Disabled text");
ImGui::TextWrapped("Long text that will wrap");

// ラベル付きテキスト
ImGui::LabelText("label", "Value: %d", value);
ImGui::BulletText("Bullet point");

// 区切り線
ImGui::Separator();
ImGui::SeparatorText("Section Title");
```

### ボタン

```cpp
// 基本ボタン
if(ImGui::Button("Click me")) {
    // クリック時の処理
}

// サイズ指定ボタン
if(ImGui::Button("Button", ImVec2(100, 50))) {
    // ...
}

// 小さなボタン
if(ImGui::SmallButton("Small")) {
    // ...
}

// 矢印ボタン
if(ImGui::ArrowButton("left", ImGuiDir_Left)) {
    // ...
}

// 見えないボタン（カスタム描画用）
if(ImGui::InvisibleButton("invisible", ImVec2(100, 50))) {
    // ...
}

// 画像ボタン
if(ImGui::ImageButton(texture_id, ImVec2(64, 64))) {
    // ...
}
```

### 入力

```cpp
// テキスト入力
char buffer[256] = "";
ImGui::InputText("Label", buffer, sizeof(buffer));

// フラグ付きテキスト入力
ImGui::InputText("Password", buffer, sizeof(buffer), 
    ImGuiInputTextFlags_Password);

// 複数行テキスト入力
char multiline[1024] = "";
ImGui::InputTextMultiline("Multiline", multiline, sizeof(multiline), 
    ImVec2(0, 100));

// 数値入力
int int_value = 0;
ImGui::InputInt("Integer", &int_value);
ImGui::InputInt2("Int2", int_array);
ImGui::InputInt3("Int3", int_array);
ImGui::InputInt4("Int4", int_array);

float float_value = 0.0f;
ImGui::InputFloat("Float", &float_value);
ImGui::InputFloat2("Float2", float_array);
ImGui::InputFloat3("Float3", float_array);
ImGui::InputFloat4("Float4", float_array);

// ステップ付き入力
ImGui::InputInt("Int", &int_value, 1, 10);
ImGui::InputFloat("Float", &float_value, 0.1f, 1.0f);
```

### スライダー・ドラッグ

```cpp
// スライダー
int slider_int = 50;
ImGui::SliderInt("Slider Int", &slider_int, 0, 100);

float slider_float = 0.5f;
ImGui::SliderFloat("Slider Float", &slider_float, 0.0f, 1.0f);
ImGui::SliderFloat2("Slider Float2", float_array, 0.0f, 1.0f);
ImGui::SliderFloat3("Slider Float3", float_array, 0.0f, 1.0f);
ImGui::SliderFloat4("Slider Float4", float_array, 0.0f, 1.0f);

// 角度スライダー
float angle = 0.0f;
ImGui::SliderAngle("Angle", &angle);

// ドラッグ
int drag_int = 0;
ImGui::DragInt("Drag Int", &drag_int, 1.0f, 0, 100);

float drag_float = 0.0f;
ImGui::DragFloat("Drag Float", &drag_float, 0.1f, 0.0f, 1.0f);
ImGui::DragFloat2("Drag Float2", float_array, 0.1f);
ImGui::DragFloat3("Drag Float3", float_array, 0.1f);
ImGui::DragFloat4("Drag Float4", float_array, 0.1f);
```

### チェックボックス・ラジオボタン

```cpp
// チェックボックス
bool checked = false;
ImGui::Checkbox("Checkbox", &checked);

// 3状態チェックボックス
int check_state = 0;  // 0=unchecked, 1=checked, 2=indeterminate
ImGui::CheckboxFlags("Flags", &flags, ImGuiWindowFlags_NoTitleBar);

// ラジオボタン
int radio_value = 0;
ImGui::RadioButton("Option 1", &radio_value, 0);
ImGui::RadioButton("Option 2", &radio_value, 1);
ImGui::RadioButton("Option 3", &radio_value, 2);
```

### コンボボックス・リストボックス

```cpp
// コンボボックス
const char* items[] = { "Item 1", "Item 2", "Item 3" };
static int current_item = 0;
if(ImGui::BeginCombo("Combo", items[current_item])) {
    for(int i = 0; i < IM_ARRAYSIZE(items); i++) {
        bool is_selected = (current_item == i);
        if(ImGui::Selectable(items[i], is_selected)) {
            current_item = i;
        }
        if(is_selected) {
            ImGui::SetItemDefaultFocus();
        }
    }
    ImGui::EndCombo();
}

// リストボックス
ImGui::ListBox("List", &current_item, items, IM_ARRAYSIZE(items), 4);
```

### カラーピッカー

```cpp
// カラーエディタ
float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
ImGui::ColorEdit4("Color", color);

// カラーピッカー
ImGui::ColorPicker4("Picker", color);

// カラーボタン
ImVec4 color_vec(1, 0, 0, 1);
if(ImGui::ColorButton("Color", color_vec)) {
    // ...
}
```

### ツリーノード

```cpp
// ツリーノード
if(ImGui::TreeNode("Node")) {
    ImGui::Text("Child content");
    ImGui::TreePop();
}

// フラグ付きツリーノード
if(ImGui::TreeNodeEx("Node", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Child content");
    ImGui::TreePop();
}

// 折りたたみヘッダー
if(ImGui::CollapsingHeader("Header")) {
    ImGui::Text("Content");
}

// 折りたたみヘッダー（チェックボックス付き）
bool open = true;
if(ImGui::CollapsingHeader("Header", &open)) {
    ImGui::Text("Content");
}
```

### セレクタブル

```cpp
// セレクタブル
bool selected = false;
if(ImGui::Selectable("Item", selected)) {
    selected = !selected;
}

// サイズ指定セレクタブル
ImGui::Selectable("Item", selected, ImGuiSelectableFlags_None, ImVec2(100, 50));
```

### プログレスバー

```cpp
// プログレスバー
float progress = 0.5f;
ImGui::ProgressBar(progress, ImVec2(0, 0), "50%");

// オーバーレイ付きプログレスバー
ImGui::ProgressBar(progress, ImVec2(-1, 0), "Loading...");
```

### 画像

```cpp
// 画像表示
ImGui::Image(texture_id, ImVec2(64, 64));

// 色付き画像
ImGui::Image(texture_id, ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), 
    ImVec4(1, 1, 1, 1));

// 画像ボタン
if(ImGui::ImageButton(texture_id, ImVec2(64, 64))) {
    // ...
}
```

---

## テーブル

```cpp
// テーブルの作成
if(ImGui::BeginTable("table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
    // カラム設定
    ImGui::TableSetupColumn("Column 1", ImGuiTableColumnFlags_WidthFixed, 100.0f);
    ImGui::TableSetupColumn("Column 2", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Column 3", ImGuiTableColumnFlags_WidthFixed, 100.0f);
    ImGui::TableSetupScrollFreeze(0, 1);  // ヘッダー固定
    ImGui::TableHeadersRow();

    // データ行
    for(int row = 0; row < 10; row++) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Row %d Col 1", row);
        ImGui::TableNextColumn();
        ImGui::Text("Row %d Col 2", row);
        ImGui::TableNextColumn();
        ImGui::Text("Row %d Col 3", row);
    }
    ImGui::EndTable();
}

// テーブル操作
ImGui::TableNextRow(ImGuiTableRowFlags_None, row_height);
ImGui::TableNextColumn();
ImGui::TableSetColumnIndex(int column_index);
int column_index = ImGui::TableGetColumnIndex();
int column_count = ImGui::TableGetColumnCount();
int row_index = ImGui::TableGetRowIndex();
bool is_row_hovered = ImGui::TableIsRowHovered();

// テーブルスタイル
ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(255, 0, 0, 255));
ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 255, 0, 255), column_index);
```

---

## メニュー・ポップアップ

### メニューバー

```cpp
// メニューバー
if(ImGui::BeginMenuBar()) {
    if(ImGui::BeginMenu("File")) {
        if(ImGui::MenuItem("New")) { /* ... */ }
        if(ImGui::MenuItem("Open", "Ctrl+O")) { /* ... */ }
        ImGui::Separator();
        if(ImGui::MenuItem("Exit")) { /* ... */ }
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}
```

### ポップアップ

```cpp
// ポップアップの開閉
if(ImGui::Button("Open Popup")) {
    ImGui::OpenPopup("popup_id");
}

if(ImGui::BeginPopup("popup_id")) {
    ImGui::Text("Popup content");
    if(ImGui::Button("Close")) {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

// コンテキストメニュー
if(ImGui::BeginPopupContextWindow()) {
    if(ImGui::MenuItem("Option 1")) { /* ... */ }
    if(ImGui::MenuItem("Option 2")) { /* ... */ }
    ImGui::EndPopup();
}

// アイテムコンテキストメニュー
if(ImGui::BeginPopupContextItem("item_id")) {
    if(ImGui::MenuItem("Edit")) { /* ... */ }
    if(ImGui::MenuItem("Delete")) { /* ... */ }
    ImGui::EndPopup();
}

// モーダル
if(ImGui::BeginPopupModal("Modal", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Modal content");
    if(ImGui::Button("OK")) {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}
```

---

## ドラッグ&ドロップ

```cpp
// ドラッグソース
if(ImGui::BeginDragDropSource()) {
    ImGui::SetDragDropPayload("MY_PAYLOAD", &data, sizeof(data));
    ImGui::Text("Dragging...");
    ImGui::EndDragDropSource();
}

// ドロップターゲット
if(ImGui::BeginDragDropTarget()) {
    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MY_PAYLOAD")) {
        MyData* data = (MyData*)payload->Data;
        // データ処理
    }
    ImGui::EndDragDropTarget();
}
```

---

## スタイル

```cpp
// スタイルの設定
ImGuiStyle& style = ImGui::GetStyle();

// 色の設定
style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.8f, 1.0f);

// サイズの設定
style.WindowPadding = ImVec2(10, 10);
style.FramePadding = ImVec2(5, 5);
style.ItemSpacing = ImVec2(5, 5);
style.ItemInnerSpacing = ImVec2(5, 5);

// スタイルのプッシュ・ポップ
ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
ImGui::Button("Red Button");
ImGui::PopStyleColor();

ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
ImGui::Button("Padded Button");
ImGui::PopStyleVar();

// 複数のスタイル変数
ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20, 20));
// ...
ImGui::PopStyleVar(2);

// プリセットスタイル
ImGui::StyleColorsDark();
ImGui::StyleColorsLight();
ImGui::StyleColorsClassic();
```

---

## 描画API

```cpp
// 描画リストの取得
ImDrawList* draw_list = ImGui::GetWindowDrawList();
ImDrawList* foreground_draw_list = ImGui::GetForegroundDrawList();
ImDrawList* background_draw_list = ImGui::GetBackgroundDrawList();

// 基本図形
draw_list->AddLine(ImVec2(0, 0), ImVec2(100, 100), IM_COL32(255, 0, 0, 255));
draw_list->AddRect(ImVec2(10, 10), ImVec2(100, 100), IM_COL32(0, 255, 0, 255));
draw_list->AddRectFilled(ImVec2(10, 10), ImVec2(100, 100), IM_COL32(0, 0, 255, 255));
draw_list->AddCircle(ImVec2(50, 50), 30, IM_COL32(255, 255, 0, 255));
draw_list->AddCircleFilled(ImVec2(50, 50), 30, IM_COL32(255, 255, 0, 255));
draw_list->AddTriangle(ImVec2(0, 0), ImVec2(50, 100), ImVec2(100, 0), IM_COL32(255, 0, 255, 255));
draw_list->AddTriangleFilled(ImVec2(0, 0), ImVec2(50, 100), ImVec2(100, 0), IM_COL32(255, 0, 255, 255));

// テキスト
draw_list->AddText(ImVec2(10, 10), IM_COL32(255, 255, 255, 255), "Text");

// 画像
draw_list->AddImage(texture_id, ImVec2(0, 0), ImVec2(100, 100));

// ベジェ曲線
draw_list->AddBezierCubic(ImVec2(0, 0), ImVec2(50, 100), ImVec2(100, 0), ImVec2(150, 100), 
    IM_COL32(255, 0, 0, 255), 2.0f, 10);

// ポリゴン
ImVec2 points[5] = { ... };
draw_list->AddPolyline(points, 5, IM_COL32(255, 0, 0, 255), ImDrawFlags_Closed, 2.0f);
draw_list->AddConvexPolyFilled(points, 5, IM_COL32(255, 0, 0, 255));
```

---

## ユーティリティ

```cpp
// ID管理
ImGuiID id = ImGui::GetID("unique_id");
ImGui::PushID("id");
// ...
ImGui::PopID();

// カーソル位置
ImVec2 cursor_pos = ImGui::GetCursorPos();
ImGui::SetCursorPos(ImVec2(10, 10));
ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();
ImVec2 cursor_start_pos = ImGui::GetCursorStartPos();

// コンテンツ領域
ImVec2 avail = ImGui::GetContentRegionAvail();
ImVec2 max = ImGui::GetContentRegionMax();

// アイテム状態
bool hovered = ImGui::IsItemHovered();
bool active = ImGui::IsItemActive();
bool focused = ImGui::IsItemFocused();
bool clicked = ImGui::IsItemClicked();
bool visible = ImGui::IsItemVisible();
bool edited = ImGui::IsItemEdited();
bool activated = ImGui::IsItemActivated();
bool deactivated = ImGui::IsItemDeactivated();
bool toggled_open = ImGui::IsItemToggledOpen();

// アイテム操作
ImGui::SetItemAllowOverlap();
ImGui::SetItemDefaultFocus();
ImGui::SetKeyboardFocusHere();
ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

// レイアウト
ImGui::SameLine();
ImGui::NewLine();
ImGui::Spacing();
ImGui::Dummy(ImVec2(10, 10));
ImGui::Indent();
ImGui::Unindent();
ImGui::BeginGroup();
ImGui::EndGroup();

// ツールチップ
ImGui::BeginTooltip();
ImGui::Text("Tooltip text");
ImGui::EndTooltip();

if(ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Tooltip");
}

// ヘルプマーカー
ImGui::Text("Help");
ImGui::SameLine();
ImGui::HelpMarker("Help text here");

// デモウィンドウ
ImGui::ShowDemoWindow(&show_demo);

// メトリクスウィンドウ
ImGui::ShowMetricsWindow(&show_metrics);
```

---

## よく使うパターン

### 基本的なウィンドウ

```cpp
void ShowMainWindow() {
    bool open = true;
    if(ImGui::Begin("Main Window", &open)) {
        ImGui::Text("Hello, world!");
        
        if(ImGui::Button("Click me")) {
            // ボタンクリック処理
        }
    }
    ImGui::End();
}
```

### 設定ウィンドウ

```cpp
void ShowSettingsWindow() {
    static float volume = 0.5f;
    static bool fullscreen = false;
    
    if(ImGui::Begin("Settings")) {
        ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f);
        ImGui::Checkbox("Fullscreen", &fullscreen);
        
        if(ImGui::Button("Apply")) {
            // 設定適用
        }
    }
    ImGui::End();
}
```

### デバッグ情報表示

```cpp
void ShowDebugInfo() {
    if(ImGui::Begin("Debug")) {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
        
        ImVec2 mouse_pos = ImGui::GetIO().MousePos;
        ImGui::Text("Mouse: (%.0f, %.0f)", mouse_pos.x, mouse_pos.y);
        
        if(ImGui::CollapsingHeader("Entities")) {
            // エンティティ情報
        }
    }
    ImGui::End();
}
```

### ツリービュー

```cpp
void ShowTreeView() {
    if(ImGui::TreeNode("Root")) {
        if(ImGui::TreeNode("Child 1")) {
            ImGui::Text("Content 1");
            ImGui::TreePop();
        }
        if(ImGui::TreeNode("Child 2")) {
            ImGui::Text("Content 2");
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}
```

### テーブルビュー

```cpp
void ShowTable() {
    if(ImGui::BeginTable("Data", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Value");
        ImGui::TableSetupColumn("Action");
        ImGui::TableHeadersRow();
        
        for(int i = 0; i < 10; i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Item %d", i);
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", values[i]);
            ImGui::TableNextColumn();
            if(ImGui::Button("Edit")) {
                // 編集処理
            }
        }
        ImGui::EndTable();
    }
}
```

### カスタム描画

```cpp
void ShowCustomDrawing() {
    if(ImGui::Begin("Custom Drawing")) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        
        // 背景
        draw_list->AddRectFilled(canvas_pos, 
            ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
            IM_COL32(50, 50, 50, 255));
        
        // カスタム図形
        draw_list->AddCircle(ImVec2(canvas_pos.x + 50, canvas_pos.y + 50), 
            30, IM_COL32(255, 0, 0, 255), 32, 2.0f);
    }
    ImGui::End();
}
```

### ドラッグ&ドロップ

```cpp
void ShowDragDrop() {
    // ドラッグソース
    if(ImGui::BeginListBox("Source")) {
        for(int i = 0; i < items.size(); i++) {
            if(ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("ITEM", &i, sizeof(int));
                ImGui::Text("Dragging item %d", i);
                ImGui::EndDragDropSource();
            }
            ImGui::Selectable(items[i].c_str());
        }
        ImGui::EndListBox();
    }
    
    // ドロップターゲット
    if(ImGui::BeginListBox("Target")) {
        if(ImGui::BeginDragDropTarget()) {
            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ITEM")) {
                int index = *(int*)payload->Data;
                // アイテム追加処理
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::EndListBox();
    }
}
```

---

## 検索インデックス

### カテゴリ別検索

- **ウィンドウ**: `Begin`, `End`, `BeginChild`, `SetNextWindowPos`, `IsWindowFocused`
- **テキスト**: `Text`, `TextColored`, `LabelText`, `BulletText`
- **ボタン**: `Button`, `SmallButton`, `ArrowButton`, `ImageButton`
- **入力**: `InputText`, `InputInt`, `InputFloat`, `InputTextMultiline`
- **スライダー**: `SliderInt`, `SliderFloat`, `DragInt`, `DragFloat`
- **チェック**: `Checkbox`, `RadioButton`
- **コンボ**: `BeginCombo`, `EndCombo`, `ListBox`
- **カラー**: `ColorEdit4`, `ColorPicker4`, `ColorButton`
- **ツリー**: `TreeNode`, `TreePop`, `CollapsingHeader`
- **テーブル**: `BeginTable`, `EndTable`, `TableNextRow`, `TableNextColumn`
- **メニュー**: `BeginMenuBar`, `BeginMenu`, `MenuItem`, `BeginPopup`
- **スタイル**: `PushStyleColor`, `PopStyleColor`, `PushStyleVar`, `PopStyleVar`
- **描画**: `GetWindowDrawList`, `AddLine`, `AddRect`, `AddCircle`
- **ユーティリティ**: `GetID`, `PushID`, `PopID`, `IsItemHovered`, `SameLine`

### よく使う関数

- **ウィンドウ作成**: `ImGui::Begin()`, `ImGui::End()`
- **テキスト表示**: `ImGui::Text()`, `ImGui::TextColored()`
- **ボタン**: `ImGui::Button()`
- **入力**: `ImGui::InputText()`, `ImGui::InputFloat()`
- **スライダー**: `ImGui::SliderFloat()`, `ImGui::DragFloat()`
- **チェックボックス**: `ImGui::Checkbox()`
- **ツリーノード**: `ImGui::TreeNode()`, `ImGui::TreePop()`
- **テーブル**: `ImGui::BeginTable()`, `ImGui::EndTable()`
- **スタイル**: `ImGui::PushStyleColor()`, `ImGui::PopStyleColor()`

---

**注意**: このチートシートは Dear ImGui の主要な機能をまとめています。詳細なパラメータや使用例については、[Dear ImGui公式ドキュメント](https://github.com/ocornut/imgui)や`ImGui::ShowDemoWindow()`を参照してください。

