#include "../UISystemAPI.hpp"

namespace game {
namespace core {

void UISystemAPI::DrawPanel(const ui::Rect& rect, Color color) const {
    if (!systemAPI_) {
        return;
    }
    systemAPI_->Render().DrawRectangle(rect.x, rect.y, rect.width, rect.height,
                                       color);
}

void UISystemAPI::DrawPanelGradient(const ui::Rect& rect) const {
    if (!systemAPI_) {
        return;
    }
    ui::UIEffects::DrawGradientPanel(systemAPI_, rect.x, rect.y, rect.width, rect.height);
}

void UISystemAPI::DrawText(const std::string& text, float x, float y,
                           int fontSize, Color color) const {
    if (!systemAPI_) {
        return;
    }
    systemAPI_->Render().DrawTextDefault(text, x, y,
                                         static_cast<float>(fontSize), color);
}

void UISystemAPI::DrawUiTexture(const std::string& textureKey,
                                Rectangle dest, Color tint) const {
    if (!systemAPI_) {
        return;
    }
    systemAPI_->Render().DrawUiTexture(textureKey, dest, tint);
}

void UISystemAPI::DrawUiNineSlice(const std::string& textureKey, Rectangle dest,
                                  int left, int top, int right, int bottom, Color tint) const {
    if (!systemAPI_) {
        return;
    }
    systemAPI_->Render().DrawUiNineSlice(textureKey, dest, left, top, right,
                                         bottom, tint);
}

Texture2D* UISystemAPI::GetTexturePtr(const std::string& name) const {
    if (!systemAPI_) {
        return nullptr;
    }
    return systemAPI_->Resource().GetTexturePtr(name);
}

Color UISystemAPI::GetReadableTextColor(const std::string& textureKey,
                                        float luminanceThreshold) const {
    if (!systemAPI_) {
        return WHITE;
    }
    return systemAPI_->Render().GetReadableTextColor(textureKey,
                                                     luminanceThreshold);
}

} // namespace core
} // namespace game
