#include "Data/Graphics/AnimationRenderer.h"
#include <raymath.h>
#include <cmath>

namespace Shared::Data::Graphics {

void AnimationRenderer::DrawAnimation(
    IFrameProvider& provider,
    const std::string& clipName,
    int frameIndex,
    Vector2 position,
    Vector2 scale,
    float rotation,
    Color tint,
    bool flipH,
    bool flipV
) {
    // FrameRef 取得
    auto frame = provider.GetFrame(clipName, frameIndex);
    if (!frame.valid || !frame.texture) {
        return;
    }
    
    // ===== offset + origin 補正で足元ベース描画 =====
    // 
    // offset: trim による位置ずれ補正（AsepriteのspriteSourceSize対応）
    // origin: 回転・スケール中心（足元に設定）
    // position: 世界座標（足元）
    //
    // DrawTexturePro の origin パラメータで自動補正される
    
    Vector2 destPosition = {
        position.x + (frame.offset.x * scale.x),
        position.y + (frame.offset.y * scale.y)
    };
    
    // ミラーリング対応: 負の幅/高さで反転
    float width = frame.src.width * scale.x;
    float height = frame.src.height * scale.y;
    if (flipH) width = -std::abs(width);
    if (flipV) height = -std::abs(height);
    
    Rectangle destRect = {
        destPosition.x,
        destPosition.y,
        width,
        height
    };
    
    DrawTexturePro(
        *frame.texture,
        frame.src,           // ソース矩形
        destRect,            // 描画先矩形
        frame.origin,        // 回転中心（足元）
        rotation * RAD2DEG,  // 回転角度（ラジアンから度へ）
        tint                 // 色合い
    );
}

void AnimationRenderer::DrawAnimationInArea(
    IFrameProvider& provider,
    const std::string& clipName,
    int frameIndex,
    Rectangle area,
    Color tint,
    bool flipH,
    bool flipV
) {
    // クリップの存在確認
    if (!provider.HasClip(clipName)) {
        return;
    }
    
    // フレーム数の確認
    int frameCount = provider.GetFrameCount(clipName);
    if (frameCount <= 0) {
        return;
    }
    
    // フレームインデックスの範囲チェック
    int safeFrameIndex = std::max(0, std::min(frameIndex, frameCount - 1));
    
    // FrameRef 取得
    auto frame = provider.GetFrame(clipName, safeFrameIndex);
    if (!frame.valid || !frame.texture) {
        return;
    }
    
    // エリア内に収まるようにスケールを計算
    if (frame.src.width <= 0.0f || frame.src.height <= 0.0f) {
        return;
    }
    
    float scaleX = area.width / frame.src.width;
    float scaleY = area.height / frame.src.height;
    float scale = std::min(scaleX, scaleY) * 0.9f; // 90%に縮小して余白を確保
    
    // 足元位置（エリアの下端）
    Vector2 footPosition = {
        area.x + area.width * 0.5f,
        area.y + area.height
    };
    
    DrawAnimation(
        provider,
        clipName,
        safeFrameIndex,
        footPosition,
        {scale, scale},
        0.0f,
        tint,
        flipH,
        flipV
    );
}

void AnimationRenderer::DrawAnimationDebug(
    IFrameProvider& provider,
    const std::string& clipName,
    int frameIndex,
    Vector2 position,
    Vector2 scale,
    Color outlineColor
) {
    auto frame = provider.GetFrame(clipName, frameIndex);
    if (!frame.valid || !frame.texture) {
        return;
    }
    
    Vector2 destPosition = {
        position.x + (frame.offset.x * scale.x),
        position.y + (frame.offset.y * scale.y)
    };
    
    Rectangle destRect = {
        destPosition.x,
        destPosition.y,
        frame.src.width * scale.x,
        frame.src.height * scale.y
    };
    
    // フレーム矩形を描画
    DrawRectangleLinesEx(destRect, 2.0f, outlineColor);
    
    // origin位置を描画
    Vector2 originPos = {
        destPosition.x + frame.origin.x * scale.x,
        destPosition.y + frame.origin.y * scale.y
    };
    DrawCircle(static_cast<int>(originPos.x), static_cast<int>(originPos.y), 3.0f, outlineColor);
    
    // offset位置を描画
    DrawLine(
        static_cast<int>(position.x), static_cast<int>(position.y),
        static_cast<int>(destPosition.x), static_cast<int>(destPosition.y),
        GREEN
    );
}

} // namespace Shared::Data::Graphics

