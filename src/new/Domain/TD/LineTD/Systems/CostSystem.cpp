#include "new/Domain/TD/LineTD/Systems/CostSystem.h"

#include <algorithm>

#include "Core/GameContext.h"
#include "Core/World.h"
#include "new/Game/Components/GameState.h"

namespace New::Domain::TD::LineTD::Systems {

using New::Game::Components::BattleState;

void CostSystem::Update(Core::GameContext &context, float deltaTime) {
  auto &world = context.GetWorld();
  auto stateView = world.View<BattleState>();
  if (stateView.empty()) {
    return;
  }

  auto &state = stateView.get<BattleState>(*stateView.begin());
  if (state.victory || state.defeat) {
    return;
  }

  // コスト自動回復（上限は簡易的に9999）
  if (state.cost < 9999) {
    state.cost += static_cast<int>(state.costRegenPerSec * deltaTime);
    state.cost = std::min(state.cost, 9999);
  }

  // スロットクールダウン減衰
  for (float &cooldown : state.slotCooldowns) {
    if (cooldown > 0.0f) {
      cooldown = std::max(0.0f, cooldown - deltaTime);
    }
  }
}

} // namespace New::Domain::TD::LineTD::Systems
