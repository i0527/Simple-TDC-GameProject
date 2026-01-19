#pragma once

#include "../config/RenderTypes.hpp"

// ============================================================================
// 繧ｪ繝ｼ繝舌・繝ｬ繧､繧ｷ繧ｹ繝・Β邨ｱ荳繧ｫ繝ｩ繝ｼ繝代Ξ繝・ヨ・郁ｦ冶ｪ肴ｧ驥崎ｦ厄ｼ・
// ============================================================================
// 縺吶∋縺ｦ縺ｮ繧ｪ繝ｼ繝舌・繝ｬ繧､縺ｧ菴ｿ逕ｨ縺吶ｋ濶ｲ繧堤ｵｱ荳邂｡逅・＠縺ｾ縺・
// 繧ｳ繝ｳ繝医Λ繧ｹ繝域ｯ・.5:1莉･荳翫ｒ遒ｺ菫昴＠縲∬ｦ冶ｪ肴ｧ繧呈怙蜆ｪ蜈医↓險ｭ險・

namespace game {
namespace core {
namespace ui {

// ============================================================================
// 閭梧勹繝ｻ繝吶・繧ｹ濶ｲ・域囓縺吶℃縺ｪ縺・ｸｭ髢楢ｪｿ・・
// ============================================================================
namespace OverlayColors {

// ============================================================================
// Tokyo Night鬚ｨ繝繝ｼ繧ｯ繝・・繝橸ｼ医ち繧､繝医Ν/繝帙・繝逕ｨ・・
// ============================================================================
inline constexpr Color MAIN_BG = {26, 27, 38, 255};         // 豺ｱ縺・搨邏ｫ #1a1b26
inline constexpr Color PANEL_BG_GITHUB = {36, 40, 59, 255}; // 蟆代＠譏弱ｋ繧√・繝繝ｼ繧ｯ繧ｰ繝ｬ繝ｼ/髱堤ｴｫ #24283b
inline constexpr Color CARD_BG_GITHUB = {48, 52, 70, 255};  // 繧ｫ繝ｼ繝芽レ譎ｯ #303446
inline constexpr Color ACCENT_PRIMARY = {122, 162, 247, 255}; // 諠・ｱ繝悶Ν繝ｼ #7aa2f7
inline constexpr Color TEXT_MAIN_GITHUB = {192, 202, 245, 255}; // 繧ｪ繝輔・繝ｯ繧､繝・#c0caf5
inline constexpr Color TEXT_SUB_GITHUB = {154, 165, 206, 255};  // 阮・＞髱偵げ繝ｬ繝ｼ #9aa5ce
inline constexpr Color TEXT_DISABLED_GITHUB = {103, 110, 149, 255}; // 辟｡蜉ｹ繝・く繧ｹ繝・#676e95

// 譌ｧ繧ｫ繝ｩ繝ｼ繝代Ξ繝・ヨ・亥ｾ梧婿莠呈鋤諤ｧ邯ｭ謖・ｼ・ Tokyo Night鬚ｨ縺ｫ譖ｴ譁ｰ
// 繝代ロ繝ｫ閭梧勹・域ｿ・ｴｺ・・
inline constexpr Color PANEL_BG_PRIMARY = {30, 33, 46, 255};  // #1e2132
// 繝代ロ繝ｫ閭梧勹・井ｸｭ邏ｺ・・
inline constexpr Color PANEL_BG_SECONDARY = {36, 40, 59, 255}; // #24283b
// 繧ｫ繝ｼ繝芽レ譎ｯ・磯壼ｸｸ・・
inline constexpr Color CARD_BG_NORMAL = {48, 52, 70, 255};    // #303446
// 繧ｫ繝ｼ繝芽レ譎ｯ・磯∈謚樊凾・・
inline constexpr Color CARD_BG_SELECTED = {65, 72, 104, 255};  // #414868

// 蠕梧婿莠呈鋤諤ｧ縺ｮ縺溘ａ縺ｮ繧ｨ繧､繝ｪ繧｢繧ｹ
inline constexpr Color PANEL_BG = PANEL_BG_SECONDARY;
inline constexpr Color PANEL_BG_DARK = PANEL_BG_PRIMARY;
inline constexpr Color PANEL_BG_BROWN = CARD_BG_NORMAL;
inline constexpr Color PANEL_BG_ORANGE = PANEL_BG_SECONDARY;
inline constexpr Color PANEL_BG_ORANGE_DARK = PANEL_BG_PRIMARY;
inline constexpr Color PANEL_BG_ORANGE_LIGHT = CARD_BG_SELECTED;
inline constexpr Color PANEL_ACCENT = CARD_BG_SELECTED;

// 繧ｪ繝ｼ繝舌・繝ｬ繧､閭梧勹・亥濠騾乗・・・ Tokyo Night鬚ｨ
inline constexpr Color OVERLAY_BG = {30, 33, 46, 230};

// 繧ｹ繝ｭ繝・ヨ閭梧勹
inline constexpr Color SLOT_EMPTY = CARD_BG_NORMAL;
inline constexpr Color SLOT_HOVER = CARD_BG_SELECTED;
inline constexpr Color SLOT_SELECTED = CARD_BG_SELECTED;
inline constexpr Color SLOT_ASSIGNED = {55, 70, 84, 200};
inline constexpr Color SLOT_ORANGE_EMPTY = CARD_BG_NORMAL;
inline constexpr Color SLOT_ORANGE_SELECTED = CARD_BG_SELECTED;

// ============================================================================
// 繧｢繧ｯ繧ｻ繝ｳ繝郁牡・郁ｦ冶ｪ肴ｧ譛蜆ｪ蜈茨ｼ・ Tokyo Night鬚ｨ
// ============================================================================
inline constexpr Color ACCENT_GOLD = {224, 175, 104, 255};      // 繧ｴ繝ｼ繝ｫ繝・#e0af68
inline constexpr Color ACCENT_BLUE = {122, 162, 247, 255};     // 繝悶Ν繝ｼ #7aa2f7
inline constexpr Color SUCCESS_GREEN = {158, 206, 106, 255};   // 繧ｽ繝輔ヨ繧ｰ繝ｪ繝ｼ繝ｳ #9ece6a
inline constexpr Color ACCENT_SUCCESS_GITHUB = {158, 206, 106, 255};  // 謌仙粥繧ｰ繝ｪ繝ｼ繝ｳ
inline constexpr Color WARNING_ORANGE = {255, 158, 100, 255}; // 繧ｪ繝ｬ繝ｳ繧ｸ #ff9e64
inline constexpr Color ACCENT_WARNING_GITHUB = {255, 158, 100, 255};  // 隴ｦ蜻翫が繝ｬ繝ｳ繧ｸ
inline constexpr Color DANGER_RED = {247, 118, 142, 255};      // 繧ｽ繝輔ヨ繝ｬ繝・ラ #f7768e

// ============================================================================
// 譫邱壹・繝懊・繝繝ｼ・域・遒ｺ縺ｪ隕冶ｦ夂噪蛹ｺ蛻･・・ Tokyo Night鬚ｨ
// ============================================================================
inline constexpr Color CARD_BORDER_NORMAL = {103, 110, 149, 255};  // #676e95
inline constexpr Color CARD_BORDER_HOVER = ACCENT_GOLD;            // #e0af68
inline constexpr Color CARD_BORDER_SELECTED = {224, 175, 104, 255}; // #e0af68

// 蠕梧婿莠呈鋤諤ｧ縺ｮ縺溘ａ縺ｮ繧ｨ繧､繝ｪ繧｢繧ｹ
inline constexpr Color BORDER_DEFAULT = CARD_BORDER_NORMAL;
inline constexpr Color BORDER_HOVER = CARD_BORDER_HOVER;
inline constexpr Color BORDER_GOLD = ACCENT_GOLD;
inline constexpr Color BORDER_BLUE = ACCENT_BLUE;
inline constexpr Color DIVIDER = CARD_BORDER_NORMAL;

// ============================================================================
// 繝・く繧ｹ繝磯嚴螻､・医さ繝ｳ繝医Λ繧ｹ繝・.5:1莉･荳奇ｼ・ Tokyo Night鬚ｨ
// ============================================================================
inline constexpr Color TEXT_PRIMARY = {192, 202, 245, 255};    // 繧ｪ繝輔・繝ｯ繧､繝・#c0caf5
inline constexpr Color TEXT_SECONDARY = {154, 165, 206, 255};  // 阮・＞髱偵げ繝ｬ繝ｼ #9aa5ce
inline constexpr Color TEXT_MUTED = {103, 110, 149, 255};      // 繝溘Η繝ｼ繝・#676e95
inline constexpr Color TEXT_DISABLED = {65, 72, 104, 255};     // 辟｡蜉ｹ #414868
inline constexpr Color TEXT_DARK = {26, 27, 38, 255};          // 證励＞繝・く繧ｹ繝茨ｼ域・繧九＞閭梧勹逕ｨ・・#1a1b26

// 蠕梧婿莠呈鋤諤ｧ縺ｮ縺溘ａ縺ｮ繧ｨ繧､繝ｪ繧｢繧ｹ
inline constexpr Color TEXT_ACCENT = ACCENT_GOLD;
inline constexpr Color TEXT_GOLD = ACCENT_GOLD;
inline constexpr Color TEXT_ERROR = DANGER_RED;
inline constexpr Color TEXT_SUCCESS = SUCCESS_GREEN;
inline constexpr Color TEXT_BLUE = ACCENT_BLUE;

// ============================================================================
// 迥ｶ諷玖牡・域・遒ｺ縺ｪ隕冶ｦ夂噪蛹ｺ蛻･・・
// ============================================================================
inline constexpr Color BUTTON_PRIMARY = SUCCESS_GREEN;              // #9ece6a
inline constexpr Color BUTTON_PRIMARY_HOVER = {174, 220, 130, 255};  // 譏弱ｋ縺・げ繝ｪ繝ｼ繝ｳ
inline constexpr Color BUTTON_PRIMARY_ACTIVE = {142, 192, 94, 255};  // 繧｢繧ｯ繝・ぅ繝悶げ繝ｪ繝ｼ繝ｳ

inline constexpr Color BUTTON_SECONDARY = ACCENT_BLUE;               // #7aa2f7
inline constexpr Color BUTTON_SECONDARY_HOVER = {147, 178, 250, 255}; // 譏弱ｋ縺・ヶ繝ｫ繝ｼ

inline constexpr Color BUTTON_DISABLED = TEXT_MUTED;

inline constexpr Color BUTTON_RESET = CARD_BG_NORMAL;
inline constexpr Color BUTTON_RESET_HOVER = CARD_BG_SELECTED;

inline constexpr Color BUTTON_BLUE = ACCENT_BLUE;                    // #7aa2f7
inline constexpr Color BUTTON_BLUE_HOVER = {147, 178, 250, 255};    // 譏弱ｋ縺・ヶ繝ｫ繝ｼ

// 繝懊ち繝ｳ濶ｲ・磯∈謚槭ワ繧､繝ｩ繧､繝育畑・・ 蜻ｽ蜷阪→螳滓・繧剃ｸ閾ｴ
inline constexpr Color BUTTON_ORANGE = {255, 158, 100, 255};      // 繧ｪ繝ｬ繝ｳ繧ｸ #ff9e64
inline constexpr Color BUTTON_ORANGE_HOVER = {255, 183, 130, 255}; // 譏弱ｋ縺・が繝ｬ繝ｳ繧ｸ
inline constexpr Color BUTTON_ORANGE_DARK = CARD_BG_NORMAL;
inline constexpr Color BUTTON_ORANGE_DARK_HOVER = CARD_BG_SELECTED;

// 繧ｹ繝・・繧ｸ迥ｶ諷玖牡
inline constexpr Color STAGE_LOCKED = {96, 125, 139, 255};
inline constexpr Color STAGE_BOSS = {255, 87, 34, 255};
inline constexpr Color STAGE_CLEAR = {139, 195, 74, 255};

// ============================================================================
// 繧ｹ繝・・繧ｿ繧ｹ陦ｨ遉ｺ濶ｲ
// ============================================================================
inline constexpr Color STATUS_POSITIVE = SUCCESS_GREEN;
inline constexpr Color STATUS_NEGATIVE = DANGER_RED;
inline constexpr Color STATUS_NEUTRAL = ACCENT_GOLD;

// ============================================================================
// 繝倥ャ繝繝ｼ濶ｲ
// ============================================================================
inline constexpr Color HEADER_BG = PANEL_BG_SECONDARY;
inline constexpr Color HEADER_BG_DARK = PANEL_BG_PRIMARY;

// ============================================================================
// 繧ｰ繝ｩ繝・・繧ｷ繝ｧ繝ｳ閭梧勹・亥･･陦後″諢滂ｼ・ Tokyo Night鬚ｨ
// ============================================================================
inline constexpr Color PANEL_GRADIENT_TOP = {26, 27, 38, 255};    // 豺ｱ縺・搨邏ｫ #1a1b26
inline constexpr Color PANEL_GRADIENT_BOTTOM = {36, 40, 59, 255}; // 繝代ロ繝ｫ閭梧勹 #24283b
inline constexpr Color PANEL_GRADIENT_EDGE = {48, 52, 70, 255};   // 繧ｫ繝ｼ繝芽レ譎ｯ #303446

// ============================================================================
// 繧ｨ繝輔ぉ繧ｯ繝郁牡・亥ｽｱ繝ｻ繧ｰ繝ｭ繝ｼ繝ｻ繝上う繝ｩ繧､繝茨ｼ・ Tokyo Night鬚ｨ
// ============================================================================
inline constexpr Color SHADOW_COLOR = {0, 0, 0, 120};              // 繝峨Ο繝・・繧ｷ繝｣繝峨え
inline constexpr Color GLOW_GREEN = {158, 206, 106, 76};          // 邱代げ繝ｭ繝ｼ (alpha 30%)
inline constexpr Color GLOW_GOLD = {224, 175, 104, 102};          // 驥代げ繝ｭ繝ｼ (alpha 40%)
inline constexpr Color HIGHLIGHT_TOP = {224, 175, 104, 153};      // 荳企Κ繝上う繝ｩ繧､繝・(alpha 60%)
inline constexpr Color PARTICLE_GOLD = {224, 175, 104, 30};       // 邊貞ｭ宣≡濶ｲ (alpha 10-30)

// ============================================================================
// 繝懊ち繝ｳ繧ｰ繝ｩ繝・・繧ｷ繝ｧ繝ｳ濶ｲ - Tokyo Night鬚ｨ
// ============================================================================
inline constexpr Color BUTTON_PRIMARY_DARK = {142, 192, 94, 255};  // 繧｢繧ｯ繝・ぅ繝悶げ繝ｪ繝ｼ繝ｳ
inline constexpr Color BUTTON_PRIMARY_BRIGHT = {158, 206, 106, 255}; // 繧ｽ繝輔ヨ繧ｰ繝ｪ繝ｼ繝ｳ
inline constexpr Color BUTTON_SECONDARY_DARK = {98, 142, 230, 255}; // 證鈴搨
inline constexpr Color BUTTON_SECONDARY_BRIGHT = {122, 162, 247, 255}; // 繝悶Ν繝ｼ

// ============================================================================
// 蠕梧婿莠呈鋤諤ｧ縺ｮ縺溘ａ縺ｮ繧ｨ繧､繝ｪ繧｢繧ｹ・域立繧ｫ繝ｩ繝ｼ繝代Ξ繝・ヨ・・
// ============================================================================
inline constexpr Color ORANGE_PANEL_BG = PANEL_BG_SECONDARY;
inline constexpr Color ORANGE_PANEL_BG_DARK = PANEL_BG_PRIMARY;
inline constexpr Color ORANGE_PANEL_BG_LIGHT = CARD_BG_SELECTED;

} // namespace OverlayColors

} // namespace ui
} // namespace core
} // namespace game
