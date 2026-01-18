#pragma once

#include <raylib.h>

// ============================================================================
// オーバーレイシステム統一カラーパレット（視認性重視）
// ============================================================================
// すべてのオーバーレイで使用する色を統一管理します
// コントラスト比8.5:1以上を確保し、視認性を最優先に設計

namespace game {
namespace core {
namespace ui {

// ============================================================================
// 背景・ベース色（暗すぎない中間調）
// ============================================================================
namespace OverlayColors {

// ============================================================================
// Tokyo Night風ダークテーマ（タイトル/ホーム用）
// ============================================================================
inline constexpr Color MAIN_BG = {26, 27, 38, 255};         // 深い青紫 #1a1b26
inline constexpr Color PANEL_BG_GITHUB = {36, 40, 59, 255}; // 少し明るめのダークグレー/青紫 #24283b
inline constexpr Color CARD_BG_GITHUB = {48, 52, 70, 255};  // カード背景 #303446
inline constexpr Color ACCENT_PRIMARY = {122, 162, 247, 255}; // 情報ブルー #7aa2f7
inline constexpr Color TEXT_MAIN_GITHUB = {192, 202, 245, 255}; // オフホワイト #c0caf5
inline constexpr Color TEXT_SUB_GITHUB = {154, 165, 206, 255};  // 薄い青グレー #9aa5ce
inline constexpr Color TEXT_DISABLED_GITHUB = {103, 110, 149, 255}; // 無効テキスト #676e95

// 旧カラーパレット（後方互換性維持）- Tokyo Night風に更新
// パネル背景（濃紺）
inline constexpr Color PANEL_BG_PRIMARY = {30, 33, 46, 255};  // #1e2132
// パネル背景（中紺）
inline constexpr Color PANEL_BG_SECONDARY = {36, 40, 59, 255}; // #24283b
// カード背景（通常）
inline constexpr Color CARD_BG_NORMAL = {48, 52, 70, 255};    // #303446
// カード背景（選択時）
inline constexpr Color CARD_BG_SELECTED = {65, 72, 104, 255};  // #414868

// 後方互換性のためのエイリアス
inline constexpr Color PANEL_BG = PANEL_BG_SECONDARY;
inline constexpr Color PANEL_BG_DARK = PANEL_BG_PRIMARY;
inline constexpr Color PANEL_BG_BROWN = CARD_BG_NORMAL;
inline constexpr Color PANEL_BG_ORANGE = PANEL_BG_SECONDARY;
inline constexpr Color PANEL_BG_ORANGE_DARK = PANEL_BG_PRIMARY;
inline constexpr Color PANEL_BG_ORANGE_LIGHT = CARD_BG_SELECTED;
inline constexpr Color PANEL_ACCENT = CARD_BG_SELECTED;

// オーバーレイ背景（半透明）- Tokyo Night風
inline constexpr Color OVERLAY_BG = {30, 33, 46, 230};

// スロット背景
inline constexpr Color SLOT_EMPTY = CARD_BG_NORMAL;
inline constexpr Color SLOT_HOVER = CARD_BG_SELECTED;
inline constexpr Color SLOT_SELECTED = CARD_BG_SELECTED;
inline constexpr Color SLOT_ASSIGNED = {55, 70, 84, 200};
inline constexpr Color SLOT_ORANGE_EMPTY = CARD_BG_NORMAL;
inline constexpr Color SLOT_ORANGE_SELECTED = CARD_BG_SELECTED;

// ============================================================================
// アクセント色（視認性最優先）- Tokyo Night風
// ============================================================================
inline constexpr Color ACCENT_GOLD = {224, 175, 104, 255};      // ゴールド #e0af68
inline constexpr Color ACCENT_BLUE = {122, 162, 247, 255};     // ブルー #7aa2f7
inline constexpr Color SUCCESS_GREEN = {158, 206, 106, 255};   // ソフトグリーン #9ece6a
inline constexpr Color ACCENT_SUCCESS_GITHUB = {158, 206, 106, 255};  // 成功グリーン
inline constexpr Color WARNING_ORANGE = {255, 158, 100, 255}; // オレンジ #ff9e64
inline constexpr Color ACCENT_WARNING_GITHUB = {255, 158, 100, 255};  // 警告オレンジ
inline constexpr Color DANGER_RED = {247, 118, 142, 255};      // ソフトレッド #f7768e

// ============================================================================
// 枠線・ボーダー（明確な視覚的区別）- Tokyo Night風
// ============================================================================
inline constexpr Color CARD_BORDER_NORMAL = {103, 110, 149, 255};  // #676e95
inline constexpr Color CARD_BORDER_HOVER = ACCENT_GOLD;            // #e0af68
inline constexpr Color CARD_BORDER_SELECTED = {224, 175, 104, 255}; // #e0af68

// 後方互換性のためのエイリアス
inline constexpr Color BORDER_DEFAULT = CARD_BORDER_NORMAL;
inline constexpr Color BORDER_HOVER = CARD_BORDER_HOVER;
inline constexpr Color BORDER_GOLD = ACCENT_GOLD;
inline constexpr Color BORDER_BLUE = ACCENT_BLUE;
inline constexpr Color DIVIDER = CARD_BORDER_NORMAL;

// ============================================================================
// テキスト階層（コントラスト8.5:1以上）- Tokyo Night風
// ============================================================================
inline constexpr Color TEXT_PRIMARY = {192, 202, 245, 255};    // オフホワイト #c0caf5
inline constexpr Color TEXT_SECONDARY = {154, 165, 206, 255};  // 薄い青グレー #9aa5ce
inline constexpr Color TEXT_MUTED = {103, 110, 149, 255};      // ミュート #676e95
inline constexpr Color TEXT_DISABLED = {65, 72, 104, 255};     // 無効 #414868
inline constexpr Color TEXT_DARK = {26, 27, 38, 255};          // 暗いテキスト（明るい背景用） #1a1b26

// 後方互換性のためのエイリアス
inline constexpr Color TEXT_ACCENT = ACCENT_GOLD;
inline constexpr Color TEXT_GOLD = ACCENT_GOLD;
inline constexpr Color TEXT_ERROR = DANGER_RED;
inline constexpr Color TEXT_SUCCESS = SUCCESS_GREEN;
inline constexpr Color TEXT_BLUE = ACCENT_BLUE;

// ============================================================================
// 状態色（明確な視覚的区別）
// ============================================================================
inline constexpr Color BUTTON_PRIMARY = SUCCESS_GREEN;              // #9ece6a
inline constexpr Color BUTTON_PRIMARY_HOVER = {174, 220, 130, 255};  // 明るいグリーン
inline constexpr Color BUTTON_PRIMARY_ACTIVE = {142, 192, 94, 255};  // アクティブグリーン

inline constexpr Color BUTTON_SECONDARY = ACCENT_BLUE;               // #7aa2f7
inline constexpr Color BUTTON_SECONDARY_HOVER = {147, 178, 250, 255}; // 明るいブルー

inline constexpr Color BUTTON_DISABLED = TEXT_MUTED;

inline constexpr Color BUTTON_RESET = CARD_BG_NORMAL;
inline constexpr Color BUTTON_RESET_HOVER = CARD_BG_SELECTED;

inline constexpr Color BUTTON_BLUE = ACCENT_BLUE;                    // #7aa2f7
inline constexpr Color BUTTON_BLUE_HOVER = {147, 178, 250, 255};    // 明るいブルー

// ボタン色（選択ハイライト用）- 命名と実態を一致
inline constexpr Color BUTTON_ORANGE = {255, 158, 100, 255};      // オレンジ #ff9e64
inline constexpr Color BUTTON_ORANGE_HOVER = {255, 183, 130, 255}; // 明るいオレンジ
inline constexpr Color BUTTON_ORANGE_DARK = CARD_BG_NORMAL;
inline constexpr Color BUTTON_ORANGE_DARK_HOVER = CARD_BG_SELECTED;

// ステージ状態色
inline constexpr Color STAGE_LOCKED = {96, 125, 139, 255};
inline constexpr Color STAGE_BOSS = {255, 87, 34, 255};
inline constexpr Color STAGE_CLEAR = {139, 195, 74, 255};

// ============================================================================
// ステータス表示色
// ============================================================================
inline constexpr Color STATUS_POSITIVE = SUCCESS_GREEN;
inline constexpr Color STATUS_NEGATIVE = DANGER_RED;
inline constexpr Color STATUS_NEUTRAL = ACCENT_GOLD;

// ============================================================================
// ヘッダー色
// ============================================================================
inline constexpr Color HEADER_BG = PANEL_BG_SECONDARY;
inline constexpr Color HEADER_BG_DARK = PANEL_BG_PRIMARY;

// ============================================================================
// グラデーション背景（奥行き感）- Tokyo Night風
// ============================================================================
inline constexpr Color PANEL_GRADIENT_TOP = {26, 27, 38, 255};    // 深い青紫 #1a1b26
inline constexpr Color PANEL_GRADIENT_BOTTOM = {36, 40, 59, 255}; // パネル背景 #24283b
inline constexpr Color PANEL_GRADIENT_EDGE = {48, 52, 70, 255};   // カード背景 #303446

// ============================================================================
// エフェクト色（影・グロー・ハイライト）- Tokyo Night風
// ============================================================================
inline constexpr Color SHADOW_COLOR = {0, 0, 0, 120};              // ドロップシャドウ
inline constexpr Color GLOW_GREEN = {158, 206, 106, 76};          // 緑グロー (alpha 30%)
inline constexpr Color GLOW_GOLD = {224, 175, 104, 102};          // 金グロー (alpha 40%)
inline constexpr Color HIGHLIGHT_TOP = {224, 175, 104, 153};      // 上部ハイライト (alpha 60%)
inline constexpr Color PARTICLE_GOLD = {224, 175, 104, 30};       // 粒子金色 (alpha 10-30)

// ============================================================================
// ボタングラデーション色 - Tokyo Night風
// ============================================================================
inline constexpr Color BUTTON_PRIMARY_DARK = {142, 192, 94, 255};  // アクティブグリーン
inline constexpr Color BUTTON_PRIMARY_BRIGHT = {158, 206, 106, 255}; // ソフトグリーン
inline constexpr Color BUTTON_SECONDARY_DARK = {98, 142, 230, 255}; // 暗青
inline constexpr Color BUTTON_SECONDARY_BRIGHT = {122, 162, 247, 255}; // ブルー

// ============================================================================
// 後方互換性のためのエイリアス（旧カラーパレット）
// ============================================================================
inline constexpr Color ORANGE_PANEL_BG = PANEL_BG_SECONDARY;
inline constexpr Color ORANGE_PANEL_BG_DARK = PANEL_BG_PRIMARY;
inline constexpr Color ORANGE_PANEL_BG_LIGHT = CARD_BG_SELECTED;

} // namespace OverlayColors

} // namespace ui
} // namespace core
} // namespace game
