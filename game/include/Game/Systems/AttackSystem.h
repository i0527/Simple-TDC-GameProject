#pragma once

#include <entt/entt.hpp>

#include "Game/Components/CoreComponents.h"

namespace Game::Systems {

class AttackSystem {
public:
  void Update(entt::registry &registry, float delta_time);

private:
  entt::entity FindTarget(entt::registry &registry, entt::entity attacker,
                          const Components::Transform &attacker_transform,
                          float range) const;

  void ApplyDamage(entt::registry &registry, entt::entity attacker,
                   entt::entity target) const;
};

} // namespace Game::Systems
