#pragma once

#include <string>

namespace Shared::Data {

/// @brief アビリティ定義
struct AbilityDef {
    std::string id;
    std::string name;
    std::string description;
    std::string type;  // "stat_boost" / "special_effect"

    // 効果パラメータ（JSONで柔軟に対応）
    struct Effect {
        std::string stat_type;  // "hp" / "attack" / "attack_speed" / "range" / "move_speed"
        float value = 0.0f;
        bool is_percentage = false;
    };
    Effect effect;

    std::vector<std::string> tags;
};

} // namespace Shared::Data
