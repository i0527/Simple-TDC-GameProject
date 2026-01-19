#include "BattleRenderer.hpp"

// 繝励Ο繧ｸ繧ｧ繧ｯ繝亥・
#include "../../utils/Log.h"

namespace game {
namespace core {
namespace game {

BattleRenderer::BattleRenderer(BaseSystemAPI* systemAPI, ECSystemAPI* ecsAPI)
    : systemAPI_(systemAPI), ecsAPI_(ecsAPI) {
}

void BattleRenderer::UpdateAnimations(ECSystemAPI* ecsAPI, float deltaTime) {
    if (!ecsAPI) {
        ecsAPI = ecsAPI_;
    }
    if (!ecsAPI) {
        return;
    }
    auto view = ecsAPI->View<ecs::components::Animation>();
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

void BattleRenderer::RenderEntities(ECSystemAPI* ecsAPI) {
    if (!ecsAPI) {
        ecsAPI = ecsAPI_;
    }
    if (!ecsAPI) {
        return;
    }
    auto view = ecsAPI->View<ecs::components::Position, ecs::components::Sprite>();
    for (auto e : view) {
        const auto& pos = view.get<ecs::components::Position>(e);
        const auto& sprite = view.get<ecs::components::Sprite>(e);
        const auto* anim = ecsAPI->Try<ecs::components::Animation>(e);
        const auto* team = ecsAPI->Try<ecs::components::Team>(e);
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

    void* texturePtr = systemAPI_->Resource().GetTexture(sprite.sheet_path);
    if (!texturePtr) {
        LOG_WARN("Texture not found: {}", sprite.sheet_path);
        return;
    }
    auto* texture = static_cast<Texture2D*>(texturePtr);
    if (!texture || texture->id == 0) {
        LOG_WARN("Texture invalid: {}", sprite.sheet_path);
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

    // 菴咲ｽｮ縺ｯ縲瑚ｶｳ蜈・渕貅悶阪〒縺ｯ縺ｪ縺冗ｰ｡譏薙↓蟾ｦ荳雁渕貅厄ｼ亥ｾ後〒隱ｿ謨ｴ・・
    systemAPI_->Render().DrawTexturePro(*texture, src, dst, {0.0f, 0.0f}, 0.0f,
                                        WHITE);
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
        // DrawTexturePro縺ｧ豌ｴ蟷ｳ蜿崎ｻ｢: sourceRect.width繧定ｲ縺ｫ縺吶ｋ
        src.x += fw;
        src.width = -fw;
    }

    return src;
}

} // namespace game
} // namespace core
} // namespace game

