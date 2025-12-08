#include "new/Domain/TD/LineTD/Systems/CombatResolveSystem.h"

#include <cmath>

#include "Core/GameContext.h"
#include "Core/World.h"
#include "new/Domain/TD/LineTD/Components/AttackEventQueue.h"
#include "new/Domain/TD/LineTD/Components/LineTDComponents.h"

namespace New::Domain::TD::LineTD::Systems {

using New::Core::Components::Health;
using New::Domain::TD::LineTD::Components::AttackEventQueue;
using New::Domain::TD::LineTD::Components::Lane;
using New::Domain::TD::LineTD::Components::LaneMovement;

void CombatResolveSystem::Update(Core::GameContext &context,
                                 float /*deltaTime*/) {
  auto &world = context.GetWorld();
  auto queueView = world.View<AttackEventQueue>();
  if (queueView.empty()) {
    return;
  }
  auto &queue = queueView.get<AttackEventQueue>(*queueView.begin());
  if (queue.events.empty()) {
    return;
  }

  for (const auto &evt : queue.events) {
    if (evt.target == entt::null || !world.IsValid(evt.target)) {
      continue;
    }
    if (!world.Has<Health>(evt.target)) {
      continue;
    }
    auto &hp = world.Get<Health>(evt.target);
    hp.ApplyDamage(static_cast<int>(std::round(evt.damage)));

    // 簡易ノックバック
    if (world.Has<LaneMovement>(evt.target) && world.Has<Lane>(evt.target)) {
      auto &mv = world.Get<LaneMovement>(evt.target);
      const auto &lane = world.Get<Lane>(evt.target);
      mv.currentX += evt.knockback;
      if (mv.movingRight) {
        mv.currentX = std::min(mv.currentX, lane.endX);
      } else {
        mv.currentX = std::max(mv.currentX, lane.startX);
      }
    }
  }

  queue.events.clear();
}

} // namespace New::Domain::TD::LineTD::Systems
