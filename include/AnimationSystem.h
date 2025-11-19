#pragma once

#include <entt/entt.hpp>
#include "Components.h"
#include "ResourceManager.h"
#include <raylib.h>

namespace Systems {
    // スプライトアニメーションシステム
    // SpriteAnimation コンポーネントを持つエンティティのアニメーション進行を管理
    class AnimationSystem {
    public:
        // アニメーションを更新（毎フレーム呼び出し）
        static void Update(entt::registry& registry, float deltaTime) {
            auto view = registry.view<Components::SpriteAnimation, Components::SpriteFrame>();
            Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();
            auto& imageMgr = rm.GetImageManager();

            for (auto entity : view) {
                auto& anim = view.get<Components::SpriteAnimation>(entity);
                auto& sprite = view.get<Components::SpriteFrame>(entity);

                // 再生中でない場合はスキップ
                if (!anim.isPlaying || anim.frames.empty()) {
                    continue;
                }

                // 経過時間を加算
                anim.elapsedTime += deltaTime;

                // 現在フレームの duration を取得
                const auto& currentFrameName = anim.frames[anim.currentFrameIndex];
                auto frameInfo = imageMgr.GetFrameInfo(currentFrameName);
                float frameDuration = frameInfo.duration / 1000.0f; // ミリ秒を秒に変換

                // フレームアニメーションを進める
                if (anim.elapsedTime >= frameDuration) {
                    anim.elapsedTime -= frameDuration;
                    anim.currentFrameIndex++;

                    // ループ処理
                    if (anim.currentFrameIndex >= anim.frames.size()) {
                        if (anim.isLooping) {
                            anim.currentFrameIndex = 0;
                        } else {
                            anim.isPlaying = false;
                            anim.currentFrameIndex = anim.frames.size() - 1;
                        }
                    }

                    // スプライトフレーム情報を更新
                    const auto& newFrameName = anim.frames[anim.currentFrameIndex];
                    auto newFrameInfo = imageMgr.GetFrameInfo(newFrameName);
                    sprite.frameName = newFrameName;
                    sprite.sourceRect = newFrameInfo.rect;
                }
            }
        }

        // アニメーションを開始
        static void Play(entt::registry& registry, entt::entity entity, bool loop = true) {
            if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
                anim->isPlaying = true;
                anim->isLooping = loop;
                anim->elapsedTime = 0.0f;
                anim->currentFrameIndex = 0;
            }
        }

        // アニメーションを停止
        static void Stop(entt::registry& registry, entt::entity entity) {
            if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
                anim->isPlaying = false;
                anim->elapsedTime = 0.0f;
                anim->currentFrameIndex = 0;
            }
        }

        // アニメーションを一時停止
        static void Pause(entt::registry& registry, entt::entity entity) {
            if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
                anim->isPlaying = false;
            }
        }

        // アニメーションを再開
        static void Resume(entt::registry& registry, entt::entity entity) {
            if (auto* anim = registry.try_get<Components::SpriteAnimation>(entity)) {
                anim->isPlaying = true;
            }
        }
    };

    // スプライト描画システム
    // SpriteFrame, SpriteTexture, Position を持つエンティティを描画
    class SpriteRenderSystem {
    public:
        static void Render(entt::registry& registry) {
            auto view = registry.view<Components::Position, Components::SpriteFrame, Components::SpriteTexture>();
            Resources::ResourceManager& rm = Resources::ResourceManager::GetInstance();
            auto& texMgr = rm.GetTextureManager();

            for (auto entity : view) {
                auto& pos = view.get<Components::Position>(entity);
                auto& sprite = view.get<Components::SpriteFrame>(entity);
                auto& texRef = view.get<Components::SpriteTexture>(entity);

                // スケール取得（デフォルト 1.0）
                float scaleX = 1.0f;
                float scaleY = 1.0f;
                if (auto* scale = registry.try_get<Components::Scale>(entity)) {
                    scaleX = scale->x;
                    scaleY = scale->y;
                }

                // テクスチャ取得
                try {
                    Texture2D texture = texMgr.GetTexture(texRef.textureName);
                    Rectangle src = sprite.sourceRect;

                    // フレーム矩形が有効か確認
                    if (src.width <= 0 || src.height <= 0) {
                        continue;
                    }

                    // スケーリング後のサイズ計算
                    float scaledWidth = src.width * scaleX;
                    float scaledHeight = src.height * scaleY;

                    // 描画位置：フレームを中心に配置
                    Vector2 destPos{ pos.x - scaledWidth * 0.5f, pos.y - scaledHeight * 0.5f };

                    // スケーリング後の目的矩形
                    Rectangle dest{ destPos.x, destPos.y, scaledWidth, scaledHeight };

                    // DrawTexturePro でテクスチャの指定矩形部分をスケーリング描画
                    DrawTexturePro(texture, src, dest, {0, 0}, 0.0f, WHITE);
                } catch (const std::exception& e) {
                    // テクスチャが見つからない場合は描画スキップ
                    continue;
                }
            }
        }
    };

    // 移動システム
    // Position と Velocity を持つエンティティを移動
    class MovementSystem {
    public:
        static void Update(entt::registry& registry, float deltaTime) {
            auto view = registry.view<Components::Position, Components::Velocity>();

            for (auto entity : view) {
                auto& pos = view.get<Components::Position>(entity);
                auto& vel = view.get<Components::Velocity>(entity);

                // 速度に基づいて位置を更新
                pos.x += vel.dx * deltaTime;
                pos.y += vel.dy * deltaTime;
            }
        }
    };

    // キー入力処理システム（テスト用）
    // Player タグを持つエンティティのキー入力を処理
    class InputSystem {
    public:
        static void Update(entt::registry& registry) {
            auto view = registry.view<Components::Player, Components::Velocity>();

            for (auto entity : view) {
                auto& vel = view.get<Components::Velocity>(entity);

                // キー入力で速度を設定
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
