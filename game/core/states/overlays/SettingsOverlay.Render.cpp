#include "SettingsOverlay.hpp"

// 標準ライブラリ
#include <cstdio>

// 外部ライブラリ

// プロジェクト内
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UiAssetKeys.hpp"
#include "../../config/RenderPrimitives.hpp"

namespace game {
namespace core {

void SettingsOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) {
        return;
    }

    // ウィンドウの位置とサイズ
    const float windowX = 200.0f;
    const float windowY = 150.0f;
    const float windowWidth = 1520.0f;
    const float windowHeight = 780.0f;

    systemAPI_->Render().DrawRectangle(
        static_cast<int>(windowX),
        static_cast<int>(windowY),
        static_cast<int>(windowWidth),
        static_cast<int>(windowHeight),
        ToCoreColor(ui::OverlayColors::OVERLAY_BG));
    systemAPI_->Render().DrawRectangleLines(
        static_cast<int>(windowX),
        static_cast<int>(windowY),
        static_cast<int>(windowWidth),
        static_cast<int>(windowHeight),
        2.0f,
        ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

    ColorRGBA panelTextColor = ToCoreColor(ui::OverlayColors::TEXT_PRIMARY);

    // タイトル
    const char* titleText = "設定";
    float titleFontSize = 36.0f;
    Vec2 titleSize =
        systemAPI_->Render().MeasureTextDefaultCore(titleText, titleFontSize, 1.0f);
    float titleX = windowX + (windowWidth - titleSize.x) / 2.0f;
    float titleY = windowY + 20.0f;
    systemAPI_->Render().DrawTextDefault(titleText, titleX, titleY,
                                         titleFontSize, panelTextColor);

    // 音量設定セクション（左側）
    float sectionY = titleY + titleSize.y + 40.0f;
    float sectionWidth = (windowWidth - 60.0f) / 2.0f;
    float sectionHeight = windowHeight - (sectionY - windowY) - 120.0f;
    RenderVolumeSection(windowX + 20.0f, sectionY, sectionWidth, sectionHeight);

    // 表示設定セクション（右側）
    RenderDisplaySection(ctx, windowX + sectionWidth + 40.0f, sectionY, sectionWidth, sectionHeight);

    // 操作ボタン（下部）
    float buttonY = windowY + windowHeight - 80.0f;
    float buttonWidth = 150.0f;
    float buttonHeight = 40.0f;
    float buttonSpacing = 20.0f;
    const int buttonCount = 5;
    float totalButtonWidth =
        buttonWidth * static_cast<float>(buttonCount) +
        buttonSpacing * static_cast<float>(buttonCount - 1);

    // 操作ボタンの配置
    float applyButtonX = windowX + (windowWidth - totalButtonWidth) / 2.0f;
    float resetButtonX = applyButtonX + buttonWidth + buttonSpacing;
    float titleButtonX = resetButtonX + buttonWidth + buttonSpacing;
    float quitButtonX = titleButtonX + buttonWidth + buttonSpacing;
    float closeButtonX = quitButtonX + buttonWidth + buttonSpacing;

    // 適用ボタン
    RenderButton(ctx, applyButtonX, buttonY, buttonWidth, buttonHeight, "適用",
                 applyButtonHovered_);

    // リセットボタン
    RenderButton(ctx, resetButtonX, buttonY, buttonWidth, buttonHeight, "リセット",
                 resetButtonHovered_);

    // タイトルへボタン（タイトル中は無効）
    bool canReturnTitle = ctx.currentState != GameState::Title;
    RenderButton(ctx, titleButtonX, buttonY, buttonWidth, buttonHeight, "タイトルへ",
                 titleButtonHovered_, canReturnTitle);

    // ゲーム終了ボタン
    RenderButton(ctx, quitButtonX, buttonY, buttonWidth, buttonHeight, "ゲーム終了",
                 quitButtonHovered_);

    // 閉じるボタン
    RenderButton(ctx, closeButtonX, buttonY, buttonWidth, buttonHeight, "閉じる",
                 closeButtonHovered_);
}

void SettingsOverlay::RenderVolumeSection(float x, float y, float width, float height) {
    (void)height;
    float fontSize = 28.0f;
    float sliderHeight = 40.0f;
    float sliderSpacing = 60.0f;
    float startY = y + 40.0f;

    // セクションタイトル
    const char* sectionTitle = "音量設定";
    Vec2 titleSize =
        systemAPI_->Render().MeasureTextDefaultCore(sectionTitle, fontSize, 1.0f);
    float titleX = x + (width - titleSize.x) / 2.0f;
    systemAPI_->Render().DrawTextDefault(sectionTitle, titleX, y, fontSize,
                                         ui::OverlayColors::TEXT_PRIMARY);

    // マスターボリューム
    RenderSlider(x, startY, width - 20.0f, sliderHeight, "マスター", &currentSettings_.masterVolume, 0);

    // BGMボリューム
    RenderSlider(x, startY + sliderSpacing, width - 20.0f, sliderHeight, "BGM",
                 &currentSettings_.bgmVolume, 1);

    // SEボリューム
    RenderSlider(x, startY + sliderSpacing * 2, width - 20.0f, sliderHeight, "SE", &currentSettings_.seVolume, 2);

}

void SettingsOverlay::RenderDisplaySection(SharedContext& ctx, float x, float y, float width, float height) {
    (void)height;
    float fontSize = 28.0f;
    float buttonHeight = 40.0f;
    float buttonSpacing = 50.0f;
    float startY = y + 40.0f;

    // セクションタイトル
    const char* sectionTitle = "表示設定";
    Vec2 titleSize =
        systemAPI_->Render().MeasureTextDefaultCore(sectionTitle, fontSize, 1.0f);
    float titleX = x + (width - titleSize.x) / 2.0f;
    systemAPI_->Render().DrawTextDefault(sectionTitle, titleX, y, fontSize,
                                         ui::OverlayColors::TEXT_PRIMARY);

    // フルスクリーンボタン
    const char* fullscreenText = currentSettings_.isFullscreen ? "フルスクリーン: ON" : "フルスクリーン: OFF";
    RenderButton(ctx, x, startY, width - 20.0f, buttonHeight, fullscreenText,
                 fullscreenButtonHovered_);

    // モニター選択（フルスクリーン時のみ表示）
    if (currentSettings_.isFullscreen && systemAPI_) {
        int monitorCount = systemAPI_->Window().GetMonitorCount();
        if (monitorCount > 1) {
            float monitorY = startY + buttonSpacing;
            float monitorButtonWidth = 40.0f;
            float monitorTextWidth = width - 20.0f - monitorButtonWidth * 2.0f - 20.0f;
            
            // 前のモニターボタン
            RenderButton(ctx, x, monitorY, monitorButtonWidth, buttonHeight, "<",
                         monitorPrevButtonHovered_);
            
            // モニター情報表示
            float monitorTextX = x + monitorButtonWidth + 10.0f;
            int monitor = currentSettings_.selectedMonitor;
            if (monitor < 0 || monitor >= monitorCount) {
                monitor = 0;
            }
            const char* monitorName = systemAPI_->Window().GetMonitorName(monitor);
            char monitorText[256];
            snprintf(monitorText, sizeof(monitorText), "モニター %d/%d: %s", 
                     monitor + 1, monitorCount, monitorName);
            float labelFontSize = 22.0f;
            systemAPI_->Render().DrawTextDefault(
                monitorText, monitorTextX, monitorY + (buttonHeight - labelFontSize) / 2.0f,
                labelFontSize, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
            
            // 次のモニターボタン
            float monitorNextX = monitorTextX + monitorTextWidth + 10.0f;
            RenderButton(ctx, monitorNextX, monitorY, monitorButtonWidth, buttonHeight, ">",
                         monitorNextButtonHovered_);
            
            // FPS表示チェックボックスの位置を調整
            RenderCheckbox(ctx, x, monitorY + buttonSpacing, 30.0f, "FPS表示", &currentSettings_.showFPS,
                           fpsCheckboxHovered_);
        } else {
            // モニターが1つしかない場合は通常通りFPS表示チェックボックスを表示
            RenderCheckbox(ctx, x, startY + buttonSpacing, 30.0f, "FPS表示", &currentSettings_.showFPS,
                           fpsCheckboxHovered_);
        }
    } else {
        // ウィンドウモード時はFPS表示チェックボックスと解像度選択
        float fpsCheckboxY = startY + buttonSpacing;
        RenderCheckbox(ctx, x, fpsCheckboxY, 30.0f, "FPS表示", &currentSettings_.showFPS,
                       fpsCheckboxHovered_);
        
        // 解像度選択（次回起動時に有効）
        float resolutionY = fpsCheckboxY + buttonSpacing;
        float resolutionButtonWidth = 40.0f;
        float resolutionTextWidth = width - 20.0f - resolutionButtonWidth * 2.0f - 20.0f;
        
        // 前の解像度ボタン
        RenderButton(ctx, x, resolutionY, resolutionButtonWidth, buttonHeight, "<",
                     resolutionPrevButtonHovered_);
        
        // 解像度情報表示
        float resolutionTextX = x + resolutionButtonWidth + 10.0f;
        const char* resolutionName = currentSettings_.resolution.c_str();
        char resolutionText[256];
        snprintf(resolutionText, sizeof(resolutionText), "解像度: %s (次回起動時に有効)", resolutionName);
        float labelFontSize = 20.0f;
        systemAPI_->Render().DrawTextDefault(
            resolutionText, resolutionTextX, resolutionY + (buttonHeight - labelFontSize) / 2.0f,
            labelFontSize, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        
        // 次の解像度ボタン
        float resolutionNextX = resolutionTextX + resolutionTextWidth + 10.0f;
        RenderButton(ctx, resolutionNextX, resolutionY, resolutionButtonWidth, buttonHeight, ">",
                     resolutionNextButtonHovered_);
    }
    
    // フルスクリーン時も解像度選択を表示（FPS表示の下）
    if (currentSettings_.isFullscreen && systemAPI_) {
        float resolutionY;
        if (systemAPI_->Window().GetMonitorCount() > 1) {
            resolutionY = startY + buttonSpacing * 2;
        } else {
            resolutionY = startY + buttonSpacing;
        }
        
        float resolutionButtonWidth = 40.0f;
        float resolutionTextWidth = width - 20.0f - resolutionButtonWidth * 2.0f - 20.0f;
        
        // 前の解像度ボタン
        RenderButton(ctx, x, resolutionY, resolutionButtonWidth, buttonHeight, "<",
                     resolutionPrevButtonHovered_);
        
        // 解像度情報表示
        float resolutionTextX = x + resolutionButtonWidth + 10.0f;
        const char* resolutionName = currentSettings_.resolution.c_str();
        char resolutionText[256];
        snprintf(resolutionText, sizeof(resolutionText), "解像度: %s (次回起動時に有効)", resolutionName);
        float labelFontSize = 20.0f;
        systemAPI_->Render().DrawTextDefault(
            resolutionText, resolutionTextX, resolutionY + (buttonHeight - labelFontSize) / 2.0f,
            labelFontSize, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        
        // 次の解像度ボタン
        float resolutionNextX = resolutionTextX + resolutionTextWidth + 10.0f;
        RenderButton(ctx, resolutionNextX, resolutionY, resolutionButtonWidth, buttonHeight, ">",
                     resolutionNextButtonHovered_);
    }
}

void SettingsOverlay::RenderSlider(float x, float y, float width, float height, const char* label,
                                   float* value, int sliderId) {
    float labelFontSize = 22.0f;
    float valueFontSize = 20.0f;
    float sliderBarHeight = 8.0f;
    float sliderHandleSize = 20.0f;
    float labelWidth = 120.0f;
    float sliderX = x + labelWidth;
    float sliderWidth = width - labelWidth - 100.0f;
    float sliderY = y + (height - sliderBarHeight) / 2.0f;

    // ラベル
    systemAPI_->Render().DrawTextDefault(
        label, x, y + (height - labelFontSize) / 2.0f, labelFontSize,
        ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    systemAPI_->Render().DrawRectangle(
        static_cast<int>(sliderX),
        static_cast<int>(sliderY),
        static_cast<int>(sliderWidth),
        static_cast<int>(sliderBarHeight),
        ToCoreColor(ui::OverlayColors::PANEL_BG_DARK));

    // スライダーバー（値の部分）
    float valueWidth = sliderWidth * (*value);
    if (valueWidth > 1.0f) {
        systemAPI_->Render().DrawRectangle(
            static_cast<int>(sliderX),
            static_cast<int>(sliderY),
            static_cast<int>(valueWidth),
            static_cast<int>(sliderBarHeight),
            ToCoreColor(ui::OverlayColors::BUTTON_BLUE));
    }

    // スライダーハンドル
    float handleX = sliderX + valueWidth - sliderHandleSize / 2.0f;
    float handleY = y + height / 2.0f - sliderHandleSize / 2.0f;
    ColorRGBA handleColor = (isDraggingSlider_ && draggedSliderId_ == sliderId)
        ? ToCoreColor(ui::OverlayColors::BUTTON_BLUE_HOVER)
        : ToCoreColor(ui::OverlayColors::BUTTON_BLUE);
    systemAPI_->Render().DrawCircle(
        static_cast<int>(handleX + sliderHandleSize / 2.0f),
        static_cast<int>(handleY + sliderHandleSize / 2.0f),
        sliderHandleSize / 2.0f,
        handleColor);

    // 値表示
    char valueText[32];
    snprintf(valueText, sizeof(valueText), "%.0f%%", (*value) * 100.0f);
    float valueX = sliderX + sliderWidth + 10.0f;
    systemAPI_->Render().DrawTextDefault(
        valueText, valueX, y + (height - valueFontSize) / 2.0f, valueFontSize,
        ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
}

void SettingsOverlay::RenderButton(SharedContext& ctx, float x, float y, float width, float height,
                                   const char* label, bool& isHovered, bool isEnabled) {
    auto mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePosition()
                              : Vec2{0.0f, 0.0f};
    bool mouseInButton = isEnabled && IsPointInRect(x, y, width, height, mouse);
    isHovered = mouseInButton;

    const char* buttonTexture = mouseInButton
        ? ui::UiAssetKeys::ButtonPrimaryHover
        : ui::UiAssetKeys::ButtonPrimaryNormal;
    Rect buttonRect = {x, y, width, height};
    ColorRGBA buttonTint = isEnabled
        ? ToCoreColor(WHITE)
        : ToCoreColor(ui::OverlayColors::TEXT_DISABLED);
    systemAPI_->Render().DrawUiNineSlice(buttonTexture, buttonRect, 8, 8, 8, 8,
                                         buttonTint);

    // ボタンテキスト
    float fontSize = 22.0f;
    Vec2 textSize = systemAPI_->Render().MeasureTextDefaultCore(label, fontSize, 1.0f);
    float textX = x + (width - textSize.x) / 2.0f;
    float textY = y + (height - fontSize) / 2.0f;
    ColorRGBA textColor = isEnabled
        ? ToCoreColor(systemAPI_->Render().GetReadableTextColor(buttonTexture))
        : ToCoreColor(ui::OverlayColors::TEXT_DISABLED);
    systemAPI_->Render().DrawTextDefault(
        label, textX, textY, fontSize,
        textColor);
}

void SettingsOverlay::RenderCheckbox(SharedContext& ctx, float x, float y, float size,
                                     const char* label, bool* value, bool& isHovered) {
    auto mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePosition()
                              : Vec2{0.0f, 0.0f};
    float labelFontSize = 22.0f;
    float labelX = x + size + 10.0f;
    float labelY = y + (size - labelFontSize) / 2.0f;

    bool mouseInCheckbox = IsPointInRect(x, y, size, size, mouse);
    isHovered = mouseInCheckbox || IsPointInRect(x, y, size + 200.0f, size, mouse);

    ColorRGBA bgColor = (*value) ? ToCoreColor(ui::OverlayColors::BUTTON_BLUE)
                                 : ToCoreColor(ui::OverlayColors::PANEL_BG_DARK);
    if (mouseInCheckbox) {
        bgColor = (*value) ? ToCoreColor(ui::OverlayColors::BUTTON_BLUE_HOVER)
                           : ToCoreColor(ui::OverlayColors::PANEL_ACCENT);
    }
    systemAPI_->Render().DrawRectangle(
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(size),
        static_cast<int>(size),
        bgColor);
    systemAPI_->Render().DrawRectangleLines(
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(size),
        static_cast<int>(size),
        2.0f,
        ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

    // チェックマーク
    if (*value) {
        float checkSize = size * 0.6f;
        float checkX = x + (size - checkSize) / 2.0f;
        float checkY = y + (size - checkSize) / 2.0f;
        // 簡易的なチェックマーク（Xで代用）
        systemAPI_->Render().DrawTextDefault("✓", checkX, checkY, checkSize,
                                             ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
    }

    // ラベル
    systemAPI_->Render().DrawTextDefault(
        label, labelX, labelY, labelFontSize,
        ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
}

} // namespace core
} // namespace game
