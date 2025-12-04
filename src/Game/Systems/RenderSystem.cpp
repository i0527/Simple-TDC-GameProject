#include "Game/Systems/RenderSystem.h"
#include "Components.h"
#include <raylib.h>

namespace Systems {

void RenderSystem::Render(entt::registry& registry) {
    auto view = registry.view<Components::Position, Components::Renderable>();
    
    for (auto entity : view) {
        const auto& pos = view.get<Components::Position>(entity);
        const auto& renderable = view.get<Components::Renderable>(entity);
        
        DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), 
                   renderable.radius, renderable.color);
    }
}

} // namespace Systems

