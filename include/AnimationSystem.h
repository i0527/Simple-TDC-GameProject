#pragma once

#include <entt/entt.hpp>
#include "ComponentsNew.h"
// Note: SpriteAnimation, SpriteFrame, SpriteTexture are still in Components namespace (legacy)
#include "Components.h"
#include "ResourceManager.h"
#include <raylib.h>

namespace Systems {
    // �X�v���C�g�A�j���[�V�����V�X�e��
    // SpriteAnimation �R���|�[�l���g�����G���e�B�e�B�̃A�j���[�V�����i�s���Ǘ�
    class AnimationSystem {
    public:
        // �A�j���[�V�������X�V�i���t���[���Ăяo���j
        static void Update(entt::registry& registry, float deltaTime) {
            auto view = registry.view<Components::SpriteAnimation, Components::SpriteFrame>();
            Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();
            auto& imageMgr = rm.GetImageManager();

            for (auto entity : view) {
                auto& anim = view.get<Components::SpriteAnimation>(entity);
                auto& sprite = view.get<Components::SpriteFrame>(entity);

                // �Đ����łȂ��ꍇ�̓X�L�b�v
                if (!anim.isPlaying || anim.frames.empty()) {
                    continue;
                }

                // �o�ߎ��Ԃ����Z
                anim.elapsedTime += deltaTime;

                // ���݃t���[���� duration ���擾
                const auto& currentFrameName = anim.frames[anim.currentFrameIndex];
                auto frameInfo = imageMgr.GetFrameInfo(currentFrameName);
                float frameDuration = frameInfo.duration / 1000.0f; // �~���b��b�ɕϊ�

                // �t���[���A�j���[�V������i�߂�
                if (anim.elapsedTime >= frameDuration) {
                    anim.elapsedTime -= frameDuration;
                    anim.currentFrameIndex++;

                    // ���[�v����
                    if (anim.currentFrameIndex >= anim.frames.size()) {
                        if (anim.isLooping) {
                            anim.currentFrameIndex = 0;
                        } else {
                            anim.isPlaying = false;
                            anim.currentFrameIndex = anim.frames.size() - 1;
                        }
                    }

                    // �X�v���C�g�t���[�������X�V
                    const auto& newFrameName = anim.frames[anim.currentFrameIndex];
                    auto newFrameInfo = imageMgr.GetFrameInfo(newFrameName);
                    sprite.frameName = newFrameName;
                    sprite.sourceRect = newFrameInfo.rect;
                }
            }
        }

        // �A�j���[�V�������J�n
        static void Play(entt::registry& registry, entt::entity entity, bool loop = true) {
            if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
                anim->isPlaying = true;
                anim->isLooping = loop;
                anim->elapsedTime = 0.0f;
                anim->currentFrameIndex = 0;
            }
        }

        // �A�j���[�V�������~
        static void Stop(entt::registry& registry, entt::entity entity) {
            if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
                anim->isPlaying = false;
                anim->elapsedTime = 0.0f;
                anim->currentFrameIndex = 0;
            }
        }

        // �A�j���[�V�������ꎞ��~
        static void Pause(entt::registry& registry, entt::entity entity) {
            if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
                anim->isPlaying = false;
            }
        }

        // �A�j���[�V�������ĊJ
        static void Resume(entt::registry& registry, entt::entity entity) {
            if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
                anim->isPlaying = true;
            }
        }
    };

    // �X�v���C�g�`��V�X�e��
    // SpriteFrame, SpriteTexture, Position �����G���e�B�e�B��`��
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

                // �X�P�[���擾�i�f�t�H���g 1.0�j
                float scaleX = 1.0f;
                float scaleY = 1.0f;
                if (auto* scale = registry.try_get<Core::Components::Scale>(entity)) {
                    scaleX = scale->x;
                    scaleY = scale->y;
                }

                // �e�N�X�`���擾
                try {
                    Texture2D texture = texMgr.GetTexture(texRef.textureName);
                    Rectangle src = sprite.sourceRect;

                    // �t���[����`���L�����m�F
                    if (src.width <= 0 || src.height <= 0) {
                        continue;
                    }

                    // �X�P�[�����O��̃T�C�Y�v�Z
                    float scaledWidth = src.width * scaleX;
                    float scaledHeight = src.height * scaleY;

                    // �`��ʒu�F�t���[���𒆐S�ɔz�u
                    Vector2 destPos{ pos.x - scaledWidth * 0.5f, pos.y - scaledHeight * 0.5f };

                    // �X�P�[�����O��̖ړI��`
                    Rectangle dest{ destPos.x, destPos.y, scaledWidth, scaledHeight };

                    // DrawTexturePro �Ńe�N�X�`���̎w���`�������X�P�[�����O�`��
                    DrawTexturePro(texture, src, dest, {0, 0}, 0.0f, WHITE);
                } catch (const std::exception& e) {
                    // �e�N�X�`����������Ȃ��ꍇ�͕`��X�L�b�v
                    continue;
                }
            }
        }
    };

    // �ړ��V�X�e��
    // Position �� Velocity �����G���e�B�e�B���ړ�
    class MovementSystem {
    public:
        static void Update(entt::registry& registry, float deltaTime) {
            auto view = registry.view<Core::Components::Position, Core::Components::Velocity>();

            for (auto entity : view) {
                auto& pos = view.get<Core::Components::Position>(entity);
                auto& vel = view.get<Core::Components::Velocity>(entity);

                // ���x�Ɋ�Â��Ĉʒu���X�V
                pos.x += vel.dx * deltaTime;
                pos.y += vel.dy * deltaTime;
            }
        }
    };

    // �L�[���͏����V�X�e���i�e�X�g�p�j
    // Player �^�O�����G���e�B�e�B�̃L�[���͂�����
    class InputSystem {
    public:
        static void Update(entt::registry& registry) {
            auto view = registry.view<GameComponents::PlayerControlled, Core::Components::Velocity>();

            for (auto entity : view) {
                auto& vel = view.get<Components::Velocity>(entity);

                // �L�[���͂ő��x��ݒ�
                vel.dx = 0.0f;
                vel.dy = 0.0f;

                if (IsKeyDown(KEY_RIGHT)) vel.dx = 200.0f;
                if (IsKeyDown(KEY_LEFT)) vel.dx = -200.0f;
                if (IsKeyDown(KEY_DOWN)) vel.dy = 200.0f;
                if (IsKeyDown(KEY_UP)) vel.dy = -200.0f;
            }
        }
    };
}
