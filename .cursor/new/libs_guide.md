# Simple-TDC-GameProject ライブラリ注意点ガイド

> **統合メモ**: 重複を避けるため概要は `libs-overview.md` を参照。詳細は本書（詳細版）と `gamedev_libs_guide.md` を併読。

> **AI向けドキュメント**: このプロジェクトで使用するライブラリの注意点と実装パターンをまとめたガイドです。
> **位置づけ**: 本書は「詳細版」です。手短な概要は `gamedev_libs_guide.md` を参照してください。

## 使用ライブラリ一覧

| ライブラリ | バージョン | 用途 |
|-----------|----------|------|
| **EnTT** | v3.12.2 | Entity Component System (ECS) ライブラリ |
| **nlohmann/json** | v3.11.2 | JSON パースと処理 |
| **Raylib** | 5.0 | ゲームグラフィックスとウィンドウ管理 |
| **raygui** | 4.0 | Raylib用の即座描画GUIライブラリ |
| **Dear ImGui** | v1.90.1 | ゲームエディター用GUI |
| **rlImGui** | 57efef0... | Dear ImGuiとRaylibの統合ライブラリ |

---

## 1. EnTT v3.12.2

### このプロジェクトでの使用パターン

```cpp
// ✅ 推奨: viewを使用したエンティティ取得
auto view = registry.view<Components::Position, Components::Velocity>();
for (auto entity : view) {
    auto& pos = view.get<Components::Position>(entity);
    auto& vel = view.get<Components::Velocity>(entity);
    // 処理
}

// ✅ 推奨: try_getでオプショナルなコンポーネント取得
if (auto* anim = registry.try_get<Components::Animation>(entity)) {
    // アニメーションがある場合のみ処理
}
```

### 重要な注意点

#### ❌ ネストされたグループは非対応

```cpp
// ❌ 非対応: グループのネスト
auto group = registry.group(get<A, B>, exclude<C, D>);
// group 内で別のグループを作成しないこと
```

#### ✅ v3.12.0 でのAPI変更（既に対応済み）

```cpp
// 旧API（使用禁止）          新API（使用すべき）
basic_sparse_set::emplace()   → push()
basic_sparse_set::get()       → value()
basic_sparse_set::sort()      → sort_as()
```

#### コンポーネント設計ルール

```cpp
// ✅ コンポーネントはPOD型（データのみ）
struct Position {
    float x;
    float y;
};

struct Velocity {
    float dx;
    float dy;
};

// ❌ コンポーネントにロジックを入れない
struct BadComponent {
    void Update(); // NG - ロジックはSystemに実装
};
```

---

## 2. nlohmann/json v3.11.2

### このプロジェクトでの使用パターン

```cpp
// ✅ 推奨: DataLoaderBaseを継承したローダーパターン
json LoadJsonFile(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            errorHandler_(filePath, "Failed to open file");
            return json();
        }
        
        json j;
        file >> j;
        return j;
    } catch (const json::parse_error& e) {
        errorHandler_(filePath, "JSON parse error: " + std::string(e.what()));
        return json();
    } catch (const std::exception& e) {
        errorHandler_(filePath, "Error: " + std::string(e.what()));
        return json();
    }
}
```

### 重要な注意点

#### ✅ 必須: try-catchでエラーハンドリング

```cpp
// ✅ 正しいパターン
try {
    json config = json::parse(file);
    // 処理
} catch (const json::parse_error& e) {
    std::cerr << "JSON parse error: " << e.what() << std::endl;
    // デフォルト値で継続
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}

// ❌ 危険: try-catchなし
json config = json::parse(file); // パース失敗時に例外が飛ぶ
```

#### ✅ 推奨: 安全な値取得

```cpp
// ✅ value()でデフォルト値を指定
std::string name = j.value("name", "unknown");
int health = j.value("health", 100);

// ✅ contains()で存在チェック
if (j.contains("optional_field")) {
    auto value = j["optional_field"];
}

// ❌ 危険: 直接アクセス（キーが無いと例外）
auto value = j["nonexistent_key"]; // 例外発生の可能性
```

#### 非推奨API（v4.0.0で削除予定）

```cpp
// ❌ 非推奨
iterator_wrapper()      // → items() を使用
```

---

## 3. Raylib 5.0

### このプロジェクトでの使用パターン

#### GameRendererクラスによるFHD固定レンダリング

```cpp
// このプロジェクトはFHD(1920x1080)固定レンダリングを採用
class GameRenderer {
    static constexpr int RENDER_WIDTH = 1920;
    static constexpr int RENDER_HEIGHT = 1080;
    
    void BeginRender() {
        BeginTextureMode(renderTarget_);
        ClearBackground(RAYWHITE);
    }
    
    void EndRender() {
        EndTextureMode();
        BeginDrawing();
        // スケーリング描画
        DrawTexturePro(renderTarget_.texture, ...);
        EndDrawing();
    }
};
```

### 重要な注意点

#### ✅ リソース管理の基本原則

```cpp
// ✅ ロードしたら必ずアンロード
Texture2D texture = LoadTexture("sprite.png");
// 使用
UnloadTexture(texture);  // 必須

// ✅ SoundManagerでの正しいパターン（本プロジェクト）
void Shutdown() {
    for (auto& inst : soundInstances_) {
        if (IsSoundReady(inst.sound)) {
            StopSound(inst.sound);
            UnloadSound(inst.sound);
        }
    }
    soundInstances_.clear();
    CloseAudioDevice();
}
```

#### ✅ オーディオ初期化順序

```cpp
// ✅ 正しい順序
InitAudioDevice();                    // 最初に初期化
Sound sound = LoadSound("jump.wav");  // その後にロード

// ❌ 間違い
Sound sound = LoadSound("jump.wav");  // 失敗
InitAudioDevice();
```

#### ✅ 音楽ストリーミングの更新

```cpp
// 毎フレーム必須
if (IsMusicStreamPlaying(bgm)) {
    UpdateMusicStream(bgm);  // バッファ管理
}
```

#### ✅ v5.0での関数名変更

```cpp
// 旧API（使用禁止）              新API（使用すべき）
GetMouseRay()                    → GetScreenToWorldRay()
WaitTime(500)                    → WaitTime(0.5f)  // ミリ秒→秒
GetDirectoryFiles()              → LoadDirectoryFiles()
ClearDirectoryFiles()            → UnloadDirectoryFiles()
```

#### ✅ リソース確認関数を使用

```cpp
// ✅ 描画前に確認
if (IsTextureReady(texture)) {
    DrawTexture(texture, 0, 0, WHITE);
}

if (IsSoundReady(sound)) {
    PlaySound(sound);
}

if (IsMusicReady(music)) {
    UpdateMusicStream(music);
}
```

---

## 4. raygui 4.0

### このプロジェクトでの使用状況

このプロジェクトでは主にDear ImGuiを使用しており、rayguiの使用は限定的です。

### 注意点

```cpp
// イミディエイトモードGUI
// 毎フレーム呼び出しが必要
if (GuiButton((Rectangle){10, 10, 100, 30}, "Click")) {
    // ボタンがクリックされた
}
```

---

## 5. Dear ImGui v1.90.1

### このプロジェクトでの使用パターン

```cpp
// EditorManagerでの使用例
void RenderEditor() {
    rlImGuiBegin();
    
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save All", "Ctrl+S")) {
                SaveAll();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    
    // 子ウィンドウ
    ImGui::BeginChild("CharacterList", ImVec2(200, 0), true);
    // リスト表示
    ImGui::EndChild();
    
    rlImGuiEnd();
}
```

### 重要な注意点

#### ✅ v1.90.0でのBeginChild()変更

```cpp
// ✅ 新API（このプロジェクトで使用）
ImGui::BeginChild("##id", size, ImGuiChildFlags_Border);

// 互換性: true == ImGuiChildFlags_Border なので既存コードも動作
ImGui::BeginChild("##id", size, true);  // 引き続き動作
```

#### ✅ スレッドセーフティ

```cpp
// Dear ImGuiは非スレッドセーフ
// 必ずメインスレッドでのみ使用
rlImGuiBegin();
  // ImGuiコード（メインスレッドのみ）
rlImGuiEnd();
```

---

## 6. rlImGui

### このプロジェクトでの使用パターン

```cpp
// 初期化（UnifiedGame等で実行）
rlImGuiSetup(true);  // ダークテーマ

// ゲームループ内
BeginDrawing();
{
    // Raylib描画（rlImGuiBegin前に実行）
    ClearBackground(RAYWHITE);
    DrawGame();
    
    // ImGuiフレーム
    rlImGuiBegin();
    {
        // エディターUI描画
        editorManager_->Render();
    }
    rlImGuiEnd();
}
EndDrawing();

// シャットダウン
rlImGuiShutdown();
```

### 重要な注意点

#### ✅ 描画順序の遵守

```cpp
BeginDrawing();
    // 1. Raylib描画（rlImGuiBegin前）
    ClearBackground(RAYWHITE);
    DrawRectangle(10, 10, 100, 100, RED);
    
    // 2. ImGuiフレーム
    rlImGuiBegin();
    {
        ImGui::ShowDemoWindow();
    }
    rlImGuiEnd();
    
    // 3. rlImGuiEnd後の描画は避ける
EndDrawing();
```

#### ✅ テクスチャ表示

```cpp
// BeginDrawing/EndDrawing内でのみ使用可能
rlImGuiBegin();
{
    rlImGuiImage(&texture);  // OK
    
    if (rlImGuiImageButton("MyButton", &texture)) {
        // クリック処理
    }
}
rlImGuiEnd();

// ❌ 外側では使用不可
rlImGuiImage(&texture);  // クラッシュ
```

---

## プロジェクト固有のパターン

### インクルード順序

```cpp
// 推奨順序
#include "raylib.h"
#include "raygui.h"
#include "rlImGui.hpp"
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <entt/entt.hpp>
```

### リソース管理パターン

```cpp
// このプロジェクトでは ResourceManager パターンを採用
class ResourceManager {
    std::unordered_map<std::string, Texture2D> textures_;
    
    Texture2D GetTexture(const std::string& path) {
        if (textures_.find(path) == textures_.end()) {
            textures_[path] = LoadTexture(path.c_str());
        }
        return textures_[path];
    }
    
    void Cleanup() {
        for (auto& [path, texture] : textures_) {
            UnloadTexture(texture);
        }
        textures_.clear();
    }
};
```

### ECSシステム実装パターン

```cpp
// システムはregistry参照を受け取る静的関数として実装
void RenderSystem::Render(entt::registry& registry) {
    auto view = registry.view<Components::Position, Components::Renderable>();
    for (auto entity : view) {
        // 描画処理
    }
}

void MovementSystem::Update(entt::registry& registry, float deltaTime) {
    auto view = registry.view<Components::Position, Components::Velocity>();
    for (auto entity : view) {
        // 移動処理
    }
}
```

---

## よくある問題と解決策

### 1. テクスチャ関連

| 問題 | 原因 | 解決策 |
|------|------|--------|
| テクスチャが表示されない | アンロード忘れ後の再利用 | IsTextureReady()で確認 |
| メモリリーク | UnloadTexture()忘れ | ResourceManagerで一元管理 |

### 2. JSON関連

| 問題 | 原因 | 解決策 |
|------|------|--------|
| クラッシュ | try-catch無しでパース | 必ずtry-catchで囲む |
| キー不在エラー | 直接アクセス | value()かcontains()使用 |

### 3. EnTT関連

| 問題 | 原因 | 解決策 |
|------|------|--------|
| コンポーネント取得失敗 | エンティティに無い | try_get()使用 |
| イテレーション中エラー | 削除操作 | 削除は別途collectして実行 |

### 4. ImGui関連

| 問題 | 原因 | 解決策 |
|------|------|--------|
| 表示されない | rlImGuiBegin/End外 | 必ずBegin/End内で描画 |
| 入力が効かない | フォーカス問題 | SetNextWindowFocus()使用 |

---

## チェックリスト

### コード作成時

- [ ] JSON読み込みはtry-catchで囲んでいるか
- [ ] コンポーネント取得にtry_get()を使用しているか
- [ ] Raylibリソースのアンロード処理があるか
- [ ] ImGui描画はrlImGuiBegin/End内か

### リソース管理

- [ ] InitAudioDevice()はサウンドロード前に呼んでいるか
- [ ] UpdateMusicStream()を毎フレーム呼んでいるか
- [ ] シャットダウン時に全リソースをアンロードしているか

---

**作成日**: 2024年12月
**対象**: AI開発エージェント向け
**プロジェクト**: Simple-TDC-GameProject
