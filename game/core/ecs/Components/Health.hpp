#pragma once

namespace game {
namespace core {
namespace ecs {
namespace components {

/// @brief HPコンポーネント
struct Health {
    int current = 100;
    int max = 100;

    Health() = default;
    Health(int max_hp) : current(max_hp), max(max_hp) {}
    Health(int current_hp, int max_hp) : current(current_hp), max(max_hp) {}

    bool IsAlive() const { return current > 0; }
    float GetPercentage() const { 
        return (max > 0) ? (static_cast<float>(current) / static_cast<float>(max)) : 0.0f;
    }
};

} // namespace components
} // namespace ecs
} // namespace core
} // namespace game
