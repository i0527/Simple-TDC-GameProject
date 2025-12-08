#include "new/Domain/TD/LineTD/Systems/PlacementSystem.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "Core/GameContext.h"
#include "Core/World.h"
#include "new/Core/Components/CoreComponents.h"
#include "new/Game/Components/GameState.h"
#include "new/Game/Components/Placement.h"

namespace New::Domain::TD::LineTD::Systems {

using New::Core::Components::Health;
using New::Core::Components::Sprite;
using New::Core::Components::Transform;
using New::Domain::TD::LineTD::Components::CombatStats;
using New::Domain::TD::LineTD::Components::Lane;
using New::Domain::TD::LineTD::Components::LaneMovement;
using New::Domain::TD::LineTD::Components::Team;
using New::Domain::TD::LineTD::Components::TeamSide;
using New::Game::Components::BattleState;

PlacementSystem::PlacementSystem(
    const New::Data::StageDef *stage,
    std::array<New::Game::Components::UnitPreset, 10> deckSlots)
    : stage_(stage), deckSlots_(deckSlots) {}

void PlacementSystem::Update(Core::GameContext &context, float deltaTime) {
  if (!stage_) {
    return;
  }

  auto &world = context.GetWorld();
  auto stateView = world.View<BattleState>();
  if (stateView.empty()) {
    return;
  }
  auto &state = stateView.get<BattleState>(*stateView.begin());
  if (state.victory || state.defeat) {
    return;
  }

  // JSONで指定されたレーン高さを使用（未定義時のみ中央へフォールバック）
  const float laneYDefault = context.GetRenderer().GetVirtualHeight() * 0.5f;

  auto placementView = world.View<New::Game::Components::PlacementRequest>();
  if (placementView.empty()) {
    return;
  }
  auto &placement = placementView.get<New::Game::Components::PlacementRequest>(
      *placementView.begin());
  if (!placement.valid) {
    return;
  }

  const int slot = std::clamp(state.selectedSlot, 0, 9);
  const auto &preset = deckSlots_[slot];

  // 最寄りレーン探索
  if (stage_->lanes.empty()) {
    state.lastMessage = "No lane found";
    state.messageTtl = 1.5f;
    placement.valid = false;
    return;
  }
  const auto &laneDef = stage_->lanes.front();
  const float laneY = laneDef.y != 0.0f ? laneDef.y : laneYDefault;

  // クールダウン / コストチェック
  if (state.slotCooldowns[slot] > 0.0f) {
    state.lastMessage = TextFormat("Slot %d cooling (%.1fs)", slot + 1,
                                   state.slotCooldowns[slot]);
    state.messageTtl = 1.2f;
    placement.valid = false;
    return;
  }
  if (state.cost < preset.cost) {
    state.lastMessage = "Not enough cost!";
    state.messageTtl = 1.5f;
    placement.valid = false;
    return;
  }

  // 召喚数制限チェック（メインスロットのみ）
  if (state.slotSummonLimits[slot] > 0) {
    if (state.slotSummonCounts[slot] >= state.slotSummonLimits[slot]) {
      state.lastMessage = TextFormat("Slot %d limit reached (%d/%d)", slot + 1,
                                     state.slotSummonCounts[slot],
                                     state.slotSummonLimits[slot]);
      state.messageTtl = 1.5f;
      placement.valid = false;
      return;
    }
  }

  // ブロック判定（終端付近の味方と重なりを防ぐ）
  const float minDistance = 48.0f;
  bool blocked = false;
  auto allyView = world.View<Team, Lane, Transform>();
  for (auto e : allyView) {
    const auto &team = allyView.get<Team>(e);
    if (team.side != TeamSide::Player) {
      continue;
    }
    const auto &lane = allyView.get<Lane>(e);
    if (lane.laneIndex != laneDef.index) {
      continue;
    }
    const auto &tr = allyView.get<Transform>(e);
    if (std::abs(tr.x - laneDef.endX) < minDistance) {
      blocked = true;
      break;
    }
  }
  if (blocked) {
    state.lastMessage = "Lane blocked: too close to ally";
    state.messageTtl = 1.5f;
    placement.valid = false;
    return;
  }

  // 生成
  const float spawnOffset = 32.0f;
  const float spawnX = std::max(laneDef.startX, laneDef.endX - spawnOffset);
  const float spawnY = laneY;

  const entt::entity player =
      world.CreateEntity("player_unit_" + std::to_string(rand()));
  world.Add<Transform>(player, Transform{spawnX, spawnY});
  world.Add<Sprite>(player, Sprite{64.0f, 64.0f, SKYBLUE});
  world.Add<Health>(player, Health{static_cast<int>(preset.health),
                                   static_cast<int>(preset.health)});
  world.Add<Team>(player, Team{TeamSide::Player});
  world.Add<CombatStats>(player,
                         CombatStats{preset.attackDamage, preset.attackRange,
                                     preset.attackCooldown, 0.0f});
  // UnitPresetを付与して攻撃タイプ/Knockbackを参照可能にする
  world.Add<New::Game::Components::UnitPreset>(
      player,
      New::Game::Components::UnitPreset{
          preset.cost, preset.health, preset.attackDamage, preset.attackRange,
          preset.attackCooldown, preset.spawnCooldown, preset.knockback,
          preset.attackType, preset.hitCount});
  // スロットIDを付与
  world.Add<New::Game::Components::SlotId>(player,
                                           New::Game::Components::SlotId{slot});
  world.Add<Lane>(player,
                  Lane{laneDef.index, laneY, laneDef.startX, laneDef.endX});
  world.Add<LaneMovement>(player, LaneMovement{laneDef.index, 140.0f, spawnX,
                                               /*movingRight*/ false});

  // 消費とクールダウン
  state.cost -= preset.cost;
  state.slotCooldowns[slot] = preset.spawnCooldown;
  // 召喚数カウント増加（メインスロットのみ）
  if (state.slotSummonLimits[slot] > 0) {
    state.slotSummonCounts[slot]++;
  }
  state.lastMessage = TextFormat("Placed slot %d (-%d)", slot + 1, preset.cost);
  state.messageTtl = 1.5f;
  placement.valid = false;
}

} // namespace New::Domain::TD::LineTD::Systems
