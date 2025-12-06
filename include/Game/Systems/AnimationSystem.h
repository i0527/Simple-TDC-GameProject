#pragma once

#include <entt/entt.hpp>
#include "ComponentsNew.h"
#include <raylib.h>

namespace Core {
    class GameContext;
}

namespace Systems {
    class AnimationSystem {
    public:
        AnimationSystem(Core::GameContext& context);
        
        void Update(entt::registry& registry, float deltaTime);
        void Play(entt::registry& registry, entt::entity entity, bool loop = true);
        void Stop(entt::registry& registry, entt::entity entity);
        void Pause(entt::registry& registry, entt::entity entity);
        void Resume(entt::registry& registry, entt::entity entity);
        
    private:
        Core::GameContext& context_;
    };

    class SpriteRenderSystem {
    public:
        SpriteRenderSystem(Core::GameContext& context);
        
        void Render(entt::registry& registry);
        
    private:
        Core::GameContext& context_;
    };
}

