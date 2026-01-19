#pragma once

// 標準ライブラリ
#include <memory>
#include <string>

// プロジェクト内
#include "../config/RenderTypes.hpp"
#include "BaseSystemAPI.hpp"
#include "../ui/IUIComponent.hpp"
#include "../ui/UIEvent.hpp"
#include "../ui/OverlayColors.hpp"
#include "../ui/UiAssetKeys.hpp"
#include "../ui/UIEffects.hpp"

namespace game {
namespace core {

/// @brief UI共通機能を統括するAPI
///
/// BaseSystemAPIの上にUI共通の描画・イベント・テーマ/アセット参照を集約します。
class UISystemAPI {
public:
    enum class ColorRole {
        PanelBg,
        PanelBgDark,
        CardBgNormal,
        CardBgSelected,
        TextPrimary,
        TextSecondary,
        TextMuted,
        TextDisabled,
        AccentGold,
        AccentBlue,
        SuccessGreen,
        WarningOrange,
        DangerRed,
        ButtonPrimary,
        ButtonSecondary,
        ButtonDisabled,
        BorderNormal,
        BorderHover,
        BorderSelected,
        OverlayBg
    };

    enum class AssetRole {
        ButtonPrimaryNormal,
        ButtonPrimaryHover,
        ButtonSecondaryNormal,
        ButtonSecondaryHover,
        PanelBackground,
        PanelBorder,
        FantasyPanelLight,
        FantasyPanelDark,
        FantasyBorderLight,
        FantasyBorderDark,
        FantasyPanelTransparent,
        FantasyBorderTransparent,
        FantasyDivider,
        FantasyDividerFade,
        CursorPointer
    };

    UISystemAPI();
    ~UISystemAPI();

    // 初期化・終了
    bool Initialize(BaseSystemAPI* systemAPI);
    void Shutdown();
    bool IsInitialized() const;

    BaseSystemAPI* SystemAPI() const { return systemAPI_; }

    // UIイベント生成
    ui::UIEvent MakeClickEvent(float x, float y) const;
    ui::UIEvent MakeHoverEvent(float x, float y) const;
    ui::UIEvent MakeKeyEvent(int key) const;

    // UIイベント配信
    ui::UIEventResult DispatchEvent(const ui::UIEvent& ev,
                                    const std::shared_ptr<ui::IUIComponent>& root) const;
    ui::UIEventResult DispatchEvent(const ui::UIEvent& ev,
                                    ui::IUIComponent* root) const;

    // 描画ヘルパー（共通）
    void DrawPanel(const ui::Rect& rect, Color color) const;
    void DrawPanelGradient(const ui::Rect& rect) const;
    void DrawText(const std::string& text, float x, float y, int fontSize, Color color) const;
    void DrawUiTexture(const std::string& textureKey, Rectangle dest, Color tint = WHITE) const;
    void DrawUiNineSlice(const std::string& textureKey, Rectangle dest,
                         int left, int top, int right, int bottom, Color tint = WHITE) const;

    // リソース参照
    Texture2D* GetTexturePtr(const std::string& name) const;
    Color GetReadableTextColor(const std::string& textureKey, float luminanceThreshold = 0.6f) const;

    // テーマ/アセット参照
    Color GetColor(ColorRole role) const;
    const char* GetAssetKey(AssetRole role) const;

    // UIエフェクトラッパー
    void DrawCard3D(float x, float y, float width, float height,
                    Color cardBg, bool isSelected = false, bool isHovered = false) const;
    void DrawGlowingBorder(float x, float y, float width, float height,
                           float pulseAlpha = 1.0f, bool isHovered = false) const;
    void DrawModernButton(float x, float y, float width, float height,
                          Color darkColor, Color brightColor,
                          bool isHovered = false, bool isDisabled = false) const;
    float CalculatePulseAlpha(float time, float period = 1.5f,
                              float minAlpha = 0.8f, float maxAlpha = 1.0f) const;
    void DrawParticles(float time, float areaX, float areaY, float areaW, float areaH,
                       int count = 15) const;

private:
    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
};

} // namespace core
} // namespace game
