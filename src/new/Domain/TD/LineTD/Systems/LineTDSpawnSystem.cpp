#include "new/Domain/TD/LineTD/Systems/LineTDSpawnSystem.h"

#include <algorithm>
#include <cstdlib>

#include "Core/GameContext.h"
#include "Core/World.h"
#include "Data/DefinitionRegistry.h"
#include "new/Game/Components/GameState.h"

namespace New::Domain::TD::LineTD::Systems {

using New::Core::Components::Health;
using New::Core::Components::Sprite;
using New::Core::Components::Transform;
using New::Domain::TD::LineTD::Components::CombatStats;
using New::Domain::TD::LineTD::Components::Lane;
using New::Domain::TD::LineTD::Components::LaneMovement;
using New::Domain::TD::LineTD::Components::Team;
using New::Domain::TD::LineTD::Components::TeamSide;

void LineTDSpawnSystem::Update(Core::GameContext &context,
                               float /*deltaTime*/) {
  auto &world = context.GetWorld();
  const float laneYDefault = context.GetRenderer().GetVirtualHeight() * 0.5f;
  auto resolveLane = [&](int /*laneIndex*/) -> New::Data::LaneDef {
    if (!stage_.lanes.empty()) {
      // 1レーン前提：常に最初のレーンを使用
      return stage_.lanes.front();
    }
    // レーン定義が無い場合のフォールバック
    return New::Data::LaneDef{
        0, laneYDefault, 0.0f,
        static_cast<float>(context.GetRenderer().GetVirtualWidth())};
  };
  auto stateView = world.View<New::Game::Components::BattleState>();
  if (stateView.empty()) {
    return;
  }
  auto &state =
      stateView.get<New::Game::Components::BattleState>(*stateView.begin());
  if (state.victory || state.defeat) {
    return;
  }

  auto queueView =
      world.View<New::Domain::TD::LineTD::Components::SpawnQueue>();
  if (queueView.empty()) {
    return;
  }
  auto &queue = queueView.get<New::Domain::TD::LineTD::Components::SpawnQueue>(
      *queueView.begin());
  if (queue.jobs.empty()) {
    return;
  }

  for (const auto &req : queue.jobs) {
    const New::Data::LaneDef laneDef = resolveLane(req.laneIndex);

    const float spawnOffset = 32.0f;
    const float spawnX =
        std::min(laneDef.endX - spawnOffset, laneDef.startX + spawnOffset);
    const float unifiedY = laneDef.y;

    const entt::entity e = world.CreateEntity("enemy_" + req.enemyId + "_" +
                                              std::to_string(rand()));
    world.Add<Transform>(e, Transform{spawnX, unifiedY});
    world.Add<Sprite>(e, Sprite{64.0f, 64.0f, ORANGE});
    world.Add<Health>(e, Health{80, 80});
    world.Add<Team>(e, Team{TeamSide::Enemy});
    world.Add<Lane>(
        e, Lane{laneDef.index, unifiedY, laneDef.startX, laneDef.endX});
    world.Add<LaneMovement>(e,
                            LaneMovement{laneDef.index, 140.0f, spawnX, true});
    world.Add<CombatStats>(e, CombatStats{10.0f, 200.0f, 1.2f, 0.0f});

    const auto *def = definitions_.GetEntity(req.enemyId);
    if (def) {
      auto &health = world.Get<Health>(e);
      health.current = def->health;
      health.max = def->health;
    }
    state.waveIndex = std::min(state.waveIndex, state.totalWaves - 1);
  }

  queue.jobs.clear();
}

} // namespace New::Domain::TD::LineTD::Systems
