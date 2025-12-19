#include "Game/Systems/NewRenderingSystem.h"

#include "Game/Components/NewCoreComponents.h"

namespace Game::Systems {

void NewRenderingSystem::DrawEntities(entt::registry& registry) const {
    auto view = registry.view<Game::Components::Transform, Game::Components::Animation, Game::Components::Sprite, Game::Components::Team>();

    for (auto entity : view) {
        auto& transform = view.get<Game::Components::Transform>(entity);
        auto& anim = view.get<Game::Components::Animation>(entity);
        auto& sprite = view.get<Game::Components::Sprite>(entity);
        auto& team = view.get<Game::Components::Team>(entity);

        if (!sprite.provider) continue;

        Color tint = (team.type == Game::Components::Team::Type::Player) ? BLUE : RED;

        SpriteRenderer::DrawSprite(
            *sprite.provider,
            anim.currentClip,
            anim.frameIndex,
            Vector2{transform.x, transform.y},
            Vector2{transform.scaleX, transform.scaleY},
            transform.rotation,
            tint
        );
    }
}

} // namespace Game::Systems
