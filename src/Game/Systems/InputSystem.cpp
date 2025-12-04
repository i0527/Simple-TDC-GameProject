#include "Game/Systems/InputSystem.h"
#include "Components.h"
#include <raylib.h>

namespace Systems {

void InputSystem::ProcessInput(entt::registry& registry) {
    auto view = registry.view<Components::Player, Components::Velocity>();
    
    for (auto entity : view) {
        auto& vel = view.get<Components::Velocity>(entity);
        
        const float speed = 200.0f;
        vel.dx = 0.0f;
        vel.dy = 0.0f;
        
        if (IsKeyDown(KEY_RIGHT)) vel.dx = speed;
        if (IsKeyDown(KEY_LEFT)) vel.dx = -speed;
        if (IsKeyDown(KEY_DOWN)) vel.dy = speed;
        if (IsKeyDown(KEY_UP)) vel.dy = -speed;
    }
}

} // namespace Systems

