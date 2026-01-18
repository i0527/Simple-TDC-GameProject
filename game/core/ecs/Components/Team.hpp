#pragma once

namespace game {
namespace core {
namespace ecs {
namespace components {

/// @brief 陣営
enum class Faction {
    Player,
    Enemy
};

/// @brief 陣営コンポーネント（POD）
struct Team {
    Faction faction = Faction::Enemy;

    Team() = default;
    explicit Team(Faction f) : faction(f) {}
};

} // namespace components
} // namespace ecs
} // namespace core
} // namespace game

