#pragma once

// 標準ライブラリ
#include <array>
#include <algorithm>
#include <unordered_map>

// プロジェクト内
#include "PlayerDataManager.hpp"
#include "../ecs/entities/TowerAttachment.hpp"

namespace game {
namespace core {
namespace system {

/// @brief タワー強化による最終補正（乗算）
struct TowerEnhancementMultipliers {
    float playerTowerHpMul = 1.0f;
    float walletGrowthMul = 1.0f;
    float costRegenMul = 1.0f;
    float allyAttackMul = 1.0f;
    float allyHpMul = 1.0f;
    float enemyHpMul = 1.0f;
    float enemyAttackMul = 1.0f;
    float enemyMoveSpeedMul = 1.0f;
};

namespace detail {

inline int ClampLevel(int v, int maxLevel) {
    return std::max(0, std::min(maxLevel, v));
}

inline float MulFromPercentPerLevel(int level, float percentPerLevel) {
    // level=0 => 1.0
    return 1.0f + percentPerLevel * static_cast<float>(level);
}

} // namespace detail

/// @brief セーブのタワー強化レベルから、各種乗算補正を計算
inline TowerEnhancementMultipliers CalculateTowerEnhancementMultipliers(
    const PlayerDataManager::PlayerSaveData::TowerEnhancementState& st,
    const std::array<PlayerDataManager::PlayerSaveData::TowerAttachmentSlot, 3>& attachments,
    const std::unordered_map<std::string, entities::TowerAttachment>& attachmentMasters) {

    // v1: 数値は暫定（後で data/ のJSONへ分離しやすいようにここへ集約）
    constexpr int MAX_LEVEL = 50;

    constexpr float TOWER_HP_PERCENT_PER_LV = 0.05f;       // +5%/Lv
    constexpr float WALLET_GROWTH_PERCENT_PER_LV = 0.05f;  // +5%/Lv
    constexpr float COST_REGEN_PERCENT_PER_LV = 0.05f;     // +5%/Lv
    constexpr float ALLY_ATK_PERCENT_PER_LV = 0.02f;       // +2%/Lv
    constexpr float ALLY_HP_PERCENT_PER_LV = 0.02f;        // +2%/Lv

    const int towerHpLv = detail::ClampLevel(st.towerHpLevel, MAX_LEVEL);
    const int walletGrowthLv = detail::ClampLevel(st.walletGrowthLevel, MAX_LEVEL);
    const int costRegenLv = detail::ClampLevel(st.costRegenLevel, MAX_LEVEL);
    const int allyAtkLv = detail::ClampLevel(st.allyAttackLevel, MAX_LEVEL);
    const int allyHpLv = detail::ClampLevel(st.allyHpLevel, MAX_LEVEL);

    TowerEnhancementMultipliers m;
    m.playerTowerHpMul = detail::MulFromPercentPerLevel(towerHpLv, TOWER_HP_PERCENT_PER_LV);
    m.walletGrowthMul = detail::MulFromPercentPerLevel(walletGrowthLv, WALLET_GROWTH_PERCENT_PER_LV);
    m.costRegenMul = detail::MulFromPercentPerLevel(costRegenLv, COST_REGEN_PERCENT_PER_LV);
    m.allyAttackMul = detail::MulFromPercentPerLevel(allyAtkLv, ALLY_ATK_PERCENT_PER_LV);
    m.allyHpMul = detail::MulFromPercentPerLevel(allyHpLv, ALLY_HP_PERCENT_PER_LV);

    for (const auto& slot : attachments) {
        if (slot.id.empty()) continue;
        auto it = attachmentMasters.find(slot.id);
        if (it == attachmentMasters.end()) continue;
        const auto& attachment = it->second;
        const int level = detail::ClampLevel(slot.level, std::max(1, attachment.max_level));
        const float mul = std::max(0.0f, 1.0f + attachment.value_per_level * static_cast<float>(level));

        switch (attachment.target_stat) {
        case entities::TowerAttachmentTargetStat::TowerHp:
            m.playerTowerHpMul = std::max(0.0f, m.playerTowerHpMul * mul);
            break;
        case entities::TowerAttachmentTargetStat::WalletGrowth:
            m.walletGrowthMul = std::max(0.0f, m.walletGrowthMul * mul);
            break;
        case entities::TowerAttachmentTargetStat::CostRegen:
            m.costRegenMul = std::max(0.0f, m.costRegenMul * mul);
            break;
        case entities::TowerAttachmentTargetStat::AllyAttack:
            m.allyAttackMul = std::max(0.0f, m.allyAttackMul * mul);
            break;
        case entities::TowerAttachmentTargetStat::AllyHp:
            m.allyHpMul = std::max(0.0f, m.allyHpMul * mul);
            break;
        case entities::TowerAttachmentTargetStat::EnemyHp:
            m.enemyHpMul = std::max(0.0f, m.enemyHpMul * mul);
            break;
        case entities::TowerAttachmentTargetStat::EnemyAttack:
            m.enemyAttackMul = std::max(0.0f, m.enemyAttackMul * mul);
            break;
        case entities::TowerAttachmentTargetStat::EnemyMoveSpeed:
            m.enemyMoveSpeedMul = std::max(0.0f, m.enemyMoveSpeedMul * mul);
            break;
        }
    }
    return m;
}

} // namespace system
} // namespace core
} // namespace game

