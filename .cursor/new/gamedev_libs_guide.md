# ゲーム開発ライブラリ統合 開発上の注意点

> **統合メモ**: ライブラリ概要の集約版は `libs-overview.md`。本書は概要版、詳細は `libs_guide.md` を参照。
>
> 本書は「概要版」です。詳細な実装パターンやコード例は `libs_guide.md`（詳細版）を参照してください。
>
> ## 概要
>
> このドキュメントは、ゲーム開発プロジェクトで使用される6つの主要ライブラリ・フレームワークの開発上の注意点をまとめています。

| ライブラリ | バージョン | 用途 |
|-----------|----------|------|
| **EnTT** | v3.12.2 | Entity Component System (ECS) ライブラリ |
| **nlohmann/json** | v3.11.2 | JSON パースと処理 |
| **Raylib** | 5.0 | ゲームグラフィックスとウィンドウ管理 |
| **raygui** | 4.0 | Raylib用の即座描画GUIライブラリ |
| **Dear ImGui** | v1.90.1 | ゲームエディター用GUI |
| **rlImGui** | 57efef0... | Dear ImGuiとRaylibの統合ライブラリ |

---

## 1. EnTT v3.12.2 (Entity Component System)

### 主要な変更点

EnTT v3.12.2 は v3.12.0 からのマイナーアップデートで、主に後方互換性の改善が含まれています。

### 重要な注意点

#### 1.1 関数リネーミング (v3.12.0 で実施)

```cpp
// 旧 API
basic_sparse_set<...>::emplace()      // → push()
basic_sparse_set<...>::insert()       // → push()
basic_sparse_set<...>::get()          // → value()
basic_sparse_set<...>::sort()         // → sort_as()
basic_group<...>::sort()              // → sort_as()
```

#### 1.2 型リネーミング

```cpp
// 旧 API
basic_registry<...>::base_type        // → common_type
basic_group<...>::base_type           // → common_type
runtime_view<...>::base_type          // → common_type
basic_view<...>::base_type            // → common_type
```

#### 1.3 非推奨関数 (削除予定)

以下の関数は v3.12.0 で削除されました：

- `ignore_as_empty_v` → `component_traits<...>::page_size` を使用
- `entt_traits::reserved` → `entity_mask` と `version_mask` を組み合わせて使用
- `basic_sparse_set<...>::move_element()` → `swap_or_move()` を使用

#### 1.4 API 変更

```cpp
// v3.12.0 での重要な変更
basic_view<...>::storage<...>()      // 参照 → ポインタを返すように変更
basic_group<...>::storage<...>()     // 参照 → ポインタを返すように変更
basic_view<...>::refresh()           // 非const関数に変更
basic_view<...>::use<T>()            // 非const関数に変更
```

#### 1.5 グループのネストは非対応

**重要**: v3.12.0 以降、ネストされたグループはサポートされなくなりました。

```cpp
// 非対応
auto group = registry.group(get<A, B>, exclude<C, D>);
// グループ内に別のグループを作成しないこと
```

#### 1.6 ストレージ関連の改善

- 実体ストレージが registry に統合 (`registry.storage<Entity>()`)
- 削除ポリシー: `swap_and_pop` と `in_place_pop` が統合
- アロケーター対応の改善

#### 1.7 メタシステム (Meta)

```cpp
// 非推奨 (削除予定)
meta_any_policy           // → any_policy を使用
meta_any::policy()        // → .base().policy() を使用
meta_any::data()          // → .base().data() を使用
meta<T>()                 // → meta_factory<T> を直接使用
```

---

## 2. nlohmann/json v3.11.2

### 概要

JSON パーサ・シリアライザ。v3.11.2 は v3.11.1 のバグ修正リリースで、全ての変更が後方互換です。

### 主要な変更点

#### 2.1 構造体のリネーミング

**v3.11.2 では特に重要な変更はありませんが、v3.11.0 での変更を参照してください**

#### 2.2 新機能 (v3.11.0)

- **文字列ビューサポート**: オブジェクトキーに `std::string_view` を使用可能
- **BJData フォーマット**: BSON, CBOR, MessagePack, UBJSON に加え、Binary JData フォーマット対応
- **デフォルト値サポート**: `NLOHMANN_DEFINE_TYPE_INTRUSIVE` と `NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE` でデフォルト値を指定可能

```cpp
// オブジェクトキーに string_view を使用可能 (v3.11.0+)
json j = /* ... */;
auto value = j.at(std::string_view("key"));
```

#### 2.3 非推奨関数

以下の関数は v4.0.0 で削除予定です：

- `iterator_wrapper()` → `items()` を使用
- `friend operator<<` / `friend operator>>` → 標準ストリーム演算子を使用
- JSON Pointer を文字列と直接比較 → JSON Pointer に変換後に比較

#### 2.4 コンパイル設定

```cpp
// JSON グローバルユーザー定義リテラル
#define JSON_USE_GLOBAL_UDLS 1  // デフォルト: on

// JSON 複数ヘッダーモード
#define JSON_MultipleHeaders ON  // CMake オプション
```

#### 2.5 バイナリフォーマット

```cpp
// CBOR, MessagePack, UBJSON, BSON, BJData に対応
json j = json::from_cbor(buffer);
json j = json::from_msgpack(buffer);
json j = json::from_ubjson(buffer);
json j = json::from_bson(buffer);
json j = json::from_bjdata(buffer);  // v3.11.0+
```

#### 2.6 メモリ管理

```cpp
// カスタムアロケーター対応
using custom_json = nlohmann::basic_json<
    nlohmann::ordered_map,
    std::vector,
    std::string,
    bool, std::int64_t, std::uint64_t, double,
    std::allocator<void>  // カスタムアロケーター
>;
```

---

## 3. Raylib 5.0

### 重要な変更

Raylib 5.0 は過去10年間で最大の再設計です。詳細は Raylib 5.0 専用ドキュメントを参照。

### クイック参照

#### 3.1 プラットフォーム分割

**rcore モジュールの分割**により、プラットフォームの保守性が大幅に向上しました。

- `rcore_desktop_glfw.c` (デフォルト)
- `rcore_desktop_sdl.c` (オプション)
- `rcore_web.c`
- `rcore_android.c`
- `rcore_drm.c` (Linux DRM)
- `rcore_switch.cpp` (Nintendo Switch)

#### 3.2 函数名の変更

```cpp
// Raylib 4.x → 5.0
GetMouseRay()              // → GetScreenToWorldRay()
GetViewRay()               // → GetScreenToWorldRayEx()
GetDirectoryFiles()        // → LoadDirectoryFiles()
ClearDirectoryFiles()      // → UnloadDirectoryFiles()
WaitTime(500)              // → WaitTime(0.5f)  // ミリ秒 → 秒
```

#### 3.3 新しいプラットフォームバックエンド

- **SDL2 バックエンド**: GLFW の代替として利用可能
- **Nintendo Switch**: わずか2日で実装可能な新しい設計

#### 3.4 rcamera モジュール

グローバル状態を削除し、より単純で柔軟な設計に変更。

```cpp
// 新しい API
UpdateCameraPro()         // ユーザー入力に依存しない
GetScreenToWorldRay()     // 改名
```

---

## 4. raygui 4.0

### 概要

Raylib用のイミディエイトモード GUI ライブラリ。v4.0 は大幅な再設計を実施。

### 主要な変更

#### 4.1 新規コントロール

```c
GuiToggleSlider()     // 新規
GuiColorPickerHSV()   // 新規 (RGB に加えて HSV もサポート)
```

#### 4.2 スタイルシステムの改善

```c
// スタイル設定が強化
GuiLoadStyle(const char* fileName)
GuiLoadStyleDefault()
GuiSetStyle(int control, int property, int value)
```

#### 4.3 アイコンサポート

raygui 4.0 ではアイコンが 1-bit パック として組み込まれています。

#### 4.4 スタンドアロンモード

Raylib に依存しない使用も可能（入出力関数を提供すれば利用可能）。

#### 4.5 非推奨と削除

- いくつかの古い API が廃止されました
- 公式ドキュメントで最新の API を確認してください

---

## 5. Dear ImGui v1.90.1

### 概要

強力な C++ GUI ライブラリ。エディターツール開発に最適。

### 主要な変更 (v1.90.0)

#### 5.1 BeginChild() のシグネチャ変更

```cpp
// 旧
BeginChild("##id", size, border);

// 新 (v1.90.0+)
BeginChild("##id", size, ImGuiChildFlags_Border);
```

**互換性**: `ImGuiChildFlags_Border == true` が保証されているため、既存コードは動作します。

#### 5.2 削除された API

- `io.MetricsActiveAllocations` (内部的に計算可能)

#### 5.3 新規フラグ

```cpp
ImGuiChildFlags_ResizeX       // 右辺からリサイズ可能
ImGuiChildFlags_ResizeY       // 下辺からリサイズ可能
ImGuiChildFlags_FrameStyle    // BeginChildFrame() の代替
```

#### 5.4 設定オプション

```cpp
ImGuiConfigFlags_DpiEnableScaleFonts
ImGuiConfigFlags_DpiEnableScaleViewports
io.ConfigDpiScaleFonts
style.FontScaleMain
style.FontScaleDpi
```

#### 5.5 レイアウトと描画

```cpp
ImDrawList::AddEllipse()              // 新規
ImDrawList::AddEllipseFilled()        // 新規
ImDrawList::PathEllipticalArcTo()    // 新規
```

#### 5.6 コンテナ操作

```cpp
ImGui::BeginMultiSelect()             // マルチセレクト対応
ImGui::EndMultiSelect()
ImGui::IsItemHovered()                // 改善
```

#### 5.7 入力テキスト

```cpp
ImGui::InputTextMultiline()
  ImGuiInputTextFlags_WordWrap       // v1.92.0 で追加 (ベータ)
```

#### 5.8 タブバー

```cpp
ImGui::TabBar()
  ImGuiTabBarFlags_FittingPolicyShrink
  ImGuiTabBarFlags_FittingPolicyScroll
```

#### 5.9 ナビゲーション

キーボード・ゲームパッド ナビゲーションが強化されました。

```cpp
ImGuiKey_NavGamepadMenu              // メニューレイヤー切り替え
ImGuiKey_AppBack                     // アプリケーション戻る
ImGuiKey_AppForward                  // アプリケーション進む
```

#### 5.10 デバッグツール

```cpp
// ID衝突検出
#define IMGUI_DEBUG_HIGHLIGHT_ALL_ID_CONFLICTS

// メトリクス
ImGui::ShowMetricsWindow()
```

---

## 6. rlImGui (Dear ImGui + Raylib 統合)

### 概要

Dear ImGui を Raylib に統合するための公式ブリッジライブラリ。現在のコミット: `57efef0...`

### セットアップ

#### 6.1 初期化

```cpp
#include "rlImGui.hpp"

// メインループの前に初期化
rlImGuiSetup(true);  // true = ダークテーマ, false = ライトテーマ

// メインループ
while (!WindowShouldClose()) {
    BeginDrawing();
    {
        // Raylib 描画コード
        ClearBackground(RAYWHITE);
        
        // ImGui フレーム開始
        rlImGuiBegin();
        {
            // ImGui コード
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Exit")) {
                        /* ... */
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
            
            ImGui::ShowDemoWindow();
        }
        // ImGui フレーム終了
        rlImGuiEnd();
    }
    EndDrawing();
}

// シャットダウン
rlImGuiShutdown();
```

#### 6.2 主要な関数

```cpp
void rlImGuiSetup(bool dark);                    // 初期化
void rlImGuiBegin();                             // フレーム開始
void rlImGuiEnd();                               // フレーム終了
void rlImGuiShutdown();                          // シャットダウン
void rlImGuiReloadFonts();                       // フォント再読み込み

// 画像操作
void rlImGuiImage(const Texture* image);
bool rlImGuiImageButton(const char* name, const Texture* image);
void rlImGuiImageRect(const Texture* image, int destWidth, int destHeight, Rectangle sourceRect);
void rlImGuiImageRenderTexture(const RenderTexture* image);
```

#### 6.3 コンテキスト管理

```cpp
// 複数のコンテキストサポート
ImGui::SetCurrentContext(GlobalContext);

// フォントの動的変更
rlImGuiReloadFonts();
```

#### 6.4 Raylib テクスチャ統合

```cpp
// Raylib Texture を ImGui で表示
Texture rayTexture = LoadTexture("image.png");
rlImGuiImage(&rayTexture);

// ImGui ボタンとして表示
if (rlImGuiImageButton("MyButton", &rayTexture)) {
    // クリック処理
}

// RenderTexture のサポート
RenderTexture rt = LoadRenderTexture(800, 600);
rlImGuiImageRenderTexture(&rt);
```

### 注意点

#### 6.5 入力処理

- rlImGui は Raylib の入力システムを自動的に処理
- ImGui がフォーカスを持つ場合、Raylib への入力は制限される可能性あり

#### 6.6 描画順序

```cpp
BeginDrawing();
    // Raylib 描画は rlImGuiBegin() の前に実施
    ClearBackground(RAYWHITE);
    DrawRectangle(10, 10, 100, 100, RED);
    
    rlImGuiBegin();
    {
        // ImGui コード
    }
    rlImGuiEnd();
    // rlImGuiEnd() 後は Raylib の描画は避ける
EndDrawing();
```

#### 6.7 Dear ImGui バージョン互換性

- rlImGui は Dear ImGui v1.90.1 以降に対応
- Raylib 5.0 以上推奨

#### 6.8 パフォーマンス考慮

```cpp
// イミディエイトモード GUI の特性
// - 毎フレーム全ウィジェットを再構築
// - 大量のウィジェット使用時は最適化が重要

// CPU 負荷軽減のコツ
- BeginChild() で描画領域を制限
- ImGuiListClipper で大量リスト表示時に最適化
- ImGui の IsItemHovered() などで無駄な処理を削減
```

---

## 7. 統合開発時の注意点

### 7.1 依存関係の順序

```cpp
// インクルード順序の推奨
#include "raylib.h"
#include "raygui.h"
#include "rlImGui.hpp"
#include "imgui.h"
#include "nlohmann/json.hpp"
#include "entt/entt.hpp"
```

### 7.2 メモリ管理

**EnTT**: アロケーター対応

```cpp
using allocator_type = std::allocator<void>;
using registry_type = entt::basic_registry<entt::entity, allocator_type>;
registry_type registry(allocator_type{});
```

**nlohmann/json**: カスタムアロケーター対応

```cpp
using custom_json = nlohmann::basic_json<...>;
```

**Raylib**: 自動メモリ管理

```cpp
Texture tex = LoadTexture("image.png");
// 明示的に解放
UnloadTexture(tex);
```

### 7.3 スレッドセーフティ

- **Dear ImGui**: 非スレッドセーフ（メインスレッドのみ）
- **rlImGui**: BeginDrawing() / EndDrawing() 内のみ使用
- **EnTT**: 複数スレッド対応（適切なロック機構が必要）
- **nlohmann/json**: スレッドセーフ（個別のオブジェクトに対して）

### 7.4 コンパイル設定

```cmake
# CMakeLists.txt 推奨設定
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ライブラリ有効化フラグ
add_definitions(-DRAYGUI_IMPLEMENTATION)
add_definitions(-DRLIMGUI_IMPLEMENTATION)
```

### 7.5 パフォーマンスプロファイリング

```cpp
// Dear ImGui パフォーマンス測定
ImGui::ShowMetricsWindow();

// Raylib フレームレート確認
int fps = GetFPS();
DrawFPS(10, 10);
```

---

## 8. よくある問題と解決策

### 8.1 EnTT グループ使用時のクラッシュ

**問題**: ネストされたグループを使用するとクラッシュ

**解決策**:

```cpp
// 非対応
auto group = registry.group(get<A>());
// group 内で別のグループを作成しない

// 対応: ビューの組み合わせを使用
auto view = registry.view<A>();
```

### 8.2 JSON パース時のメモリリーク

**問題**: `GetCodepoints()` 使用時にメモリが解放されない

**解決策**:

```cpp
// 旧 (v3.10.x 以前)
int* codepoints = GetCodepoints(text, &count);
// → メモリリーク

// 新 (v3.11.0+)
int* codepoints = LoadCodepoints(text, &count);
// 使用後に明示的に解放
UnloadCodepoints(codepoints);
```

### 8.3 Raylib 5.0 での WaitTime() エラー

**問題**: `WaitTime(500)` でコンパイルエラー

**解決策**:

```cpp
// 旧: ミリ秒で指定
WaitTime(500);

// 新: 秒で指定
WaitTime(0.5f);
```

### 8.4 Dear ImGui テクスチャが表示されない

**問題**: rlImGui で Raylib テクスチャが表示されない

**解決策**:

```cpp
// BeginDrawing() / EndDrawing() 内のみ使用
BeginDrawing();
{
    rlImGuiBegin();
    {
        rlImGuiImage(&texture);  // OK
    }
    rlImGuiEnd();
}
EndDrawing();

// 外側では使用不可
rlImGuiImage(&texture);  // NG - エラーまたはクラッシュ
```

### 8.5 ImGui のウィンドウが応答しない

**問題**: Dear ImGui ウィンドウがキーボード入力に応答しない

**解決策**:

```cpp
// rlImGui が入力を正しく処理しているか確認
rlImGuiBegin();
{
    // ImGui::IsItemActive() で確認
    static bool show_window = true;
    ImGui::SetNextWindowFocus();  // フォーキュー
    ImGui::Begin("Window", &show_window);
    {
        ImGui::Text("Some content");
    }
    ImGui::End();
}
rlImGuiEnd();
```

---

## 9. リファレンスとドキュメント

### 公式リソース

- **EnTT**: <https://github.com/skypjack/entt>
- **nlohmann/json**: <https://github.com/nlohmann/json>
- **Raylib**: <https://www.raylib.com>
- **raygui**: <https://github.com/raysan5/raygui>
- **Dear ImGui**: <https://github.com/ocornut/imgui>
- **rlImGui**: <https://github.com/raylib-extras/rlImGui>

### チュートリアルと例

- Raylib 公式例: <https://www.raylib.com/examples.html>
- Dear ImGui デモ: `ImGui::ShowDemoWindow()`
- rlImGui 統合例: raylib-extras リポジトリ

---

## 10. 更新対応のチェックリスト

Raylib 5.0 へのアップグレード時:

- [ ] `GetMouseRay()` → `GetScreenToWorldRay()` に変更
- [ ] `WaitTime()` の引数を秒単位に変更
- [ ] ファイル読み込み関数を `Load*` 形式に変更
- [ ] グリフ関連を `CharInfo` → `GlyphInfo` に更新
- [ ] プラットフォーム設定を確認

EnTT v3.12.0 へのアップグレード時:

- [ ] `emplace()` → `push()` に変更
- [ ] `get()` → `value()` に変更
- [ ] `sort()` → `sort_as()` に変更
- [ ] グループの使用方法を確認
- [ ] メタシステム API を確認

Dear ImGui v1.90.0 へのアップグレード時:

- [ ] `BeginChild()` のフラグ形式に変更
- [ ] マルチセレクト API を確認
- [ ] レイアウト関数を確認

---

**作成日**: 2024年12月
**バージョン**: 1.0
**対象**: ゲーム開発チーム向け
