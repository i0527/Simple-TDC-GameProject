#include "BattleRenderer.hpp"

// 外部ライブラリ
#include <raylib.h>

// プロジェクト内
#include "../../utils/Log.h"

namespace game {
namespace core {
namespace game {

BattleRenderer::BattleRenderer(BaseSystemAPI* systemAPI)
    : systemAPI_(systemAPI) {
}

void BattleRenderer::UpdateAnimations(entt::registry& registry, float deltaTime) {
    auto view = registry.view<ecs::components::Animation>();
    for (auto e : view) {
        auto& anim = view.get<ecs::components::Animation>(e);
        if (anim.frame_count <= 1) {
            continue;
        }
        anim.frame_timer += deltaTime;
        while (anim.frame_timer >= anim.frame_duration && anim.frame_duration > 0.0f) {
            anim.frame_timer -= anim.frame_duration;
            anim.current_frame++;
            if (anim.current_frame >= anim.frame_count) {
                if (anim.is_looping) {
                    anim.current_frame = 0;
                } else {
                    anim.current_frame = anim.frame_count - 1;
                }
            }
        }
    }
}

void BattleRenderer::RenderEntities(entt::registry& registry) {
    auto view = registry.view<ecs::components::Position, ecs::components::Sprite>();
    for (auto e : view) {
        const auto& pos = view.get<ecs::components::Position>(e);
        const auto& sprite = view.get<ecs::components::Sprite>(e);
        const auto* anim = registry.try_get<ecs::components::Animation>(e);
        const auto* team = registry.try_get<ecs::components::Team>(e);
        RenderEntity(pos, sprite, anim, team);
    }
}

void BattleRenderer::RenderEntity(const ecs::components::Position& pos,
                                 const ecs::components::Sprite& sprite,
                                 const ecs::components::Animation* anim,
                                 const ecs::components::Team* team) {
    if (!systemAPI_) {
        return;
    }

    void* texturePtr = systemAPI_->GetTexture(sprite.sheet_path);
    if (!texturePtr) {
        return;
    }
    auto* texture = static_cast<Texture2D*>(texturePtr);
    if (!texture || texture->id == 0) {
        return;
    }

    const bool flip = (team && team->faction == ecs::components::Faction::Player);
    Rectangle src = MakeSourceRect(sprite, anim, flip);

    Rectangle dst{
        pos.x,
        pos.y,
        static_cast<float>(sprite.frame_width),
        static_cast<float>(sprite.frame_height)
    };

    // 位置は「足元基準」ではなく簡易に左上基準（後で調整）
    systemAPI_->DrawTexturePro(*texture, src, dst, {0.0f, 0.0f}, 0.0f, WHITE);
}

Rectangle BattleRenderer::MakeSourceRect(const ecs::components::Sprite& sprite,
                                        const ecs::components::Animation* anim,
                                        bool flipHorizontally) {
    const int frame = (anim) ? anim->current_frame : 0;
    const float fw = static_cast<float>(sprite.frame_width);
    const float fh = static_cast<float>(sprite.frame_height);

    Rectangle src{
        fw * static_cast<float>(frame),
        0.0f,
        fw,
        fh
    };

    if (flipHorizontally) {
        // DrawTextureProで水平反転: sourceRect.widthを負にする
        src.x += fw;
        src.width = -fw;
    }

    return src;
}

} // namespace game
} // namespace core
} // namespace game

