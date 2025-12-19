#include "Game/Systems/AnimationSystem.h"

#include "Data/Graphics/IFrameProvider.h"
#include "Game/Components/NewCoreComponents.h"

namespace Game::Systems {

void AnimationSystem::Update(entt::registry& registry, float deltaTime) {
    auto view = registry.view<Game::Components::Animation, Game::Components::Sprite>();

    for (auto entity : view) {
        auto& anim = view.get<Game::Components::Animation>(entity);
        auto& sprite = view.get<Game::Components::Sprite>(entity);

        if (!anim.isPlaying || !sprite.provider) continue;

        anim.elapsedTime += deltaTime;

        float fps = sprite.provider->GetClipFps(anim.currentClip);
        float frameDuration = 1.0f / fps;
        int nextFrameIndex = (int)(anim.elapsedTime / frameDuration);
        int frameCount = sprite.provider->GetFrameCount(anim.currentClip);

        if (nextFrameIndex >= frameCount) {
            if (sprite.provider->IsLooping(anim.currentClip)) {
                anim.elapsedTime = 0.0f;
                anim.frameIndex = 0;
            } else {
                anim.isPlaying = false;
                anim.frameIndex = frameCount - 1;
            }
        } else {
            anim.frameIndex = nextFrameIndex;
        }
    }
}

} // namespace Game::Systems
