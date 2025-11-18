#include "Systems.h"
#include "Components.h"
#include <raylib.h>

namespace Systems {

void UpdateMovement(entt::registry& registry, float deltaTime) {
    // Update all entities with Position and Velocity components
    auto view = registry.view<Components::Position, Components::Velocity>();
    
    for (auto entity : view) {
        auto& pos = view.get<Components::Position>(entity);
        auto& vel = view.get<Components::Velocity>(entity);
        
        pos.x += vel.dx * deltaTime;
        pos.y += vel.dy * deltaTime;
        
        // Simple screen wrapping
        if (pos.x < 0) pos.x = GetScreenWidth();
        if (pos.x > GetScreenWidth()) pos.x = 0;
        if (pos.y < 0) pos.y = GetScreenHeight();
        if (pos.y > GetScreenHeight()) pos.y = 0;
    }
}

void RenderEntities(entt::registry& registry) {
    // Render all entities with Position and Renderable components
    auto view = registry.view<Components::Position, Components::Renderable>();
    
    for (auto entity : view) {
        const auto& pos = view.get<Components::Position>(entity);
        const auto& renderable = view.get<Components::Renderable>(entity);
        
        DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), 
                   renderable.radius, renderable.color);
    }
}

void ProcessPlayerInput(entt::registry& registry) {
    // Process input for player entities
    auto view = registry.view<Components::Player, Components::Velocity>();
    
    for (auto entity : view) {
        auto& vel = view.get<Components::Velocity>(entity);
        
        const float speed = 200.0f;
        
        // Reset velocity
        vel.dx = 0.0f;
        vel.dy = 0.0f;
        
        // Update velocity based on input
        if (IsKeyDown(KEY_RIGHT)) vel.dx = speed;
        if (IsKeyDown(KEY_LEFT)) vel.dx = -speed;
        if (IsKeyDown(KEY_DOWN)) vel.dy = speed;
        if (IsKeyDown(KEY_UP)) vel.dy = -speed;
    }
}

} // namespace Systems
