#include "Game/Scenes/TDGameScene.h"
#include <algorithm>

namespace {
constexpr float BASE_WIDTH = 110.0f;
constexpr float BASE_HEIGHT = 160.0f;
constexpr float BASE_MARGIN = 10.0f;
constexpr float UNIT_HEIGHT = 40.0f;
constexpr float BETWEEN_WAVE_MESSAGE_Y = 120.0f;
} // namespace

namespace Game::Scenes {

TDGameScene::TDGameScene(entt::registry &registry,
                         Game::Systems::RenderingSystem &renderer,
                         Shared::Data::DefinitionRegistry &definitions,
                         Shared::Core::SettingsManager &settings,
                         const Font &font, const std::string &stage_id)
    : registry_(registry), renderer_(renderer), definitions_(definitions),
      settings_(settings), font_(font), current_stage_id_(stage_id) {
  settings_draft_ = settings_.Data();
  ApplySettings(settings_draft_);
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

  // Pause & speed UI handling (raw delta_time)
  HandleTopUI(delta_time);

  float speed = CurrentSpeedMultiplier();
  float dt = pause_overlay_open_ ? 0.0f : delta_time * speed;

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

  UpdateResource(dt);
  UpdateDeckCooldowns(dt);

  // Input: deck selection & spawn
  for (int i = 0; i < static_cast<int>(deck_.size()) && i < 9; ++i) {
    if (IsKeyPressed(KEY_ONE + i)) {
      selected_slot_ = i;
      TrySpawnFromDeck(i);
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
  auto e = registry_.create();
  registry_.emplace<Components::Transform>(e, pos.x, pos.y, 0.0f);
  registry_.emplace<Components::Velocity>(e, vel.x, vel.y);
  registry_.emplace<Components::Stats>(e, stats);
  registry_.emplace<Components::Team>(e, team);
  registry_.emplace<Components::AttackCooldown>(e);
  registry_.emplace<Components::SkillHolder>(e);
  registry_.emplace<Components::SkillCooldown>(e);
  if (!entity_id.empty()) {
    registry_.emplace<Components::EntityDefId>(e, entity_id);
  }
  const Shared::Data::EntityDef *def =
      entity_id.empty() ? nullptr : definitions_.GetEntity(entity_id);
  if (def && !def->display.sprite_sheet.empty()) {
    Components::Sprite sprite{};
    sprite.texturePath = def->display.sprite_sheet;
    registry_.emplace_or_replace<Components::Sprite>(e, sprite);

    Components::Animation anim{};
    anim.columns = 4;
    anim.rows = (!def->display.walk_animation.empty() ||
                 !def->display.attack_animation.empty() ||
                 !def->display.death_animation.empty())
                    ? 4
                    : 1;
    anim.frames_per_state = 4;
    registry_.emplace_or_replace<Components::Animation>(e, anim);
  }

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
}

void TDGameScene::SpawnBases() {
  // 敵ベース（左）
  enemy_base_entity_ = registry_.create();
  registry_.emplace<Components::Transform>(enemy_base_entity_, enemy_base_x_,
                                           lane_y_ - UNIT_HEIGHT, 0.0f);
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
                                           lane_y_ - UNIT_HEIGHT, 0.0f);
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
      break; // 最小UI用に5枠まで
    }
  }

  if (deck_.empty()) {
    DeckSlot default_slot;
    default_slot.entity_id = "debug_player";
    default_slot.cost = 50;
    default_slot.cooldown = 1.0f;
    deck_.push_back(default_slot);
  }
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
  float effective = Clamp01(data.masterVolume);
  if (data.bgmMuted && data.sfxMuted) {
    effective = 0.0f;
  }
  SetMasterVolume(effective);
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
  const float padding = 14.0f;
  const float btn_w = 60.0f;
  const float btn_h = 36.0f;
  float x = static_cast<float>(GetScreenWidth()) - btn_w - padding;
  float y = padding;

  Rectangle pause_btn{x, y, btn_w, btn_h};
  Rectangle speed_btn{x - btn_w - 10.0f, y, btn_w, btn_h};

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
  draw_btn(speed_btn, speed_label.c_str(), false);
  draw_btn(pause_btn, pause_overlay_open_ ? "再開" : "ポーズ",
           pause_overlay_open_);
}

void TDGameScene::DrawPauseOverlay() const {
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 160});

  const float panel_w = 520.0f;
  const float panel_h = 440.0f;
  Rectangle panel{(GetScreenWidth() - panel_w) * 0.5f,
                  (GetScreenHeight() - panel_h) * 0.5f, panel_w, panel_h};
  DrawRectangleRounded(panel, 0.12f, 6, Color{40, 60, 90, 230});
  DrawRectangleLinesEx(panel, 2.0f, Color{180, 210, 255, 240});

  const char *title = "一時停止中";
  Vector2 ts = MeasureTextEx(font_, title, 36.0f, 2.0f);
  DrawTextEx(font_, title,
             {panel.x + (panel.width - ts.x) * 0.5f, panel.y + 28.0f}, 36.0f,
             2.0f, RAYWHITE);

  const float btn_w = 220.0f;
  const float btn_h = 56.0f;
  const float gap = 14.0f;
  float cx = panel.x + panel.width * 0.5f;
  float start_y = panel.y + 96.0f;

  Rectangle resume_btn{cx - btn_w * 0.5f, start_y, btn_w, btn_h};
  Rectangle retry_btn{cx - btn_w * 0.5f, start_y + (btn_h + gap), btn_w, btn_h};
  Rectangle select_btn{cx - btn_w * 0.5f, start_y + 2 * (btn_h + gap), btn_w,
                       btn_h};
  Rectangle settings_btn{cx - btn_w * 0.5f, start_y + 3 * (btn_h + gap), btn_w,
                         btn_h};

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

  draw_btn(resume_btn, "再開");
  draw_btn(retry_btn, "リトライ");
  draw_btn(select_btn, "ステージ選択へ戻る");
  draw_btn(settings_btn, settings_panel_open_ ? "設定を閉じる" : "設定", false);

  if (settings_panel_open_) {
    float sx = panel.x + 40.0f;
    float sy = start_y + 4 * (btn_h + gap) + 10.0f;
    float slider_w = panel.width - 80.0f;

    auto draw_toggle = [&](const Rectangle &r, const char *label, bool on) {
      DrawRectangleRounded(
          r, 0.12f, 6, on ? Color{80, 140, 200, 230} : Color{70, 70, 90, 200});
      DrawRectangleLinesEx(r, 2.0f, Color{180, 210, 255, 230});
      Vector2 ls = MeasureTextEx(font_, label, 20.0f, 2.0f);
      DrawTextEx(font_, label, {r.x + 10.0f, r.y + (r.height - ls.y) * 0.5f},
                 20.0f, 2.0f, RAYWHITE);
    };

    DrawTextEx(font_, "マスター音量", {sx, sy - 6.0f}, 18.0f, 2.0f, LIGHTGRAY);
    Rectangle slider{sx, sy + 10.0f, slider_w, 12.0f};
    DrawRectangleRec(slider, Color{60, 80, 110, 200});
    float knob_x =
        slider.x + slider.width * Clamp01(settings_draft_.masterVolume);
    DrawRectangleRec({slider.x, slider.y, knob_x - slider.x, slider.height},
                     Color{120, 180, 240, 200});
    DrawCircleV({knob_x, slider.y + slider.height * 0.5f}, 10.0f,
                Color{200, 220, 255, 230});

    float toggle_y = slider.y + 32.0f;
    Rectangle bgm_toggle{sx, toggle_y, 160.0f, 34.0f};
    Rectangle sfx_toggle{sx + 180.0f, toggle_y, 160.0f, 34.0f};
    draw_toggle(bgm_toggle,
                settings_draft_.bgmMuted ? "BGMミュート ON" : "BGMミュート OFF",
                settings_draft_.bgmMuted);
    draw_toggle(sfx_toggle,
                settings_draft_.sfxMuted ? "SFXミュート ON" : "SFXミュート OFF",
                settings_draft_.sfxMuted);

    float select_y = toggle_y + 46.0f;
    auto draw_select = [&](const Rectangle &r, const std::string &label,
                           const std::string &value) {
      DrawRectangleRounded(r, 0.12f, 6, Color{60, 90, 130, 220});
      DrawRectangleLinesEx(r, 2.0f, Color{180, 210, 255, 230});
      Vector2 ls = MeasureTextEx(font_, label.c_str(), 18.0f, 2.0f);
      Vector2 vs = MeasureTextEx(font_, value.c_str(), 20.0f, 2.0f);
      DrawTextEx(font_, label.c_str(),
                 {r.x + 10.0f, r.y + (r.height - ls.y) * 0.5f}, 18.0f, 2.0f,
                 LIGHTGRAY);
      DrawTextEx(font_, value.c_str(),
                 {r.x + r.width - vs.x - 12.0f, r.y + (r.height - vs.y) * 0.5f},
                 20.0f, 2.0f, RAYWHITE);
    };

    Rectangle lang_sel{sx, select_y, 220.0f, 40.0f};
    Rectangle qual_sel{sx, select_y + 48.0f, 220.0f, 40.0f};
    Rectangle win_sel{sx, select_y + 96.0f, 220.0f, 40.0f};
    draw_select(lang_sel, "言語", settings_draft_.language);
    draw_select(qual_sel, "画質", settings_draft_.quality);
    draw_select(win_sel, "ウィンドウ", settings_draft_.windowMode);

    Rectangle apply_btn{panel.x + panel.width - 200.0f,
                        panel.y + panel.height - 70.0f, 90.0f, 44.0f};
    Rectangle cancel_btn{panel.x + panel.width - 100.0f,
                         panel.y + panel.height - 70.0f, 90.0f, 44.0f};
    draw_btn(apply_btn, "保存");
    draw_btn(cancel_btn, "戻る");
  }
}

void TDGameScene::HandleTopUI(float) {
  // 戦闘結果表示中はポーズを閉じて入力を無効化
  if (victory_ || defeat_) {
    pause_overlay_open_ = false;
    return;
  }

  const float padding = 14.0f;
  const float btn_w = 60.0f;
  const float btn_h = 36.0f;
  float x = static_cast<float>(GetScreenWidth()) - btn_w - padding;
  float y = padding;

  Rectangle pause_btn{x, y, btn_w, btn_h};
  Rectangle speed_btn{x - btn_w - 10.0f, y, btn_w, btn_h};

  Vector2 mouse = GetMousePosition();
  bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  if (click) {
    if (CheckCollisionPointRec(mouse, pause_btn)) {
      pause_overlay_open_ = !pause_overlay_open_;
    } else if (CheckCollisionPointRec(mouse, speed_btn)) {
      speed_index_ =
          (speed_index_ + 1) % static_cast<int>(speed_options_.size());
    }
  }

  // キー操作でもポーズ切替
  if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_PAUSE)) {
    pause_overlay_open_ = !pause_overlay_open_;
  }

  if (!pause_overlay_open_) {
    return;
  }

  // オーバーレイ内ボタン
  const float panel_w = 520.0f;
  const float panel_h = 360.0f;
  Rectangle panel{(GetScreenWidth() - panel_w) * 0.5f,
                  (GetScreenHeight() - panel_h) * 0.5f, panel_w, panel_h};
  const float btn_overlay_w = 220.0f;
  const float btn_overlay_h = 56.0f;
  const float gap = 14.0f;
  float cx = panel.x + panel.width * 0.5f;
  float start_y = panel.y + 96.0f;

  Rectangle resume_btn{cx - btn_overlay_w * 0.5f, start_y, btn_overlay_w,
                       btn_overlay_h};
  Rectangle retry_btn{cx - btn_overlay_w * 0.5f,
                      start_y + (btn_overlay_h + gap), btn_overlay_w,
                      btn_overlay_h};
  Rectangle select_btn{cx - btn_overlay_w * 0.5f,
                       start_y + 2 * (btn_overlay_h + gap), btn_overlay_w,
                       btn_overlay_h};
  Rectangle settings_btn{cx - btn_overlay_w * 0.5f,
                         start_y + 3 * (btn_overlay_h + gap), btn_overlay_w,
                         btn_overlay_h};

  auto clamp_settings = [&](Shared::Core::SettingsData &d) {
    d.masterVolume = Clamp01(d.masterVolume);
    if (d.language.empty()) {
      d.language = "ja";
    }
    if (d.quality != "low" && d.quality != "medium" && d.quality != "high") {
      d.quality = "high";
    }
    if (d.windowMode != "window" && d.windowMode != "fullscreen") {
      d.windowMode = "window";
    }
  };

  if (click) {
    if (CheckCollisionPointRec(mouse, resume_btn)) {
      pause_overlay_open_ = false;
    } else if (CheckCollisionPointRec(mouse, retry_btn)) {
      pause_overlay_open_ = false;
      retry_requested_ = true;
    } else if (CheckCollisionPointRec(mouse, select_btn)) {
      pause_overlay_open_ = false;
      return_stage_select_requested_ = true;
    } else if (CheckCollisionPointRec(mouse, settings_btn)) {
      settings_panel_open_ = !settings_panel_open_;
      if (settings_panel_open_) {
        settings_draft_ = settings_.Data();
      }
    }
  }

  if (!settings_panel_open_) {
    return;
  }

  float sx = panel.x + 40.0f;
  float sy = start_y + 4 * (btn_overlay_h + gap) + 10.0f;
  float slider_w = panel.width - 80.0f;
  Rectangle slider{sx, sy + 10.0f, slider_w, 12.0f};
  Rectangle bgm_toggle{sx, slider.y + 32.0f, 160.0f, 34.0f};
  Rectangle sfx_toggle{sx + 180.0f, slider.y + 32.0f, 160.0f, 34.0f};
  Rectangle lang_sel{sx, slider.y + 78.0f, 220.0f, 40.0f};
  Rectangle qual_sel{sx, slider.y + 126.0f, 220.0f, 40.0f};
  Rectangle win_sel{sx, slider.y + 174.0f, 220.0f, 40.0f};
  Rectangle apply_btn{panel.x + panel.width - 200.0f,
                      panel.y + panel.height - 70.0f, 90.0f, 44.0f};
  Rectangle cancel_btn{panel.x + panel.width - 100.0f,
                       panel.y + panel.height - 70.0f, 90.0f, 44.0f};

  if (click) {
    if (CheckCollisionPointRec(mouse, slider)) {
      float ratio = std::clamp((mouse.x - slider.x) / slider.width, 0.0f, 1.0f);
      settings_draft_.masterVolume = ratio;
    } else if (CheckCollisionPointRec(mouse, bgm_toggle)) {
      settings_draft_.bgmMuted = !settings_draft_.bgmMuted;
    } else if (CheckCollisionPointRec(mouse, sfx_toggle)) {
      settings_draft_.sfxMuted = !settings_draft_.sfxMuted;
    } else if (CheckCollisionPointRec(mouse, lang_sel)) {
      settings_draft_.language =
          (settings_draft_.language == "ja") ? "en" : "ja";
    } else if (CheckCollisionPointRec(mouse, qual_sel)) {
      if (settings_draft_.quality == "low")
        settings_draft_.quality = "medium";
      else if (settings_draft_.quality == "medium")
        settings_draft_.quality = "high";
      else
        settings_draft_.quality = "low";
    } else if (CheckCollisionPointRec(mouse, win_sel)) {
      settings_draft_.windowMode =
          (settings_draft_.windowMode == "window") ? "fullscreen" : "window";
    } else if (CheckCollisionPointRec(mouse, apply_btn)) {
      clamp_settings(settings_draft_);
      settings_.Data() = settings_draft_;
      settings_.Save(settings_path_);
      ApplySettings(settings_.Data());
      settings_panel_open_ = false;
    } else if (CheckCollisionPointRec(mouse, cancel_btn)) {
      settings_panel_open_ = false;
      settings_draft_ = settings_.Data();
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

} // namespace Game::Scenes
