#pragma once

namespace game {
namespace core {
namespace ecs {
namespace components {

/// @brief ステータスコンポーネント（攻撃力・防御力）
struct Stats {
    int attack = 0;
    int defense = 0;

    Stats() = default;
    Stats(int attack, int defense) : attack(attack), defense(defense) {}
};

} // namespace components
} // namespace ecs
} // namespace core
} // namespace game
