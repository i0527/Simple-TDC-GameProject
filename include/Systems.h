#pragma once

#include <entt/entt.hpp>

namespace Systems {
    // Movement system - updates positions based on velocities
    void UpdateMovement(entt::registry& registry, float deltaTime);

    // Render system - draws all renderable entities
    void RenderEntities(entt::registry& registry);

    // Input system - processes player input
    void ProcessPlayerInput(entt::registry& registry);
}
