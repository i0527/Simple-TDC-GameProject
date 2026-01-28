#pragma once

// 標準ライブラリ
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

// プロジェクト内
#include "GachaOverlay.hpp"

namespace game {
namespace core {
namespace gacha_internal {

constexpr int COST_SINGLE = 1;
constexpr int COST_TEN = 9;
constexpr float kPi = 3.14159265358979323846f;
constexpr float HOME_HEADER_H = 90.0f;
constexpr float HOME_TAB_H = 90.0f;
constexpr int PITY_HARD = 50;
constexpr int HISTORY_DISPLAY_LIMIT = 100;
constexpr int DUST_FOR_TICKET = 10;
constexpr int DUST_FOR_TEN_TICKETS = 90;
constexpr float TAB_BUTTON_W = 140.0f;
constexpr float TAB_BUTTON_H = 36.0f;
constexpr float TAB_BUTTON_SPACING = 8.0f;
constexpr float GACHA_HEADER_H = 96.0f;
constexpr float GACHA_HEADER_PADDING_X = 32.0f;
constexpr float GACHA_HEADER_PADDING_Y = 18.0f;
constexpr float GACHA_HEADER_DIVIDER_Y = 86.0f;
constexpr float GACHA_TAB_ROW_GAP = 24.0f;
constexpr float GACHA_CONTENT_PADDING = 20.0f;
constexpr float GACHA_CONTENT_RADIUS = 8.0f;
constexpr float GACHA_STATUS_BADGE_RADIUS = 10.0f;
constexpr float GACHA_STATUS_BADGE_PAD_X = 10.0f;
constexpr float GACHA_STATUS_BADGE_PAD_Y = 4.0f;
constexpr float GACHA_STATUS_BADGE_SPACING = 6.0f;
constexpr float GACHA_TAB_BG_H = 52.0f;
constexpr float GACHA_TAB_ACTIVE_UNDERLINE_H = 3.0f;
constexpr float GACHA_NOTICE_Y_OFFSET = 70.0f;
constexpr float GACHA_FOOTNOTE_Y_OFFSET = 90.0f;

constexpr float GACHA_PANEL_BORDER_R = 1.0f;
constexpr float GACHA_PANEL_BORDER_G = 1.0f;
constexpr float GACHA_PANEL_BORDER_B = 1.0f;
constexpr float GACHA_PANEL_BORDER_A = 0.08f;

constexpr float GACHA_HEADER_BG_R = 0.08f;
constexpr float GACHA_HEADER_BG_G = 0.09f;
constexpr float GACHA_HEADER_BG_B = 0.12f;
constexpr float GACHA_HEADER_BG_A = 0.85f;

constexpr float GACHA_TAB_BG_R = 0.07f;
constexpr float GACHA_TAB_BG_G = 0.08f;
constexpr float GACHA_TAB_BG_B = 0.1f;
constexpr float GACHA_TAB_BG_A = 0.6f;
constexpr float GACHA_TAB_ACTIVE_BG_R = 0.12f;
constexpr float GACHA_TAB_ACTIVE_BG_G = 0.13f;
constexpr float GACHA_TAB_ACTIVE_BG_B = 0.16f;
constexpr float GACHA_TAB_ACTIVE_BG_A = 0.8f;
constexpr float GACHA_TAB_HOVER_BG_R = 0.1f;
constexpr float GACHA_TAB_HOVER_BG_G = 0.11f;
constexpr float GACHA_TAB_HOVER_BG_B = 0.13f;
constexpr float GACHA_TAB_HOVER_BG_A = 0.7f;

constexpr float GACHA_CONTENT_BG_R = 0.06f;
constexpr float GACHA_CONTENT_BG_G = 0.07f;
constexpr float GACHA_CONTENT_BG_B = 0.09f;
constexpr float GACHA_CONTENT_BG_A = 0.55f;

constexpr float GACHA_BADGE_BG_R = 0.15f;
constexpr float GACHA_BADGE_BG_G = 0.16f;
constexpr float GACHA_BADGE_BG_B = 0.2f;
constexpr float GACHA_BADGE_BG_A = 0.7f;

constexpr float GACHA_ACCENT_R = 0.95f;
constexpr float GACHA_ACCENT_G = 0.78f;
constexpr float GACHA_ACCENT_B = 0.3f;
constexpr float GACHA_ACCENT_A = 0.9f;

// レアリティカラー定義
// N: グレー系
constexpr float GACHA_RARITY_N_BG_R = 0.25f;
constexpr float GACHA_RARITY_N_BG_G = 0.25f;
constexpr float GACHA_RARITY_N_BG_B = 0.25f;
constexpr float GACHA_RARITY_N_BG_A = 0.8f;
constexpr float GACHA_RARITY_N_BORDER_R = 0.4f;
constexpr float GACHA_RARITY_N_BORDER_G = 0.4f;
constexpr float GACHA_RARITY_N_BORDER_B = 0.4f;
constexpr float GACHA_RARITY_N_BORDER_A = 0.9f;

// R: 青系
constexpr float GACHA_RARITY_R_BG_R = 0.2f;
constexpr float GACHA_RARITY_R_BG_G = 0.3f;
constexpr float GACHA_RARITY_R_BG_B = 0.5f;
constexpr float GACHA_RARITY_R_BG_A = 0.8f;
constexpr float GACHA_RARITY_R_BORDER_R = 0.48f;
constexpr float GACHA_RARITY_R_BORDER_G = 0.64f;
constexpr float GACHA_RARITY_R_BORDER_B = 0.97f;
constexpr float GACHA_RARITY_R_BORDER_A = 0.9f;

// SR: 紫系
constexpr float GACHA_RARITY_SR_BG_R = 0.35f;
constexpr float GACHA_RARITY_SR_BG_G = 0.2f;
constexpr float GACHA_RARITY_SR_BG_B = 0.45f;
constexpr float GACHA_RARITY_SR_BG_A = 0.8f;
constexpr float GACHA_RARITY_SR_BORDER_R = 0.7f;
constexpr float GACHA_RARITY_SR_BORDER_G = 0.4f;
constexpr float GACHA_RARITY_SR_BORDER_B = 0.85f;
constexpr float GACHA_RARITY_SR_BORDER_A = 0.9f;

// SSR: 金色/オレンジ系
constexpr float GACHA_RARITY_SSR_BG_R = 0.5f;
constexpr float GACHA_RARITY_SSR_BG_G = 0.35f;
constexpr float GACHA_RARITY_SSR_BG_B = 0.15f;
constexpr float GACHA_RARITY_SSR_BG_A = 0.85f;
constexpr float GACHA_RARITY_SSR_BORDER_R = 0.95f;
constexpr float GACHA_RARITY_SSR_BORDER_G = 0.78f;
constexpr float GACHA_RARITY_SSR_BORDER_B = 0.3f;
constexpr float GACHA_RARITY_SSR_BORDER_A = 1.0f;

inline int GetCostByRollCount(int rollCount) {
    return rollCount == 10 ? COST_TEN : COST_SINGLE;
}

inline float EaseOutCubic(float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    const float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
}

inline int GetRarityWeightInternal(GachaRarity rarity) {
    switch (rarity) {
    case GachaRarity::N:
        return 60;
    case GachaRarity::R:
        return 30;
    case GachaRarity::SR:
        return 9;
    case GachaRarity::SSR:
        return 1;
    }
    return 1;
}

inline int GetDustRewardInternal(GachaRarity rarity) {
    switch (rarity) {
    case GachaRarity::N:
        return 1;
    case GachaRarity::R:
        return 3;
    case GachaRarity::SR:
        return 10;
    case GachaRarity::SSR:
        return 30;
    }
    return 0;
}

inline std::string FormatPercent(float value) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << value;
    return ss.str();
}

} // namespace gacha_internal
} // namespace core
} // namespace game
