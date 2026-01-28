#include "BattleRenderer.hpp"

// プロジェクト�E
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
    Rectangle src = MakeSourceRect(sprite, anim, texture->width, texture->height, flip);

    // 描画サイズ（2倍スケール）
    const float drawWidth = static_cast<float>(sprite.frame_width) * 2.0f;
    const float drawHeight = static_cast<float>(sprite.frame_height) * 2.0f;

    // 位置計算：pos.yは元のサイズでの左上Y座標（lane_.y - frame_height）
    // スケール2倍でも足元がlane_.yに来るように調整
    // pos.y = lane_.y - frame_height なので、
    // drawY = lane_.y - drawHeight = pos.y + frame_height - drawHeight = pos.y - frame_height
    const float drawX = pos.x;  // X座標は左端基準のまま
    const float drawY = pos.y - static_cast<float>(sprite.frame_height);  // 元の高さ分だけ上に

    Rectangle dst{
        drawX,
        drawY,
        drawWidth,
        drawHeight
    };

    // 位置は「足允E��準」ではなく簡易に左上基準（後で調整�E�E
    // 回転中心は足元（基底ライン上）
    // 左上基準で描画（元のコードと同じ基準点）
    systemAPI_->Render().DrawTexturePro(*texture, src, dst, {0.0f, 0.0f}, 0.0f,
                                        WHITE);
}

Rectangle BattleRenderer::MakeSourceRect(const ecs::components::Sprite& sprite,
                                        const ecs::components::Animation* anim,
                                        int sheetWidth,
                                        int sheetHeight,
                                        bool flipHorizontally) {
    const int frame = (anim) ? anim->current_frame : 0;
    const float fw = static_cast<float>(sprite.frame_width);
    const float fh = static_cast<float>(sprite.frame_height);
    if (sprite.frame_width <= 0 || sprite.frame_height <= 0) {
        return Rectangle{0.0f, 0.0f, fw, fh};
    }
    // 正方形アスペクトなどグリッド対応: 1行のコマ数で row/col を算出
    const int cols = sheetWidth / sprite.frame_width;
    const int rows = (cols > 0) ? (sheetHeight / sprite.frame_height) : 1;
    const int totalCells = cols * rows;
    const int safeFrame = (totalCells > 0) ? (frame % totalCells) : 0;
    const int row = (cols > 0) ? (safeFrame / cols) : 0;
    const int col = (cols > 0) ? (safeFrame % cols) : safeFrame;

    Rectangle src{
        fw * static_cast<float>(col),
        fh * static_cast<float>(row),
        fw,
        fh
    };

    if (flipHorizontally) {
        src.x += fw;
        src.width = -fw;
    }

    return src;
}

} // namespace game
} // namespace core
} // namespace game

