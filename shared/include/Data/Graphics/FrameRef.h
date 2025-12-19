#ifndef GRAPHICS_FRAME_REF_H
#define GRAPHICS_FRAME_REF_H

#include <raylib.h>
#include <string>

namespace Shared::Data::Graphics {

/**
 * @brief 描画の最小単位
 * 
 * グリッド形式、Asepriteアトラス、TexturePacker など
 * あらゆるスプライトシート形式を統一的に表現
 */
struct FrameRef {
    // ===== 描画に必要な情報 =====
    
    /// テクスチャポインタ（DrawTexturePro で使用）
    const Texture2D* texture = nullptr;
    
    /// テクスチャ内の切り出し矩形（ソース）
    Rectangle src = {0, 0, 0, 0};
    
    /// 描画時の回転中心（足元ベース）
    /// 通常は (src.width/2, src.height) で足元揃え
    Vector2 origin = {0, 0};
    
    /// trim による位置補正（AsepriteのspriteSourceSize対応）
    /// offset.x: 左詰めによる X方向補正
    /// offset.y: 上詰めによる Y方向補正
    Vector2 offset = {0, 0};
    
    // ===== アニメーション情報 =====
    
    /// このフレームの再生時間（秒）
    float durationSec = 1.0f / 12.0f;
    
    /// フレーム有効性（初期化/エラー判定用）
    bool valid = false;
    
    // ===== ヘルパー =====
    
    /// 足元揃えの origin を計算
    /// @return 足元 (中心X、下端Y) の座標
    static Vector2 ComputeFootOrigin(float width, float height) {
        return Vector2{width * 0.5f, height};
    }
    
    /// offset込みの有効描画高さ（ディバッグ用）
    float GetEffectiveHeight() const {
        return src.height + offset.y;
    }
};

} // namespace Shared::Data::Graphics

#endif // GRAPHICS_FRAME_REF_H
