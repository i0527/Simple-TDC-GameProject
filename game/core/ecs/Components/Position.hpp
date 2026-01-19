#pragma once

#include "../../config/RenderTypes.hpp"

namespace game {
namespace core {
namespace ecs {
namespace components {

/// @brief 位置コンポーネント
struct Position {
    float x = 0.0f;
    float y = 0.0f;

    Position() = default;
    Position(float x, float y) : x(x), y(y) {}
    Position(Vector2 pos) : x(pos.x), y(pos.y) {}

    Vector2 ToVector2() const { return Vector2{ x, y }; }
};

} // namespace components
} // namespace ecs
} // namespace core
} // namespace game
