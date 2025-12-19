#include "Editor/Application/EditorApp.h"
#include <algorithm>
#include <imgui.h>
#include <iostream>
#include <raylib.h>
#include <rlImGui.h>

#include "Data/Loaders/AbilityLoader.h"
#include "Data/Loaders/EntityLoader.h"
#include "Data/Loaders/SkillLoader.h"
#include "Data/Loaders/StageLoader.h"
#include "Data/Loaders/WaveLoader.h"
#include "Data/Validators/DataValidator.h"
#include "Editor/Windows/PreviewWindow.h"
#include "Editor/Windows/PropertyPanel.h"

namespace {
constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;
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
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple TDC Editor");
  SetWindowMinSize(SCREEN_WIDTH, SCREEN_HEIGHT);
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
  // メインメニューバー
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("ファイル")) {
      if (ImGui::MenuItem("終了")) {
        is_running_ = false;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("ウィンドウ")) {
      ImGui::MenuItem("エンティティエディタ");
      ImGui::MenuItem("スキルエディタ");
      ImGui::MenuItem("ステージエディタ");
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

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
  windows_.push_back(std::make_unique<Editor::Windows::PreviewWindow>());
  windows_.push_back(std::make_unique<Editor::Windows::PropertyPanel>());

  for (auto &w : windows_) {
    if (w) {
      w->Initialize(*context_, *definitions_);
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
  std::string entity_path = context_->GetDataPath("entities_debug.json");
  Shared::Data::EntityLoader::LoadFromJson(entity_path, *definitions_);

  // Ability定義
  std::string ability_path = context_->GetDataPath("abilities_debug.json");
  Shared::Data::AbilityLoader::LoadFromJson(ability_path, *definitions_);

  // Stage定義
  std::string stage_path = context_->GetDataPath("stages_debug.json");
  Shared::Data::StageLoader::LoadFromJson(stage_path, *definitions_);

  // Wave定義
  std::string wave_path = context_->GetDataPath("waves_debug.json");
  Shared::Data::WaveLoader::LoadFromJson(wave_path, *definitions_);

  std::cout << "Definitions loaded" << std::endl;
}

} // namespace Editor::Application
