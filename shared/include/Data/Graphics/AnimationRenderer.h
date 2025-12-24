#pragma once

#include "IFrameProvider.h"
#include "FrameRef.h"
#include <raylib.h>
#include <string>

namespace Shared::Data::Graphics {

/**
 * @brief アニメーション描画器（Sharedレイヤー）
 * 
 * IFrameProviderを使用してアニメーションを描画する
 * 足元ベース座標システムで、offset/origin を自動補正
 */
class AnimationRenderer {
public:
    /**
     * @brief アニメーションスプライトを描画
     * 
     * @param provider FrameProvider（Grid/Aseprite/TexturePacker など）
     * @param clipName クリップ名（"idle", "walk", "attack", "death" など）
     * @param frameIndex フレーム番号（0ベース）
     * @param position 世界座標（足元ベース）
     * @param scale スケール（通常は {1.0f, 1.0f}）
     * @param rotation 回転角度（ラジアン）
     * @param tint 色合い（例: RAYWHITE）
     * @param flipH 水平反転
     * @param flipV 垂直反転
     */
    static void DrawAnimation(
        IFrameProvider& provider,
        const std::string& clipName,
        int frameIndex,
        Vector2 position,
        Vector2 scale = {1.0f, 1.0f},
        float rotation = 0.0f,
        Color tint = RAYWHITE,
        bool flipH = false,
        bool flipV = false
    );
    
    /**
     * @brief アニメーションスプライトを描画（Rectangle指定版）
     * 
     * @param provider FrameProvider
     * @param clipName クリップ名
     * @param frameIndex フレーム番号
     * @param area 描画領域（足元は area.y + area.height）
     * @param tint 色合い
     * @param flipH 水平反転
     * @param flipV 垂直反転
     */
    static void DrawAnimationInArea(
        IFrameProvider& provider,
        const std::string& clipName,
        int frameIndex,
        Rectangle area,
        Color tint = RAYWHITE,
        bool flipH = false,
        bool flipV = false
    );
    
    /**
     * @brief デバッグ描画（フレーム枠線表示）
     * 
     * @param provider FrameProvider
     * @param clipName クリップ名
     * @param frameIndex フレーム番号
     * @param position 世界座標
     * @param scale スケール
     * @param outlineColor 枠線色
     */
    static void DrawAnimationDebug(
        IFrameProvider& provider,
        const std::string& clipName,
        int frameIndex,
        Vector2 position,
        Vector2 scale = {1.0f, 1.0f},
        Color outlineColor = RED
    );
};

} // namespace Shared::Data::Graphics

