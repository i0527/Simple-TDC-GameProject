#pragma once

#include <entt/entt.hpp>

#include "Game/Components/CoreComponents.h"

namespace Game::Systems {

class SkillSystem {
public:
  void Update(entt::registry &registry, float delta_time);

private:
  void UpdateCooldowns(entt::registry &registry, float delta_time) const;
  void TryCastSkills(entt::registry &registry) const;
};

} // namespace Game::Systems
