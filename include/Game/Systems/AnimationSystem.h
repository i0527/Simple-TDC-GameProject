#pragma once

#include <entt/entt.hpp>
#include "ComponentsNew.h"
#include "Components.h"
#include "ResourceManager.h"
#include <raylib.h>

namespace Systems {
    // スプライトアニメーションを更新するシステム
    class AnimationSystem {
    public:
        static void Update(entt::registry& registry, float deltaTime) {
            auto view = registry.view<Components::SpriteAnimation, Components::SpriteFrame>();
            Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();
            auto& imageMgr = rm.GetImageManager();

            for (auto entity : view) {
                auto& anim = view.get<Components::SpriteAnimation>(entity);
                auto& sprite = view.get<Components::SpriteFrame>(entity);

                if (!anim.isPlaying || anim.frames.empty()) {
                    continue;
                }

                anim.elapsedTime += deltaTime;

                const auto& currentFrameName = anim.frames[anim.currentFrameIndex];
                auto frameInfo = imageMgr.GetFrameInfo(currentFrameName);
                float frameDuration = frameInfo.duration / 1000.0f;

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

                    const auto& newFrameName = anim.frames[anim.currentFrameIndex];
                    auto newFrameInfo = imageMgr.GetFrameInfo(newFrameName);
                    sprite.frameName = newFrameName;
                    sprite.sourceRect = newFrameInfo.rect;
                }
            }
        }

        static void Play(entt::registry& registry, entt::entity entity, bool loop = true) {
            if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
                anim->isPlaying = true;
                anim->isLooping = loop;
                anim->elapsedTime = 0.0f;
                anim->currentFrameIndex = 0;
            }
        }

        static void Stop(entt::registry& registry, entt::entity entity) {
            if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
                anim->isPlaying = false;
                anim->elapsedTime = 0.0f;
                anim->currentFrameIndex = 0;
            }
        }

        static void Pause(entt::registry& registry, entt::entity entity) {
            if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
                anim->isPlaying = false;
            }
        }

        static void Resume(entt::registry& registry, entt::entity entity) {
            if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
                anim->isPlaying = true;
            }
        }
    };

    // スプライトベースの描画システム
    class SpriteRenderSystem {
    public:
        static void Render(entt::registry& registry) {
            auto view = registry.view<Core::Components::Position, Components::SpriteFrame, Components::SpriteTexture>();
            Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();
            auto& texMgr = rm.GetTextureManager();

            for (auto entity : view) {
                auto& pos = view.get<Components::Position>(entity);
                auto& sprite = view.get<Components::SpriteFrame>(entity);
                auto& texRef = view.get<Components::SpriteTexture>(entity);

                float scaleX = 1.0f;
                float scaleY = 1.0f;
                if (auto* scale = registry.try_get<Core::Components::Scale>(entity)) {
                    scaleX = scale->x;
                    scaleY = scale->y;
                }

                try {
                    Texture2D texture = texMgr.GetTexture(texRef.textureName);
                    Rectangle src = sprite.sourceRect;

                    if (src.width <= 0 || src.height <= 0) {
                        continue;
                    }

                    float scaledWidth = src.width * scaleX;
                    float scaledHeight = src.height * scaleY;
                    Vector2 destPos{ pos.x - scaledWidth * 0.5f, pos.y - scaledHeight * 0.5f };
                    Rectangle dest{ destPos.x, destPos.y, scaledWidth, scaledHeight };

                    DrawTexturePro(texture, src, dest, {0, 0}, 0.0f, WHITE);
                } catch (const std::exception&) {
                    continue;
                }
            }
        }
    };
}

