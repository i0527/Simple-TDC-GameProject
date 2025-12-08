#pragma once

#include <vector>

#include "ComponentsNew.h"
#include "new/Domain/TD/LineTD/Components/LineTDComponents.h"

namespace New::Domain::TD::LineTD::Components {

struct AttackEvent {
  entt::entity attacker = entt::null;
  entt::entity target = entt::null;
  float damage = 0.0f;
  float knockback = 0.0f;
};

struct AttackEventQueue {
  std::vector<AttackEvent> events;
};

} // namespace New::Domain::TD::LineTD::Components
