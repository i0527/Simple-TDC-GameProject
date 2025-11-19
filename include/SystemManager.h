#pragma once

#include "System.h"
#include <vector>
#include <memory>

namespace Core {
    // Manages lifecycle and execution of game systems
    class SystemManager {
    public:
        SystemManager() = default;
        ~SystemManager() = default;
        
        // Add a system to the manager
        void AddSystem(std::unique_ptr<ISystem> system);
        
        // Process input for all systems
        void ProcessInput(entt::registry& registry);
        
        // Update all systems
        void Update(entt::registry& registry, float deltaTime);
        
        // Render all systems
        void Render(entt::registry& registry);
        
    private:
        std::vector<std::unique_ptr<ISystem>> systems_;
    };
}
