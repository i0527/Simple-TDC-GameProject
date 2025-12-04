/**
 * @file StatusEffectDef.h
 * @brief ステータス効果定義
 *
 * さまざまなバフ/デバフを表現するためのデータ定義。
 */
#pragma once

#include <string>

namespace Data {

/**
 * @brief ステータス効果の種類
 */
enum class StatusEffectType {
    None,
    Slow,
    Stun,
    Poison,
    Burn,
    Freeze,
    AttackUp,
    AttackDown,
    DefenseUp,
    DefenseDown,
    SpeedUp,
    Regeneration,
    Shield,
    Invincible
};

/**
 * @brief ステータス効果定義
 */
struct StatusEffectDef {
    std::string id;
    StatusEffectType type = StatusEffectType::None;
    float value = 0.0f;
    float duration = 0.0f;
    float tickInterval = 0.0f;
    bool isPercentage = true;
    std::string iconPath;
};

} // namespace Data


