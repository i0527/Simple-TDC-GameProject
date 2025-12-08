#include "new/Game/Systems/CostSystem.h"

#include "Core/GameContext.h"
#include "Core/World.h"
#include "new/Game/Components/GameState.h"

namespace New::Game::Systems {

using New::Game::Components::BattleState;

void CostSystem::Update(Core::GameContext &context, float deltaTime) {
  auto &world = context.GetWorld();
  auto view = world.View<BattleState>();
  if (view.empty()) {
    return;
  }

  auto &state = view.get<BattleState>(*view.begin());
  if (state.victory || state.defeat) {
    return;
  }

  // コスト自動回復（上限は暫定 9999）
  if (state.cost < 9999) {
    state.cost += static_cast<int>(state.costRegenPerSec * deltaTime);
    if (state.cost > 9999) {
      state.cost = 9999;
    }
  }

  // スロットクールダウン減衰
  for (float &cooldown : state.slotCooldowns) {
    if (cooldown > 0.0f) {
      cooldown = std::max(0.0f, cooldown - deltaTime);
    }
  }
}

} // namespace New::Game::Systems
