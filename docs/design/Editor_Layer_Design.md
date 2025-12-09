# Phase 5: Editor Layer 基本設計 - ImGui統合構成（統合最適版）

**プロジェクト**: SimpleTDCGame_NewArch  
**バージョン**: 1.0.0（Editor Layer基本設計版）  
**作成日**: 2025-12-08 / 08:07 JST  
**目的**: ゲーム実装と並行してデータ編集を効率化するEditor Layerの仕様確定

---

## 📑 目次

1. [Editor Layer 全体概要](#editor-layer-全体概要)
2. [EditorApp 設計（エディタメインアプリ）](#editorapp-設計エディタメインアプリ)
3. [Editor Windows（エディタウィンドウ）](#editor-windowsエディタウィンドウ)
4. [ImGui統合（UI描画）](#imgui統合ui描画)
5. [DataBinding & Serialization](#databinding--serialization)
6. [ホットリロード & HotWatch](#ホットリロード--hotwatch)
7. [Validation & Error Handling](#validation--error-handling)
8. [ワークフロー & ユースケース](#ワークフロー--ユースケース)

---

## Editor Layer 全体概要

### アーキテクチャ図

```
Editor Executable (SimpleTDCEditor)
┌────────────────────────────────────────────────────┐
│                                                     │
│  ┌─ EditorApp（メインアプリケーション） ───────────┐│
│  │  - Window管理（Dear ImGui）                     ││
│  │  - MainLoop + Update/Render                     ││
│  │  - Workspace管理                                ││
│  └──────────────────────────────────────────────────┘│
│                                                     │
│  ┌─ Editor Windows（複数ウィンドウ） ──────────────┐│
│  │  ✅ EntityEditorWindow（キャラクター編集）      ││
│  │  ✅ SkillEditorWindow（スキル編集）            ││
│  │  ✅ StageEditorWindow（ステージ編集）          ││
│  │  ✅ EffectEditorWindow（エフェクト編集）       ││
│  │  ✅ DeckEditorWindow（デッキ編集）             ││
│  │  ✅ HierarchyWindow（オブジェクト一覧）        ││
│  │  ✅ InspectorWindow（プロパティ編集）          ││
│  │  ✅ ConsoleWindow（ログ出力）                  ││
│  │  ✅ AssetBrowserWindow（ファイル管理）         ││
│  └──────────────────────────────────────────────────┘│
│                                                     │
│  ┌─ Services（機能提供層） ──────────────────────┐│
│  │  - WorkspaceManager（ワークスペース管理）      ││
│  │  - DataBindingService（双方向バインディング）  ││
│  │  - ValidationService（検証・エラーチェック）   ││
│  │  - ProjectManager（プロジェクト管理）          ││
│  │  - HotWatchService（ファイル監視）            ││
│  └──────────────────────────────────────────────────┘│
│                                                     │
│  ┌─ Shared Layer（共有層） ──────────────────────┐│
│  │  - GameContext, FileWatcher, EventSystem       ││
│  │  - DefinitionRegistry（データ定義レジストリ）  ││
│  │  - Loaders & Validators                        ││
│  └──────────────────────────────────────────────────┘│
│                                                     │
└────────────────────────────────────────────────────┘
         ↓
    Dear ImGui Renderer
         ↓
    Raylib/GLFW Window
```

### 設計方針

```yaml
EditorApp特徴:
  ✅ ImGui統合（軽量・高速UI）
  ✅ マルチウィンドウ（プラグイン型）
  ✅ HotReload対応（ファイル変更自動反映）
  ✅ ゲーム実行中に編集可能
  ✅ JSON直接編集対応

Services特徴:
  ✅ WorkspaceManager（複数プロジェクト管理）
  ✅ DataBindingService（ImGui ↔ JSON双方向）
  ✅ ValidationService（スキーマ・参照チェック）
  ✅ HotWatchService（ファイル変更監視）
  ✅ ProjectManager（Save/Load/Export）

EditorWindow特徴:
  ✅ IEditorWindow統一インターフェース
  ✅ 独立したUI・状態管理
  ✅ プリセット・テンプレート機能
  ✅ Undo/Redo対応（オプション）
```

---

## EditorApp 設計（エディタメインアプリ）

### EditorApp インターフェース

```cpp
// editor/include/Editor/Application/EditorApp.h
namespace Editor::Application {

class EditorApp {
private:
  // ===== ImGui関連 =====
  int screen_width_ = 1600;
  int screen_height_ = 900;
  bool is_running_ = true;
  
  // ===== ウィンドウ管理 =====
  std::vector<std::unique_ptr<Editor::Windows::IEditorWindow>> windows_;
  std::unordered_map<std::string, Editor::Windows::IEditorWindow*> window_map_;
  
  // ===== サービス層 =====
  std::unique_ptr<Editor::Services::WorkspaceManager> workspace_mgr_;
  std::unique_ptr<Editor::Services::DataBindingService> data_binding_;
  std::unique_ptr<Editor::Services::ValidationService> validation_;
  std::unique_ptr<Editor::Services::HotWatchService> hot_watch_;
  std::unique_ptr<Editor::Services::ProjectManager> project_mgr_;
  
  // ===== 共有層 =====
  std::unique_ptr<Shared::Core::GameContext> context_;
  std::shared_ptr<Shared::Data::DefinitionRegistry> definitions_;
  
  // ===== 状態 =====
  std::string current_project_path_;
  bool unsaved_changes_ = false;

public:
  EditorApp(int width = 1600, int height = 900);
  ~EditorApp();
  
  // ===== メインループ =====
  bool Initialize();
  void Run();
  void Shutdown();

private:
  // ===== 初期化 =====
  void InitializeImGui();
  void InitializeServices();
  void InitializeEditorWindows();
  void LoadConfiguration();
  
  // ===== フレーム処理 =====
  void UpdateFrame(float delta_time);
  void RenderFrame();
  void UpdateImGuiFrame();
  
  // ===== ウィンドウ管理 =====
  void RegisterWindow(std::unique_ptr<Editor::Windows::IEditorWindow> window);
  void DrawMenuBar();
  void DrawDockSpace();
  
  // ===== ファイル操作 =====
  void OnFileSave();
  void OnFileLoad();
  void OnFileNew();
  void OnFileExit();
};

} // namespace Editor::Application
```

### EditorApp 主要フロー

```cpp
// editor/src/main_editor.cpp
#include "Editor/Application/EditorApp.h"
#include "raylib.h"

int main() {
  auto app = std::make_unique<Editor::Application::EditorApp>(1600, 900);
  
  if (!app->Initialize()) {
    std::cerr << "Failed to initialize EditorApp" << std::endl;
    return 1;
  }
  
  app->Run();
  app->Shutdown();
  
  return 0;
}
```

```cpp
// editor/src/Editor/Application/EditorApp.cpp
namespace Editor::Application {

bool EditorApp::Initialize() {
  // 1. Raylib初期化
  InitWindow(screen_width_, screen_height_, "SimpleTDCGame Editor");
  SetTargetFPS(60);
  
  // 2. ImGui初期化
  InitializeImGui();
  
  // 3. GameContext初期化
  context_ = std::make_unique<Shared::Core::GameContext>();
  if (!context_->Initialize("config.json")) {
    return false;
  }
  
  // 4. DefinitionRegistry初期化
  definitions_ = std::make_shared<Shared::Data::DefinitionRegistry>();
  
  // 5. Services初期化
  InitializeServices();
  
  // 6. EditorWindows初期化
  InitializeEditorWindows();
  
  // 7. コンフィグロード
  LoadConfiguration();
  
  return true;
}

void EditorApp::Run() {
  while (is_running_ && !WindowShouldClose()) {
    float delta_time = GetFrameTime();
    
    UpdateFrame(delta_time);
    RenderFrame();
  }
}

void EditorApp::UpdateFrame(float delta_time) {
  // GameContext更新（FileWatcher含む）
  context_->GetFileWatcher().CheckChanges();
  
  // ImGuiフレーム開始
  ImGui_ImplRaylib_NewFrame();
  ImGui::NewFrame();
  
  // メニューバー
  DrawMenuBar();
  
  // DockSpace（レイアウト）
  DrawDockSpace();
  
  // 各EditorWindowの更新
  for (auto& window : windows_) {
    if (window->IsOpen()) {
      window->OnDrawUI();
      window->OnUpdate(delta_time);
    }
  }
  
  // ImGui描画準備完了
  ImGui::Render();
}

void EditorApp::RenderFrame() {
  BeginDrawing();
  ClearBackground(RAYWHITE);
  
  // ImGui描画
  ImGui_ImplRaylib_RenderDrawData(ImGui::GetDrawData());
  
  EndDrawing();
}

} // namespace Editor::Application
```

---

## Editor Windows（エディタウィンドウ）

### IEditorWindow インターフェース

```cpp
// editor/include/Editor/Windows/IEditorWindow.h
namespace Editor::Windows {

class IEditorWindow {
public:
  virtual ~IEditorWindow() = default;
  
  // ===== ライフサイクル =====
  virtual void Initialize(Shared::Core::GameContext& context) = 0;
  virtual void Shutdown() = 0;
  
  // ===== フレーム処理 =====
  virtual void OnDrawUI() = 0;        // ImGui UI描画
  virtual void OnUpdate(float delta_time) = 0;
  
  // ===== ウィンドウ状態 =====
  virtual std::string GetWindowTitle() const = 0;
  virtual bool IsOpen() const = 0;
  virtual void SetOpen(bool open) = 0;
  
  // ===== データアクセス =====
  virtual std::string GetWindowId() const = 0;
  virtual void SaveState(nlohmann::json& state) const = 0;
  virtual void LoadState(const nlohmann::json& state) = 0;
};

} // namespace Editor::Windows
```

### EditorWindow実装例：EntityEditorWindow

```cpp
// editor/include/Editor/Windows/EntityEditorWindow.h
namespace Editor::Windows {

class EntityEditorWindow : public IEditorWindow {
private:
  // ===== 依存 =====
  Shared::Core::GameContext* context_;
  Shared::Data::DefinitionRegistry* definitions_;
  Editor::Services::DataBindingService* data_binding_;
  
  // ===== UI状態 =====
  bool is_open_ = true;
  char search_filter_[256] = {0};
  
  // ===== 選択状態 =====
  std::string selected_entity_id_;
  int selected_entity_index_ = -1;
  std::vector<std::string> entity_list_;
  
  // ===== 編集状態（ImGui用） =====
  struct EditState {
    std::string id;
    std::string name;
    std::string description;
    int rarity = 1;
    std::string type;
    bool is_enemy = false;
    int cost = 0;
    float cooldown = 1.0f;
    
    // Stats
    int max_hp = 100;
    int attack = 10;
    float attack_speed = 1.0f;
    int range = 100;
    
    std::vector<std::string> skill_ids;
    std::vector<std::string> tags;
  };
  EditState edit_state_;
  
  // ===== マルチペイン =====
  float left_panel_width_ = 300.0f;
  bool show_details_ = true;

public:
  EntityEditorWindow(
    Shared::Data::DefinitionRegistry* definitions,
    Editor::Services::DataBindingService* data_binding);
  
  void Initialize(Shared::Core::GameContext& context) override;
  void Shutdown() override;
  
  void OnDrawUI() override;
  void OnUpdate(float delta_time) override;
  
  std::string GetWindowTitle() const override { 
    return "Entity Editor"; 
  }
  bool IsOpen() const override { return is_open_; }
  void SetOpen(bool open) override { is_open_ = open; }
  
  std::string GetWindowId() const override { 
    return "entity_editor"; 
  }
  void SaveState(nlohmann::json& state) const override;
  void LoadState(const nlohmann::json& state) override;

private:
  void DrawEntityList();
  void DrawEntityDetails();
  void DrawStatsPanel();
  void DrawSkillsPanel();
  void DrawTagsPanel();
  
  void OnEntitySelected(const std::string& entity_id);
  void OnEntityCreate();
  void OnEntityDelete();
  void OnEntityDuplicate();
  void OnEntitySave();
  void OnEntityReload();
};

} // namespace Editor::Windows
```

### EntityEditorWindow 実装

```cpp
// editor/src/Editor/Windows/EntityEditorWindow.cpp
namespace Editor::Windows {

void EntityEditorWindow::OnDrawUI() {
  if (!is_open_) return;
  
  ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);
  
  if (ImGui::Begin("Entity Editor", &is_open_, ImGuiWindowFlags_NoCollapse)) {
    // ツールバー
    if (ImGui::Button("New Entity")) {
      OnEntityCreate();
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete")) {
      OnEntityDelete();
    }
    ImGui::SameLine();
    if (ImGui::Button("Duplicate")) {
      OnEntityDuplicate();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
      OnEntitySave();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload")) {
      OnEntityReload();
    }
    
    ImGui::Separator();
    
    // メイン領域
    ImGui::BeginChild("entity_splitter", ImVec2(0, 0), false);
    {
      // 左パネル：Entity一覧
      ImGui::BeginChild("left_panel", ImVec2(left_panel_width_, 0), true);
      {
        ImGui::TextUnformatted("Entities:");
        ImGui::InputText("##search", search_filter_, sizeof(search_filter_));
        
        ImGui::BeginChild("entity_list");
        {
          for (const auto& entity_id : entity_list_) {
            // フィルタリング
            if (search_filter_[0] != '\0' && 
                entity_id.find(search_filter_) == std::string::npos) {
              continue;
            }
            
            bool is_selected = (entity_id == selected_entity_id_);
            if (ImGui::Selectable(entity_id.c_str(), is_selected)) {
              OnEntitySelected(entity_id);
            }
          }
        }
        ImGui::EndChild();
      }
      ImGui::EndChild();
      
      ImGui::SameLine();
      
      // 分割バードラッグ
      ImGui::InvisibleButton("splitter", ImVec2(5, -1));
      if (ImGui::IsItemActive()) {
        left_panel_width_ += ImGui::GetIO().MouseDelta.x;
        left_panel_width_ = glm::clamp(left_panel_width_, 200.0f, 500.0f);
      }
      
      ImGui::SameLine();
      
      // 右パネル：詳細編集
      ImGui::BeginChild("right_panel");
      {
        DrawEntityDetails();
      }
      ImGui::EndChild();
    }
    ImGui::EndChild();
  }
  
  ImGui::End();
}

void EntityEditorWindow::DrawEntityDetails() {
  if (selected_entity_id_.empty()) {
    ImGui::TextDisabled("Select an entity to edit");
    return;
  }
  
  ImGui::Text("Editing: %s", selected_entity_id_.c_str());
  ImGui::Separator();
  
  // 基本情報
  if (ImGui::CollapsingHeader("Basic Info", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::InputText("ID##entity_id", 
      edit_state_.id.data(), edit_state_.id.capacity());
    ImGui::InputText("Name##entity_name",
      edit_state_.name.data(), edit_state_.name.capacity());
    ImGui::InputTextMultiline("Description",
      edit_state_.description.data(), edit_state_.description.capacity(),
      ImVec2(-1, 60));
    
    ImGui::SliderInt("Rarity##entity_rarity", &edit_state_.rarity, 1, 5);
    ImGui::Checkbox("Is Enemy##entity_is_enemy", &edit_state_.is_enemy);
  }
  
  // ステータス
  DrawStatsPanel();
  
  // スキル
  DrawSkillsPanel();
  
  // タグ
  DrawTagsPanel();
}

void EntityEditorWindow::DrawStatsPanel() {
  if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::InputInt("Max HP##entity_hp", &edit_state_.max_hp);
    ImGui::InputInt("Attack##entity_attack", &edit_state_.attack);
    ImGui::InputFloat("Attack Speed##entity_atk_spd", 
      &edit_state_.attack_speed);
    ImGui::InputInt("Range##entity_range", &edit_state_.range);
    ImGui::InputInt("Cost##entity_cost", &edit_state_.cost);
    ImGui::InputFloat("Cooldown##entity_cd", &edit_state_.cooldown);
  }
}

void EntityEditorWindow::DrawSkillsPanel() {
  if (ImGui::CollapsingHeader("Skills")) {
    ImGui::BeginChild("skills_list", ImVec2(-1, 100), true);
    {
      for (size_t i = 0; i < edit_state_.skill_ids.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        
        ImGui::TextUnformatted(edit_state_.skill_ids[i].c_str());
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
          edit_state_.skill_ids.erase(
            edit_state_.skill_ids.begin() + i);
        }
        
        ImGui::PopID();
      }
    }
    ImGui::EndChild();
    
    if (ImGui::Button("Add Skill")) {
      // スキル選択ダイアログ起動
    }
  }
}

void EntityEditorWindow::DrawTagsPanel() {
  if (ImGui::CollapsingHeader("Tags")) {
    ImGui::BeginChild("tags_list", ImVec2(-1, 80), true);
    {
      for (size_t i = 0; i < edit_state_.tags.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        ImGui::TextUnformatted(edit_state_.tags[i].c_str());
        ImGui::PopID();
      }
    }
    ImGui::EndChild();
  }
}

void EntityEditorWindow::OnEntitySelected(const std::string& entity_id) {
  selected_entity_id_ = entity_id;
  
  // JSONからEditStateへロード
  auto entity_def = definitions_->GetEntity(entity_id);
  if (entity_def) {
    edit_state_.id = entity_def->id;
    edit_state_.name = entity_def->name;
    edit_state_.description = entity_def->description;
    edit_state_.rarity = entity_def->rarity;
    edit_state_.max_hp = entity_def->stats.hp;
    edit_state_.attack = entity_def->stats.attack;
    // ... 他のフィールド
  }
}

void EntityEditorWindow::OnEntitySave() {
  // EditStateからJSONへ変換
  data_binding_->SaveDefinition("entities", selected_entity_id_);
}

} // namespace Editor::Windows
```

### その他のEditorWindow（簡潔版）

#### SkillEditorWindow

```cpp
class SkillEditorWindow : public IEditorWindow {
  // スキル定義編集
  // - パッシブ/インタラプト/イベント型の切り替え
  // - トリガー条件設定
  // - 効果パラメータ編集
};
```

#### StageEditorWindow

```cpp
class StageEditorWindow : public IEditorWindow {
  // ステージ定義編集
  // - ウェーブスケジュール（タイムライン表示）
  // - 敵配置（ビジュアルエディタ）
  // - 報酬設定
  // - 難易度パラメータ
};
```

#### HierarchyWindow

```cpp
class HierarchyWindow : public IEditorWindow {
  // オブジェクト一覧（ツリービュー）
  // - Entities / Skills / Stages / Effects
  // - ドラッグ&ドロップ対応
  // - 名前変更・削除
};
```

#### InspectorWindow

```cpp
class InspectorWindow : public IEditorWindow {
  // 選択中オブジェクトの詳細表示
  // - HierarchyWindow選択 → InspectorWindowに詳細表示
  // - PropertyGrid（自動生成UI）
};
```

#### ConsoleWindow

```cpp
class ConsoleWindow : public IEditorWindow {
  // ログ出力
  // - Game実行ログ
  // - 検証エラー
  // - 警告メッセージ
};
```

---

## ImGui統合（UI描画）

### ImGui初期化

```cpp
// editor/include/Editor/Renderer/ImGuiRenderer.h
namespace Editor::Renderer {

class ImGuiRenderer {
public:
  static void Initialize(int width, int height);
  static void Shutdown();
  static void NewFrame();
  static void Render();
  
  static void SetDarkTheme();
  static void SetLightTheme();
};

} // namespace Editor::Renderer
```

```cpp
// editor/src/Editor/Renderer/ImGuiRenderer.cpp
namespace Editor::Renderer {

void ImGuiRenderer::Initialize(int width, int height) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  
  // ImGui設定
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  
  // Raylib統合
  ImGui_ImplRaylib_Init(width, height);
  ImGui_ImplOpenGL3_Init();
  
  // テーマ
  SetDarkTheme();
}

void ImGuiRenderer::SetDarkTheme() {
  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  
  // カスタマイズ
  style.WindowRounding = 5.0f;
  style.FrameRounding = 3.0f;
  style.GrabRounding = 3.0f;
  style.TabRounding = 3.0f;
}

} // namespace Editor::Renderer
```

### ImGui Widgets ユーティリティ

```cpp
// editor/include/Editor/Widgets/ImGuiHelper.h
namespace Editor::Widgets {

class ImGuiHelper {
public:
  // ===== 便利マクロ =====
  static bool InputInt(const char* label, int& value, int min = 0, int max = 100);
  static bool InputFloat(const char* label, float& value, 
                         float min = 0.0f, float max = 1.0f);
  static bool InputText(const char* label, std::string& value);
  
  // ===== PropertyGrid（自動UI生成） =====
  static void BeginPropertyGrid();
  static void EndPropertyGrid();
  static void PropertyInt(const char* label, int& value);
  static void PropertyFloat(const char* label, float& value);
  static void PropertyString(const char* label, std::string& value);
  static void PropertyBool(const char* label, bool& value);
  
  // ===== その他 =====
  static void HelpMarker(const char* desc);
  static bool BeginCombo(const char* label, const std::vector<std::string>& items,
                         int& selected_index);
  static bool BeginMultiSelect(const char* label, 
                               const std::vector<std::string>& items,
                               std::vector<int>& selected_indices);
};

} // namespace Editor::Widgets
```

---

## DataBinding & Serialization

### DataBindingService

```cpp
// editor/include/Editor/Services/DataBindingService.h
namespace Editor::Services {

class DataBindingService {
private:
  Shared::Core::GameContext& context_;
  Shared::Data::DefinitionRegistry& definitions_;
  
  // ===== バインディング管理 =====
  std::unordered_map<std::string, nlohmann::json> bindings_;
  std::unordered_set<std::string> dirty_keys_;

public:
  DataBindingService(Shared::Core::GameContext& context,
                    Shared::Data::DefinitionRegistry& definitions);
  
  // ===== ImGui → JSON =====
  void BindValue(const std::string& key, bool& value);
  void BindValue(const std::string& key, int& value);
  void BindValue(const std::string& key, float& value);
  void BindValue(const std::string& key, std::string& value);
  
  // ===== JSON → ImGui =====
  void LoadBinding(const std::string& key);
  void SaveBinding(const std::string& key);
  
  // ===== ファイル操作 =====
  bool WriteToFile(const std::string& type);
  bool ReadFromFile(const std::string& type);
  
  // ===== 変更追跡 =====
  void MarkDirty(const std::string& key);
  bool IsDirty(const std::string& key) const;
  void ClearDirty();
  
  // ===== バッチ操作 =====
  void SaveAllDirty();
};

} // namespace Editor::Services
```

### Serializer

```cpp
// shared/include/Data/Serialization/EntitySerializer.h
namespace Shared::Data {

class EntitySerializer {
public:
  // ===== JSON → Struct =====
  static EntityDef FromJson(const nlohmann::json& json);
  
  // ===== Struct → JSON =====
  static nlohmann::json ToJson(const EntityDef& entity);
  
  // ===== パーシャル更新 =====
  static void UpdateFromJson(EntityDef& entity, const nlohmann::json& json);
};

} // namespace Shared::Data
```

---

## ホットリロード & HotWatch

### HotWatchService

```cpp
// editor/include/Editor/Services/HotWatchService.h
namespace Editor::Services {

class HotWatchService {
private:
  struct WatchedFile {
    std::string filepath;
    std::filesystem::file_time_type last_write_time;
    std::function<void()> on_changed;
  };
  
  std::vector<WatchedFile> watched_files_;
  Shared::Core::GameContext& context_;
  Shared::Data::DefinitionRegistry& definitions_;

public:
  HotWatchService(Shared::Core::GameContext& context,
                 Shared::Data::DefinitionRegistry& definitions);
  
  // ===== ファイル監視 =====
  void WatchFile(const std::string& filepath,
                std::function<void()> on_changed);
  void UnwatchFile(const std::string& filepath);
  
  // ===== 毎フレーム呼び出し =====
  void CheckChanges();
  
private:
  void OnFileChanged(const std::string& filepath);
  void ReloadDefinition(const std::string& filepath);
};

} // namespace Editor::Services
```

### ホットリロード流れ

```
Timeline:

1. Editor でEntity編集
   ├─ ImGui UI表示
   └─ DataBindingService がバインディング管理

2. Save ボタン押下
   ├─ EditState → nlohmann::json 変換
   ├─ JSON ファイル出力（data/definitions/entities/entities.json）
   └─ HotWatchService が変更検知

3. Game実行中
   ├─ FileWatcher が JSON ファイル変更を検知
   ├─ EventSystem が "EntitiesReloaded" 発行
   ├─ EntityManager が定義を再ロード
   └─ 実行中の Entity Component を更新

4. Editor側も自動更新
   ├─ HotWatchService が変更検知
   ├─ DefinitionRegistry を再ロード
   └─ HierarchyWindow / InspectorWindow 自動更新
```

---

## Validation & Error Handling

### ValidationService

```cpp
// editor/include/Editor/Services/ValidationService.h
namespace Editor::Services {

struct ValidationError {
  std::string error_type;   // "missing_reference" / "invalid_value" etc.
  std::string object_id;
  std::string field_name;
  std::string message;
  int severity;             // 0=info, 1=warning, 2=error
};

class ValidationService {
private:
  Shared::Data::DefinitionRegistry& definitions_;
  std::vector<ValidationError> errors_;

public:
  ValidationService(Shared::Data::DefinitionRegistry& definitions);
  
  // ===== 検証実行 =====
  void ValidateAll();
  void ValidateEntity(const std::string& entity_id);
  void ValidateSkill(const std::string& skill_id);
  void ValidateStage(const std::string& stage_id);
  
  // ===== 結果取得 =====
  const std::vector<ValidationError>& GetErrors() const;
  void ClearErrors();
  
  // ===== 詳細検証 =====
  bool ValidateReference(const std::string& type, const std::string& id);
  bool ValidateSchema(const nlohmann::json& data, 
                     const std::string& schema_type);

private:
  void AddError(const ValidationError& error);
  void ValidateSkillReferences(const std::string& entity_id);
  void ValidateEnemyWaveReferences(const std::string& stage_id);
};

} // namespace Editor::Services
```

### ConsoleWindow での表示

```cpp
// editor/include/Editor/Windows/ConsoleWindow.h
namespace Editor::Windows {

class ConsoleWindow : public IEditorWindow {
private:
  std::vector<std::string> log_messages_;
  std::vector<int> log_levels_;  // 0=info, 1=warning, 2=error
  int filter_level_ = 0;         // 表示レベル
  bool auto_scroll_ = true;

public:
  void OnDrawUI() override {
    ImGui::Begin("Console");
    
    // フィルタ
    ImGui::Checkbox("Info", &show_info_);
    ImGui::SameLine();
    ImGui::Checkbox("Warning", &show_warning_);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &show_error_);
    ImGui::Separator();
    
    // ログリスト
    ImGui::BeginChild("log_list");
    for (size_t i = 0; i < log_messages_.size(); ++i) {
      // レベルに応じてカラー変更
      Color color = GetColorForLevel(log_levels_[i]);
      ImGui::TextColored(ImGui::GetColorU32(color), 
        "%s", log_messages_[i].c_str());
    }
    
    if (auto_scroll_) {
      ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    
    ImGui::End();
  }
};

} // namespace Editor::Windows
```

---

## ワークフロー & ユースケース

### ユースケース 1: 新しいキャラクター追加

```
1. EntityEditorWindow で "New Entity" ボタン
   ├─ ID入力: "char_001_new"
   └─ テンプレート選択（必須値自動入力）

2. 詳細編集パネルで情報入力
   ├─ Name: "新キャラ"
   ├─ Stats入力（HP, 攻撃力など）
   ├─ スキル選択・付与
   └─ タグ追加

3. Save ボタン
   ├─ Validation実行
   │  └─ スキル参照チェック OK
   ├─ entities.json に追記
   └─ FileWatcher が変更検知

4. Game実行中に確認
   ├─ ゲーム再スタート（またはHotReload）
   └─ 新キャラが使用可能に
```

### ユースケース 2: ステージ編集（ビジュアル）

```
1. StageEditorWindow でステージ選択
   ├─ ウェーブタイムライン表示
   └─ 敵配置ビジュアル編集

2. 新規ウェーブ追加
   ├─ タイムスライダーで出現時刻指定
   ├─ 敵リストから選択
   └─ 数量・出現間隔設定

3. Waveパラメータ微調整
   ├─ リアルタイムプレビュー（オプション）
   └─ スケジュール確認

4. Save
   ├─ stages.json に反映
   └─ Game側でプリビルド時に読み込み
```

### ユースケース 3: データ検証 & エラー修正

```
1. Validation実行（メニュー → Validate All）
   ├─ 全Definition スキーマチェック
   ├─ 参照チェック（存在しないID参照検出）
   └─ 警告・エラーを ConsoleWindow に出力

2. エラーリストから問題をクリック
   ├─ 該当オブジェクトを自動選択
   └─ InspectorWindow に詳細表示

3. 修正 & 再検証
   ├─ 値修正
   ├─ Save
   └─ Validation再実行
```

### ユースケース 4: JSON直接編集（高度なユーザー向け）

```
1. AssetBrowserWindow で entities.json を選択
   ├─ テキストエディタで直接編集可能
   └─ または ImGui Tree View で JSON構造表示

2. Save
   ├─ Validation実行
   └─ エラー表示（構文エラー、型エラーなど）

3. ゲーム側で自動リロード
```

---

## EditorWindow プリセット & テンプレート

### テンプレート機能

```cpp
// editor/include/Editor/Services/TemplateManager.h
namespace Editor::Services {

class TemplateManager {
public:
  // ===== テンプレート管理 =====
  std::vector<std::string> GetEntityTemplates() const;
  std::vector<std::string> GetSkillTemplates() const;
  std::vector<std::string> GetStageTemplates() const;
  
  // ===== テンプレート適用 =====
  nlohmann::json LoadTemplate(const std::string& category,
                             const std::string& template_name);
  void ApplyTemplate(const std::string& template_name);

private:
  // テンプレート: data/templates/entity/melee.json など
  // 最小限の必須フィールドを持つJSON
};

} // namespace Editor::Services
```

### テンプレート例

```json
// data/templates/entity/melee.json
{
  "id": "new_melee_unit",
  "name": "新規近接ユニット",
  "description": "",
  "rarity": 1,
  "type": "main",
  "is_enemy": false,
  "cost": 300,
  "cooldown": 5.0,
  "stats": {
    "hp": 100,
    "attack": 50,
    "attack_speed": 1.0,
    "range": 50
  },
  "skills": [],
  "tags": []
}
```

---

## ProjectManager（プロジェクト管理）

```cpp
// editor/include/Editor/Services/ProjectManager.h
namespace Editor::Services {

class ProjectManager {
private:
  std::string current_project_path_;
  bool unsaved_changes_ = false;

public:
  // ===== プロジェクト操作 =====
  bool NewProject(const std::string& project_path);
  bool OpenProject(const std::string& project_path);
  bool SaveProject();
  bool SaveProjectAs(const std::string& new_path);
  bool CloseProject();
  
  // ===== 状態 =====
  bool HasUnsavedChanges() const { return unsaved_changes_; }
  std::string GetProjectPath() const { return current_project_path_; }
  
  // ===== エクスポート =====
  bool ExportForGame(const std::string& export_path);
  bool ExportAsZip(const std::string& zip_path);

private:
  void SaveProjectMetadata();
  void LoadProjectMetadata();
};

} // namespace Editor::Services
```

---

## WorkspaceManager（ワークスペース管理）

```cpp
// editor/include/Editor/Services/WorkspaceManager.h
namespace Editor::Services {

class WorkspaceManager {
private:
  struct WindowLayout {
    std::string name;
    nlohmann::json layout_data;  // ImGui DockSpace状態
    std::vector<std::string> open_windows;
  };
  
  std::vector<WindowLayout> saved_layouts_;
  std::string current_layout_name_;

public:
  // ===== レイアウト管理 =====
  void SaveLayout(const std::string& name);
  void LoadLayout(const std::string& name);
  void DeleteLayout(const std::string& name);
  std::vector<std::string> GetSavedLayouts() const;
  
  // ===== ウィンドウ管理 =====
  void OpenWindow(const std::string& window_id);
  void CloseWindow(const std::string& window_id);
  void ResetLayout();
};

} // namespace Editor::Services
```

---

## 実装優先度

### Phase 5.1: Core Infrastructure（3日）

```
Day 1:
  ✅ EditorApp メインループ
  ✅ ImGui初期化 & Raylib統合
  ✅ DockSpace & メニューバー

Day 2:
  ✅ IEditorWindow インターフェース
  ✅ ImGuiHelper ユーティリティ
  ✅ HierarchyWindow, ConsoleWindow

Day 3:
  ✅ DataBindingService
  ✅ ValidationService
  ✅ HotWatchService
```

### Phase 5.2: EditorWindow実装（1週間）

```
Day 1-2:
  ✅ EntityEditorWindow（基本機能）
  ✅ SkillEditorWindow
  ✅ PropertyGrid自動生成

Day 3-4:
  ✅ StageEditorWindow（タイムライン）
  ✅ EffectEditorWindow
  ✅ DeckEditorWindow

Day 5-6:
  ✅ ホットリロード統合テスト
  ✅ Validation & エラー表示
  ✅ パフォーマンス計測

Day 7:
  ✅ 全Window統合テスト
  ✅ ドキュメント作成
  ✅ ユーザーテスト
```

---

## チェックリスト

```
EditorApp:
  ☐ ImGui初期化 & DockSpace
  ☐ メニューバー実装
  ☐ ウィンドウ管理システム
  ☐ メインループ実装

EditorWindows:
  ☐ IEditorWindow インターフェース
  ☐ EntityEditorWindow
  ☐ SkillEditorWindow
  ☐ StageEditorWindow
  ☐ EffectEditorWindow
  ☐ DeckEditorWindow
  ☐ HierarchyWindow
  ☐ InspectorWindow
  ☐ ConsoleWindow
  ☐ AssetBrowserWindow

Services:
  ☐ DataBindingService
  ☐ ValidationService
  ☐ HotWatchService
  ☐ ProjectManager
  ☐ WorkspaceManager
  ☐ TemplateManager

Integration:
  ☐ Shared層との連携
  ☐ HotReload動作確認
  ☐ Game ↔ Editor同期テスト
```

---

## 次のドキュメント

- [ ] **Raylib Graphics Integration** (ゲーム描画実装)
- [ ] **実装スケジュール詳細** (全6フェーズ)
- [ ] **テスト戦略** (Unit/Integration/E2E)
- [ ] **デプロイメント & リリース手順**

---

## サマリー

Editor Layer（ImGui統合構成）の基本設計が完成しました：

```
✅ EditorApp - メインアプリケーション & ImGui統合
✅ 9種類のEditorWindow（プラグイン型）
✅ DataBindingService（ImGui ↔ JSON双方向バインディング）
✅ ValidationService（スキーマ・参照チェック）
✅ HotWatchService（ファイル自動監視 & リロード）
✅ ProjectManager（プロジェクト管理・Export）
✅ WorkspaceManager（レイアウト・ウィンドウ管理）

🎉 Game実行中に Editor でデータ編集可能な
   完全統合開発環境が実現！

アーキテクチャ完成度：
  ✅ Core Layer（基盤）
  ✅ Game Layer（管理・制御）
  ✅ TD Layer（ゲームロジック - ECS）
  ✅ Application Layer（UI・シーン）
  ✅ Editor Layer（開発ツール）

→ 6層完全アーキテクチャ確立！
```

