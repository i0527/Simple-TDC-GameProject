#include "../UISystemAPI.hpp"

namespace game {
namespace core {

Color UISystemAPI::GetColor(ColorRole role) const {
    using namespace ui::OverlayColors;
    switch (role) {
    case ColorRole::PanelBg: return PANEL_BG;
    case ColorRole::PanelBgDark: return PANEL_BG_DARK;
    case ColorRole::CardBgNormal: return CARD_BG_NORMAL;
    case ColorRole::CardBgSelected: return CARD_BG_SELECTED;
    case ColorRole::TextPrimary: return TEXT_PRIMARY;
    case ColorRole::TextSecondary: return TEXT_SECONDARY;
    case ColorRole::TextMuted: return TEXT_MUTED;
    case ColorRole::TextDisabled: return TEXT_DISABLED;
    case ColorRole::AccentGold: return ACCENT_GOLD;
    case ColorRole::AccentBlue: return ACCENT_BLUE;
    case ColorRole::SuccessGreen: return SUCCESS_GREEN;
    case ColorRole::WarningOrange: return WARNING_ORANGE;
    case ColorRole::DangerRed: return DANGER_RED;
    case ColorRole::ButtonPrimary: return BUTTON_PRIMARY;
    case ColorRole::ButtonSecondary: return BUTTON_SECONDARY;
    case ColorRole::ButtonDisabled: return BUTTON_DISABLED;
    case ColorRole::BorderNormal: return CARD_BORDER_NORMAL;
    case ColorRole::BorderHover: return CARD_BORDER_HOVER;
    case ColorRole::BorderSelected: return CARD_BORDER_SELECTED;
    case ColorRole::OverlayBg: return OVERLAY_BG;
    }
    return WHITE;
}

const char* UISystemAPI::GetAssetKey(AssetRole role) const {
    using ui::UiAssetKeys;
    switch (role) {
    case AssetRole::ButtonPrimaryNormal: return UiAssetKeys::ButtonPrimaryNormal;
    case AssetRole::ButtonPrimaryHover: return UiAssetKeys::ButtonPrimaryHover;
    case AssetRole::ButtonSecondaryNormal: return UiAssetKeys::ButtonSecondaryNormal;
    case AssetRole::ButtonSecondaryHover: return UiAssetKeys::ButtonSecondaryHover;
    case AssetRole::PanelBackground: return UiAssetKeys::PanelBackground;
    case AssetRole::PanelBorder: return UiAssetKeys::PanelBorder;
    case AssetRole::FantasyPanelLight: return UiAssetKeys::FantasyPanelLight;
    case AssetRole::FantasyPanelDark: return UiAssetKeys::FantasyPanelDark;
    case AssetRole::FantasyBorderLight: return UiAssetKeys::FantasyBorderLight;
    case AssetRole::FantasyBorderDark: return UiAssetKeys::FantasyBorderDark;
    case AssetRole::FantasyPanelTransparent: return UiAssetKeys::FantasyPanelTransparent;
    case AssetRole::FantasyBorderTransparent: return UiAssetKeys::FantasyBorderTransparent;
    case AssetRole::FantasyDivider: return UiAssetKeys::FantasyDivider;
    case AssetRole::FantasyDividerFade: return UiAssetKeys::FantasyDividerFade;
    case AssetRole::CursorPointer: return UiAssetKeys::CursorPointer;
    }
    return "";
}

} // namespace core
} // namespace game
