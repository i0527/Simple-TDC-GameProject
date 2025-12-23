#ifndef GAME_GRAPHICS_SPRITERENDERER_H
#define GAME_GRAPHICS_SPRITERENDERER_H

#include "Data/Graphics/IFrameProvider.h"
#include <raylib.h>
#include <string>

namespace Game::Graphics {

/**
 * @brief 統一スプライト描画器
 * 
 * Provider から FrameRef を取得し、DrawTexturePro で描画
 * 
 * Grid形式/Asepriteアトラス/TexturePackerアトラス etc
 * あらゆる形式に対応（Provider がそれぞれ FrameRef で統一）
 */
class SpriteRenderer {
public:
    /**
     * @brief スプライトを描画
     * 
     * 足元ベース座標システムで、offset/origin を自動補正
     * 
     * @param provider FrameProvider（Grid/Aseprite/TexturePacker など）
     * @param clipName クリップ名（"idle", "walk" など）
     * @param frameIndex フレーム番号（0ベース）
     * @param position 世界座標（足元ベース）
     * @param scale スケール（通常は {1.0f, 1.0f}）
     * @param rotation 回転角度（ラジアン）
     * @param tint 色合い（例: RAYWHITE, GetTeamTint(team)）
     */
    static void DrawSprite(
        Shared::Data::Graphics::IFrameProvider& provider,
        const std::string& clipName,
        int frameIndex,
        Vector2 position,
        Vector2 scale,
        float rotation,
        Color tint
    );
    
    /**
     * @brief デバッグ描画（フレーム枠線表示）
     * 
     * FrameRef の src 矩形や offset を可視化
     * 
     * @param provider FrameProvider
     * @param clipName クリップ名
     * @param frameIndex フレーム番号
     * @param position 世界座標
     * @param scale スケール
     * @param outlineColor 枠線色
     */
    static void DrawSpriteDebug(
        Shared::Data::Graphics::IFrameProvider& provider,
        const std::string& clipName,
        int frameIndex,
        Vector2 position,
        Vector2 scale,
        Color outlineColor = RED
    );
};

} // namespace Game::Graphics

#endif // GAME_GRAPHICS_SPRITERENDERER_H
