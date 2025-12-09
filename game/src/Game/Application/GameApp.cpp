#include "Game/Application/GameApp.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <raylib.h>
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

  if (!IsAudioDeviceReady()) {
    InitAudioDevice();
    audio_initialized_ = true;
  } else {
    audio_initialized_ = true;
  }

  // フォント読み込み（日本語対応）
  font_manager_ = std::make_unique<Shared::Core::FontManager>();
  default_font_ =
      font_manager_->LoadJapaneseFont("assets/fonts/NotoSansJP-Medium.ttf", 32);
  owns_font_ = default_font_.texture.id != GetFontDefault().texture.id;

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

  entity_manager_->Shutdown();
  skill_manager_->Shutdown();
  stage_manager_->Shutdown();

  rendering_system_.Shutdown(registry_);
  registry_.clear();

  context_->Shutdown();

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
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT),
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
      movement_system_.Update(registry_, effective_dt);
      attack_system_.Update(registry_, effective_dt);
      skill_system_.Update(registry_, effective_dt);
      rendering_system_.UpdateAnimations(registry_, effective_dt);

      if (auto *title = dynamic_cast<Game::Scenes::TitleScene *>(scene)) {
        auto action = title->ConsumeAction();
        switch (action) {
        case Game::Scenes::TitleScene::MenuAction::Exit:
          is_running_ = false;
          return;
        case Game::Scenes::TitleScene::MenuAction::NewGame:
        case Game::Scenes::TitleScene::MenuAction::ContinueGame:
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::HomeScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT),
              Game::Scenes::SceneManager::TransitionType::FADE);
          break;
        case Game::Scenes::TitleScene::MenuAction::Settings:
        case Game::Scenes::TitleScene::MenuAction::None:
        default:
          break;
        }

        if (title->ConsumeExit()) {
          is_running_ = false;
          return;
        }
        if (title->ConsumeStart() &&
            action == Game::Scenes::TitleScene::MenuAction::None) {
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::HomeScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT),
              Game::Scenes::SceneManager::TransitionType::SLIDE_LEFT);
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
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT),
              Game::Scenes::SceneManager::TransitionType::SLIDE_LEFT);
          return;
        case Game::Scenes::HomeScene::Action::SaveAndTitle:
          QuickSaveSlot0();
          rendering_system_.Shutdown(registry_);
          registry_.clear();
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::TitleScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT),
              Game::Scenes::SceneManager::TransitionType::SLIDE_RIGHT);
          return;
        case Game::Scenes::HomeScene::Action::SaveAndExit:
          QuickSaveSlot0();
          is_running_ = false;
          return;
        case Game::Scenes::HomeScene::Action::QuitWithoutSave:
          rendering_system_.Shutdown(registry_);
          registry_.clear();
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::TitleScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT),
              Game::Scenes::SceneManager::TransitionType::SLIDE_RIGHT);
          return;
        case Game::Scenes::HomeScene::Action::None:
        default:
          break;
        }
      } else if (auto *select =
                     dynamic_cast<Game::Scenes::StageSelectScene *>(scene)) {
        if (select->ConsumeQuitRequest()) {
          rendering_system_.Shutdown(registry_);
          registry_.clear();
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::HomeScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT),
              Game::Scenes::SceneManager::TransitionType::SLIDE_RIGHT);
          return;
        }

        if (select->ShouldStartGame()) {
          std::string stage_id = select->ConsumeSelectedStage();
          if (!stage_id.empty()) {
            rendering_system_.Shutdown(registry_);
            registry_.clear();
            scene_manager_->ReplaceScene(
                std::make_unique<Game::Scenes::TDGameScene>(
                    registry_, rendering_system_, *definitions_, Settings(),
                    default_font_, stage_id),
                Game::Scenes::SceneManager::TransitionType::SLIDE_LEFT);
            return;
          }
        }
      } else if (auto *formation =
                     dynamic_cast<Game::Scenes::FormationScene *>(scene)) {
        if (formation->ConsumeReturnHome()) {
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::HomeScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT),
              Game::Scenes::SceneManager::TransitionType::SLIDE_RIGHT);
          return;
        }
      } else if (auto *td = dynamic_cast<Game::Scenes::TDGameScene *>(scene)) {
        if (td->ConsumeReturnToTitleRequest()) {
          rendering_system_.Shutdown(registry_);
          registry_.clear();
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::TitleScene>(
                  default_font_, SCREEN_WIDTH, SCREEN_HEIGHT),
              Game::Scenes::SceneManager::TransitionType::SLIDE_RIGHT);
          return;
        }
        if (td->ConsumeRetryRequest()) {
          rendering_system_.Shutdown(registry_);
          registry_.clear();
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::TDGameScene>(
                  registry_, rendering_system_, *definitions_, Settings(),
                  default_font_, td->GetCurrentStageId()),
              Game::Scenes::SceneManager::TransitionType::SLIDE_LEFT);
          return;
        }
        std::string next_stage = td->ConsumeNextStageId();
        if (!next_stage.empty()) {
          rendering_system_.Shutdown(registry_);
          registry_.clear();
          scene_manager_->ReplaceScene(
              std::make_unique<Game::Scenes::TDGameScene>(
                  registry_, rendering_system_, *definitions_, Settings(),
                  default_font_, next_stage),
              Game::Scenes::SceneManager::TransitionType::SLIDE_LEFT);
          return;
        }
        if (td->ConsumeReturnToStageSelectRequest()) {
          rendering_system_.Shutdown(registry_);
          registry_.clear();
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

  if (scene_manager_) {
    scene_manager_->Draw();
  } else {
    // シーンが無い場合の簡易描画
    DrawTextEx(default_font_, "No Scene", {50.0f, 50.0f}, 32.0f, 2.0f,
               RAYWHITE);
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

bool GameApp::QuickSaveSlot0() {
  if (!user_data_manager_) {
    std::cerr << "[GameApp] Save manager not initialized" << std::endl;
    return false;
  }

  Shared::Data::SaveData data = BuildPlaceholderSaveData();
  if (!user_data_manager_->SaveSlot(data)) {
    std::cerr << "[GameApp] Quick save failed" << std::endl;
    return false;
  }
  std::cout << "[GameApp] Quick saved to slot 0" << std::endl;
  return true;
}

Shared::Data::SaveData GameApp::BuildPlaceholderSaveData() const {
  Shared::Data::SaveData data;
  data.slot_id = 0;

  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm *utc = std::gmtime(&t);
  if (utc) {
    std::stringstream ss;
    ss << std::put_time(utc, "%FT%TZ");
    data.saved_at = ss.str();
  } else {
    data.saved_at = "";
  }

  // ひとまず空の進行状況を保存（将来の拡張用）
  data.stage_progress = {};
  data.characters.clear();
  data.gold = 0;
  data.tower.hp_level = 1;

  return data;
}

bool GameApp::LoadDefinitions() {
  std::cout << "Loading definitions..." << std::endl;

  // Entity定義
  std::string entity_path = context_->GetDataPath("entities_debug.json");
  if (!Shared::Data::EntityLoader::LoadFromJson(entity_path, *definitions_)) {
    return false;
  }

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
  skill_manager_ =
      std::make_unique<Game::Managers::SkillManager>(*context_, *definitions_);
  stage_manager_ =
      std::make_unique<Game::Managers::StageManager>(*context_, *definitions_);

  entity_manager_->Initialize();
  skill_manager_->Initialize();
  stage_manager_->Initialize();

  message = "Done.";
  return true;
}

} // namespace Game::Application
