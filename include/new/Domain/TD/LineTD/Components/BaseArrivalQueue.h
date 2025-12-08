#pragma once

#include <vector>

#include "new/Domain/TD/LineTD/Components/LineTDComponents.h"

namespace New::Domain::TD::LineTD::Components {

struct BaseArrivalEvent {
  TeamSide side = TeamSide::Enemy;
  int damage = 1;
  entt::entity entity = entt::null;
};

struct BaseArrivalQueue {
  std::vector<BaseArrivalEvent> events;
};

} // namespace New::Domain::TD::LineTD::Components
