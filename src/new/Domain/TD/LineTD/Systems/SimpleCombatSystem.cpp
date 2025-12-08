#include "new/Domain/TD/LineTD/Systems/SimpleCombatSystem.h"

#include <cmath>
#include <limits>

#include "Core/GameContext.h"
#include "Core/World.h"
#include "new/Domain/TD/LineTD/Components/AttackEventQueue.h"
#include "new/Game/Components/UnitPreset.h"

namespace New::Domain::TD::LineTD::Systems {

using New::Core::Components::Health;
using New::Core::Components::Transform;
using New::Domain::TD::LineTD::Components::AttackEvent;
using New::Domain::TD::LineTD::Components::AttackEventQueue;
using New::Domain::TD::LineTD::Components::CombatStats;
using New::Domain::TD::LineTD::Components::Lane;
using New::Domain::TD::LineTD::Components::LaneMovement;
using New::Domain::TD::LineTD::Components::Team;
using New::Domain::TD::LineTD::Components::TeamSide;
using New::Game::Components::BattleState;

void SimpleCombatSystem::Update(Core::GameContext &context, float deltaTime) {
  auto &world = context.GetWorld();
  auto attackers =
      world.View<CombatStats, Team, Transform, Lane, LaneMovement>();
  auto targets = world.View<Health, Team, Transform, Lane>();
  auto stateView = world.View<BattleState>();
  BattleState *state = stateView.empty()
                           ? nullptr
                           : &stateView.get<BattleState>(*stateView.begin());
  auto queueView = world.View<AttackEventQueue>();
  AttackEventQueue *queue =
      queueView.empty() ? nullptr
                        : &queueView.get<AttackEventQueue>(*queueView.begin());
  if (!queue) {
    return;
  }

  for (auto attackerEntity : attackers) {
    auto &stats = attackers.get<CombatStats>(attackerEntity);
    auto &attackerTeam = attackers.get<Team>(attackerEntity);
    auto &attackerTransform = attackers.get<Transform>(attackerEntity);

    if (state && (state->victory || state->defeat)) {
      continue;
    }

    stats.attackTimer += deltaTime;
    if (stats.attackTimer < stats.attackCooldown) {
      continue;
    }

    // 同レーン内の敵を収集
    std::vector<std::pair<float, entt::entity>> candidates;
    const int laneIndex = attackers.get<Lane>(attackerEntity).laneIndex;
    for (auto targetEntity : targets) {
      if (targetEntity == attackerEntity) {
        continue;
      }
      auto &targetTeam = targets.get<Team>(targetEntity);
      if (targetTeam.side == attackerTeam.side) {
        continue;
      }
      auto &targetTransform = targets.get<Transform>(targetEntity);
      auto &targetLane = targets.get<Lane>(targetEntity);
      if (targetLane.laneIndex != laneIndex) {
        continue;
      }
      const float dx = targetTransform.x - attackerTransform.x;
      const float dy = targetTransform.y - attackerTransform.y;
      const float dist = std::sqrt(dx * dx + dy * dy);
      if (dist <= stats.attackRange) {
        candidates.emplace_back(dist, targetEntity);
      }
    }
    if (candidates.empty()) {
      continue;
    }
    std::sort(candidates.begin(), candidates.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });

    // 攻撃タイプとヒット数を決定
    int hitCount = 1;
    std::string attackType = "single";
    if (world.Has<New::Game::Components::UnitPreset>(attackerEntity)) {
      const auto &preset =
          world.Get<New::Game::Components::UnitPreset>(attackerEntity);
      hitCount = std::max(1, preset.hitCount);
      attackType = preset.attackType;
    }
    if (attackType != "multi" && attackType != "pierce") {
      hitCount = 1;
    }
    const int hits =
        std::min<int>(hitCount, static_cast<int>(candidates.size()));

    const bool movingRight =
        attackers.get<LaneMovement>(attackerEntity).movingRight;
    for (int i = 0; i < hits; ++i) {
      const entt::entity target = candidates[i].second;
      float knockback = movingRight ? 12.0f : -12.0f;
      if (world.Has<New::Game::Components::UnitPreset>(attackerEntity)) {
        const auto &preset =
            world.Get<New::Game::Components::UnitPreset>(attackerEntity);
        knockback = movingRight ? preset.knockback : -preset.knockback;
      }
      if (state && state->debugKnockback > 0.0f) {
        knockback =
            movingRight ? state->debugKnockback : -state->debugKnockback;
      }
      queue->events.push_back(
          AttackEvent{attackerEntity, target, stats.attackDamage, knockback});
    }

    stats.attackTimer = 0.0f;
  }
}

} // namespace New::Domain::TD::LineTD::Systems
