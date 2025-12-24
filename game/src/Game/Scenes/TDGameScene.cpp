#include "Game/Scenes/TDGameScene.h"
#include <algorithm>
#include <imgui.h>
#include <filesystem>
#include <unordered_map>

namespace {
constexpr float BASE_WIDTH = 110.0f;
constexpr float BASE_HEIGHT = 160.0f;
constexpr float BASE_MARGIN = 10.0f;
constexpr float UNIT_HEIGHT = 40.0f;
constexpr float UNIT_SPACING = 80.0f;
constexpr float BETWEEN_WAVE_MESSAGE_Y = 120.0f;

struct TopUiLayout {
  Rectangle pause_btn{};
  Rectangle speed_btn{};
};

struct PauseUiLayout {
  Rectangle panel{};
  Rectangle resume_btn{};
  Rectangle retry_btn{};
  Rectangle select_btn{};
  Rectangle settings_btn{};
  Rectangle settings_panel{};
};

TopUiLayout BuildTopUiLayout() {
  TopUiLayout l{};
  const float padding = 14.0f;
  const float btn_w = 60.0f;
  const float btn_h = 36.0f;
  float x = static_cast<float>(GetScreenWidth()) - btn_w - padding;
  float y = padding;
  l.pause_btn = {x, y, btn_w, btn_h};
  l.speed_btn = {x - btn_w - 10.0f, y, btn_w, btn_h};
  return l;
}

PauseUiLayout BuildPauseUiLayout(bool settings_open) {
  PauseUiLayout l{};
  float sw = static_cast<float>(GetScreenWidth());
  float sh = static_cast<float>(GetScreenHeight());

  const float panel_w = settings_open ? std::min(1080.0f, sw * 0.9f) : 520.0f;
  const float panel_h = settings_open ? 520.0f : 440.0f;
  l.panel = {(sw - panel_w) * 0.5f, (sh - panel_h) * 0.5f, panel_w, panel_h};

  if (!settings_open) {
    const float btn_w = 220.0f;
    const float btn_h = 56.0f;
    const float gap = 14.0f;
    float cx = l.panel.x + l.panel.width * 0.5f;
    float start_y = l.panel.y + 96.0f;
    l.resume_btn = {cx - btn_w * 0.5f, start_y, btn_w, btn_h};
    l.retry_btn = {cx - btn_w * 0.5f, start_y + (btn_h + gap), btn_w, btn_h};
    l.select_btn = {cx - btn_w * 0.5f, start_y + 2 * (btn_h + gap), btn_w,
                    btn_h};
    l.settings_btn = {cx - btn_w * 0.5f, start_y + 3 * (btn_h + gap), btn_w,
                      btn_h};
  } else {
    l.settings_panel = l.panel;
  }

  return l;
}
} // namespace

namespace Game::Scenes {

TDGameScene::TDGameScene(Shared::Simulation::SimulationContext &simulation,
                         Game::Systems::RenderingSystem &renderer,
                         Game::Systems::NewRenderingSystem &new_renderer,
                         Shared::Data::DefinitionRegistry &definitions,
                         Shared::Core::SettingsManager &settings,
                         const Font &font, const std::string &stage_id,
                         Game::Managers::FormationManager *formation_manager)
    : simulation_(simulation), registry_(simulation.GetRegistry()), renderer_(renderer), new_renderer_(new_renderer),
      definitions_(definitions), settings_(settings),
      formation_manager_(formation_manager), font_(font),
      current_stage_id_(stage_id) {
  ApplySettings(settings_.Data());
  entity_reload_handle_ = definitions_.OnEntityDefinitionReloaded.Connect(
      [this](const std::string &id) {
        simulation_.ReloadAllInstances(id, Shared::Simulation::SimulationContext::ReloadPolicy::PreserveState);
      });
}

TDGameScene::~TDGameScene() {
  if (entity_reload_handle_ != 0) {
    definitions_.OnEntityDefinitionReloaded.Disconnect(entity_reload_handle_);
    entity_reload_handle_ = 0;
  }
}

bool TDGameScene::ConsumeRetryRequest() {
  bool v = retry_requested_;
  retry_requested_ = false;
  return v;
}

bool TDGameScene::ConsumeReturnToTitleRequest() {
  bool v = return_title_requested_;
  return_title_requested_ = false;
  return v;
}

std::string TDGameScene::ConsumeNextStageId() {
  if (!next_stage_requested_) {
    return {};
  }
  next_stage_requested_ = false;
  return next_stage_id_;
}

bool TDGameScene::ConsumeReturnToStageSelectRequest() {
  bool v = return_stage_select_requested_;
  return_stage_select_requested_ = false;
  return v;
}

void TDGameScene::Update(float delta_time) {
  if (!initialized_) {
    PrepareStage();
    BuildWaveList();
    BuildDeckFromDefinitions();
    StartWave(0);
    initialized_ = true;
  }

  if (IsKeyPressed(KEY_F1)) {
    debug_window_open_ = !debug_window_open_;
  }

  const bool input_blocked = debug_ui_wants_input_ && debug_window_open_;

  // Pause & speed UI handling (raw delta_time)
  if (!input_blocked) {
    HandleTopUI(delta_time);
  }

  float speed = CurrentSpeedMultiplier();
  float dt = pause_overlay_open_ ? 0.0f : delta_time * speed;

  bool settings_ui_open = pause_overlay_open_ && settings_panel_.IsOpen();

  if (auto *player_base = GetBaseStats(Components::Team::Type::Player)) {
    if (player_base->current_hp <= 0) {
      defeat_ = true;
    }
  }
  if (auto *enemy_base = GetBaseStats(Components::Team::Type::Enemy)) {
    if (enemy_base->current_hp <= 0) {
      victory_ = true;
    }
  }

  if (victory_ || defeat_) {
    result_timer_ += delta_time; // 結果画面は加速しない

    if (IsKeyPressed(KEY_R)) {
      retry_requested_ = true;
    }
    if (IsKeyPressed(KEY_T)) {
      return_stage_select_requested_ = true;
    }
    if (victory_ && IsKeyPressed(KEY_N) && !next_stage_id_.empty()) {
      next_stage_requested_ = true;
    }

    // Auto transition after victory
    if (victory_ && !auto_return_triggered_ &&
        result_timer_ >= result_auto_return_seconds_) {
      if (!next_stage_id_.empty()) {
        next_stage_requested_ = true;
      } else {
        return_stage_select_requested_ = true;
      }
      auto_return_triggered_ = true;
    }

    // Mouse buttons on overlay
    Vector2 mouse = GetMousePosition();
    bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    const float button_w = 220.0f;
    const float button_h = 60.0f;
    const float button_gap = 16.0f;
    float center_x = static_cast<float>(GetScreenWidth()) * 0.5f;
    float start_y = 460.0f;

    Rectangle retry_btn{center_x - button_w - button_gap * 0.5f, start_y,
                        button_w, button_h};
    Rectangle title_btn{center_x + button_gap * 0.5f, start_y, button_w,
                        button_h};
    Rectangle next_btn{center_x - button_w * 0.5f,
                       start_y + button_h + button_gap, button_w, button_h};

    if (click) {
      if (CheckCollisionPointRec(mouse, retry_btn)) {
        retry_requested_ = true;
      } else if (CheckCollisionPointRec(mouse, title_btn)) {
        return_stage_select_requested_ = true;
      } else if (victory_ && CheckCollisionPointRec(mouse, next_btn) &&
                 !next_stage_id_.empty()) {
        next_stage_requested_ = true;
      }
    }

    return;
  }

  simulation_.Update(dt);
  UpdateResource(dt);
  UpdateDeckCooldowns(dt);

  // Input: deck selection & spawn
  if (!input_blocked && !settings_ui_open) {
    for (int i = 0; i < static_cast<int>(deck_.size()) && i < 9; ++i) {
      if (IsKeyPressed(KEY_ONE + i)) {
        selected_slot_ = i;
        TrySpawnFromDeck(i);
      }
    }
  }

  Vector2 mouse = GetMousePosition();
  bool mouse_click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  // HUD deck click to spawn (layout matches DrawDeckHud skewed grid)
  const float slot_width = 180.0f;
  const float slot_height = 72.0f;
  const float slot_gap_x = 18.0f;
  const float slot_gap_y = 18.0f;
  const int cols = 5;
  const int rows = 2;
  const float margin_bottom = 60.0f;
  const float skew = 18.0f;

  float deck_width = cols * slot_width + (cols - 1) * slot_gap_x + skew;
  float deck_height =
      rows * slot_height + (rows - 1) * slot_gap_y + skew * 0.5f;
  float start_x =
      static_cast<float>(GetScreenWidth()) * 0.5f - deck_width * 0.5f;
  float start_y =
      static_cast<float>(GetScreenHeight()) - deck_height - margin_bottom;

  if (!input_blocked && !settings_ui_open) {
    for (int i = 0; i < static_cast<int>(deck_.size()); ++i) {
      int row = i / cols;
      int col = i % cols;
      float offset_y = (row == 0) ? 0.0f : skew * 0.5f;
      Rectangle rect{start_x + col * (slot_width + slot_gap_x),
                     start_y + offset_y + row * (slot_height + slot_gap_y),
                     slot_width, slot_height};
      // クリック判定は矩形で代用
      if (CheckCollisionPointRec(mouse, rect)) {
        if (mouse_click) {
          selected_slot_ = i;
          if (!deck_[i].entity_id.empty()) {
            TrySpawnFromDeck(i);
          }
        }
      }
    }
  }

  // Wave timer
  wave_timer_ += dt;

  for (auto &ev : spawn_events_) {
    if (ev.spawned)
      continue;
    if (wave_timer_ >= ev.spawn_time) {
      SpawnEntity(ev.pos, ev.team, ev.stats, ev.velocity, ev.entity_id);
      ev.spawned = true;
    }
  }

  HandleCastleDamage();
  CheckWaveState();
  HandleWaveCompletion(dt);
  CleanupDeadEntities(dt);
}

void TDGameScene::Draw() {
  ClearBackground(DARKGRAY);

  // 背景の仮矩形（マップ代わり）
  DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(),
                         Color{18, 26, 36, 255}, Color{26, 34, 44, 255});

  // 地面レーンの簡易描画
  const float lane_height = 140.0f;
  float lane_top = lane_y_ - UNIT_HEIGHT - 40.0f;
  Rectangle lane_rect{0.0f, lane_top, static_cast<float>(GetScreenWidth()),
                      lane_height};
  DrawRectangleRec(lane_rect, Color{48, 58, 68, 255});
  DrawRectangle(0, static_cast<int>(lane_top + lane_height) - 8,
                GetScreenWidth(), 8, Color{30, 30, 30, 255});

  // 左右のベースを簡易表示（左=敵, 右=味方）
  const float base_width = BASE_WIDTH;
  const float base_height = BASE_HEIGHT;
  float base_enemy_y = lane_top + lane_height - base_height;
  float base_player_y = base_enemy_y;
  Rectangle base_enemy{BASE_MARGIN, base_enemy_y, base_width, base_height};
  Rectangle base_player{static_cast<float>(GetScreenWidth()) - base_width -
                            BASE_MARGIN,
                        base_player_y, base_width, base_height};
  DrawRectangleRounded(base_enemy, 0.12f, 4, Color{150, 90, 90, 120});
  DrawRectangleLinesEx(base_enemy, 2.0f, Color{255, 180, 180, 200});
  DrawRectangleRounded(base_player, 0.12f, 4, Color{80, 110, 150, 120});
  DrawRectangleLinesEx(base_player, 2.0f, Color{160, 200, 255, 200});

  renderer_.Draw(registry_, font_);

  // Phase 2: 新しいアニメーションシステムでの描画
  new_renderer_.DrawEntities(registry_);
  DrawDeckHud();
  DrawTopUI();

  const float size = 22.0f;
  Vector2 pos{20.0f, 90.0f};
  std::string wave_text =
      "Wave: " + std::to_string(wave_index_ + 1) + "/" +
      std::to_string(std::max(1, static_cast<int>(wave_defs_.size()))) +
      "  Time: " + std::to_string(static_cast<int>(wave_timer_));
  if (waiting_next_wave_) {
    wave_text += "  Next incoming...";
  } else if (wave_finished_) {
    wave_text += "  COMPLETE";
  }
  DrawTextEx(font_, wave_text.c_str(), pos, size, 2.0f, RAYWHITE);

  if (waiting_next_wave_) {
    std::string msg = "Preparing next wave";
    Vector2 ts = MeasureTextEx(font_, msg.c_str(), 26.0f, 2.0f);
    DrawTextEx(font_, msg.c_str(),
               {20.0f, pos.y + BETWEEN_WAVE_MESSAGE_Y * 0.3f}, 26.0f, 2.0f,
               LIGHTGRAY);
  }

  // Castle HP text (left: enemy, right: player) using base entities
  auto *enemy_base = GetBaseStats(Components::Team::Type::Enemy);
  auto *player_base = GetBaseStats(Components::Team::Type::Player);
  float hp_y = base_enemy_y - 28.0f;
  auto draw_hp_text = [&](float x, float y, Components::Stats *st) {
    if (!st)
      return;
    std::string txt = std::to_string(st->current_hp) + "/" +
                      std::to_string(std::max(1, st->max_hp));
    Vector2 ts = MeasureTextEx(font_, txt.c_str(), 20.0f, 2.0f);
    DrawTextEx(font_, txt.c_str(), {x - ts.x * 0.5f, y - ts.y * 0.5f}, 20.0f,
               2.0f, RAYWHITE);
  };
  draw_hp_text(BASE_MARGIN + BASE_WIDTH * 0.5f, hp_y, enemy_base);
  draw_hp_text(static_cast<float>(GetScreenWidth()) - BASE_MARGIN -
                   BASE_WIDTH * 0.5f,
               hp_y, player_base);

  DrawDebugWindow();

  if (victory_ || defeat_) {
    DrawResultOverlay();
    return;
  }

  if (pause_overlay_open_) {
    DrawPauseOverlay();
  }
}

void TDGameScene::SpawnInitialWave() {
  const float y = lane_y_ - UNIT_HEIGHT;

  // ベースは別途生成済み（SpawnBases）
  Components::Stats player_stats{};
  player_stats.current_hp = player_stats.max_hp = 120;
  player_stats.attack = 12;
  player_stats.attack_speed = 1.0f;
  player_stats.move_speed = 40.0f;
  Components::Velocity player_vel{-1.0f, 0.0f};

  SpawnEntity({player_spawn_x_, y}, Components::Team::Type::Player,
              player_stats, player_vel, "debug_player");
  SpawnEntity({player_spawn_x_ - 40.0f, y}, Components::Team::Type::Player,
              player_stats, player_vel, "debug_player");

  Components::Stats enemy_stats{};
  enemy_stats.current_hp = enemy_stats.max_hp = 80;
  enemy_stats.attack = 8;
  enemy_stats.attack_speed = 0.8f;
  enemy_stats.move_speed = 30.0f;
  Components::Velocity enemy_vel{1.0f, 0.0f};

  SpawnEntity({enemy_spawn_x_, y}, Components::Team::Type::Enemy, enemy_stats,
              enemy_vel, "debug_enemy");
  SpawnEntity({enemy_spawn_x_ + 50.0f, y}, Components::Team::Type::Enemy,
              enemy_stats, enemy_vel, "debug_enemy");
  SpawnEntity({enemy_spawn_x_ + 100.0f, y}, Components::Team::Type::Enemy,
              enemy_stats, enemy_vel, "debug_enemy");
}

Components::Stats
TDGameScene::BuildStatsFromDef(const Shared::Data::EntityDef *def) const {
  Components::Stats stats{};
  if (!def) {
    stats.current_hp = stats.max_hp = 80;
    stats.attack = 10;
    stats.attack_speed = 1.0f;
    stats.range = 80;
    stats.move_speed = 40.0f;
    return stats;
  }

  stats.max_hp = stats.current_hp = def->stats.hp;
  stats.attack = def->stats.attack;
  stats.attack_speed = def->stats.attack_speed;
  stats.range = def->stats.range;
  stats.move_speed = def->stats.move_speed;
  stats.knockback = def->stats.knockback;
  return stats;
}

Components::Velocity
TDGameScene::BuildVelocityFromTeam(Components::Team::Type team) const {
  Components::Velocity vel{};
  if (team == Components::Team::Type::Enemy) {
    vel.x = 1.0f; // 敵は右（プレイヤー拠点）へ
  } else {
    vel.x = -1.0f; // 味方は左（敵拠点）へ
  }
  return vel;
}

void TDGameScene::SpawnEntity(const Vector2 &pos, Components::Team::Type team,
                              const Components::Stats &stats,
                              const Components::Velocity &vel,
                              const std::string &entity_id) {
  entt::entity e = entt::null;

  if (!entity_id.empty()) {
    e = simulation_.SpawnEntity(entity_id, pos, team);
  }

  if (e == entt::null) {
    e = registry_.create();
    registry_.emplace<Components::Transform>(e, pos.x, pos.y, 1.0f, 1.0f, 0.0f, false, false);
    registry_.emplace<Components::Team>(e, team);
    registry_.emplace<Components::AttackCooldown>(e);
    registry_.emplace<Components::SkillHolder>(e);
    registry_.emplace<Components::SkillCooldown>(e);
    // Animationコンポーネントを追加（NewRenderingSystemで必要）
    registry_.emplace<Components::Animation>(e);
    // Spriteコンポーネントを追加（後方互換性のため）
    registry_.emplace<Components::Sprite>(e);
    if (!entity_id.empty()) {
      registry_.emplace<Components::EntityDefId>(e, entity_id);
    }
  }

  // 上書きパラメータ
  if (registry_.all_of<Components::Transform>(e)) {
    auto &tf = registry_.get<Components::Transform>(e);
    tf.x = pos.x;
    tf.y = pos.y;
  } else {
    registry_.emplace<Components::Transform>(e, pos.x, pos.y, 1.0f, 1.0f, 0.0f, false, false);
  }
  registry_.emplace_or_replace<Components::Velocity>(e, vel.x, vel.y);
  registry_.emplace_or_replace<Components::Stats>(e, stats);
  registry_.emplace_or_replace<Components::Team>(e, team);

  // ダメージポップ初期値（不要なら未付与）
  // ※初期付与をやめ、ダメージ適用時のみ付与
}

void TDGameScene::BuildWaveList() {
  wave_defs_.clear();

  if (current_stage_ && !current_stage_->wave_ids.empty()) {
    for (const auto &wave_id : current_stage_->wave_ids) {
      const auto *wave = definitions_.GetWave(wave_id);
      if (wave) {
        wave_defs_.push_back(wave);
      }
    }
  }

  if (wave_defs_.empty()) {
    wave_defs_ = definitions_.GetAllWaves();
    std::sort(
        wave_defs_.begin(), wave_defs_.end(),
        [](const Shared::Data::WaveDef *a, const Shared::Data::WaveDef *b) {
          return a->wave_number < b->wave_number;
        });
  }
}

void TDGameScene::StartWave(int index) {
  spawn_events_.clear();
  wave_finished_ = false;
  wave_result_ = WaveResult::None;
  waiting_next_wave_ = false;
  wait_timer_ = 0.0f;
  wave_timer_ = 0.0f;
  wave_index_ = index;

  if (!wave_defs_.empty() && index < static_cast<int>(wave_defs_.size())) {
    BuildSpawnQueueFromDefinitions(index);
    return;
  }

  SpawnInitialWave();
}

void TDGameScene::BuildSpawnQueueFromDefinitions(int index) {
  if (index >= static_cast<int>(wave_defs_.size()))
    return;

  const auto *wave = wave_defs_[index];
  float base_y = lane_y_ - UNIT_HEIGHT;

  for (const auto &grp : wave->spawn_groups) {
    const auto *ent_def = definitions_.GetEntity(grp.entity_id);
    Components::Stats stats = BuildStatsFromDef(ent_def);
    Components::Team::Type team = Components::Team::Type::Enemy;
    if (ent_def && !ent_def->is_enemy) {
      team = Components::Team::Type::Player;
    }
    Components::Velocity vel = BuildVelocityFromTeam(team);

    for (int i = 0; i < grp.count; ++i) {
      SpawnEvent ev;
      ev.spawn_time = grp.delay_from_wave_start + grp.spawn_interval * i;
      ev.team = team;
      ev.stats = stats;
      ev.velocity = vel;
      ev.entity_id = grp.entity_id;
      ev.pos = (team == Components::Team::Type::Enemy)
                   ? Vector2{enemy_spawn_x_ + 30.0f * i, base_y}
                   : Vector2{player_spawn_x_ - 30.0f * i, base_y};
      spawn_events_.push_back(ev);
    }
  }
}

void TDGameScene::PrepareStage() {
  current_stage_ = nullptr;
  if (!current_stage_id_.empty()) {
    current_stage_ = definitions_.GetStage(current_stage_id_);
  }

  if (!current_stage_) {
    auto stages = SortedStages();
    if (!stages.empty()) {
      current_stage_ = stages.front();
      current_stage_id_ = current_stage_->id;
    }
  }

  UpdateNextStageInfo();

  if (current_stage_) {
    base_arrival_damage_ = std::max(1, current_stage_->base_arrival_damage);
  } else {
    base_arrival_damage_ = 1;
  }

  // 拠点位置とスポーン位置（左=敵拠点, 右=味方拠点）
  enemy_base_x_ = BASE_MARGIN + BASE_WIDTH * 0.5f;
  player_base_x_ =
      static_cast<float>(GetScreenWidth()) - BASE_MARGIN - BASE_WIDTH * 0.5f;
  enemy_spawn_x_ = enemy_base_x_;
  player_spawn_x_ = player_base_x_;

  SpawnBases();

  // 味方ユニットを編成からスポーン
  if (formation_manager_) {
    const auto &slots = formation_manager_->GetSlots();
    for (size_t i = 0; i < slots.size(); ++i) {
      if (!slots[i].empty()) {
        float x = player_spawn_x_ - (static_cast<float>(i) - static_cast<float>(slots.size() - 1) * 0.5f) * UNIT_SPACING;
        Components::Stats stats = BuildStatsFromDef(definitions_.GetEntity(slots[i]));
        Components::Velocity vel = BuildVelocityFromTeam(Components::Team::Type::Player);
        SpawnEntity({x, lane_y_}, Components::Team::Type::Player, stats, vel, slots[i]);
      }
    }
  }
}

void TDGameScene::SpawnBases() {
  // 敵ベース（左）
  enemy_base_entity_ = registry_.create();
  registry_.emplace<Components::Transform>(enemy_base_entity_, enemy_base_x_,
                                           lane_y_ - UNIT_HEIGHT, 1.0f, 1.0f, 0.0f, false, false);
  registry_.emplace<Components::Team>(enemy_base_entity_,
                                      Components::Team::Type::Enemy);
  registry_.emplace<Components::BaseMarker>(enemy_base_entity_);
  Components::Stats enemy_base_stats{};
  enemy_base_stats.current_hp =
      current_stage_ ? current_stage_->castle_hp.enemy_castle_hp : 10000;
  enemy_base_stats.max_hp = enemy_base_stats.current_hp;
  registry_.emplace<Components::Stats>(enemy_base_entity_, enemy_base_stats);

  // 味方ベース（右）
  player_base_entity_ = registry_.create();
  registry_.emplace<Components::Transform>(player_base_entity_, player_base_x_,
                                           lane_y_ - UNIT_HEIGHT, 1.0f, 1.0f, 0.0f, false, false);
  registry_.emplace<Components::Team>(player_base_entity_,
                                      Components::Team::Type::Player);
  registry_.emplace<Components::BaseMarker>(player_base_entity_);
  Components::Stats player_base_stats{};
  player_base_stats.current_hp = 1000; // 後でセーブデータに差し替え予定
  player_base_stats.max_hp = player_base_stats.current_hp;
  registry_.emplace<Components::Stats>(player_base_entity_, player_base_stats);
}

Components::Stats *
TDGameScene::GetBaseStats(Components::Team::Type team) const {
  entt::entity e = (team == Components::Team::Type::Enemy)
                       ? enemy_base_entity_
                       : player_base_entity_;
  if (e == entt::null || !registry_.valid(e) ||
      !registry_.all_of<Components::Stats>(e)) {
    return nullptr;
  }
  return &registry_.get<Components::Stats>(e);
}

std::vector<const Shared::Data::StageDef *> TDGameScene::SortedStages() const {
  auto stages = definitions_.GetAllStages();
  std::sort(
      stages.begin(), stages.end(),
      [](const Shared::Data::StageDef *a, const Shared::Data::StageDef *b) {
        if (a->domain == b->domain) {
          return a->subdomain < b->subdomain;
        }
        return a->domain < b->domain;
      });
  return stages;
}

void TDGameScene::UpdateNextStageInfo() {
  next_stage_id_.clear();

  if (current_stage_id_.empty()) {
    return;
  }

  auto stages = SortedStages();
  for (size_t i = 0; i < stages.size(); ++i) {
    if (stages[i]->id != current_stage_id_) {
      continue;
    }
    for (size_t j = i + 1; j < stages.size(); ++j) {
      if (stages[j]->domain == stages[i]->domain) {
        next_stage_id_ = stages[j]->id;
        return;
      }
    }
    break;
  }
}

bool TDGameScene::IsTeamAlive(Components::Team::Type team) const {
  auto view = registry_.view<Components::Team, Components::Stats>();
  for (auto entity : view) {
    const auto &t = view.get<Components::Team>(entity);
    const auto &st = view.get<Components::Stats>(entity);
    if (t.type == team && st.current_hp > 0) {
      return true;
    }
  }
  return false;
}

void TDGameScene::CheckWaveState() {
  bool all_spawned = std::all_of(spawn_events_.begin(), spawn_events_.end(),
                                 [](const auto &ev) { return ev.spawned; });
  bool enemies_alive = IsTeamAlive(Components::Team::Type::Enemy);

  if (!all_spawned) {
    wave_finished_ = false;
    wave_result_ = WaveResult::None;
    return;
  }

  if (!enemies_alive) {
    wave_finished_ = true;
    wave_result_ = WaveResult::Cleared;
  }
}

void TDGameScene::HandleWaveCompletion(float delta_time) {
  if (!wave_finished_) {
    return;
  }

  if (wave_result_ == WaveResult::Failed) {
    defeat_ = true;
    return;
  }

  if (!waiting_next_wave_) {
    waiting_next_wave_ = true;
    wait_timer_ = 0.0f;
  }

  if (waiting_next_wave_) {
    wait_timer_ += delta_time;
    if (wait_timer_ >= wait_duration_) {
      int next = wave_index_ + 1;
      if (next < static_cast<int>(wave_defs_.size())) {
        StartWave(next);
      } else {
        victory_ = true;
      }
    }
  }
}

void TDGameScene::CleanupDeadEntities(float delta_time) {
  auto view = registry_.view<Components::Dead>();
  std::vector<entt::entity> to_destroy;
  for (auto entity : view) {
    auto &dead = view.get<Components::Dead>(entity);
    dead.death_timer += delta_time;
    if (dead.death_timer >= dead.death_duration) {
      to_destroy.push_back(entity);
    }
  }
  for (auto e : to_destroy) {
    registry_.destroy(e);
  }
}

void TDGameScene::BuildDeckFromDefinitions() {
  deck_.clear();

  // 編成マネージャー優先
  if (formation_manager_ != nullptr) {
    bool has_entry = false;
    const auto &slots = formation_manager_->GetSlots();
    const int max_slots_raw = formation_manager_->GetMaxSlots();
    const size_t max_slots =
        max_slots_raw > 0 ? static_cast<size_t>(max_slots_raw) : slots.size();
    const size_t deck_size = std::max(max_slots, slots.size());
    deck_.resize(deck_size);
    for (size_t i = 0; i < deck_.size(); ++i) {
      const std::string id = (i < slots.size()) ? slots[i] : "";
      DeckSlot slot;
      if (!id.empty()) {
        const auto *ent = definitions_.GetEntity(id);
        if (ent != nullptr && !ent->is_enemy) {
          slot.entity_id = ent->id;
          slot.cost = ent->cost;
          slot.cooldown = ent->cooldown;
          has_entry = true;
        }
      }
      deck_[i] = slot;
    }

    // 全て空だった場合のみフォールバックを利用
    if (has_entry) {
      selected_slot_ = 0;
      return;
    }
  }

  // デフォルトのフォールバック（友軍定義から先頭数件）
  if (deck_.empty()) {
    auto entities = definitions_.GetAllEntities();
    for (const auto *ent : entities) {
      if (ent->is_enemy)
        continue;
      DeckSlot slot;
      slot.entity_id = ent->id;
      slot.cost = ent->cost;
      slot.cooldown = ent->cooldown;
      deck_.push_back(slot);
      if (static_cast<int>(deck_.size()) >= 5) {
        break; // UIの最小枠に合わせる
      }
    }
  }

  if (deck_.empty()) {
    DeckSlot default_slot;
    default_slot.entity_id = "debug_player";
    default_slot.cost = 50;
    default_slot.cooldown = 1.0f;
    deck_.push_back(default_slot);
  }

  selected_slot_ = 0;
}

void TDGameScene::TrySpawnFromDeck(int slot_index) {
  if (slot_index < 0 || slot_index >= static_cast<int>(deck_.size())) {
    return;
  }
  auto &slot = deck_[slot_index];
  if (slot.entity_id.empty()) {
    return;
  }
  if (slot.cooldown_remaining > 0.0f) {
    return;
  }
  if (player_cost_ < slot.cost) {
    slot.cost_flash_timer = cost_flash_duration_;
    return;
  }

  const auto *def = definitions_.GetEntity(slot.entity_id);
  Components::Stats stats = BuildStatsFromDef(def);
  Components::Velocity vel =
      BuildVelocityFromTeam(Components::Team::Type::Player);
  float y = lane_y_ - UNIT_HEIGHT;

  SpawnEntity({player_spawn_x_, y}, Components::Team::Type::Player, stats, vel,
              slot.entity_id);

  player_cost_ = std::max(0, player_cost_ - slot.cost);
  slot.cooldown_remaining = slot.cooldown;
}

void TDGameScene::UpdateResource(float delta_time) {
  cost_buffer_ += cost_recovery_rate_ * delta_time;
  if (cost_buffer_ >= 1.0f) {
    int gain = static_cast<int>(cost_buffer_);
    cost_buffer_ -= static_cast<float>(gain);
    player_cost_ = std::min(player_cost_cap_, player_cost_ + gain);
  }
}

void TDGameScene::UpdateDeckCooldowns(float delta_time) {
  for (auto &slot : deck_) {
    if (slot.cost_flash_timer > 0.0f) {
      slot.cost_flash_timer =
          std::max(0.0f, slot.cost_flash_timer - delta_time);
    }
    if (slot.cooldown_remaining > 0.0f) {
      slot.cooldown_remaining =
          std::max(0.0f, slot.cooldown_remaining - delta_time);
    }
  }
}

float TDGameScene::CurrentSpeedMultiplier() const {
  if (speed_index_ < 0 ||
      speed_index_ >= static_cast<int>(speed_options_.size())) {
    return 1.0f;
  }
  return speed_options_[speed_index_];
}

float TDGameScene::GetSpeedMultiplier() const {
  return CurrentSpeedMultiplier();
}

static float Clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

void TDGameScene::ApplySettings(const Shared::Core::SettingsData &data) {
  float effective = data.masterMuted ? 0.0f : Clamp01(data.masterVolume);
  SetMasterVolume(effective);

  if (data.speedMultiplier == 2.0f) {
    speed_index_ = 1;
  } else if (data.speedMultiplier == 4.0f) {
    speed_index_ = 2;
  } else {
    speed_index_ = 0;
  }
}

void TDGameScene::DrawDeckHud() const {
  const float slot_width = 180.0f;
  const float slot_height = 72.0f;
  const float slot_gap_x = 18.0f;
  const float slot_gap_y = 18.0f;
  const int cols = 5;
  const int rows = 2;
  const float margin_bottom = 60.0f;
  const float skew = 18.0f; // 右上がりの傾き

  float deck_width = cols * slot_width + (cols - 1) * slot_gap_x + skew;
  float deck_height =
      rows * slot_height + (rows - 1) * slot_gap_y + skew * 0.5f;
  float start_x =
      static_cast<float>(GetScreenWidth()) * 0.5f - deck_width * 0.5f;
  float start_y =
      static_cast<float>(GetScreenHeight()) - deck_height - margin_bottom;

  DrawTextEx(font_, "Deck", {start_x, start_y - 30.0f}, 22.0f, 2.0f, LIGHTGRAY);

  auto draw_skewed = [&](const Rectangle &rect, bool selected, float cd_frac,
                         bool on_cooldown, bool flashing) {
    Vector2 p1{rect.x + skew, rect.y};
    Vector2 p2{rect.x + rect.width + skew, rect.y};
    Vector2 p3{rect.x + rect.width, rect.y + rect.height};
    Vector2 p4{rect.x, rect.y + rect.height};
    Vector2 poly[4] = {p1, p2, p3, p4};

    Color base = selected ? Color{60, 110, 170, 255} : Color{40, 60, 90, 220};
    if (on_cooldown) {
      base.a = 170;
    }
    Color border = Color{140, 180, 220, 255};
    if (flashing) {
      base = Color{200, 60, 60, 200};
      border = Color{255, 180, 180, 255};
    }

    DrawTriangleFan(poly, 4, base);
    DrawLineEx(p1, p2, 2.0f, border);
    DrawLineEx(p2, p3, 2.0f, border);
    DrawLineEx(p3, p4, 2.0f, border);
    DrawLineEx(p4, p1, 2.0f, border);

    if (on_cooldown) {
      // 傾斜形状のクールダウンオーバーレイ
      float h = rect.height * cd_frac;
      float ratio = (rect.height - h) / rect.height;
      Vector2 tl{p1.x + (p4.x - p1.x) * ratio, p1.y + (p4.y - p1.y) * ratio};
      Vector2 tr{p2.x + (p3.x - p2.x) * ratio, p2.y + (p3.y - p2.y) * ratio};
      Vector2 poly_cd[4] = {tl, tr, p3, p4};
      DrawTriangleFan(poly_cd, 4, Color{0, 0, 0, 90});
    }
  };

  for (int i = 0; i < static_cast<int>(deck_.size()); ++i) {
    int row = i / cols;
    int col = i % cols;
    float offset_y = (row == 0) ? 0.0f : skew * 0.5f;
    Rectangle rect{start_x + col * (slot_width + slot_gap_x),
                   start_y + offset_y + row * (slot_height + slot_gap_y),
                   slot_width, slot_height};

    bool selected = (i == selected_slot_);
    const auto &slot = deck_[i];
    bool empty = slot.entity_id.empty();
    float cd_frac =
        (slot.cooldown > 0.0f)
            ? std::clamp(slot.cooldown_remaining / slot.cooldown, 0.0f, 1.0f)
            : 0.0f;

    Color base = selected ? Color{60, 110, 170, 255} : Color{40, 60, 90, 220};
    if (empty) {
      base = Color{50, 50, 50, 200};
    } else if (slot.cooldown_remaining > 0.0f) {
      base.a = 170;
    }
    Color border = Color{140, 180, 220, 255};
    if (empty) {
      border = Color{120, 120, 120, 200};
    } else if (slot.cost_flash_timer > 0.0f) {
      float t = slot.cost_flash_timer / cost_flash_duration_;
      unsigned char flash = static_cast<unsigned char>(120 + 80 * t);
      base = Color{200, 60, 60, static_cast<unsigned char>(200)};
      border = Color{255, flash, flash, 255};
    }

    draw_skewed(rect, selected, cd_frac, slot.cooldown_remaining > 0.0f,
                slot.cost_flash_timer > 0.0f);

    std::string label =
        empty ? "未編成" : std::to_string(i + 1) + ": " + slot.entity_id;
    std::string cost = empty ? "-" : "コスト " + std::to_string(slot.cost);
    std::string cd =
        empty ? ""
              : "再使用 " +
                    std::to_string(static_cast<int>(slot.cooldown_remaining)) +
                    "s";

    float text_offset_x = 16.0f; // 右にずらす
    DrawTextEx(font_, label.c_str(), {rect.x + text_offset_x, rect.y + 8.0f},
               18.0f, 2.0f, empty ? GRAY : RAYWHITE);
    DrawTextEx(font_, cost.c_str(), {rect.x + text_offset_x, rect.y + 34.0f},
               16.0f, 2.0f, LIGHTGRAY);

    if (!empty && slot.cooldown_remaining > 0.0f) {
      DrawTextEx(font_, cd.c_str(),
                 {rect.x + text_offset_x, rect.y + rect.height - 20.0f}, 16.0f,
                 2.0f, Color{240, 200, 120, 255});
      // 簡易丸ゲージ
      Vector2 c{rect.x + rect.width - 18.0f, rect.y + 18.0f};
      DrawCircleSector(c, 14.0f, -90.0f, -90.0f + 360.0f * (1.0f - cd_frac), 24,
                       Color{240, 200, 120, 180});
      DrawCircleLinesV(c, 14.0f, Color{200, 200, 220, 200});
    }
  }

  std::string cost_text = "コスト: " + std::to_string(player_cost_) + "/" +
                          std::to_string(player_cost_cap_);
  DrawTextEx(font_, cost_text.c_str(), {start_x, start_y + deck_height + 10.0f},
             20.0f, 2.0f, RAYWHITE);

  std::string helper = "[1-9]/クリックで出撃  |  リトライでCT/コストリセット";
  DrawTextEx(font_, helper.c_str(), {start_x, start_y + deck_height + 34.0f},
             16.0f, 2.0f, LIGHTGRAY);
}

void TDGameScene::DrawResultOverlay() const {
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 180});

  const float title_size = 42.0f;
  std::string title = victory_ ? "勝利!  R: リトライ / T: ステージ選択へ戻る"
                               : "敗北... R: リトライ / T: ステージ選択";
  Color title_color = victory_ ? GREEN : RED;
  Vector2 ts = MeasureTextEx(font_, title.c_str(), title_size, 2.0f);
  Vector2 tp{(GetScreenWidth() - ts.x) * 0.5f, 300.0f};
  DrawTextEx(font_, title.c_str(), tp, title_size, 2.0f, title_color);

  std::string msg =
      victory_ ? "クリックでも選択できます / 自動でステージ選択に戻ります"
               : "クリックまたはR/Tで選択してください";
  Vector2 ms = MeasureTextEx(font_, msg.c_str(), 24.0f, 2.0f);
  Vector2 mp{(GetScreenWidth() - ms.x) * 0.5f, tp.y + 80.0f};
  DrawTextEx(font_, msg.c_str(), mp, 24.0f, 2.0f, RAYWHITE);

  const float button_w = 220.0f;
  const float button_h = 60.0f;
  const float button_gap = 16.0f;
  float center_x = static_cast<float>(GetScreenWidth()) * 0.5f;
  float start_y = mp.y + 60.0f;

  Rectangle retry_btn{center_x - button_w - button_gap * 0.5f, start_y,
                      button_w, button_h};
  Rectangle title_btn{center_x + button_gap * 0.5f, start_y, button_w,
                      button_h};
  Rectangle next_btn{center_x - button_w * 0.5f,
                     start_y + button_h + button_gap, button_w, button_h};

  Vector2 mouse = GetMousePosition();
  bool hover_retry = CheckCollisionPointRec(mouse, retry_btn);
  bool hover_title = CheckCollisionPointRec(mouse, title_btn);
  bool hover_next = CheckCollisionPointRec(mouse, next_btn);

  auto fill = [](bool hover, Color base) {
    return Color{
        static_cast<unsigned char>(std::min(255, base.r + (hover ? 20 : 0))),
        static_cast<unsigned char>(std::min(255, base.g + (hover ? 20 : 0))),
        static_cast<unsigned char>(std::min(255, base.b + (hover ? 20 : 0))),
        base.a};
  };

  DrawRectangleRounded(retry_btn, 0.16f, 6,
                       fill(hover_retry, Color{50, 90, 150, 240}));
  DrawRectangleLinesEx(retry_btn, 2.0f, Color{180, 210, 255, 255});
  DrawRectangleRounded(title_btn, 0.16f, 6,
                       fill(hover_title, Color{60, 110, 170, 240}));
  DrawRectangleLinesEx(title_btn, 2.0f, Color{180, 210, 255, 255});

  bool next_enabled = victory_ && !next_stage_id_.empty();
  Color next_base =
      next_enabled ? Color{70, 130, 180, 220} : Color{90, 90, 90, 160};
  DrawRectangleRounded(next_btn, 0.16f, 6, fill(hover_next, next_base));
  DrawRectangleLinesEx(next_btn, 2.0f, Color{180, 210, 255, 180});

  std::string retry_label = "Retry (R)";
  std::string title_label = "Stage Select (T)";
  std::string next_label = next_enabled ? "Next (N)" : "Next (なし)";
  Vector2 rl = MeasureTextEx(font_, retry_label.c_str(), 22.0f, 2.0f);
  Vector2 tl = MeasureTextEx(font_, title_label.c_str(), 22.0f, 2.0f);
  Vector2 nl = MeasureTextEx(font_, next_label.c_str(), 18.0f, 2.0f);
  DrawTextEx(font_, retry_label.c_str(),
             {retry_btn.x + (retry_btn.width - rl.x) * 0.5f,
              retry_btn.y + (retry_btn.height - rl.y) * 0.5f},
             22.0f, 2.0f, RAYWHITE);
  DrawTextEx(font_, title_label.c_str(),
             {title_btn.x + (title_btn.width - tl.x) * 0.5f,
              title_btn.y + (title_btn.height - tl.y) * 0.5f},
             22.0f, 2.0f, RAYWHITE);
  DrawTextEx(font_, next_label.c_str(),
             {next_btn.x + (next_btn.width - nl.x) * 0.5f,
              next_btn.y + (next_btn.height - nl.y) * 0.5f},
             18.0f, 2.0f, RAYWHITE);

  if (victory_) {
    std::string auto_msg;
    int remain = static_cast<int>(
        std::max(0.0f, result_auto_return_seconds_ - result_timer_));
    if (next_enabled) {
      auto_msg = "自動で次ステージへ: " + std::to_string(remain) + "s";
    } else {
      auto_msg = "自動でステージ選択に戻る: " + std::to_string(remain) + "s";
    }
    Vector2 am = MeasureTextEx(font_, auto_msg.c_str(), 18.0f, 2.0f);
    DrawTextEx(font_, auto_msg.c_str(),
               {center_x - am.x * 0.5f, next_btn.y + next_btn.height + 12.0f},
               18.0f, 2.0f, LIGHTGRAY);
  }
}

void TDGameScene::DrawTopUI() const {
  TopUiLayout ui = BuildTopUiLayout();

  auto draw_btn = [&](const Rectangle &r, const char *label, bool active) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, r);
    Color base = active ? Color{70, 120, 190, 240} : Color{50, 80, 130, 220};
    if (hover) {
      base = Color{static_cast<unsigned char>(base.r + 15),
                   static_cast<unsigned char>(base.g + 15),
                   static_cast<unsigned char>(base.b + 15), base.a};
    }
    DrawRectangleRounded(r, 0.18f, 6, base);
    DrawRectangleLinesEx(r, 2.0f, Color{180, 210, 255, 230});
    Vector2 ts = MeasureTextEx(font_, label, 18.0f, 2.0f);
    DrawTextEx(font_, label,
               {r.x + (r.width - ts.x) * 0.5f, r.y + (r.height - ts.y) * 0.5f},
               18.0f, 2.0f, RAYWHITE);
  };

  std::string speed_label =
      "x" + std::to_string(static_cast<int>(CurrentSpeedMultiplier()));
  draw_btn(ui.speed_btn, speed_label.c_str(), false);
  draw_btn(ui.pause_btn, pause_overlay_open_ ? "再開" : "ポーズ",
           pause_overlay_open_);
}

void TDGameScene::DrawPauseOverlay() const {
  float sw = static_cast<float>(GetScreenWidth());
  float sh = static_cast<float>(GetScreenHeight());

  DrawRectangle(0, 0, static_cast<int>(sw), static_cast<int>(sh),
                Color{0, 0, 0, 160});

  PauseUiLayout ui = BuildPauseUiLayout(settings_panel_.IsOpen());

  if (settings_panel_.IsOpen()) {
    DrawRectangleRounded(ui.settings_panel, 0.08f, 6, Color{20, 30, 50, 230});
    DrawRectangleLinesEx(ui.settings_panel, 2.0f, Color{180, 210, 255, 240});
    settings_panel_.Draw(ui.settings_panel, font_);
    return;
  }

  DrawRectangleRounded(ui.panel, 0.12f, 6, Color{40, 60, 90, 230});
  DrawRectangleLinesEx(ui.panel, 2.0f, Color{180, 210, 255, 240});

  const char *title = "一時停止中";
  Vector2 ts = MeasureTextEx(font_, title, 36.0f, 2.0f);
  DrawTextEx(font_, title,
             {ui.panel.x + (ui.panel.width - ts.x) * 0.5f, ui.panel.y + 28.0f},
             36.0f, 2.0f, RAYWHITE);

  auto draw_btn = [&](const Rectangle &r, const std::string &label,
                      bool disabled = false) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, r);
    Color base =
        disabled ? Color{80, 80, 80, 200}
                 : (hover ? Color{70, 120, 190, 240} : Color{50, 80, 130, 220});
    DrawRectangleRounded(r, 0.14f, 6, base);
    DrawRectangleLinesEx(r, 2.0f, Color{180, 210, 255, 230});
    Vector2 ls = MeasureTextEx(font_, label.c_str(), 22.0f, 2.0f);
    DrawTextEx(font_, label.c_str(),
               {r.x + (r.width - ls.x) * 0.5f, r.y + (r.height - ls.y) * 0.5f},
               22.0f, 2.0f, RAYWHITE);
  };

  draw_btn(ui.resume_btn, "再開");
  draw_btn(ui.retry_btn, "リトライ");
  draw_btn(ui.select_btn, "ステージ選択へ戻る");
  draw_btn(ui.settings_btn,
           settings_panel_.IsOpen() ? "設定を閉じる" : "設定", false);
}

void TDGameScene::HandleTopUI(float) {
  // 戦闘結果表示中はポーズを閉じて入力を無効化
  if (victory_ || defeat_) {
    pause_overlay_open_ = false;
    settings_panel_.Close();
    return;
  }

  TopUiLayout top = BuildTopUiLayout();

  Vector2 mouse = GetMousePosition();
  bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  bool settings_open = settings_panel_.IsOpen();

  // 設定パネル表示中は他のUI操作をブロックし、設定のみ処理
  if (pause_overlay_open_ && settings_open) {
    float sw = static_cast<float>(GetScreenWidth());
    float sh = static_cast<float>(GetScreenHeight());
    const float panel_w = std::min(1080.0f, sw * 0.9f);
    const float panel_h = 520.0f;
    Rectangle panel{(sw - panel_w) * 0.5f, (sh - panel_h) * 0.5f, panel_w,
                    panel_h};
    Game::UI::SettingsPanel::Action act = settings_panel_.Update(panel, mouse);
    if (IsKeyPressed(KEY_ESCAPE)) {
      settings_panel_.Close();
      return;
    }
    if (act == Game::UI::SettingsPanel::Action::Apply) {
      settings_.Data() = settings_panel_.Draft();
      settings_.Save(settings_path_);
      ApplySettings(settings_.Data());
      settings_panel_.Close();
    } else if (act == Game::UI::SettingsPanel::Action::Cancel) {
      settings_panel_.Close();
    }
    return;
  }

  if (click) {
    if (CheckCollisionPointRec(mouse, top.pause_btn)) {
      pause_overlay_open_ = !pause_overlay_open_;
      if (!pause_overlay_open_) {
        settings_panel_.Close();
      }
    } else if (CheckCollisionPointRec(mouse, top.speed_btn)) {
      speed_index_ =
          (speed_index_ + 1) % static_cast<int>(speed_options_.size());
    }
  }

  // キー操作でもポーズ切替
  if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_PAUSE)) {
    pause_overlay_open_ = !pause_overlay_open_;
    if (!pause_overlay_open_) {
      settings_panel_.Close();
    }
  }

  if (!pause_overlay_open_) {
    settings_panel_.Close();
    return;
  }

  // オーバーレイ内ボタン
  PauseUiLayout ui = BuildPauseUiLayout(settings_open);
  Rectangle panel = settings_open ? ui.settings_panel : ui.panel;

  if (click) {
    if (!settings_open && CheckCollisionPointRec(mouse, ui.resume_btn)) {
      pause_overlay_open_ = false;
      settings_panel_.Close();
    } else if (!settings_open && CheckCollisionPointRec(mouse, ui.retry_btn)) {
      pause_overlay_open_ = false;
      retry_requested_ = true;
      settings_panel_.Close();
    } else if (!settings_open && CheckCollisionPointRec(mouse, ui.select_btn)) {
      pause_overlay_open_ = false;
      return_stage_select_requested_ = true;
      settings_panel_.Close();
    } else if (!settings_open &&
               CheckCollisionPointRec(mouse, ui.settings_btn)) {
      if (settings_open) {
        settings_panel_.Close();
      } else {
        settings_panel_.Open(settings_.Data());
      }
    }
  }

  if (settings_panel_.IsOpen()) {
    Game::UI::SettingsPanel::Action act = settings_panel_.Update(panel, mouse);
    if (act == Game::UI::SettingsPanel::Action::Apply) {
      settings_.Data() = settings_panel_.Draft();
      settings_.Save(settings_path_);
      ApplySettings(settings_.Data());
      settings_panel_.Close();
    } else if (act == Game::UI::SettingsPanel::Action::Cancel) {
      settings_panel_.Close();
    }
  }
}

void TDGameScene::HandleCastleDamage() {
  const float enemy_base_x = enemy_base_x_;
  const float player_base_x = player_base_x_;

  auto *enemy_base = GetBaseStats(Components::Team::Type::Enemy);
  auto *player_base = GetBaseStats(Components::Team::Type::Player);
  if (!enemy_base && !player_base)
    return;

  std::vector<entt::entity> to_destroy;
  auto view = registry_.view<Components::Transform, Components::Team>();
  for (auto entity : view) {
    auto &tr = view.get<Components::Transform>(entity);
    auto &team = view.get<Components::Team>(entity);

    if (team.type == Components::Team::Type::Enemy && tr.x >= player_base_x) {
      if (player_base) {
        player_base->current_hp =
            std::max(0, player_base->current_hp - base_arrival_damage_);
      }
      to_destroy.push_back(entity);
    } else if (team.type == Components::Team::Type::Player &&
               tr.x <= enemy_base_x) {
      if (enemy_base) {
        enemy_base->current_hp =
            std::max(0, enemy_base->current_hp - base_arrival_damage_);
      }
      to_destroy.push_back(entity);
    }
  }

  for (auto e : to_destroy) {
    registry_.destroy(e);
  }
}

void TDGameScene::DrawDebugWindow() {
  debug_ui_wants_input_ = false;

  if (!debug_window_open_) {
    return;
  }

  if (ImGui::GetCurrentContext() == nullptr) {
    return;
  }

  ImGui::SetNextWindowBgAlpha(0.9f);
  bool open = ImGui::Begin(u8"デバッグ (F1で表示/非表示)", &debug_window_open_);
  if (open) {
    if (ImGui::BeginTabBar("DebugTabs")) {
      if (ImGui::BeginTabItem(u8"デッキ")) {
        DrawDeckDebugTab();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(u8"出撃中")) {
        DrawEntitiesDebugTab();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(u8"拠点")) {
        DrawBaseDebugTab();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }

  debug_ui_wants_input_ =
      ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse;
  ImGui::End();
}

void TDGameScene::DrawDeckDebugTab() {
  if (deck_.empty()) {
    ImGui::TextUnformatted(u8"デッキがありません");
    return;
  }

  constexpr ImGuiTableFlags flags =
      ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable;

  if (ImGui::BeginTable("DeckTable", 5, flags)) {
    ImGui::TableSetupColumn(u8"スロット");
    ImGui::TableSetupColumn(u8"ID");
    ImGui::TableSetupColumn(u8"コスト");
    ImGui::TableSetupColumn(u8"クールダウン");
    ImGui::TableSetupColumn(u8"ステータス");
    ImGui::TableHeadersRow();

    for (int i = 0; i < static_cast<int>(deck_.size()); ++i) {
      const auto &slot = deck_[i];
      const auto *def = definitions_.GetEntity(slot.entity_id);

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("%d%s", i + 1, (i == selected_slot_) ? "*" : "");

      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(slot.entity_id.empty() ? "-" : slot.entity_id.c_str());

      ImGui::TableSetColumnIndex(2);
      ImGui::Text("%d", slot.cost);

      ImGui::TableSetColumnIndex(3);
      if (slot.cooldown > 0.0f) {
        ImGui::Text("%.2f / %.2f", slot.cooldown_remaining, slot.cooldown);
      } else {
        ImGui::TextUnformatted("-");
      }

      ImGui::TableSetColumnIndex(4);
      if (def) {
        ImGui::Text("HP:%d  ATK:%d  SPD:%.1f", def->stats.hp, def->stats.attack,
                    def->stats.move_speed);
      } else {
        ImGui::TextUnformatted("-");
      }
    }
    ImGui::EndTable();
  }
}

void TDGameScene::DrawEntitiesDebugTab() {
  auto view =
      registry_.view<Components::Transform, Components::Stats, Components::Team>();

  constexpr ImGuiTableFlags flags =
      ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable;

  if (ImGui::BeginTable("EntityTable", 6, flags)) {
    ImGui::TableSetupColumn(u8"チーム");
    ImGui::TableSetupColumn(u8"ID");
    ImGui::TableSetupColumn(u8"HP");
    ImGui::TableSetupColumn(u8"攻撃");
    ImGui::TableSetupColumn(u8"位置");
    ImGui::TableSetupColumn(u8"選択");
    ImGui::TableHeadersRow();

    for (auto entity : view) {
      if (registry_.any_of<Components::BaseMarker>(entity)) {
        continue; // 拠点は別タブで扱う
      }

      const auto &tr = view.get<Components::Transform>(entity);
      const auto &st = view.get<Components::Stats>(entity);
      const auto &team = view.get<Components::Team>(entity);
      const auto *id = registry_.try_get<Components::EntityDefId>(entity);
      bool selected = (debug_selected_entity_ == entity);

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      const char *team_label =
          (team.type == Components::Team::Type::Player) ? u8"味方" : u8"敵";
      ImGui::TextUnformatted(team_label);

      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(id ? id->id.c_str() : "-");

      ImGui::TableSetColumnIndex(2);
      ImGui::Text("%d / %d", st.current_hp, st.max_hp);

      ImGui::TableSetColumnIndex(3);
      ImGui::Text("%d", st.attack);

      ImGui::TableSetColumnIndex(4);
      ImGui::Text("(%.1f, %.1f)", tr.x, tr.y);

      ImGui::TableSetColumnIndex(5);
      if (ImGui::Selectable("選択", selected)) {
        debug_selected_entity_ = entity;
      }
    }
    ImGui::EndTable();
  }

  if (debug_selected_entity_ != entt::null &&
      registry_.valid(debug_selected_entity_) &&
      registry_.all_of<Components::Stats, Components::Team>(debug_selected_entity_)) {
    auto &stats = registry_.get<Components::Stats>(debug_selected_entity_);
    ImGui::SeparatorText(u8"選択中エンティティ");

    int hp = stats.current_hp;
    int max_hp = stats.max_hp;
    int attack = stats.attack;
    float move_speed = stats.move_speed;

    ImGui::InputInt(u8"現在HP", &hp);
    ImGui::InputInt(u8"最大HP", &max_hp);
    ImGui::InputInt(u8"攻撃力", &attack);
    ImGui::InputFloat(u8"移動速度", &move_speed, 1.0f, 5.0f, "%.1f");

    max_hp = std::max(1, max_hp);
    hp = std::clamp(hp, 0, max_hp);
    attack = std::max(0, attack);
    move_speed = std::max(0.0f, move_speed);

    stats.max_hp = max_hp;
    stats.current_hp = hp;
    stats.attack = attack;
    stats.move_speed = move_speed;
  } else {
    ImGui::SeparatorText(u8"選択中エンティティ");
    ImGui::TextUnformatted(u8"なし");
  }
}

void TDGameScene::DrawBaseDebugTab() {
  ImGui::SeparatorText(u8"拠点ステータス");

  auto draw_base = [&](const char *label, Components::Stats *st) {
    if (!st) {
      ImGui::Text("%s: -", label);
      return;
    }
    int hp = st->current_hp;
    int max_hp = st->max_hp;
    ImGui::TextUnformatted(label);
    ImGui::InputInt("HP", &hp);
    ImGui::InputInt("Max HP", &max_hp);

    max_hp = std::max(1, max_hp);
    hp = std::clamp(hp, 0, max_hp);
    st->max_hp = max_hp;
    st->current_hp = hp;
  };

  draw_base(u8"敵拠点", GetBaseStats(Components::Team::Type::Enemy));
  draw_base(u8"味方拠点", GetBaseStats(Components::Team::Type::Player));
}

std::string TDGameScene::ResolveIconPath(const Shared::Data::EntityDef* def) const {
  if (!def) return {};
  namespace fs = std::filesystem;

  auto exists_path = [](const fs::path& p) { return !p.empty() && fs::exists(p); };

  // 1. display.iconが存在する場合はそれを使用
  if (exists_path(def->display.icon)) {
    return fs::path(def->display.icon).lexically_normal().generic_string();
  }

  // 2. atlas_textureのパスからフォルダ名を取得
  fs::path hint = def->display.icon.empty() ? fs::path(def->display.atlas_texture) : fs::path(def->display.icon);
  if (!hint.empty()) {
    fs::path folder = hint.parent_path().filename();
    std::string tier = def->type.empty() ? "main" : def->type;
    fs::path candidate = fs::path("assets/textures/icons/characters") / tier / folder / "icon.png";
    if (exists_path(candidate)) {
      return candidate.lexically_normal().generic_string();
    }
  }

  // 3. source_pathからフォルダ名を取得（フォールバック）
  if (!def->source_path.empty()) {
    fs::path src(def->source_path);
    fs::path folder = src.parent_path().filename();
    std::string tier = def->type.empty() ? "main" : def->type;
    fs::path candidate = fs::path("assets/textures/icons/characters") / tier / folder / "icon.png";
    if (exists_path(candidate)) {
      return candidate.lexically_normal().generic_string();
    }
    
    // 4. キャラクターフォルダから直接icon.pngを取得
    fs::path char_folder = fs::path("assets/characters") / tier / folder;
    fs::path icon_in_folder = char_folder / "icon.png";
    if (exists_path(icon_in_folder)) {
      return icon_in_folder.lexically_normal().generic_string();
    }
  }

  return {};
}

void TDGameScene::DrawDeckIcon(const Rectangle& rect, const std::string& entity_id) const {
  const auto* def = definitions_.GetEntity(entity_id);
  if (!def) return;

  const std::string icon_path = ResolveIconPath(def);

  // Try to draw icon first
  if (!icon_path.empty()) {
    auto it = icon_cache_.find(entity_id);
    if (it == icon_cache_.end()) {
      Texture2D tex = LoadTexture(icon_path.c_str());
      if (tex.id != 0) {
        icon_cache_[entity_id] = tex;
        it = icon_cache_.find(entity_id);
      }
    }
    if (it != icon_cache_.end()) {
      // 味方（is_enemy == false）の場合は左右反転
      Rectangle src_rect{0.0f, 0.0f, (float)it->second.width, (float)it->second.height};
      if (def && !def->is_enemy) {
        src_rect.width = -src_rect.width; // 左右反転
      }
      DrawTexturePro(it->second, 
                     src_rect, 
                     rect, 
                     {0.0f, 0.0f}, 
                     0.0f, 
                     WHITE);
      return;
    }
  }
  
  // Fallback: draw placeholder rectangle
  DrawRectangleRec(rect, Color{60, 100, 200, 180});
  DrawRectangleLinesEx(rect, 2.0f, Color{120, 170, 240, 200});
}

} // namespace Game::Scenes
