#pragma once

// 標準ライブラリ
#include <string>

namespace game {
namespace core {
namespace entities {

// タワーアタッチメント効果タイプ
enum class TowerAttachmentEffectType {
    Percentage, // 割合（例: +10% => value_per_level=0.10）
    Flat        // 固定値（未使用: 将来拡張用）
};

// タワーアタッチメント対象ステータス
enum class TowerAttachmentTargetStat {
    TowerHp,         // 城HP
    WalletGrowth,    // お金成長/秒
    CostRegen,       // コスト回復/秒
    AllyAttack,      // 味方攻撃
    AllyHp,          // 味方HP
    EnemyHp,         // 敵HP
    EnemyAttack,     // 敵攻撃
    EnemyMoveSpeed   // 敵移動速度
};

// タワーアタッチメント定義
struct TowerAttachment {
    std::string id;
    std::string name;
    std::string description;
    TowerAttachmentEffectType effect_type = TowerAttachmentEffectType::Percentage;
    TowerAttachmentTargetStat target_stat = TowerAttachmentTargetStat::TowerHp;
    float value_per_level = 0.0f;
    int max_level = 1;
    int rarity = 1;
};

} // namespace entities
} // namespace core
} // namespace game
