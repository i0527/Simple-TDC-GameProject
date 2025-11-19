#pragma once

#include <entt/entt.hpp>

namespace Core {
    // Abstract interface for all game systems
    class ISystem {
    public:
        virtual ~ISystem() = default;
        
        // Process input for this system
        virtual void ProcessInput(entt::registry& registry) {}
        
        // Update system logic
        virtual void Update(entt::registry& registry, float deltaTime) {}
        
        // Render system entities
        virtual void Render(entt::registry& registry) {}
    };
}
