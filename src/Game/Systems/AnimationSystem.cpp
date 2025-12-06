#include "Game/Systems/AnimationSystem.h"
#include "Core/GameContext.h"
#include <iostream>

namespace Systems {

AnimationSystem::AnimationSystem(Core::GameContext& context) : context_(context) {}

void AnimationSystem::Update(entt::registry& registry, float deltaTime) {
    auto view = registry.view<Components::SpriteAnimation, Components::SpriteFrame>();

    for (auto entity : view) {
        auto& anim = view.get<Components::SpriteAnimation>(entity);
        auto& sprite = view.get<Components::SpriteFrame>(entity);

        if (!anim.isPlaying || anim.frames.empty()) {
            continue;
        }

        anim.elapsedTime += deltaTime;

        const auto& currentFrameName = anim.frames[anim.currentFrameIndex];
        float frameDuration = 0.1f;
        
        if (anim.elapsedTime >= frameDuration) {
            anim.elapsedTime -= frameDuration;
            anim.currentFrameIndex++;

            if (anim.currentFrameIndex >= anim.frames.size()) {
                if (anim.isLooping) {
                    anim.currentFrameIndex = 0;
                } else {
                    anim.isPlaying = false;
                    anim.currentFrameIndex = anim.frames.size() - 1;
                }
            }
        }
    }
}

void AnimationSystem::Play(entt::registry& registry, entt::entity entity, bool loop) {
    if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
        anim->isPlaying = true;
        anim->isLooping = loop;
        anim->elapsedTime = 0.0f;
        anim->currentFrameIndex = 0;
    }
}

void AnimationSystem::Stop(entt::registry& registry, entt::entity entity) {
    if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
        anim->isPlaying = false;
        anim->elapsedTime = 0.0f;
        anim->currentFrameIndex = 0;
    }
}

void AnimationSystem::Pause(entt::registry& registry, entt::entity entity) {
    if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
        anim->isPlaying = false;
    }
}

void AnimationSystem::Resume(entt::registry& registry, entt::entity entity) {
    if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
        anim->isPlaying = true;
    }
}

SpriteRenderSystem::SpriteRenderSystem(Core::GameContext& context) : context_(context) {}

void SpriteRenderSystem::Render(entt::registry& registry) {
    auto view = registry.view<Core::Components::Position, Components::SpriteFrame, Components::SpriteTexture>();

    for (auto entity : view) {
        auto& pos = view.get<Core::Components::Position>(entity);
        auto& sprite = view.get<Components::SpriteFrame>(entity);

        float scaleX = 1.0f;
        float scaleY = 1.0f;
        if (auto* scale = registry.try_get<Core::Components::Scale>(entity)) {
            scaleX = scale->x;
            scaleY = scale->y;
        }

        try {
            Rectangle src = sprite.sourceRect;

            if (src.width <= 0 || src.height <= 0) {
                continue;
            }

            float scaledWidth = src.width * scaleX;
            float scaledHeight = src.height * scaleY;
            Vector2 destPos{ pos.x - scaledWidth * 0.5f, pos.y - scaledHeight * 0.5f };
            Rectangle dest{ destPos.x, destPos.y, scaledWidth, scaledHeight };

        } catch (const std::exception&) {
            continue;
        }
    }
}

} // namespace Systems


