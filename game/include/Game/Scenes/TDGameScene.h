#pragma once

#include <array>
#include <entt/entt.hpp>
#include <raylib.h>
#include <string>
#include <vector>

#include "Core/SettingsManager.h"
#include "Data/DefinitionRegistry.h"
#include "Game/Scenes/IScene.h"
#include "Game/Systems/RenderingSystem.h"


namespace Game::Scenes {

class TDGameScene : public IScene {
public:
  TDGameScene(entt::registry &registry,
              Game::Systems::RenderingSystem &renderer,
              Shared::Data::DefinitionRegistry &definitions,
              Shared::Core::SettingsManager &settings, const Font &font,
              const std::string &stage_id);

  void Update(float delta_time) override;
  void Draw() override;

  bool ConsumeRetryRequest();
  bool ConsumeReturnToTitleRequest();
  std::string ConsumeNextStageId();
  bool ConsumeReturnToStageSelectRequest();
  std::string GetCurrentStageId() const { return current_stage_id_; }
  bool IsVictory() const { return victory_; }
  bool IsDefeat() const { return defeat_; }
  bool IsPaused() const { return pause_overlay_open_; }
  float GetSpeedMultiplier() const;

private:
  struct SpawnEvent {
    float spawn_time = 0.0f;
    Vector2 pos{};
    Components::Team::Type team = Components::Team::Type::Enemy;
    Components::Stats stats{};
    Components::Velocity velocity{};
    std::string entity_id;
    bool spawned = false;
  };

  struct DeckSlot {
    std::string entity_id;
    int cost = 0;
    float cooldown = 0.0f;
    float cooldown_remaining = 0.0f;
    float cost_flash_timer = 0.0f;
  };

  enum class WaveResult { None, Cleared, Failed };

  void SpawnInitialWave();
  void SpawnEntity(const Vector2 &pos, Components::Team::Type team,
                   const Components::Stats &stats,
                   const Components::Velocity &vel,
                   const std::string &entity_id = {});
  void BuildWaveList();
  void StartWave(int index);
  void BuildSpawnQueueFromDefinitions(int index);
  void PrepareStage();
  void UpdateNextStageInfo();
  std::vector<const Shared::Data::StageDef *> SortedStages() const;
  bool IsTeamAlive(Components::Team::Type team) const;
  void CheckWaveState();
  void HandleWaveCompletion(float delta_time);
  void CleanupDeadEntities(float delta_time);

  void BuildDeckFromDefinitions();
  void TrySpawnFromDeck(int slot_index);
  Components::Stats BuildStatsFromDef(const Shared::Data::EntityDef *def) const;
  Components::Velocity BuildVelocityFromTeam(Components::Team::Type team) const;
  void UpdateResource(float delta_time);
  void UpdateDeckCooldowns(float delta_time);
  void DrawDeckHud() const;
  void DrawResultOverlay() const;
  void DrawTopUI() const;
  void DrawPauseOverlay() const;
  void HandleTopUI(float raw_delta_time);
  float CurrentSpeedMultiplier() const;
  void ApplySettings(const Shared::Core::SettingsData &data);
  void HandleCastleDamage();
  void SpawnBases();
  Components::Stats *GetBaseStats(Components::Team::Type team) const;

  entt::registry &registry_;
  Game::Systems::RenderingSystem &renderer_;
  Shared::Data::DefinitionRegistry &definitions_;
  Shared::Core::SettingsManager &settings_;
  Font font_;
  std::string current_stage_id_;
  const Shared::Data::StageDef *current_stage_ = nullptr;
  std::string next_stage_id_;
  float wave_timer_ = 0.0f;
  int wave_index_ = 0;
  bool initialized_ = false;
  bool wave_finished_ = false;
  WaveResult wave_result_ = WaveResult::None;
  bool waiting_next_wave_ = false;
  float wait_timer_ = 0.0f;
  float wait_duration_ = 1.5f;
  std::vector<SpawnEvent> spawn_events_;
  float lane_y_ = 760.0f; // 単一レーンのY座標（下寄せ）
  std::vector<const Shared::Data::WaveDef *> wave_defs_;

  // Deck / resources
  std::vector<DeckSlot> deck_;
  int selected_slot_ = 0;
  int player_cost_ = 200;
  int player_cost_cap_ = 400;
  float cost_recovery_rate_ = 20.0f;
  float cost_buffer_ = 0.0f;

  bool victory_ = false;
  bool defeat_ = false;
  bool retry_requested_ = false;
  bool return_title_requested_ = false;
  bool next_stage_requested_ = false;
  bool return_stage_select_requested_ = false;
  float result_timer_ = 0.0f;
  float result_auto_return_seconds_ = 3.0f;
  bool auto_return_triggered_ = false;

  // Deck feedback
  float cost_flash_duration_ = 0.6f;

  // Bases / spawn positions
  float enemy_base_x_ = 0.0f;
  float player_base_x_ = 0.0f;
  float player_spawn_x_ = 0.0f;
  float enemy_spawn_x_ = 0.0f;
  entt::entity enemy_base_entity_{entt::null};
  entt::entity player_base_entity_{entt::null};
  int base_arrival_damage_ = 1;

  // Pause / speed control
  bool pause_overlay_open_ = false;
  int speed_index_ = 0; // 0:x1,1:x2,2:x4
  std::array<float, 3> speed_options_{1.0f, 2.0f, 4.0f};

  bool settings_panel_open_ = false;
  Shared::Core::SettingsData settings_draft_{};
  std::string settings_path_ = "saves/settings.json";
};

} // namespace Game::Scenes
