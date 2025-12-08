#include "new/Game/Systems/PlacementSystem.h"

#include <algorithm>
#include <cmath>

#include "Core/GameContext.h"
#include "Core/World.h"
#include "new/Core/Components/CoreComponents.h"
#include "new/Domain/TD/LineTD/Components/LineTDComponents.h"
#include "new/Game/Components/GameState.h"
#include "new/Game/Components/Placement.h"
#include "new/Game/Components/UnitPreset.h"
#include "raylib.h"

namespace New::Game::Systems {

using New::Core::Components::Health;
using New::Core::Components::Sprite;
using New::Core::Components::Transform;
using New::Domain::TD::LineTD::Components::CombatStats;
using New::Domain::TD::LineTD::Components::Lane;
using New::Domain::TD::LineTD::Components::LaneMovement;
using New::Domain::TD::LineTD::Components::Team;
using New::Domain::TD::LineTD::Components::TeamSide;
using New::Game::Components::BattleState;
using New::Game::Components::PlacementRequest;

PlacementSystem::PlacementSystem(const New::Data::StageDef *stage,
                                 const std::array<UnitPreset, 10> &deckSlots)
    : stage_(stage), deckSlots_(deckSlots) {}

static void SetMessage(BattleState &state, const std::string &text,
                       float ttl = 1.5f) {
  state.lastMessage = text;
  state.messageTtl = ttl;
}

void PlacementSystem::Update(Core::GameContext &context, float /*deltaTime*/) {
  if (!stage_) {
    return;
  }

  auto &world = context.GetWorld();
  auto stateView = world.View<BattleState, PlacementRequest>();
  if (stateView.begin() == stateView.end()) {
    return;
  }

  const auto entity = *stateView.begin();
  auto &state = stateView.get<BattleState>(entity);
  auto &request = stateView.get<PlacementRequest>(entity);
  if (!request.valid) {
    return;
  }
  request.valid = false;

  if (state.victory || state.defeat) {
    return;
  }

  const int slot = std::clamp(state.selectedSlot, 0, 9);
  const UnitPreset &preset = deckSlots_[slot];
  if (state.slotCooldowns[slot] > 0.0f) {
    SetMessage(state,
               TextFormat("Slot %d cooling (%.1fs)", slot + 1,
                          state.slotCooldowns[slot]),
               1.2f);
    return;
  }
  if (state.cost < preset.cost) {
    SetMessage(state, "Not enough cost!");
    return;
  }
  // 召喚数上限チェック（Main スロット等）
  if (state.slotSummonLimits[slot] > 0 &&
      state.slotSummonCounts[slot] >= state.slotSummonLimits[slot]) {
    SetMessage(state, "Summon limit reached");
    return;
  }

  // 1レーン前提: ステージ定義の先頭を使う（無い場合は中央にフォールバック）
  const bool hasLane = !stage_->lanes.empty();
  const auto laneY = hasLane ? stage_->lanes.front().y
                             : context.GetRenderer().GetVirtualHeight() * 0.5f;
  const auto laneStartX = hasLane ? stage_->lanes.front().startX : 0.0f;
  const auto laneEndX =
      hasLane ? stage_->lanes.front().endX
              : static_cast<float>(context.GetRenderer().GetVirtualWidth());
  const int laneIdx = hasLane ? stage_->lanes.front().index : 0;

  // 右端付近の重なりを簡易チェック
  const float spawnOffset = 32.0f;
  // 敵と同じ側（startX付近）から出るように揃える
  const float spawnX =
      std::min(laneEndX - spawnOffset, laneStartX + spawnOffset);
  const float spawnY = laneY;

  const float minDistance = 48.0f;
  bool blocked = false;
  auto allyView = world.View<Team, Lane, Transform>();
  for (auto e : allyView) {
    const auto &team = allyView.get<Team>(e);
    if (team.side != TeamSide::Player) {
      continue;
    }
    const auto &lane = allyView.get<Lane>(e);
    if (lane.laneIndex != laneIdx) {
      continue;
    }
    const auto &tr = allyView.get<Transform>(e);
    if (std::abs(tr.x - laneEndX) < minDistance) {
      blocked = true;
      break;
    }
  }
  if (blocked) {
    SetMessage(state, "Lane blocked: too close to ally");
    return;
  }

  // スポーン
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
  world.Add<Lane>(player, Lane{laneIdx, laneY, laneStartX, laneEndX});
  world.Add<LaneMovement>(player, LaneMovement{laneIdx, 140.0f, spawnX, false});
  world.Add<New::Game::Components::SlotId>(player,
                                           New::Game::Components::SlotId{slot});
  world.Add<New::Game::Components::UnitPreset>(player, preset);

  state.cost -= preset.cost;
  state.slotCooldowns[slot] = preset.spawnCooldown;
  if (state.slotSummonLimits[slot] > 0) {
    state.slotSummonCounts[slot] = std::min(state.slotSummonLimits[slot],
                                            state.slotSummonCounts[slot] + 1);
  }
  SetMessage(state, TextFormat("Placed slot %d (-%d)", slot + 1, preset.cost));
}

} // namespace New::Game::Systems
