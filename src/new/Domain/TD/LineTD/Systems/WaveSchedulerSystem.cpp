#include "new/Domain/TD/LineTD/Systems/WaveSchedulerSystem.h"

#include "Core/GameContext.h"
#include "Core/World.h"
#include "new/Domain/TD/LineTD/Components/SpawnQueue.h"
#include "new/Game/Components/GameState.h"

namespace New::Domain::TD::LineTD::Systems {

using New::Domain::TD::LineTD::Components::SpawnJob;
using New::Domain::TD::LineTD::Components::SpawnQueue;
using New::Game::Components::BattleState;

void WaveSchedulerSystem::Update(Core::GameContext &context, float deltaTime) {
  auto &world = context.GetWorld();
  auto stateView = world.View<BattleState>();
  if (stateView.empty()) {
    return;
  }
  auto &state = stateView.get<BattleState>(*stateView.begin());
  if (state.victory || state.defeat) {
    return;
  }

  auto queueView = world.View<SpawnQueue>();
  if (queueView.empty()) {
    return;
  }
  auto &queue = queueView.get<SpawnQueue>(*queueView.begin());

  waveManager_.Update(deltaTime, [&](const Managers::SpawnRequest &req) {
    queue.jobs.push_back(SpawnJob{req.enemyId, req.laneIndex});
  });
}

} // namespace New::Domain::TD::LineTD::Systems
