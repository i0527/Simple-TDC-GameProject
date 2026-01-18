#pragma once

// 標準ライブラリ
#include <array>
#include <string>

namespace game {
namespace core {
namespace ecs {
namespace components {

/// @brief 装備スロットコンポーネント（POD）
///
/// PlayerDataManagerの保存データ（装備ID）をECSへ反映するための器です。
/// 実際の効果適用はシステム/計算側で行います。
struct Equipment {
    std::array<std::string, 3> ids{};

    Equipment() = default;
    explicit Equipment(const std::array<std::string, 3>& equipmentIds) : ids(equipmentIds) {}
};

} // namespace components
} // namespace ecs
} // namespace core
} // namespace game

