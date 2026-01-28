#pragma once

#include "../../config/RenderTypes.hpp"
#include "../../ecs/entities/TowerAttachment.hpp"
#include <sstream>
#include <string>

namespace game {
namespace core {
namespace enhancement_overlay_internal {

// フォントサイズ定数
constexpr float FONT_TITLE = 64.0f;
constexpr float FONT_SECTION = 36.0f;
constexpr float FONT_HEADER = 28.0f;
constexpr float FONT_BODY = 22.0f;
constexpr float FONT_BUTTON = 22.0f;
constexpr float FONT_CAPTION = 18.0f;

// レイアウト定数
constexpr float PANEL_GAP = 20.0f;
constexpr float PANEL_PADDING = 20.0f;
constexpr float CARD_PADDING = 20.0f;
constexpr float TABLE_ROW_HEIGHT = 80.0f;
constexpr float BUTTON_GAP = 10.0f;

// 基礎強化エリア用（枠内に収まるコンパクト版）
constexpr float BASE_TABLE_TOP_OFFSET = 38.0f;
constexpr float BASE_TABLE_ROW_HEIGHT = 64.0f;
constexpr float BASE_TABLE_HEADER_HEIGHT = 24.0f;
constexpr float BASE_BUTTON_HEIGHT = 36.0f;
constexpr float LEFT_PANEL_WIDTH_RATIO = 0.52f;

// 中央ボタンエリア（ユニットオーバーレイに合わせた2列×3行）
constexpr float BASE_CENTER_BUTTON_H = 44.0f;
constexpr float BASE_CENTER_BUTTON_ROW_GAP = 8.0f;
constexpr float BASE_CENTER_BUTTON_COL_GAP = 10.0f;
constexpr float BASE_CENTER_BUTTON_TOP_MARGIN = 20.0f;

// アタッチメント効果表示・計算で使用する固定レベル（レベル機能なし）
constexpr int ATTACHMENT_EFFECT_DISPLAY_LEVEL = 20;

// 基礎強化レベルアップのGold消費（Lv L→L+1 で 100*(L+1) G）
constexpr int TOWER_BASE_COST_PER_LEVEL = 100;
// 基礎強化を下げたときのGold返却率（0.9 = 90%）
constexpr double TOWER_BASE_REFUND_RATIO = 0.9;

/// @brief 基礎強化を levelsToAdd 段階上げるのに必要な合計Gold
inline int ComputeTowerBaseLevelUpCost(int currentLevel, int levelsToAdd) {
    if (levelsToAdd <= 0) return 0;
    int total = 0;
    for (int i = 0; i < levelsToAdd && (currentLevel + i) < 50; ++i) {
        total += TOWER_BASE_COST_PER_LEVEL * (currentLevel + 1 + i);
    }
    return total;
}

/// @brief 基礎強化を levelsToRemove 段階下げたときの返却Gold（消費コストの 90%）
inline int ComputeTowerBaseRefund(int newLevel, int levelsToRemove) {
    if (levelsToRemove <= 0) return 0;
    const int costPaid = ComputeTowerBaseLevelUpCost(newLevel, levelsToRemove);
    return static_cast<int>(TOWER_BASE_REFUND_RATIO * costPaid);
}

// 基礎強化5項目の短い説明（項目順）
inline const char* GetBaseEnhancementDescription(int rowIndex) {
    static const char* kDescriptions[] = {
        "城の最大HP",
        "お金の成長率/秒",
        "コスト回復量/秒",
        "味方の攻撃力",
        "味方のHP"
    };
    if (rowIndex >= 0 && rowIndex < 5) return kDescriptions[rowIndex];
    return "";
}

inline const char* ToAttachmentTargetLabel(entities::TowerAttachmentTargetStat stat) {
    switch (stat) {
    case entities::TowerAttachmentTargetStat::TowerHp:
        return "城HP";
    case entities::TowerAttachmentTargetStat::WalletGrowth:
        return "お金成長/秒";
    case entities::TowerAttachmentTargetStat::CostRegen:
        return "コスト回復/秒";
    case entities::TowerAttachmentTargetStat::AllyAttack:
        return "味方攻撃";
    case entities::TowerAttachmentTargetStat::AllyHp:
        return "味方HP";
    case entities::TowerAttachmentTargetStat::EnemyHp:
        return "敵HP";
    case entities::TowerAttachmentTargetStat::EnemyAttack:
        return "敵攻撃";
    case entities::TowerAttachmentTargetStat::EnemyMoveSpeed:
        return "敵移動速度";
    default:
        return "不明";
    }
}

inline std::string BuildAttachmentEffectText(const entities::TowerAttachment& attachment, int level) {
    const float raw = attachment.value_per_level * static_cast<float>(level);
    const float value = raw * 100.0f;
    std::ostringstream oss;
    if (value >= 0.0f) {
        oss << "+";
    }
    oss.setf(std::ios::fixed);
    oss.precision(1);
    oss << value;
    if (attachment.effect_type == entities::TowerAttachmentEffectType::Percentage) {
        oss << "%";
    }
    return oss.str();
}

inline std::string FormatFloat(float value, int precision) {
    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss.precision(precision);
    oss << value;
    return oss.str();
}

inline Rect GetScaledButtonRect(const Rect& rect, bool isHovered) {
    constexpr float scale = 1.05f;
    const float actualScale = isHovered ? scale : 1.0f;
    const float scaled_w = rect.width * actualScale;
    const float scaled_h = rect.height * actualScale;
    const float scaled_x = rect.x - (scaled_w - rect.width) / 2.0f;
    const float scaled_y = rect.y - (scaled_h - rect.height) / 2.0f;
    return Rect{scaled_x, scaled_y, scaled_w, scaled_h};
}

} // namespace enhancement_overlay_internal
} // namespace core
} // namespace game
