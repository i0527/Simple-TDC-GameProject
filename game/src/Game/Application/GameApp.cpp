#include "Game/Application/GameApp.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <imgui.h>
#include <iomanip>
#include <iostream>
#include <raylib.h>
#include <rlImGui.h>
#include <sstream>

#include "Data/Loaders/AbilityLoader.h"
#include "Data/Loaders/EntityLoader.h"
#include "Data/Loaders/SkillLoader.h"
#include "Data/Loaders/StageLoader.h"
#include "Data/Loaders/WaveLoader.h"
#include "Data/UserDataManager.h"
#include "Data/Validators/DataValidator.h"
#include "Game/Scenes/FormationScene.h"
#include "Game/Scenes/HomeScene.h"
#include "Game/Scenes/LoadingScene.h"
#include "Game/Scenes/StageSelectScene.h"

namespace {
constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;
constexpr float TARGET_FPS = 60.0f;
} // namespace

namespace Game::Application {

GameApp::GameApp()
    : is_running_(false), delta_time_(0.0f), render_scale_(1.0f),
      viewport_x_(0), viewport_y_(0), viewport_width_(SCREEN_WIDTH),
      viewport_height_(SCREEN_HEIGHT), default_font_{}, owns_font_(false) {}

GameApp::~GameApp() { Shutdown(); }

bool GameApp::Initialize() {
  std::cout << "=== GameApp Initialization ===" << std::endl;

  // GameContext初期化
  context_ = std::make_unique<Shared::Core::GameContext>();
  if (!context_->Initialize("assets/config.json")) {
    std::cerr << "Failed to initialize GameContext" << std::endl;
    return false;
  }

  // DefinitionRegistryはローディングシーン内でロード
  definitions_ = std::make_unique<Shared::Data::DefinitionRegistry>();
  context_->BindDefinitions(definitions_.get());
  registry_ = &context_->GetSimulation().GetRegistry();
  user_data_manager_ = std::make_unique<Shared::Data::UserDataManager>();
  if (!user_data_manager_->Initialize("saves")) {
    std::cerr << "[GameApp] Failed to initialize save directory" << std::endl;
  }

  // Raylib初期化
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple TDC Game");
  SetWindowMinSize(SCREEN_WIDTH, SCREEN_HEIGHT);
  SetTargetFPS(static_cast<int>(TARGET_FPS));
  // ESC終了のデフォルト挙動を無効化（シーン側で管理）
  SetExitKey(KEY_NULL);

  rlImGuiSetup(true);
  imgui_initialized_ = true;

  if (!IsAudioDeviceReady()) {
    InitAudioDevice();
    audio_initialized_ = true;
  } else {
    audio_initialized_ = true;
  }
  bgm_service_ = std::make_unique<Game::Audio::BgmService>(&Settings());

  // フォント読み込み（日本語対応）
  font_manager_ = std::make_unique<Shared::Core::FontManager>();
  default_font_ =
      font_manager_->LoadJapaneseFont("assets/fonts/NotoSansJP-Medium.ttf", 32);
  owns_font_ = default_font_.texture.id != GetFontDefault().texture.id;
  ImGuiIO &io = ImGui::GetIO();
  imgui_font_ = font_manager_->LoadImGuiJapaneseFont(
      "assets/fonts/NotoSansJP-Medium.ttf", 18.0f);
  if (imgui_font_ != nullptr) {
    io.FontDefault = imgui_font_;
  }

  // SceneManager 初期化
  scene_manager_ = std::make_unique<Game::Scenes::SceneManager>();

  // ローディングシーンを先に表示
  auto load_task = [this](std::string &msg) -> bool {
    return SetupGameResources(msg);
  };
  scene_manager_->PushScene(
      std::make_unique<Game::Scenes::LoadingScene>(default_font_, SCREEN_WIDTH,
                                                   SCREEN_HEIGHT, load_task),
      Game::Scenes::SceneManager::TransitionType::IMMEDIATE);

  is_running_ = true;
  std::cout << "=== GameApp Initialized ===" << std::endl;
  return true;
}

void GameApp::Run() {
  while (is_running_ && !WindowShouldClose()) {
    delta_time_ = GetFrameTime();
    HandleResize();

    // FileWatcherの変更チェック
    context_->GetFileWatcher().CheckChanges();

    Update(delta_time_);
    Render();
  }
}

void GameApp::Shutdown() {
  if (!is_running_)
    return;

  std::cout << "=== GameApp Shutdown ===" << std::endl;

  if (imgui_initialized_) {
    rlImGuiShutdown();
    imgui_initialized_ = false;
  }

  entity_manager_->Shutdown();
  skill_manager_->Shutdown();
  stage_manager_->Shutdown();

  rendering_system_.Shutdown(*registry_);
  registry_->clear();

  context_->Shutdown();
  if (bgm_service_) {
    bgm_service_->Stop();
  }

  if (owns_font_ && default_font_.texture.id != 0) {
    UnloadFont(default_font_);
  }

  if (audio_initialized_) {
    CloseAudioDevice();
    audio_initialized_ = false;
  }

  CloseWindow();

  is_running_ = false;
  std::cout << "=== GameApp Shutdown Complete ===" << std::endl;
}

void GameApp::Update(float delta_time) {
  if (scene_manager_) {
    scene_manager_->Update(delta_time);

    if (auto *scene = scene_manager_->GetCurrentScene()) {
      if (auto *loading = dynamic_cast<Game::Scenes::LoadingScene *>(scene)) {
        if (loading->IsDone()) {
          if (loading->HasError()) {
            std::cerr << "[GameApp] Loading failed: "
                      << loading->GetStatusMessage() << std::endl;
            is_running_ = false;
            return;
          }
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::TitleScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT, &Settings(),
                  user_data_manager_.get(), bgm_service_.get()),
              Game::Scenes::SceneManager::TransitionType::FADE);
          return;
        }
        // ローディング中はゲーム更新をスキップ
        return;
      }

      // ローディング完了後のみゲームロジックを進める
      float effective_dt = delta_time;
      if (auto *td_scene = dynamic_cast<Game::Scenes::TDGameScene *>(scene)) {
        float speed = td_scene->GetSpeedMultiplier();
        bool paused = td_scene->IsPaused();
        effective_dt = paused ? 0.0f : delta_time * speed;
      }
      movement_system_.Update(*registry_, effective_dt);
      attack_system_.Update(*registry_, effective_dt);
      skill_system_.Update(*registry_, effective_dt);
      animation_system_.Update(*registry_, effective_dt);
      rendering_system_.UpdateAnimations(*registry_, effective_dt);

      if (auto *title = dynamic_cast<Game::Scenes::TitleScene *>(scene)) {
        auto action = title->ConsumeAction();
        switch (action) {
        case Game::Scenes::TitleScene::MenuAction::Exit:
          is_running_ = false;
          return;
        case Game::Scenes::TitleScene::MenuAction::NewGame:
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::HomeScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT,
                  user_data_manager_.get(), &Settings(), bgm_service_.get()),
              Game::Scenes::SceneManager::TransitionType::FADE);
          break;
        case Game::Scenes::TitleScene::MenuAction::ContinueGame:
          // スロット選択パネルでロード処理を行うためここでは遷移しない
          break;
        case Game::Scenes::TitleScene::MenuAction::Settings:
        case Game::Scenes::TitleScene::MenuAction::None:
        default:
          break;
        }

        int load_slot = title->ConsumeRequestedLoadSlot();
        if (load_slot >= 0) {
          bool ok = LoadFromSlot(load_slot);
          title->SetInfoMessage(ok ? "ロードしました" : "ロードに失敗しました",
                                2.0f);
          if (ok) {
            scene_manager_->ReplaceScene(
                std::make_unique<Game::Scenes::HomeScene>(
                    default_font_, SCREEN_WIDTH, SCREEN_HEIGHT,
                    user_data_manager_.get(), &Settings(), bgm_service_.get()),
                Game::Scenes::SceneManager::TransitionType::FADE);
            return;
          }
        }

        if (title->ConsumeExit()) {
          is_running_ = false;
          return;
        }
      } else if (auto *home = dynamic_cast<Game::Scenes::HomeScene *>(scene)) {
        auto action = home->ConsumeAction();
        switch (action) {
        case Game::Scenes::HomeScene::Action::StageSelect:
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::StageSelectScene>(*definitions_,
                                                               default_font_),
              Game::Scenes::SceneManager::TransitionType::SLIDE_LEFT);
          return;
        case Game::Scenes::HomeScene::Action::Formation:
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::FormationScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT, *definitions_,
                  *formation_manager_),
              Game::Scenes::SceneManager::TransitionType::SLIDE_LEFT);
          return;
        case Game::Scenes::HomeScene::Action::SaveAndTitle:
          SaveToSlot(0);
          rendering_system_.Shutdown(*registry_);
          registry_->clear();
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::TitleScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT, &Settings(),
                  user_data_manager_.get(), bgm_service_.get()),
              Game::Scenes::SceneManager::TransitionType::SLIDE_RIGHT);
          return;
        case Game::Scenes::HomeScene::Action::SaveAndExit:
          SaveToSlot(0);
          is_running_ = false;
          return;
        case Game::Scenes::HomeScene::Action::QuitWithoutSave:
          rendering_system_.Shutdown(*registry_);
          registry_->clear();
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::TitleScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT, &Settings(),
                  user_data_manager_.get(), bgm_service_.get()),
              Game::Scenes::SceneManager::TransitionType::SLIDE_RIGHT);
          return;
        case Game::Scenes::HomeScene::Action::None:
        case Game::Scenes::HomeScene::Action::Settings:
        default:
          break;
        }

        int save_slot = home->ConsumeRequestedSaveSlot();
        if (save_slot >= 0) {
          bool ok = SaveToSlot(save_slot);
          home->SetInfoMessage(ok ? "Saved" : "Save failed");
        }
        int load_slot = home->ConsumeRequestedLoadSlot();
        if (load_slot >= 0) {
          bool ok = LoadFromSlot(load_slot);
          home->SetInfoMessage(ok ? "Loaded" : "Load failed");
        }
      } else if (auto *select =
                     dynamic_cast<Game::Scenes::StageSelectScene *>(scene)) {
        if (select->ConsumeQuitRequest()) {
          rendering_system_.Shutdown(*registry_);
          registry_->clear();
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::HomeScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT,
                  user_data_manager_.get(), &Settings(), bgm_service_.get()),
              Game::Scenes::SceneManager::TransitionType::SLIDE_RIGHT);
          return;
        }

        if (select->ShouldStartGame()) {
          std::string stage_id = select->ConsumeSelectedStage();
          if (!stage_id.empty()) {
            current_stage_id_ = stage_id;
            rendering_system_.Shutdown(*registry_);
            registry_->clear();
            scene_manager_->ReplaceScene(
                std::make_unique<Game::Scenes::TDGameScene>(
                    context_->GetSimulation(), rendering_system_, new_rendering_system_,
                    *definitions_, Settings(), default_font_, stage_id,
                    formation_manager_.get()),
                Game::Scenes::SceneManager::TransitionType::SLIDE_LEFT);
            return;
          }
        }
      } else if (auto *formation =
                     dynamic_cast<Game::Scenes::FormationScene *>(scene)) {
        if (formation->ConsumeReturnHome()) {
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::HomeScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT,
                  user_data_manager_.get(), &Settings(), bgm_service_.get()),
              Game::Scenes::SceneManager::TransitionType::SLIDE_RIGHT);
          return;
        }
      } else if (auto *td = dynamic_cast<Game::Scenes::TDGameScene *>(scene)) {
        if (td->ConsumeReturnToTitleRequest()) {
          rendering_system_.Shutdown(*registry_);
          registry_->clear();
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::TitleScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT, &Settings(),
                  user_data_manager_.get(), bgm_service_.get()),
              Game::Scenes::SceneManager::TransitionType::SLIDE_RIGHT);
          return;
        }
        if (td->ConsumeRetryRequest()) {
          rendering_system_.Shutdown(*registry_);
          registry_->clear();
          current_stage_id_ = td->GetCurrentStageId();
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::TDGameScene>(
                  context_->GetSimulation(), rendering_system_, new_rendering_system_,
                  *definitions_, Settings(), default_font_, td->GetCurrentStageId(),
                  formation_manager_.get()),
              Game::Scenes::SceneManager::TransitionType::SLIDE_LEFT);
          return;
        }
        std::string next_stage = td->ConsumeNextStageId();
        if (!next_stage.empty()) {
          rendering_system_.Shutdown(*registry_);
          registry_->clear();
          current_stage_id_ = next_stage;
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::TDGameScene>(
                  context_->GetSimulation(), rendering_system_, new_rendering_system_,
                  *definitions_, Settings(), default_font_, next_stage,
                  formation_manager_.get()),
              Game::Scenes::SceneManager::TransitionType::SLIDE_LEFT);
          return;
        }
        if (td->ConsumeReturnToStageSelectRequest()) {
          rendering_system_.Shutdown(*registry_);
          registry_->clear();
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::StageSelectScene>(*definitions_,
                                                               default_font_),
              Game::Scenes::SceneManager::TransitionType::SLIDE_RIGHT);
          return;
        }
      }
    }
  }
}

void GameApp::Render() {
  BeginDrawing();
  ClearBackground(BLACK);

  if (imgui_initialized_) {
    rlImGuiBegin();
  }

  if (scene_manager_) {
    scene_manager_->Draw();
  } else {
    // シーンが無い場合の簡易描画
    DrawTextEx(default_font_, "No Scene", {50.0f, 50.0f}, 32.0f, 2.0f,
               RAYWHITE);
  }

  if (imgui_initialized_) {
    rlImGuiEnd();
  }

  EndDrawing();
}

void GameApp::HandleResize() {
  int current_width = GetScreenWidth();
  int current_height = GetScreenHeight();

  float scale_x =
      static_cast<float>(current_width) / static_cast<float>(SCREEN_WIDTH);
  float scale_y =
      static_cast<float>(current_height) / static_cast<float>(SCREEN_HEIGHT);

  render_scale_ = std::min(scale_x, scale_y);
  viewport_width_ = static_cast<int>(SCREEN_WIDTH * render_scale_);
  viewport_height_ = static_cast<int>(SCREEN_HEIGHT * render_scale_);
  viewport_x_ = (current_width - viewport_width_) / 2;
  viewport_y_ = (current_height - viewport_height_) / 2;
}

bool GameApp::SaveToSlot(int slot_id) {
  if (!user_data_manager_) {
    std::cerr << "[GameApp] Save manager not initialized" << std::endl;
    return false;
  }
  Shared::Data::SaveData data = BuildSaveData(slot_id);
  if (!user_data_manager_->SaveSlot(data)) {
    std::cerr << "[GameApp] Save failed for slot " << slot_id << std::endl;
    return false;
  }
  std::cout << "[GameApp] Saved slot " << slot_id << std::endl;
  return true;
}

bool GameApp::LoadFromSlot(int slot_id) {
  if (!user_data_manager_) {
    std::cerr << "[GameApp] Save manager not initialized" << std::endl;
    return false;
  }
  Shared::Data::SaveData data;
  if (!user_data_manager_->LoadSlot(slot_id, data)) {
    std::cerr << "[GameApp] Load failed for slot " << slot_id << std::endl;
    return false;
  }
  ApplyLoadedSave(data);
  loaded_save_ = data;
  std::cout << "[GameApp] Loaded slot " << slot_id << std::endl;
  return true;
}

Shared::Data::SaveData GameApp::BuildSaveData(int slot_id) const {
  Shared::Data::SaveData data;
  data.slot_id = slot_id;

  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm utc{};
  bool has_utc = false;
#if defined(_WIN32)
  has_utc = gmtime_s(&utc, &t) == 0;
#else
  has_utc = gmtime_r(&t, &utc) != nullptr;
#endif
  if (has_utc) {
    std::stringstream ss;
    ss << std::put_time(&utc, "%FT%TZ");
    data.saved_at = ss.str();
  } else {
    data.saved_at = "";
  }

  data.stage_progress.current_stage_id = current_stage_id_;
  data.stage_progress.cleared_stage_ids = {};
  if (formation_manager_) {
    formation_manager_->ExportForSave(data.formation_slots,
                                      data.formation_unlocked_ids);
  }
  data.gold = current_gold_;
  data.tower.hp_level = 1;
  data.settings = Settings().Data();

  return data;
}

void GameApp::ApplyLoadedSave(const Shared::Data::SaveData &data) {
  current_stage_id_ = data.stage_progress.current_stage_id;
  current_gold_ = data.gold;
  if (formation_manager_) {
    formation_manager_->SetSlots(data.formation_slots);
    formation_manager_->SetUnlocked(data.formation_unlocked_ids);
  }
  Settings().Data() = data.settings;
  Settings().Save("saves/settings.json");
}

bool GameApp::LoadDefinitions() {
  std::cout << "Loading definitions..." << std::endl;

  // main/sub キャラディレクトリを追加読み込み（Aseprite相対パスを解決）
  const auto &main_chars = context_->GetMainCharactersPath();
  const auto &sub_chars = context_->GetSubCharactersPath();
  Shared::Data::EntityLoader::LoadFromDirectory(main_chars, *definitions_,
                                                true);
  Shared::Data::EntityLoader::LoadFromDirectory(sub_chars, *definitions_, true);

  // Skill定義（abilities_debug.jsonから）
  std::string ability_path = context_->GetDataPath("abilities_debug.json");
  if (!Shared::Data::AbilityLoader::LoadFromJson(ability_path, *definitions_)) {
    return false;
  }

  // Stage定義
  std::string stage_path = context_->GetDataPath("stages_debug.json");
  if (!Shared::Data::StageLoader::LoadFromJson(stage_path, *definitions_)) {
    return false;
  }
  // 追加ステージ定義（多量スポーン検証用）
  std::string stage_path2 = context_->GetDataPath("stages_debug2.json");
  Shared::Data::StageLoader::LoadFromJson(stage_path2, *definitions_);

  // Wave定義
  std::string wave_path = context_->GetDataPath("waves_debug.json");
  if (!Shared::Data::WaveLoader::LoadFromJson(wave_path, *definitions_)) {
    return false;
  }
  // 追加Wave定義（多量スポーン検証用）
  std::string wave_path2 = context_->GetDataPath("waves_debug2.json");
  Shared::Data::WaveLoader::LoadFromJson(wave_path2, *definitions_);

  std::cout << "Definitions loaded" << std::endl;
  return true;
}

bool GameApp::SetupGameResources(std::string &message) {
  message = "Loading definitions...";
  if (!LoadDefinitions()) {
    message = "Failed to load definitions";
    return false;
  }

  // バリデーション
  if (!Shared::Data::DataValidator::Validate(*definitions_)) {
    std::cerr << "Validation failed" << std::endl;
    // 重大なエラー扱いにはしないがログ出力
  }

  message = "Initializing managers...";
  entity_manager_ =
      std::make_unique<Game::Managers::EntityManager>(*context_, *definitions_);
  formation_manager_ = std::make_unique<Game::Managers::FormationManager>(
      *context_, *definitions_);
  skill_manager_ =
      std::make_unique<Game::Managers::SkillManager>(*context_, *definitions_);
  stage_manager_ =
      std::make_unique<Game::Managers::StageManager>(*context_, *definitions_);

  entity_manager_->Initialize();
  // FormationManager はデータ読み込みのみで初期化
  skill_manager_->Initialize();
  stage_manager_->Initialize();

  message = "Done.";
  return true;
}

} // namespace Game::Application
