#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Shared::Data {

/// @brief スキル定義
struct SkillDef {
    std::string id;
    std::string name;
    std::string description;
    std::string type;  // "passive" / "interrupt" / "event"

    // Event型の場合のトリガー
    struct EventTrigger {
        std::string event_type;  // "on_spawn" / "on_attack" / "on_hit" / "on_death" / "on_interval"
        float interval = 0.0f;   // on_interval の場合の間隔（秒）
    } event_trigger;

    // スキル効果
    struct Effect {
        std::string effect_type;  // "damage" / "heal" / "buff" / "debuff" / "spawn" / "knockback"
        nlohmann::json parameters; // 効果パラメータ（柔軟に対応）
    };
    std::vector<Effect> effects;

    // ターゲット設定
    struct Target {
        std::string target_type;  // "self" / "enemy" / "ally" / "area"
        int range = 0;
        int max_targets = 1;
    } target;

    // クールダウン
    float cooldown = 0.0f;

    // 発動確率（0.0 ~ 1.0）
    float activation_chance = 1.0f;

    std::vector<std::string> tags;
};

} // namespace Shared::Data
