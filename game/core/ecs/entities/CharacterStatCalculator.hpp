#pragma once

// 標準ライブラリ
#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>

// プロジェクト内
#include "../../system/PlayerDataManager.hpp"
#include "Character.hpp"
#include "ItemPassiveManager.hpp"

namespace game {
namespace core {
namespace entities {

/// @brief 装備・パッシブを適用した最終ステータス計算（UI/戦闘で共通化）
class CharacterStatCalculator {
public:
    struct IntStat {
        int base = 0;
        int bonus = 0;
        int final = 0;
    };

    struct FloatStat {
        float base = 0.0f;
        float bonus = 0.0f;
        float final = 0.0f;
    };

    struct Result {
        IntStat hp;
        IntStat attack;
        IntStat defense;
        FloatStat moveSpeed;
        FloatStat range;
        FloatStat attackSpan;
    };

    /// @brief Characterベース値 + セーブロードアウト(装備/パッシブ) を適用して計算
    static Result Calculate(const Character& character,
                            const core::PlayerDataManager::CharacterState& state,
                            const ItemPassiveManager& itemPassiveManager);

private:
    static float ClampPositive(float v, float minValue);
};

// ===== inline implementation =====

inline float CharacterStatCalculator::ClampPositive(float v, float minValue) {
    if (std::isnan(v) || std::isinf(v)) {
        return minValue;
    }
    return std::max(minValue, v);
}

inline CharacterStatCalculator::Result CharacterStatCalculator::Calculate(
    const Character& character,
    const core::PlayerDataManager::CharacterState& state,
    const ItemPassiveManager& itemPassiveManager) {

    constexpr float MIN_ATTACK_SPAN_SEC = 0.10f;
    constexpr int MIN_LEVEL = 1;
    constexpr int MAX_LEVEL = 50;

    // Lv成長（標準プリセット）
    // - ATK/HP: +2.5%/Lv
    // - 移動速度: +1.0%/Lv
    // - 攻撃速度: +1.5%/Lv（attack_span を短縮）
    const int level = std::max(MIN_LEVEL, std::min(MAX_LEVEL, state.level));
    const float lvAtkHpMul = 1.0f + 0.025f * static_cast<float>(level - 1);
    const float lvMoveSpdMul = 1.0f + 0.01f * static_cast<float>(level - 1);
    const float lvAtkSpeedMul = 1.0f + 0.015f * static_cast<float>(level - 1);

    struct FloatAcc {
        float flat = 0.0f;
        float mul = 1.0f;
    };
    auto applyPercentage = [](FloatAcc& acc, float ratio) {
        // ratio: 0.10 => +10%
        acc.mul *= (1.0f + ratio);
    };

    Result r;

    // ベース値（Lv成長を反映）
    r.hp.base = std::max(1, static_cast<int>(std::round(static_cast<float>(character.hp) * lvAtkHpMul)));
    r.attack.base = std::max(0, static_cast<int>(std::round(static_cast<float>(character.attack) * lvAtkHpMul)));
    r.defense.base = character.defense;
    r.moveSpeed.base = character.move_speed * lvMoveSpdMul;
    r.range.base = character.attack_size.x;
    // 攻撃速度は「攻撃間隔」を短縮する（Mulが大きいほど速い）
    r.attackSpan.base = ClampPositive(character.attack_span / std::max(0.001f, lvAtkSpeedMul), MIN_ATTACK_SPAN_SEC);

    // 装備：固定値加算
    int eqHpFlat = 0;
    int eqAtkFlat = 0;
    int eqDefFlat = 0;
    for (int i = 0; i < 3; ++i) {
        const std::string& eid = state.equipment[i];
        if (eid.empty()) continue;
        const auto* eq = itemPassiveManager.GetEquipment(eid);
        if (!eq) continue;
        eqHpFlat += static_cast<int>(eq->hp_bonus);
        eqAtkFlat += static_cast<int>(eq->attack_bonus);
        eqDefFlat += static_cast<int>(eq->defense_bonus);
    }

    // パッシブ：割合/固定を stat ごとに集約
    FloatAcc hpAcc;
    FloatAcc atkAcc;
    FloatAcc defAcc;
    FloatAcc spdAcc;
    FloatAcc rangeAcc;
    FloatAcc atkSpanAcc;

    for (int i = 0; i < 3; ++i) {
        const std::string& pid = state.passives[i].id;
        if (pid.empty()) continue;
        const auto* ps = itemPassiveManager.GetPassiveSkill(pid);
        if (!ps) continue;

        const int level = std::max(1, state.passives[i].level);
        const float v = ps->value * static_cast<float>(level);

        const bool isPercent = (ps->effect_type == PassiveEffectType::Percentage);
        const bool isFlat = (ps->effect_type == PassiveEffectType::Flat);

        switch (ps->target_stat) {
        case PassiveTargetStat::Hp:
            if (isPercent) applyPercentage(hpAcc, v);
            else if (isFlat) hpAcc.flat += v;
            break;
        case PassiveTargetStat::Attack:
            if (isPercent) applyPercentage(atkAcc, v);
            else if (isFlat) atkAcc.flat += v;
            break;
        case PassiveTargetStat::Defense:
            if (isPercent) applyPercentage(defAcc, v);
            else if (isFlat) defAcc.flat += v;
            break;
        case PassiveTargetStat::MoveSpeed:
            if (isPercent) applyPercentage(spdAcc, v);
            else if (isFlat) spdAcc.flat += v;
            break;
        case PassiveTargetStat::Range:
            if (isPercent) applyPercentage(rangeAcc, v);
            else if (isFlat) rangeAcc.flat += v;
            break;
        case PassiveTargetStat::AttackSpeed:
            // AttackSpeed は攻撃間隔へ反映（速くなるほど attack_span が短くなる）
            if (isPercent) applyPercentage(atkSpanAcc, v);
            else if (isFlat) atkSpanAcc.flat += v;
            break;

        case PassiveTargetStat::CritChance:
        case PassiveTargetStat::CritDamage:
        case PassiveTargetStat::GoldGain:
        case PassiveTargetStat::ExpGain:
        default:
            // 未接続: 将来拡張用
            break;
        }
    }

    // ===== 最終値計算 =====
    // HP/ATK/DEF: (base + equipmentFlat + passiveFlat) * passiveMul
    const float hpBeforeMul = static_cast<float>(r.hp.base + eqHpFlat) + hpAcc.flat;
    const float atkBeforeMul = static_cast<float>(r.attack.base + eqAtkFlat) + atkAcc.flat;
    const float defBeforeMul = static_cast<float>(r.defense.base + eqDefFlat) + defAcc.flat;

    const int hpFinal = std::max(1, static_cast<int>(std::round(hpBeforeMul * hpAcc.mul)));
    const int atkFinal = std::max(0, static_cast<int>(std::round(atkBeforeMul * atkAcc.mul)));
    const int defFinal = std::max(0, static_cast<int>(std::round(defBeforeMul * defAcc.mul)));

    r.hp.final = hpFinal;
    r.attack.final = atkFinal;
    r.defense.final = defFinal;

    r.hp.bonus = r.hp.final - r.hp.base;
    r.attack.bonus = r.attack.final - r.attack.base;
    r.defense.bonus = r.defense.final - r.defense.base;

    // MoveSpeed: (base + flat) * mul
    const float spdFinal = ClampPositive((r.moveSpeed.base + spdAcc.flat) * spdAcc.mul, 0.0f);
    r.moveSpeed.final = spdFinal;
    r.moveSpeed.bonus = r.moveSpeed.final - r.moveSpeed.base;

    // Range: (base + flat) * mul
    const float rangeFinal = ClampPositive((r.range.base + rangeAcc.flat) * rangeAcc.mul, 0.0f);
    r.range.final = rangeFinal;
    r.range.bonus = r.range.final - r.range.base;

    // AttackSpan:
    // - Percentage: attack_span /= (1 + v) を mul でまとめる（mul>1 ほど速い）
    // - Flat: 秒数短縮として解釈（attack_span -= flat）
    float span = r.attackSpan.base;
    if (atkSpanAcc.mul > 0.0f) {
        span /= atkSpanAcc.mul;
    }
    span -= atkSpanAcc.flat;
    span = ClampPositive(span, MIN_ATTACK_SPAN_SEC);

    r.attackSpan.final = span;
    r.attackSpan.bonus = r.attackSpan.final - r.attackSpan.base;

    return r;
}

} // namespace entities
} // namespace core
} // namespace game

