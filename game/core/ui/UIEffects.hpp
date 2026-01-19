#pragma once

#include "../config/RenderTypes.hpp"
#include "OverlayColors.hpp"
#include "../api/BaseSystemAPI.hpp"
#include <cmath>

namespace game {
namespace core {
namespace ui {

// ============================================================================
// UI繧ｨ繝輔ぉ繧ｯ繝医・繝ｫ繝代・髢｢謨ｰ
// ============================================================================
// 繧ｰ繝ｩ繝・・繧ｷ繝ｧ繝ｳ縲∝ｽｱ縲√げ繝ｭ繝ｼ縺ｪ縺ｩ縺ｮ隕冶ｦ壼柑譫懊ｒ謠蝉ｾ帙＠縺ｾ縺・

namespace UIEffects {

// ============================================================================
// 繧ｰ繝ｩ繝・・繧ｷ繝ｧ繝ｳ繝代ロ繝ｫ謠冗判
// ============================================================================
inline void DrawGradientPanel(BaseSystemAPI* api, float x, float y, float width, float height) {
    if (!api) return;
    using namespace OverlayColors;
    api->Render().DrawRectangleGradientV(
        static_cast<int>(x), static_cast<int>(y),
        static_cast<int>(width), static_cast<int>(height),
        PANEL_GRADIENT_TOP,
        PANEL_GRADIENT_BOTTOM
    );
}

// ============================================================================
// 遶倶ｽ薙き繝ｼ繝画緒逕ｻ・亥ｽｱ + 蜀・・蜈画ｲ｢・・
// ============================================================================
inline void DrawCard3D(BaseSystemAPI* api, float x, float y, float width, float height,
                       Color card_bg, bool is_selected = false, bool is_hovered = false) {
    if (!api) return;
    using namespace OverlayColors;
    
    const float shadow_offset = 8.0f;
    const float corner_radius = 12.0f;
    const int segments = 10;
    
    // 1. 螟門・繝峨Ο繝・・繧ｷ繝｣繝峨え・医ヶ繝ｩ繝ｼ8px逶ｸ蠖薙∥lpha 120・・
    Rectangle shadow_rect = {
        x + shadow_offset,
        y + shadow_offset,
        width,
        height
    };
    Color shadow_color = SHADOW_COLOR;
    api->Render().DrawRectangleRounded(shadow_rect, corner_radius / width, segments, shadow_color);
    
    // 2. 繧ｫ繝ｼ繝芽レ譎ｯ・亥ｾｮ繧ｰ繝ｩ繝・・繧ｷ繝ｧ繝ｳ・・
    Rectangle card_rect = {x, y, width, height};
    api->Render().DrawRectangleRounded(card_rect, corner_radius / width, segments, card_bg);
    
    // 3. 蜀・・繝上う繝ｩ繧､繝育ｷ夲ｼ井ｸ企Κ縺ｮ縺ｿ驥題牡・・
    if (is_selected || is_hovered) {
        Rectangle highlight_rect = {x, y, width, 4.0f};
        api->Render().DrawRectangleRounded(highlight_rect, corner_radius / width, segments, HIGHLIGHT_TOP);
    }
    
    // 4. 繝懊・繝繝ｼ
    Color border_color = is_selected ? CARD_BORDER_SELECTED : 
                        (is_hovered ? CARD_BORDER_HOVER : CARD_BORDER_NORMAL);
    api->Render().DrawRectangleRoundedLines(card_rect, corner_radius / width, segments, border_color);
}

// ============================================================================
// 逋ｺ蜈牙柑譫懊・繝ｼ繝繝ｼ・磯∈謚樒憾諷具ｼ・
// ============================================================================
inline void DrawGlowingBorder(BaseSystemAPI* api, float x, float y, float width, float height,
                              float pulse_alpha = 1.0f, bool is_hovered = false) {
    if (!api) return;
    using namespace OverlayColors;
    
    const float corner_radius = 12.0f;
    const int segments = 10;
    Rectangle rect = {x, y, width, height};
    
    if (is_hovered) {
        // 繝帙ヰ繝ｼ譎・ 2px驥・+ 螟門・繧ｰ繝ｭ繝ｼ・医ヶ繝ｩ繝ｼ12px逶ｸ蠖難ｼ・
        const float glow_offset = 12.0f;
        Rectangle glow_rect = {
            x - glow_offset,
            y - glow_offset,
            width + glow_offset * 2,
            height + glow_offset * 2
        };
        Color glow_color = GLOW_GOLD;
        glow_color.a = static_cast<unsigned char>(76 * pulse_alpha); // alpha 30% * pulse
        api->Render().DrawRectangleRounded(glow_rect, corner_radius / glow_rect.width, segments, glow_color);
    }
    
    // 螟門・驥題牡繝懊・繝繝ｼ・・px逶ｸ蠖・- 隍・焚蝗樊緒逕ｻ縺ｧ螟ｪ縺擾ｼ・
    Color border_color = CARD_BORDER_SELECTED;
    border_color.a = static_cast<unsigned char>(255 * pulse_alpha);
    for (int i = 0; i < 3; ++i) {
        Rectangle border_rect = {x - i, y - i, width + i * 2, height + i * 2};
        api->Render().DrawRectangleRoundedLines(border_rect, corner_radius / width, segments, border_color);
    }
    
    // 蜀・・蜈画ｲ｢繝ｩ繧､繝ｳ・・px, alpha 180・・
    Rectangle inner_rect = {x + 1, y + 1, width - 2, height - 2};
    Color inner_color = HIGHLIGHT_TOP;
    inner_color.a = static_cast<unsigned char>(180 * pulse_alpha);
    api->Render().DrawRectangleRoundedLines(inner_rect, corner_radius / width, segments, inner_color);
}

// ============================================================================
// 繝｢繝繝ｳ縺ｪ繝懊ち繝ｳ謠冗判・医ロ繧ｪ繝ｳ鬚ｨ・・
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
        // 辟｡蜉ｹ譎ゅ・繧ｰ繝ｬ繝ｼ
        Rectangle rect = {scaled_x, scaled_y, scaled_w, scaled_h};
        api->Render().DrawRectangleRounded(rect, corner_radius / scaled_w, segments, BUTTON_DISABLED);
        api->Render().DrawRectangleRoundedLines(rect, corner_radius / scaled_w, segments, CARD_BORDER_NORMAL);
        return;
    }
    
    // 1. 螟門・蠖ｱ・域而縺医ａ縺ｪ繝峨Ο繝・・繧ｷ繝｣繝峨え・・
    const float shadow_offset = 4.0f;
    Rectangle shadow_rect = {
        scaled_x + shadow_offset,
        scaled_y + shadow_offset,
        scaled_w,
        scaled_h
    };
    Color shadow_color = SHADOW_COLOR;
    api->Render().DrawRectangleRounded(shadow_rect, corner_radius / scaled_w, segments, shadow_color);
    
    // 2. 閭梧勹繧ｰ繝ｩ繝・・繧ｷ繝ｧ繝ｳ・域ｨｪ譁ｹ蜷托ｼ壽囓竊帝ｮｮ・・
    Rectangle rect = {scaled_x, scaled_y, scaled_w, scaled_h};
    api->Render().DrawRectangleGradientH(
        static_cast<int>(scaled_x), static_cast<int>(scaled_y),
        static_cast<int>(scaled_w), static_cast<int>(scaled_h),
        dark_color,
        bright_color
    );
    
    // 3. 荳企Κ縺ｮ謗ｧ縺医ａ縺ｪ蜈画ｲ｢繝ｩ繧､繝ｳ・医・繝舌・譎ゅ・縺ｿ蠑ｷ隱ｿ・・
    if (is_hovered) {
        Rectangle gloss_rect = {scaled_x + 2, scaled_y + 2, scaled_w - 4, 2.0f};
        Color gloss_color = {255, 255, 255, 80}; // alpha ~30%
        api->Render().DrawRectangleRounded(gloss_rect, 1.0f, segments, gloss_color);
    }
    
    // 4. 繝懊・繝繝ｼ・・px逶ｸ蠖・- 隍・焚蝗樊緒逕ｻ・・
    for (int i = 0; i < 2; ++i) {
        Rectangle border_rect = {scaled_x - i, scaled_y - i, scaled_w + i * 2, scaled_h + i * 2};
        api->Render().DrawRectangleRoundedLines(border_rect, corner_radius / scaled_w, segments, bright_color);
    }
}

// ============================================================================
// 繝代Ν繧ｹ繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ逕ｨ繧｢繝ｫ繝輔ぃ蛟､險育ｮ・
// ============================================================================
inline float CalculatePulseAlpha(float time, float period = 1.5f, float min_alpha = 0.8f, float max_alpha = 1.0f) {
    float t = std::fmod(time, period) / period;
    float sine = std::sin(t * 2.0f * 3.14159f);
    return min_alpha + (max_alpha - min_alpha) * (sine * 0.5f + 0.5f);
}

// ============================================================================
// 邊貞ｭ舌お繝輔ぉ繧ｯ繝域緒逕ｻ・郁レ譎ｯ陬・｣ｾ逕ｨ・・
// ============================================================================
inline void DrawParticles(BaseSystemAPI* api, float time, float area_x, float area_y, float area_w, float area_h, int count = 15) {
    if (!api) return;
    using namespace OverlayColors;
    
    // 邁｡譏鍋噪縺ｪ邊貞ｭ先緒逕ｻ・亥ｮ滄圀縺ｮ螳溯｣・〒縺ｯ繧医ｊ鬮伜ｺｦ縺ｪ繝代・繝・ぅ繧ｯ繝ｫ繧ｷ繧ｹ繝・Β繧剃ｽｿ逕ｨ・・
    for (int i = 0; i < count; ++i) {
        float seed = static_cast<float>(i) * 123.456f;
        float x = area_x + std::fmod(seed * 17.3f, area_w);
        float y = area_y + std::fmod(seed * 23.7f + time * 20.0f, area_h);
        float alpha = 10.0f + std::fmod(seed * 7.1f, 20.0f); // alpha 10-30
        
        Color particle_color = PARTICLE_GOLD;
        particle_color.a = static_cast<unsigned char>(alpha);
        
        api->Render().DrawCircle(static_cast<int>(x), static_cast<int>(y), 2.0f, particle_color);
    }
}

} // namespace UIEffects

} // namespace ui
} // namespace core
} // namespace game
