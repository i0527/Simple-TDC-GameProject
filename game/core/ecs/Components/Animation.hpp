#pragma once

namespace game {
namespace core {
namespace ecs {
namespace components {

/// @brief アニメーションコンポーネント
enum class AnimationType {
    Idle,   // 待機
    Move,   // 移動
    Attack, // 攻撃
    None    // なし
};

struct Animation {
    int frame_count = 1;           // 総フレーム数
    float frame_duration = 0.1f;   // 1フレームの表示時間（秒）
    int current_frame = 0;         // 現在のフレーム
    float frame_timer = 0.0f;      // フレームタイマー
    AnimationType type = AnimationType::None;  // アニメーションタイプ
    bool is_looping = true;        // ループするか

    Animation() = default;
    Animation(int count, float duration, AnimationType anim_type, bool loop = true)
        : frame_count(count), frame_duration(duration), type(anim_type), is_looping(loop) {}

    void Reset() {
        current_frame = 0;
        frame_timer = 0.0f;
    }
};

} // namespace components
} // namespace ecs
} // namespace core
} // namespace game
