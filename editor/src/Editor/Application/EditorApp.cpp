#include "Editor/Application/EditorApp.h"
#include <algorithm>
#include <imgui.h>
#include <iostream>
#include <memory>
#include <raylib.h>
#include <rlImGui.h>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <ctime>
 
#include "Data/Loaders/AbilityLoader.h"
#include "Data/Loaders/EntityLoader.h"
#include "Data/Loaders/SkillLoader.h"
#include "Data/Loaders/StageLoader.h"
#include "Data/Loaders/WaveLoader.h"
#include "Data/Validators/DataValidator.h"
#include "Editor/Windows/AssetBrowserWindow.h"
#include "Editor/Windows/DefinitionEditorWindow.h"
#include "Editor/Windows/PreviewWindow.h"
#include "Editor/Windows/PropertyPanel.h"
#include "Editor/Windows/UnitEditorWindow.h"
#include "Editor/Windows/SpriteEditorWindow.h"
#include "Editor/Windows/ValidationPanel.h"
#include "Editor/Windows/SearchPaletteWindow.h"
 
 namespace {
 struct InitialWindowSize { int w; int h; };
 InitialWindowSize GetInitialWindowSize() {
   InitialWindowSize s{1920, 1080};
 #if defined(_DEBUG)
   s = {1280, 720};
 #endif
   return s;
 }
 constexpr float TARGET_FPS = 60.0f;
 } // namespace

namespace Editor::Application {

EditorApp::EditorApp() : is_running_(false), delta_time_(0.0f) {}

EditorApp::~EditorApp() { Shutdown(); }

bool EditorApp::Initialize() {
  std::cout << "=== EditorApp Initialization ===" << std::endl;

  // GameContext初期化
  context_ = std::make_unique<Shared::Core::GameContext>();
  if (!context_->Initialize("assets/config.json")) {
    std::cerr << "Failed to initialize GameContext" << std::endl;
    return false;
  }

  // DefinitionRegistry初期化
  definitions_ = std::make_unique<Shared::Data::DefinitionRegistry>();
  context_->BindDefinitions(definitions_.get());

  // 定義データのロード
  LoadDefinitions();

  // バリデーション
  if (!Shared::Data::DataValidator::Validate(*definitions_)) {
    std::cerr << "Validation failed" << std::endl;
  }

  // Raylib初期化
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  const auto initial = GetInitialWindowSize();
  InitWindow(initial.w, initial.h, "Simple TDC Editor");
  SetWindowMinSize(initial.w, initial.h);
  SetTargetFPS(static_cast<int>(TARGET_FPS));

  // ImGui初期化
  rlImGuiSetup(true);

  fontManager_ = std::make_unique<Shared::Core::FontManager>();
  ImGuiIO &io = ImGui::GetIO();
  io.Fonts->Clear();
  defaultFont_ = fontManager_->LoadImGuiJapaneseFont(
      "assets/fonts/NotoSansJP-Medium.ttf", 18.0f);
  if (defaultFont_ != nullptr) {
    io.FontDefault = defaultFont_;
    // フォントを再構築（rlImGuiReloadFonts()は利用できない場合があるため）
    io.Fonts->Build();
  }

  InitializeEditorWindows();

  is_running_ = true;
  std::cout << "=== EditorApp Initialized ===" << std::endl;
  return true;
}

void EditorApp::Run() {
  while (is_running_ && !WindowShouldClose()) {
    delta_time_ = GetFrameTime();
    HandleResize();

    // FileWatcherの変更チェック
    context_->GetFileWatcher().CheckChanges();

    Update(delta_time_);
    Render();
  }
}

void EditorApp::Shutdown() {
  if (!is_running_)
    return;

  std::cout << "=== EditorApp Shutdown ===" << std::endl;

  for (auto &w : windows_) {
    if (w) {
      w->Shutdown();
    }
  }
  windows_.clear();

  context_->Shutdown();

  rlImGuiShutdown();
  CloseWindow();

  is_running_ = false;
  std::cout << "=== EditorApp Shutdown Complete ===" << std::endl;
}

void EditorApp::Update(float delta_time) {
  UpdateEditorWindows(delta_time);
}

void EditorApp::Render() {
  BeginDrawing();
  ClearBackground(DARKGRAY);

  // ImGui描画
  rlImGuiBegin();
  RenderUI();
  RenderEditorWindows();
  rlImGuiEnd();

  EndDrawing();
}

void EditorApp::HandleResize() {
  // 最低解像度（FHD）を維持するための処理は SetWindowMinSize で対応済み。
  // 将来的にレターボックス対応が必要になった場合は、GameApp と同様の
  // ビューポートスケーリングをここに追加する。
}

void EditorApp::RenderUI() {
  // DockSpace Setup
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  ImGui::Begin("DockSpace", nullptr, window_flags);
  ImGui::PopStyleVar(3);

  ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

  // メインメニューバー
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("ファイル")) {
      if (ImGui::MenuItem("終了")) {
        is_running_ = false;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("編集")) {
      if (unitEditor_ && ImGui::MenuItem("ユニットエディタ", "Ctrl+U", unitEditor_->IsOpen())) {
        unitEditor_->SetOpen(!unitEditor_->IsOpen());
      }
      if (spriteEditor_ && ImGui::MenuItem("スプライトエディタ", "Ctrl+R", spriteEditor_->IsOpen())) {
        spriteEditor_->SetOpen(!spriteEditor_->IsOpen());
      }
      ImGui::Separator();
      if (previewWindow_ && ImGui::MenuItem("プレビュー", "Ctrl+P", previewWindow_->IsOpen())) {
        previewWindow_->SetOpen(!previewWindow_->IsOpen());
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("ウィンドウ")) {
      for (auto &w : windows_) {
        if (w) {
          bool is_open = w->IsOpen();
          if (ImGui::MenuItem(w->GetWindowTitle().c_str(), nullptr, &is_open)) {
            w->SetOpen(is_open);
          }
        }
      }
      ImGui::EndMenu();
    }

    // ツールメニュー
    if (ImGui::BeginMenu("ツール")) {
      if (ImGui::MenuItem("定義を再読込")) {
        // 定義をクリアして再読込
        if (definitions_) {
          definitions_->Clear();
        }
        LoadDefinitions();
      }

      ImGui::Separator();

      if (ImGui::MenuItem("定義を検証 (開く)")) {
        if (validationPanel_) {
          validationPanel_->SetOpen(true);
          validationPanel_->RunValidation();
        }
      }

      if (ImGui::MenuItem("クイック検索", "Ctrl+Shift+P", searchPalette_ ? searchPalette_->IsOpen() : false)) {
        if (searchPalette_) {
          searchPalette_->SetOpen(true);
        }
      }

      ImGui::Separator();

      if (ImGui::MenuItem("レイアウト保存")) {
        ImGui::SaveIniSettingsToDisk("imgui.ini");
      }
      if (ImGui::MenuItem("レイアウト読込")) {
        ImGui::LoadIniSettingsFromDisk("imgui.ini");
      }

      ImGui::Separator();

      if (ImGui::MenuItem("スクリーンショット保存")) {
        // ファイル名を生成: screenshots/screenshot_YYYYMMDD_HHMMSS.png
        try {
          std::filesystem::create_directories("screenshots");
          std::time_t t = std::time(nullptr);
          std::tm tm;
#if defined(_WIN32)
          localtime_s(&tm, &t);
#else
          localtime_r(&t, &tm);
#endif
          std::ostringstream oss;
          oss << "screenshots/screenshot_"
              << std::put_time(&tm, "%Y%m%d_%H%M%S")
              << ".png";
          TakeScreenshot(oss.str().c_str());
        } catch (const std::exception& e) {
          std::cerr << "Failed to save screenshot: " << e.what() << std::endl;
        }
      }

      ImGui::EndMenu();
    }
    
    ImGui::EndMenuBar();
  }
  ImGui::End(); // End DockSpace

  // 簡易情報ウィンドウ
  ImGui::Begin("エディタ情報");
  ImGui::Text("Simple TDC Editor (日本語フォント検証)");
  ImGui::Separator();
  ImGui::Text("FPS: %.1f", 1.0f / delta_time_);
  ImGui::Text("エンティティ数: %zu", definitions_->GetAllEntities().size());
  ImGui::Text("スキル数: %zu", definitions_->GetAllSkills().size());
  ImGui::Text("ステージ数: %zu", definitions_->GetAllStages().size());
  ImGui::End();
}

void EditorApp::InitializeEditorWindows() {
  windows_.clear();
  
  // DefinitionEditorWindow
  auto defEditor = std::make_unique<Editor::Windows::DefinitionEditorWindow>();
  definitionEditor_ = defEditor.get();
  windows_.push_back(std::move(defEditor));
  
  // UnitEditorWindow
  auto unitEditor = std::make_unique<Editor::Windows::UnitEditorWindow>();
  unitEditor_ = unitEditor.get();
  windows_.push_back(std::move(unitEditor));
  
  // SpriteEditorWindow
  auto spriteEditor = std::make_unique<Editor::Windows::SpriteEditorWindow>();
  spriteEditor_ = spriteEditor.get();
  windows_.push_back(std::move(spriteEditor));
  
  // PreviewWindow
  auto previewWindow = std::make_unique<Editor::Windows::PreviewWindow>();
  previewWindow_ = previewWindow.get();
  windows_.push_back(std::move(previewWindow));

  // ValidationPanel
  auto validationPanel = std::make_unique<Editor::Windows::ValidationPanel>();
  validationPanel_ = validationPanel.get();
  windows_.push_back(std::move(validationPanel));

  // SearchPaletteWindow
  auto searchPalette = std::make_unique<Editor::Windows::SearchPaletteWindow>();
  searchPalette_ = searchPalette.get();
  windows_.push_back(std::move(searchPalette));

  // AssetBrowserWindow
  windows_.push_back(std::make_unique<Editor::Windows::AssetBrowserWindow>());
  
  // PropertyPanel
  windows_.push_back(std::make_unique<Editor::Windows::PropertyPanel>());

  for (auto &w : windows_) {
    if (w) {
      w->Initialize(*context_, *definitions_);
    }
  }

  // ValidationPanel へ UnitEditor の参照を渡す
  if (validationPanel_ && unitEditor_) {
    validationPanel_->SetUnitEditor(unitEditor_);
  }

  // SearchPaletteWindow へ関連ウィンドウを渡す
  if (searchPalette_) {
    searchPalette_->SetUnitEditor(unitEditor_);
    searchPalette_->SetSpriteEditor(spriteEditor_);
    searchPalette_->SetPreviewWindow(previewWindow_);
    searchPalette_->SetDefinitionEditor(definitionEditor_);
  }

  // 初回バリデーションを実行しておくとUXがよい
  if (validationPanel_) {
    validationPanel_->RunValidation();
  }
  
  // PreviewWindow を SpriteEditorWindow に紐付け
  if (spriteEditor_ && previewWindow_) {
    spriteEditor_->SetPreviewWindow(previewWindow_);
  }
  
  // PropertyPanel を DefinitionEditorWindow に紐付け
  for (auto &w : windows_) {
    if (w->GetWindowId() == "definition_editor") {
      auto* def_editor = dynamic_cast<Editor::Windows::DefinitionEditorWindow*>(w.get());
      if (def_editor) {
        for (auto &wp : windows_) {
          if (wp->GetWindowId() == "property_panel") {
            auto* prop_panel = dynamic_cast<Editor::Windows::PropertyPanel*>(wp.get());
            if (prop_panel) {
              def_editor->SetPropertyPanel(prop_panel);
            }
          }
        }
      }
    }
  }
}

void EditorApp::UpdateEditorWindows(float delta_time) {
  for (auto &w : windows_) {
    if (w && w->IsOpen()) {
      w->OnUpdate(delta_time);
    }
  }
}

void EditorApp::RenderEditorWindows() {
  // ウィンドウ連携処理
  // UnitEditor → SpriteEditor, PreviewWindow への連携
  if (unitEditor_ && unitEditor_->IsOpen()) {
    const std::string& activeEntityId = unitEditor_->GetActiveEntityId();
    
    // SpriteEditorに選択中のエンティティを伝達
    if (spriteEditor_ && !activeEntityId.empty()) {
      if (spriteEditor_->GetActiveEntityId() != activeEntityId) {
        spriteEditor_->SetActiveEntity(activeEntityId);
      }
    }
    
    // PreviewWindowに選択中のエンティティを伝達
    if (previewWindow_ && !activeEntityId.empty()) {
      if (previewWindow_->GetCurrentEntityId() != activeEntityId) {
        previewWindow_->LoadEntity(activeEntityId);
      }
    }
  }
  
  // PreviewWindowとPropertyPanelの連携
  Editor::Windows::PreviewWindow* preview = nullptr;
  Editor::Windows::PropertyPanel* property = nullptr;
  
  for (auto &w : windows_) {
    if (!w || !w->IsOpen()) continue;
    
    // 型を確認してキャスト
    if (w->GetWindowId() == "preview_window") {
      preview = dynamic_cast<Editor::Windows::PreviewWindow*>(w.get());
    } else if (w->GetWindowId() == "property_panel") {
      property = dynamic_cast<Editor::Windows::PropertyPanel*>(w.get());
    }
    
    w->OnDrawUI();
  }
  
  // PreviewWindowのエンティティ情報をPropertyPanelに反映
  if (preview && property && preview->GetPreviewEntity() != entt::null) {
    const auto* sim = preview->GetSimulation();
    if (sim) {
      property->SetSelection(preview->GetPreviewEntity(), &sim->GetRegistry());
    }
  }
}

void EditorApp::LoadDefinitions() {
  std::cout << "Loading definitions..." << std::endl;

  // Entity定義
  std::string entity_path = context_->GetDataPath("entities/debug.json");
  Shared::Data::EntityLoader::LoadFromJson(entity_path, *definitions_);

  // エディタで作成したエンティティ定義を読み込む
  {
    namespace fs = std::filesystem;
    std::string chars_dir = context_->GetDataPath("entities/characters");
      std::cout << "Loading characters from: " << chars_dir << std::endl;
    if (fs::exists(chars_dir) && fs::is_directory(chars_dir)) {
      for (const auto &entry : fs::recursive_directory_iterator(chars_dir)) {
        if (entry.is_regular_file() && entry.path().filename() == "entity.json") {
                    std::cout << "  Loading entity: " << entry.path().string() << std::endl;
          Shared::Data::EntityLoader::LoadFromJson(entry.path().string(), *definitions_);
            } else {
              std::cout << "Warning: Character directory not found: " << chars_dir << std::endl;
        }
      }
    }
  }

  // Ability定義
  std::string ability_path = context_->GetDataPath("abilities/debug.json");
  Shared::Data::AbilityLoader::LoadFromJson(ability_path, *definitions_);

  // Stage定義
  std::string stage_path = context_->GetDataPath("stages/debug.json");
  Shared::Data::StageLoader::LoadFromJson(stage_path, *definitions_);

  // Wave定義
  std::string wave_path = context_->GetDataPath("waves/debug.json");
  Shared::Data::WaveLoader::LoadFromJson(wave_path, *definitions_);

  std::cout << "Definitions loaded" << std::endl;
}

} // namespace Editor::Application
