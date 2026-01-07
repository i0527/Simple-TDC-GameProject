#pragma once

#include <string>

namespace game {
namespace core {
namespace entities {
namespace animation {

// スプライトアニメーションのメタデータ
struct SpriteAnimationData {
    std::string animation_name;   // アニメーション名（"move", "attack"）
    std::string sprite_sheet_path; // スプライトシートパス
    
    int frame_width;              // 1フレームの幅（ピクセル）
    int frame_height;             // 1フレームの高さ（ピクセル）
    int frame_count;              // 総フレーム数
    
    float frame_duration;         // 各フレームの表示時間（秒）
    bool is_looping;              // ループするか
    
    // デフォルトコンストラクタ
    SpriteAnimationData()
        : animation_name("")
        , sprite_sheet_path("")
        , frame_width(0)
        , frame_height(0)
        , frame_count(0)
        , frame_duration(0.1f)
        , is_looping(true)
    {}
};

} // namespace animation
} // namespace entities
} // namespace core
} // namespace game
