#include "Game/Systems/MovementSystem.h"
#include "ComponentsNew.h"
#include <raylib.h>

namespace Systems {

void MovementSystem::Update(entt::registry& registry, float deltaTime) {
    auto view = registry.view<Components::Position, Components::Velocity>();
    
    for (auto entity : view) {
        auto& pos = view.get<Components::Position>(entity);
        auto& vel = view.get<Components::Velocity>(entity);
        
        pos.x += vel.dx * deltaTime;
        pos.y += vel.dy * deltaTime;
        
        if (pos.x < 0) pos.x = GetScreenWidth();
        if (pos.x > GetScreenWidth()) pos.x = 0;
        if (pos.y < 0) pos.y = GetScreenHeight();
        if (pos.y > GetScreenHeight()) pos.y = 0;
    }
}

} // namespace Systems

