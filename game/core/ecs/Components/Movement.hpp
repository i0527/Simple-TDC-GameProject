#pragma once

#include <raylib.h>

namespace game {
namespace core {
namespace ecs {
namespace components {

/// @brief 移動コンポーネント
struct Movement {
    float speed = 0.0f;           // 移動速度（ピクセル/秒）
    Vector2 velocity = {0.0f, 0.0f};  // 現在の速度ベクトル

    Movement() = default;
    Movement(float move_speed) : speed(move_speed) {}
};

} // namespace components
} // namespace ecs
} // namespace core
} // namespace game
