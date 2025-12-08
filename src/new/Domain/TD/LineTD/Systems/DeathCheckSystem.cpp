#include "new/Domain/TD/LineTD/Systems/DeathCheckSystem.h"

#include "Core/GameContext.h"
#include "Core/World.h"
#include "new/Core/Components/CoreComponents.h"
#include "new/Domain/TD/LineTD/Components/LineTDComponents.h"
#include "new/Game/Components/GameState.h"
#include "new/Game/Components/UnitPreset.h"

namespace New::Domain::TD::LineTD::Systems {

using New::Core::Components::Health;
using New::Domain::TD::LineTD::Components::Team;
using New::Domain::TD::LineTD::Components::TeamSide;
using New::Game::Components::BattleState;
using New::Game::Components::SlotId;

void DeathCheckSystem::Update(Core::GameContext &context, float /*deltaTime*/) {
  auto &world = context.GetWorld();
  auto stateView = world.View<BattleState>();
  BattleState *state = stateView.empty()
                           ? nullptr
                           : &stateView.get<BattleState>(*stateView.begin());
  auto view = world.View<Health, Team>();
  for (auto e : view) {
    auto &hp = view.get<Health>(e);
    if (!hp.IsDead()) {
      continue;
    }
    if (state) {
      const auto &team = view.get<Team>(e);
      if (team.side == TeamSide::Enemy) {
        state->cost += state->killReward;
      } else if (team.side == TeamSide::Player) {
        // プレイヤーユニットが死亡した場合、召喚数を減らす
        if (world.Has<SlotId>(e)) {
          const auto &slotId = world.Get<SlotId>(e);
          if (slotId.slotIndex >= 0 && slotId.slotIndex < 10) {
            if (state->slotSummonLimits[slotId.slotIndex] > 0) {
              state->slotSummonCounts[slotId.slotIndex] =
                  std::max(0, state->slotSummonCounts[slotId.slotIndex] - 1);
            }
          }
        }
      }
    }
    world.DestroyEntity(e);
  }
}

} // namespace New::Domain::TD::LineTD::Systems
