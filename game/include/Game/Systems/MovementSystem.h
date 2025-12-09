#pragma once

#include <entt/entt.hpp>

#include "Game/Components/CoreComponents.h"

namespace Game::Systems {

class MovementSystem {
public:
  void Update(entt::registry &registry, float delta_time);

private:
  void UpdatePositions(entt::registry &registry, float delta_time);
  void HandleBoundaries(entt::registry &registry) const;
  bool HasFrontObstacle(entt::registry &registry, entt::entity entity,
                        const Components::Transform &self_transform,
                        const Components::Stats &self_stats,
                        const Components::Team &self_team,
                        const Components::Velocity &self_vel) const;
};

} // namespace Game::Systems
