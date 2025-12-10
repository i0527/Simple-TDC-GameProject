#pragma once

#include <memory>

#include <entt/entt.hpp>
#include <raylib.h>
#include <imgui.h>
#include <optional>

#include "Core/FontManager.h"
#include "Core/GameContext.h"
#include "Core/SettingsManager.h"
#include "Data/DefinitionRegistry.h"
#include "Data/UserDataManager.h"
#include "Game/Managers/EntityManager.h"
#include "Game/Managers/FormationManager.h"
#include "Game/Managers/SkillManager.h"
#include "Game/Managers/StageManager.h"
#include "Game/Scenes/LoadingScene.h"
#include "Game/Scenes/SceneManager.h"
#include "Game/Scenes/TDGameScene.h"
#include "Game/Scenes/TitleScene.h"
#include "Game/Systems/AttackSystem.h"
#include "Game/Systems/MovementSystem.h"
#include "Game/Systems/RenderingSystem.h"
#include "Game/Systems/SkillSystem.h"

namespace Game::Application {

/// @brief ゲームアプリケーションのメインクラス
class GameApp {
public:
  GameApp();
  ~GameApp();

  bool Initialize();
  void Run();
  void Shutdown();

private:
  // コンテキストと定義
  std::unique_ptr<Shared::Core::GameContext> context_;
  std::unique_ptr<Shared::Data::DefinitionRegistry> definitions_;
  std::unique_ptr<Shared::Data::UserDataManager> user_data_manager_;

  // マネージャー
  std::unique_ptr<Game::Managers::EntityManager> entity_manager_;
  std::unique_ptr<Game::Managers::FormationManager> formation_manager_;
  std::unique_ptr<Game::Managers::SkillManager> skill_manager_;
  std::unique_ptr<Game::Managers::StageManager> stage_manager_;

  // ECS Registry
  entt::registry registry_;
  Game::Systems::MovementSystem movement_system_;
  Game::Systems::AttackSystem attack_system_;
  Game::Systems::SkillSystem skill_system_;
  Game::Systems::RenderingSystem rendering_system_;

  // Scene
  std::unique_ptr<Game::Scenes::SceneManager> scene_manager_;

  // ゲーム状態
  bool is_running_;
  float delta_time_;
  float render_scale_;
  int viewport_x_;
  int viewport_y_;
  int viewport_width_;
  int viewport_height_;
  bool audio_initialized_ = false;
  bool imgui_initialized_ = false;
  int current_gold_ = 0;
  std::string current_stage_id_;
  std::optional<Shared::Data::SaveData> loaded_save_;

  // フォント
  std::unique_ptr<Shared::Core::FontManager> font_manager_;
  Font default_font_;
  bool owns_font_;
  ImFont *imgui_font_ = nullptr;

  // 内部メソッド
  void Update(float delta_time);
  void Render();
  void HandleResize();
  bool LoadDefinitions(); // legacy (used via SetupGameResources)
  bool SetupGameResources(std::string &message);
  bool SaveToSlot(int slot_id);
  bool LoadFromSlot(int slot_id);
  Shared::Data::SaveData BuildSaveData(int slot_id) const;
  void ApplyLoadedSave(const Shared::Data::SaveData &data);

  Shared::Core::SettingsManager &Settings() {
    return context_->GetSettingsManager();
  }
  const Shared::Core::SettingsManager &Settings() const {
    return context_->GetSettingsManager();
  }
};

} // namespace Game::Application
