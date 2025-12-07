# Simple-TDC-GameProject ���C�u�������ӓ_�K�C�h

> **��������**: �{���������Łi�ڍׁj�ł��B�T�v�� `libs-overview.md` ���Q�Ƃ��A�� `gamedev_libs_guide.md` �̓��_�C���N�g�ƂȂ�܂����B
>
> **AI�����h�L�������g**: ���̃v���W�F�N�g�Ŏg�p���郉�C�u�����̒��ӓ_�Ǝ����p�^�[�����܂Ƃ߂��K�C�h�ł��B
> **�ʒu�Â�**: �����ŏڍ׃K�C�h

## 使用ライブラリ一覧

| ライブラリ | バ�Eジョン | 用送E|
|-----------|----------|------|
| **EnTT** | v3.12.2 | Entity Component System (ECS) ライブラリ |
| **nlohmann/json** | v3.11.2 | JSON パ�Eスと処琁E|
| **Raylib** | 5.0 | ゲームグラフィチE��スとウィンドウ管琁E|
| **raygui** | 4.0 | Raylib用の即座描画GUIライブラリ |
| **Dear ImGui** | v1.90.1 | ゲームエチE��ター用GUI |
| **rlImGui** | 57efef0... | Dear ImGuiとRaylibの統合ライブラリ |

---

## 1. EnTT v3.12.2

### こ�Eプロジェクトでの使用パターン

```cpp
// ✁E推奨: viewを使用したエンチE��チE��取征E
auto view = registry.view<Components::Position, Components::Velocity>();
for (auto entity : view) {
    auto& pos = view.get<Components::Position>(entity);
    auto& vel = view.get<Components::Velocity>(entity);
    // 処琁E
}

// ✁E推奨: try_getでオプショナルなコンポ�Eネント取征E
if (auto* anim = registry.try_get<Components::Animation>(entity)) {
    // アニメーションがある場合�Eみ処琁E
}
```

### 重要な注意点

#### ❁Eネストされたグループ�E非対忁E

```cpp
// ❁E非対忁E グループ�EネスチE
auto group = registry.group(get<A, B>, exclude<C, D>);
// group 冁E��別のグループを作�EしなぁE��と
```

#### ✁Ev3.12.0 でのAPI変更�E�既に対応済み�E�E

```cpp
// 旧API�E�使用禁止�E�E         新API�E�使用すべき！E
basic_sparse_set::emplace()   ↁEpush()
basic_sparse_set::get()       ↁEvalue()
basic_sparse_set::sort()      ↁEsort_as()
```

#### コンポ�Eネント設計ルール

```cpp
// ✁Eコンポ�Eネント�EPOD型（データのみ�E�E
struct Position {
    float x;
    float y;
};

struct Velocity {
    float dx;
    float dy;
};

// ❁Eコンポ�EネントにロジチE��を�EれなぁE
struct BadComponent {
    void Update(); // NG - ロジチE��はSystemに実裁E
};
```

---

## 2. nlohmann/json v3.11.2

### こ�Eプロジェクトでの使用パターン

```cpp
// ✁E推奨: DataLoaderBaseを継承したローダーパターン
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

#### ✁E忁E��E try-catchでエラーハンドリング

```cpp
// ✁E正しいパターン
try {
    json config = json::parse(file);
    // 処琁E
} catch (const json::parse_error& e) {
    std::cerr << "JSON parse error: " << e.what() << std::endl;
    // チE��ォルト値で継綁E
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}

// ❁E危険: try-catchなぁE
json config = json::parse(file); // パ�Eス失敗時に例外が飛�E
```

#### ✁E推奨: 安�Eな値取征E

```cpp
// ✁Evalue()でチE��ォルト値を指宁E
std::string name = j.value("name", "unknown");
int health = j.value("health", 100);

// ✁Econtains()で存在チェチE��
if (j.contains("optional_field")) {
    auto value = j["optional_field"];
}

// ❁E危険: 直接アクセス�E�キーが無ぁE��例外！E
auto value = j["nonexistent_key"]; // 例外発生�E可能性
```

#### 非推奨API�E�E4.0.0で削除予定！E

```cpp
// ❁E非推奨
iterator_wrapper()      // ↁEitems() を使用
```

---

## 3. Raylib 5.0

### こ�Eプロジェクトでの使用パターン

#### GameRendererクラスによるFHD固定レンダリング

```cpp
// こ�Eプロジェクト�EFHD(1920x1080)固定レンダリングを採用
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

#### ✁Eリソース管琁E�E基本原則

```cpp
// ✁Eロードしたら忁E��アンローチE
Texture2D texture = LoadTexture("sprite.png");
// 使用
UnloadTexture(texture);  // 忁E��E

// ✁ESoundManagerでの正しいパターン�E�本プロジェクト！E
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

#### ✁EオーチE��オ初期化頁E��E

```cpp
// ✁E正しい頁E��E
InitAudioDevice();                    // 最初に初期匁E
Sound sound = LoadSound("jump.wav");  // そ�E後にローチE

// ❁E間違ぁE
Sound sound = LoadSound("jump.wav");  // 失敁E
InitAudioDevice();
```

#### ✁E音楽ストリーミングの更新

```cpp
// 毎フレーム忁E��E
if (IsMusicStreamPlaying(bgm)) {
    UpdateMusicStream(bgm);  // バッファ管琁E
}
```

#### ✁Ev5.0での関数名変更

```cpp
// 旧API�E�使用禁止�E�E             新API�E�使用すべき！E
GetMouseRay()                    ↁEGetScreenToWorldRay()
WaitTime(500)                    ↁEWaitTime(0.5f)  // ミリ秒�E私E
GetDirectoryFiles()              ↁELoadDirectoryFiles()
ClearDirectoryFiles()            ↁEUnloadDirectoryFiles()
```

#### ✁Eリソース確認関数を使用

```cpp
// ✁E描画前に確誁E
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

### こ�Eプロジェクトでの使用状況E

こ�Eプロジェクトでは主にDear ImGuiを使用しており、rayguiの使用は限定的です、E

### 注意点

```cpp
// イミディエイトモードGUI
// 毎フレーム呼び出しが忁E��E
if (GuiButton((Rectangle){10, 10, 100, 30}, "Click")) {
    // ボタンがクリチE��されぁE
}
```

---

## 5. Dear ImGui v1.90.1

### こ�Eプロジェクトでの使用パターン

```cpp
// EditorManagerでの使用侁E
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

#### ✁Ev1.90.0でのBeginChild()変更

```cpp
// ✁E新API�E�このプロジェクトで使用�E�E
ImGui::BeginChild("##id", size, ImGuiChildFlags_Border);

// 互換性: true == ImGuiChildFlags_Border なので既存コードも動佁E
ImGui::BeginChild("##id", size, true);  // 引き続き動佁E
```

#### ✁EスレチE��セーフティ

```cpp
// Dear ImGuiは非スレチE��セーチE
// 忁E��メインスレチE��でのみ使用
rlImGuiBegin();
  // ImGuiコード（メインスレチE��のみ�E�E
rlImGuiEnd();
```

---

## 6. rlImGui

### こ�Eプロジェクトでの使用パターン

```cpp
// 初期化！EnifiedGame等で実行！E
rlImGuiSetup(true);  // ダークチE�EチE

// ゲームループ�E
BeginDrawing();
{
    // Raylib描画�E�ElImGuiBegin前に実行！E
    ClearBackground(RAYWHITE);
    DrawGame();
    
    // ImGuiフレーム
    rlImGuiBegin();
    {
        // エチE��ターUI描画
        editorManager_->Render();
    }
    rlImGuiEnd();
}
EndDrawing();

// シャチE��ダウン
rlImGuiShutdown();
```

### 重要な注意点

#### ✁E描画頁E���E遵宁E

```cpp
BeginDrawing();
    // 1. Raylib描画�E�ElImGuiBegin前！E
    ClearBackground(RAYWHITE);
    DrawRectangle(10, 10, 100, 100, RED);
    
    // 2. ImGuiフレーム
    rlImGuiBegin();
    {
        ImGui::ShowDemoWindow();
    }
    rlImGuiEnd();
    
    // 3. rlImGuiEnd後�E描画は避ける
EndDrawing();
```

#### ✁EチE��スチャ表示

```cpp
// BeginDrawing/EndDrawing冁E��のみ使用可能
rlImGuiBegin();
{
    rlImGuiImage(&texture);  // OK
    
    if (rlImGuiImageButton("MyButton", &texture)) {
        // クリチE��処琁E
    }
}
rlImGuiEnd();

// ❁E外�Eでは使用不可
rlImGuiImage(&texture);  // クラチE��ュ
```

---

## プロジェクト固有�Eパターン

### インクルード頁E��E

```cpp
// 推奨頁E��E
#include "raylib.h"
#include "raygui.h"
#include "rlImGui.hpp"
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <entt/entt.hpp>
```

### リソース管琁E��ターン

```cpp
// こ�Eプロジェクトでは ResourceManager パターンを採用
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

### ECSシスチE��実裁E��ターン

```cpp
// シスチE��はregistry参�Eを受け取る静皁E��数として実裁E
void RenderSystem::Render(entt::registry& registry) {
    auto view = registry.view<Components::Position, Components::Renderable>();
    for (auto entity : view) {
        // 描画処琁E
    }
}

void MovementSystem::Update(entt::registry& registry, float deltaTime) {
    auto view = registry.view<Components::Position, Components::Velocity>();
    for (auto entity : view) {
        // 移動�E琁E
    }
}
```

---

## よくある問題と解決筁E

### 1. チE��スチャ関連

| 問顁E| 原因 | 解決筁E|
|------|------|--------|
| チE��スチャが表示されなぁE| アンロード忘れ後�E再利用 | IsTextureReady()で確誁E|
| メモリリーク | UnloadTexture()忘れ | ResourceManagerで一允E��琁E|

### 2. JSON関連

| 問顁E| 原因 | 解決筁E|
|------|------|--------|
| クラチE��ュ | try-catch無しでパ�Eス | 忁E��try-catchで囲む |
| キー不在エラー | 直接アクセス | value()かcontains()使用 |

### 3. EnTT関連

| 問顁E| 原因 | 解決筁E|
|------|------|--------|
| コンポ�Eネント取得失敁E| エンチE��チE��に無ぁE| try_get()使用 |
| イチE��ーション中エラー | 削除操佁E| 削除は別途collectして実衁E|

### 4. ImGui関連

| 問顁E| 原因 | 解決筁E|
|------|------|--------|
| 表示されなぁE| rlImGuiBegin/End夁E| 忁E��Begin/End冁E��描画 |
| 入力が効かなぁE| フォーカス問顁E| SetNextWindowFocus()使用 |

---

## チェチE��リスチE

### コード作�E晁E

- [ ] JSON読み込みはtry-catchで囲んでぁE��ぁE
- [ ] コンポ�Eネント取得にtry_get()を使用してぁE��ぁE
- [ ] Raylibリソースのアンロード�E琁E��あるぁE
- [ ] ImGui描画はrlImGuiBegin/End冁E��

### リソース管琁E

- [ ] InitAudioDevice()はサウンドロード前に呼んでぁE��ぁE
- [ ] UpdateMusicStream()を毎フレーム呼んでぁE��ぁE
- [ ] シャチE��ダウン時に全リソースをアンロードしてぁE��ぁE

---

**作�E日**: 2024年12朁E
**対象**: AI開発エージェント向ぁE
**プロジェクチE*: Simple-TDC-GameProject
