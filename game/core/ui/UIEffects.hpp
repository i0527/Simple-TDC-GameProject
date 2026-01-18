#pragma once

#include <raylib.h>
#include "OverlayColors.hpp"
#include "../api/BaseSystemAPI.hpp"
#include <cmath>

namespace game {
namespace core {
namespace ui {

// ============================================================================
// UIエフェクトヘルパー関数
// ============================================================================
// グラデーション、影、グローなどの視覚効果を提供します

namespace UIEffects {

// ============================================================================
// グラデーションパネル描画
// ============================================================================
inline void DrawGradientPanel(BaseSystemAPI* api, float x, float y, float width, float height) {
    if (!api) return;
    using namespace OverlayColors;
    api->DrawRectangleGradientV(
        static_cast<int>(x), static_cast<int>(y),
        static_cast<int>(width), static_cast<int>(height),
        PANEL_GRADIENT_TOP,
        PANEL_GRADIENT_BOTTOM
    );
}

// ============================================================================
// 立体カード描画（影 + 内側光沢）
// ============================================================================
inline void DrawCard3D(BaseSystemAPI* api, float x, float y, float width, float height,
                       Color card_bg, bool is_selected = false, bool is_hovered = false) {
    if (!api) return;
    using namespace OverlayColors;
    
    const float shadow_offset = 8.0f;
    const float corner_radius = 12.0f;
    const int segments = 10;
    
    // 1. 外側ドロップシャドウ（ブラー8px相当、alpha 120）
    Rectangle shadow_rect = {
        x + shadow_offset,
        y + shadow_offset,
        width,
        height
    };
    Color shadow_color = SHADOW_COLOR;
    api->DrawRectangleRounded(shadow_rect, corner_radius / width, segments, shadow_color);
    
    // 2. カード背景（微グラデーション）
    Rectangle card_rect = {x, y, width, height};
    api->DrawRectangleRounded(card_rect, corner_radius / width, segments, card_bg);
    
    // 3. 内側ハイライト線（上部のみ金色）
    if (is_selected || is_hovered) {
        Rectangle highlight_rect = {x, y, width, 4.0f};
        api->DrawRectangleRounded(highlight_rect, corner_radius / width, segments, HIGHLIGHT_TOP);
    }
    
    // 4. ボーダー
    Color border_color = is_selected ? CARD_BORDER_SELECTED : 
                        (is_hovered ? CARD_BORDER_HOVER : CARD_BORDER_NORMAL);
    api->DrawRectangleRoundedLines(card_rect, corner_radius / width, segments, border_color);
}

// ============================================================================
// 発光効果ボーダー（選択状態）
// ============================================================================
inline void DrawGlowingBorder(BaseSystemAPI* api, float x, float y, float width, float height,
                              float pulse_alpha = 1.0f, bool is_hovered = false) {
    if (!api) return;
    using namespace OverlayColors;
    
    const float corner_radius = 12.0f;
    const int segments = 10;
    Rectangle rect = {x, y, width, height};
    
    if (is_hovered) {
        // ホバー時: 2px金 + 外側グロー（ブラー12px相当）
        const float glow_offset = 12.0f;
        Rectangle glow_rect = {
            x - glow_offset,
            y - glow_offset,
            width + glow_offset * 2,
            height + glow_offset * 2
        };
        Color glow_color = GLOW_GOLD;
        glow_color.a = static_cast<unsigned char>(76 * pulse_alpha); // alpha 30% * pulse
        api->DrawRectangleRounded(glow_rect, corner_radius / glow_rect.width, segments, glow_color);
    }
    
    // 外側金色ボーダー（3px相当 - 複数回描画で太く）
    Color border_color = CARD_BORDER_SELECTED;
    border_color.a = static_cast<unsigned char>(255 * pulse_alpha);
    for (int i = 0; i < 3; ++i) {
        Rectangle border_rect = {x - i, y - i, width + i * 2, height + i * 2};
        api->DrawRectangleRoundedLines(border_rect, corner_radius / width, segments, border_color);
    }
    
    // 内側光沢ライン（1px, alpha 180）
    Rectangle inner_rect = {x + 1, y + 1, width - 2, height - 2};
    Color inner_color = HIGHLIGHT_TOP;
    inner_color.a = static_cast<unsigned char>(180 * pulse_alpha);
    api->DrawRectangleRoundedLines(inner_rect, corner_radius / width, segments, inner_color);
}

// ============================================================================
// モダンなボタン描画（ネオン風）
// ============================================================================
inline void DrawModernButton(BaseSystemAPI* api, float x, float y, float width, float height,
                             Color dark_color, Color bright_color, bool is_hovered = false,
                             bool is_disabled = false) {
    if (!api) return;
    using namespace OverlayColors;
    
    const float corner_radius = 8.0f;
    const int segments = 8;
    const float scale = is_hovered ? 1.05f : 1.0f;
    const float scaled_w = width * scale;
    const float scaled_h = height * scale;
    const float scaled_x = x - (scaled_w - width) / 2.0f;
    const float scaled_y = y - (scaled_h - height) / 2.0f;
    
    if (is_disabled) {
        // 無効時はグレー
        Rectangle rect = {scaled_x, scaled_y, scaled_w, scaled_h};
        api->DrawRectangleRounded(rect, corner_radius / scaled_w, segments, BUTTON_DISABLED);
        api->DrawRectangleRoundedLines(rect, corner_radius / scaled_w, segments, CARD_BORDER_NORMAL);
        return;
    }
    
    // 1. 外側影（控えめなドロップシャドウ）
    const float shadow_offset = 4.0f;
    Rectangle shadow_rect = {
        scaled_x + shadow_offset,
        scaled_y + shadow_offset,
        scaled_w,
        scaled_h
    };
    Color shadow_color = SHADOW_COLOR;
    api->DrawRectangleRounded(shadow_rect, corner_radius / scaled_w, segments, shadow_color);
    
    // 2. 背景グラデーション（横方向：暗→鮮）
    Rectangle rect = {scaled_x, scaled_y, scaled_w, scaled_h};
    api->DrawRectangleGradientH(
        static_cast<int>(scaled_x), static_cast<int>(scaled_y),
        static_cast<int>(scaled_w), static_cast<int>(scaled_h),
        dark_color,
        bright_color
    );
    
    // 3. 上部の控えめな光沢ライン（ホバー時のみ強調）
    if (is_hovered) {
        Rectangle gloss_rect = {scaled_x + 2, scaled_y + 2, scaled_w - 4, 2.0f};
        Color gloss_color = {255, 255, 255, 80}; // alpha ~30%
        api->DrawRectangleRounded(gloss_rect, 1.0f, segments, gloss_color);
    }
    
    // 4. ボーダー（2px相当 - 複数回描画）
    for (int i = 0; i < 2; ++i) {
        Rectangle border_rect = {scaled_x - i, scaled_y - i, scaled_w + i * 2, scaled_h + i * 2};
        api->DrawRectangleRoundedLines(border_rect, corner_radius / scaled_w, segments, bright_color);
    }
}

// ============================================================================
// パルスアニメーション用アルファ値計算
// ============================================================================
inline float CalculatePulseAlpha(float time, float period = 1.5f, float min_alpha = 0.8f, float max_alpha = 1.0f) {
    float t = std::fmod(time, period) / period;
    float sine = std::sin(t * 2.0f * 3.14159f);
    return min_alpha + (max_alpha - min_alpha) * (sine * 0.5f + 0.5f);
}

// ============================================================================
// 粒子エフェクト描画（背景装飾用）
// ============================================================================
inline void DrawParticles(BaseSystemAPI* api, float time, float area_x, float area_y, float area_w, float area_h, int count = 15) {
    if (!api) return;
    using namespace OverlayColors;
    
    // 簡易的な粒子描画（実際の実装ではより高度なパーティクルシステムを使用）
    for (int i = 0; i < count; ++i) {
        float seed = static_cast<float>(i) * 123.456f;
        float x = area_x + std::fmod(seed * 17.3f, area_w);
        float y = area_y + std::fmod(seed * 23.7f + time * 20.0f, area_h);
        float alpha = 10.0f + std::fmod(seed * 7.1f, 20.0f); // alpha 10-30
        
        Color particle_color = PARTICLE_GOLD;
        particle_color.a = static_cast<unsigned char>(alpha);
        
        api->DrawCircle(static_cast<int>(x), static_cast<int>(y), 2.0f, particle_color);
    }
}

} // namespace UIEffects

} // namespace ui
} // namespace core
} // namespace game
