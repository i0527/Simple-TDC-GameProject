#include "new/Domain/TD/LineTD/Systems/BaseArrivalSystem.h"

#include "Core/GameContext.h"
#include "Core/World.h"
#include "new/Domain/TD/LineTD/Components/BaseArrivalQueue.h"
#include "new/Domain/TD/LineTD/Components/LineTDComponents.h"
#include "new/Game/Components/GameState.h"

namespace New::Domain::TD::LineTD::Systems {

using New::Domain::TD::LineTD::Components::BaseArrivalQueue;
using New::Domain::TD::LineTD::Components::TeamSide;
using New::Game::Components::BattleState;

void BaseArrivalSystem::Update(Core::GameContext &context,
                               float /*deltaTime*/) {
  auto &world = context.GetWorld();
  auto stateView = world.View<BattleState>();
  if (stateView.empty()) {
    return;
  }
  auto &state = stateView.get<BattleState>(*stateView.begin());
  if (state.victory || state.defeat) {
    return;
  }

  auto queueView = world.View<BaseArrivalQueue>();
  if (queueView.empty()) {
    return;
  }
  auto &queue = queueView.get<BaseArrivalQueue>(*queueView.begin());
  if (queue.events.empty()) {
    return;
  }

  for (const auto &evt : queue.events) {
    const int dmg =
        std::max(0, evt.damage > 0 ? evt.damage : state.baseArrivalDamage);
    if (evt.side == TeamSide::Enemy && dmg > 0) {
      state.playerLife = std::max(0, state.playerLife - dmg);
      if (state.playerLife == 0) {
        state.defeat = true;
      }
    }
    if (evt.entity != entt::null && world.IsValid(evt.entity)) {
      world.DestroyEntity(evt.entity);
    }
  }
  queue.events.clear();
}

} // namespace New::Domain::TD::LineTD::Systems
