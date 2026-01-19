#pragma once

namespace game {
namespace core {
namespace ui {

struct UiAssetKeys {
    // UI pack (Kenney UI Pack - Grey/Default)
    static constexpr const char* ButtonPrimaryNormal =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/button_rectangle_depth_gloss.png";
    static constexpr const char* ButtonPrimaryHover =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/button_rectangle_depth_gradient.png";
    static constexpr const char* ButtonSecondaryNormal =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/button_rectangle_line.png";
    static constexpr const char* ButtonSecondaryHover =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/button_rectangle_depth_line.png";
    static constexpr const char* PanelBackground =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/button_rectangle_depth_flat.png";
    static constexpr const char* PanelBorder =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/button_rectangle_depth_border.png";
    static constexpr const char* ScrollTrackVertical =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/slide_vertical_grey.png";
    static constexpr const char* ScrollTrackHorizontal =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/slide_horizontal_grey.png";
    static constexpr const char* ScrollThumb =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/slide_hangle.png";
    static constexpr const char* CheckBoxOn =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/check_square_color_checkmark.png";
    static constexpr const char* CheckBoxOff =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/check_square_grey.png";
    static constexpr const char* IconArrowUp =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/arrow_basic_n_small.png";
    static constexpr const char* IconArrowDown =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/arrow_basic_s_small.png";
    static constexpr const char* IconArrowLeft =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/arrow_basic_w_small.png";
    static constexpr const char* IconArrowRight =
        "assets/other/kenney_ui-pack/PNG/Grey/Default/arrow_basic_e_small.png";

    // Fantasy UI borders (Kenney fantasy-ui-borders)
    static constexpr const char* FantasyPanelLight =
        "assets/other/kenney_fantasy-ui-borders/PNG/Default/Panel/panel-000.png";
    static constexpr const char* FantasyPanelDark =
        "assets/other/kenney_fantasy-ui-borders/PNG/Default/Panel/panel-012.png";
    static constexpr const char* FantasyBorderLight =
        "assets/other/kenney_fantasy-ui-borders/PNG/Default/Border/panel-border-000.png";
    static constexpr const char* FantasyBorderDark =
        "assets/other/kenney_fantasy-ui-borders/PNG/Default/Border/panel-border-012.png";
    static constexpr const char* FantasyPanelTransparent =
        "assets/other/kenney_fantasy-ui-borders/PNG/Default/Transparent center/panel-transparent-center-006.png";
    static constexpr const char* FantasyBorderTransparent =
        "assets/other/kenney_fantasy-ui-borders/PNG/Default/Transparent border/panel-transparent-border-006.png";
    static constexpr const char* FantasyDivider =
        "assets/other/kenney_fantasy-ui-borders/PNG/Default/Divider/divider-000.png";
    static constexpr const char* FantasyDividerFade =
        "assets/other/kenney_fantasy-ui-borders/PNG/Default/Divider Fade/divider-fade-000.png";

    // Home UI assignments (readability-first)
    static constexpr const char* HomeHeaderPanel = FantasyPanelLight;
    static constexpr const char* HomeHeaderBorder = FantasyBorderLight;
    static constexpr const char* HomeTabBarPanel = FantasyPanelDark;
    static constexpr const char* HomeTabBarBorder = FantasyBorderDark;
    static constexpr const char* HomeOverlayPanel = FantasyPanelDark;
    static constexpr const char* HomeOverlayBorder = FantasyBorderDark;
    static constexpr const char* HomeMainPanel = FantasyPanelDark;
    static constexpr const char* HomeMainBorder = FantasyBorderDark;
    static constexpr const char* HomeCardPanel = FantasyPanelTransparent;
    static constexpr const char* HomeCardBorder = FantasyBorderTransparent;
    static constexpr const char* HomeDetailPanel = FantasyPanelLight;
    static constexpr const char* HomeDetailBorder = FantasyBorderLight;
    static constexpr const char* HomeDivider = FantasyDividerFade;

    // Cursor pack (Kenney Cursor Pack)
    static constexpr const char* CursorPointer =
        "assets/other/kenney_cursor-pack/PNG/Basic/Default/pointer_a.png";
};

} // namespace ui
} // namespace core
} // namespace game
