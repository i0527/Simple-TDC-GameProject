#pragma once

// 標準ライブラリ
#include <array>
#include <string>

namespace game {
namespace core {
namespace ecs {
namespace components {

/// @brief パッシブスキルのスロット情報（POD）
struct PassiveSkillInstance {
    std::string id;
    int level = 1;

    PassiveSkillInstance() = default;
    PassiveSkillInstance(const std::string& passiveId, int passiveLevel)
        : id(passiveId), level(passiveLevel) {}
};

/// @brief パッシブスキルスロットコンポーネント（POD）
///
/// PlayerDataManagerの保存データ（パッシブID/レベル）をECSへ反映するための器です。
/// 実際の効果適用はシステム/計算側で行います。
struct PassiveSkills {
    std::array<PassiveSkillInstance, 3> skills{};

    PassiveSkills() = default;
    explicit PassiveSkills(const std::array<PassiveSkillInstance, 3>& passiveSkills) : skills(passiveSkills) {}
};

} // namespace components
} // namespace ecs
} // namespace core
} // namespace game

