#include "Game/Graphics/SpriteRenderer.h"
#include <raymath.h>

namespace Game::Graphics {

void SpriteRenderer::DrawSprite(
    Shared::Data::Graphics::IFrameProvider& provider,
    const std::string& clipName,
    int frameIndex,
    Vector2 position,
    Vector2 scale,
    float rotation,
    Color tint
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
    
    Rectangle destRect = {
        destPosition.x,
        destPosition.y,
        frame.src.width * scale.x,
        frame.src.height * scale.y
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

void SpriteRenderer::DrawSpriteDebug(
    Shared::Data::Graphics::IFrameProvider& provider,
    const std::string& clipName,
    int frameIndex,
    Vector2 position,
    Vector2 scale,
    Color outlineColor
) {
    // FrameRef 取得
    auto frame = provider.GetFrame(clipName, frameIndex);
    if (!frame.valid) {
        return;
    }
    
    // offset を含めた描画先矩形
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
    
    // デバッグ描画：枠線
    DrawRectangleLines(
        (int)destRect.x,
        (int)destRect.y,
        (int)destRect.width,
        (int)destRect.height,
        outlineColor
    );
    
    // 足元位置を表示（小さい十字）
    DrawLine(
        (int)position.x - 3,
        (int)position.y,
        (int)position.x + 3,
        (int)position.y,
        outlineColor
    );
    DrawLine(
        (int)position.x,
        (int)position.y - 3,
        (int)position.x,
        (int)position.y + 3,
        outlineColor
    );
    
    // origin 位置を表示（小さい円）
    DrawCircle(
        (int)(destRect.x + frame.origin.x),
        (int)(destRect.y + frame.origin.y),
        2.0f,
        outlineColor
    );
}

} // namespace Game::Graphics
