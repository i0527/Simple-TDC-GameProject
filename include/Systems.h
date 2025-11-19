#pragma once

#include "System.h"
#include <entt/entt.hpp>

namespace Systems {
    // Movement system - updates positions based on velocities
    class MovementSystem : public Core::ISystem {
    public:
        void Update(entt::registry& registry, float deltaTime) override;
    };

    // Render system - draws all renderable entities
    class RenderSystem : public Core::ISystem {
    public:
        void Render(entt::registry& registry) override;
    };

    // Input system - processes player input
    class InputSystem : public Core::ISystem {
    public:
        void ProcessInput(entt::registry& registry) override;
    };
}
