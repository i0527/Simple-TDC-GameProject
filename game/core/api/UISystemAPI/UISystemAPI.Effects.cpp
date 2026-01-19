#include "../UISystemAPI.hpp"

namespace game {
namespace core {

void UISystemAPI::DrawCard3D(float x, float y, float width, float height,
                             Color cardBg, bool isSelected, bool isHovered) const {
    if (!systemAPI_) {
        return;
    }
    ui::UIEffects::DrawCard3D(systemAPI_, x, y, width, height, cardBg, isSelected, isHovered);
}

void UISystemAPI::DrawGlowingBorder(float x, float y, float width, float height,
                                    float pulseAlpha, bool isHovered) const {
    if (!systemAPI_) {
        return;
    }
    ui::UIEffects::DrawGlowingBorder(systemAPI_, x, y, width, height, pulseAlpha, isHovered);
}

void UISystemAPI::DrawModernButton(float x, float y, float width, float height,
                                   Color darkColor, Color brightColor,
                                   bool isHovered, bool isDisabled) const {
    if (!systemAPI_) {
        return;
    }
    ui::UIEffects::DrawModernButton(systemAPI_, x, y, width, height, darkColor, brightColor,
                                    isHovered, isDisabled);
}

float UISystemAPI::CalculatePulseAlpha(float time, float period,
                                       float minAlpha, float maxAlpha) const {
    return ui::UIEffects::CalculatePulseAlpha(time, period, minAlpha, maxAlpha);
}

void UISystemAPI::DrawParticles(float time, float areaX, float areaY,
                                float areaW, float areaH, int count) const {
    if (!systemAPI_) {
        return;
    }
    ui::UIEffects::DrawParticles(systemAPI_, time, areaX, areaY, areaW, areaH, count);
}

} // namespace core
} // namespace game
