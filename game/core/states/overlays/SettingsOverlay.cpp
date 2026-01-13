#include "SettingsOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../ui/OverlayColors.hpp"
#include <raylib.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

namespace game {
namespace core {

SettingsOverlay::SettingsOverlay()
    : systemAPI_(nullptr)
    , isInitialized_(false)
    , requestClose_(false)
    , hasTransitionRequest_(false)
    , requestedNextState_(GameState::Title)
    , isDraggingSlider_(false)
    , draggedSliderId_(-1)
    , settingsFilePath_("data/settings.json")
    , applyButtonHovered_(false)
    , resetButtonHovered_(false)
    , closeButtonHovered_(false)
    , fullscreenButtonHovered_(false)
    , fpsCheckboxHovered_(false)
{
    // デフォルト設定
    currentSettings_ = SettingsData();
    savedSettings_ = SettingsData();
}

bool SettingsOverlay::Initialize(BaseSystemAPI* systemAPI) {
    if (isInitialized_) {
        LOG_ERROR("SettingsOverlay already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("SettingsOverlay: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    requestClose_ = false;
    hasTransitionRequest_ = false;

    // 設定を読み込む
    LoadSettings();
    
    // 読み込んだ設定を適用
    ApplySettings();

    isInitialized_ = true;
    LOG_INFO("SettingsOverlay initialized");
    return true;
}

void SettingsOverlay::Update(SharedContext& ctx, float deltaTime) {
    if (!isInitialized_) {
        return;
    }

    // ESCキーで閉じる
    if (systemAPI_->IsKeyPressed(256)) {
        if (HasUnsavedChanges()) {
            LOG_WARN("SettingsOverlay: Unsaved changes detected, but closing anyway");
        }
        requestClose_ = true;
    }
    
    // マウス入力処理
    ProcessMouseInput(ctx);
    
    // スライダードラッグ処理
    ProcessSliderDrag(ctx);
}

void SettingsOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) {
        return;
    }

    // ウィンドウの位置とサイズ
    const float windowX = 200.0f;
    const float windowY = 150.0f;
    const float windowWidth = 1520.0f;
    const float windowHeight = 780.0f;
    
    // ウィンドウ背景（半透明の暗い背景）
    systemAPI_->DrawRectangle(
        static_cast<int>(windowX),
        static_cast<int>(windowY),
        static_cast<int>(windowWidth),
        static_cast<int>(windowHeight),
        ui::OverlayColors::OVERLAY_BG
    );
    
    // ウィンドウ枠線
    systemAPI_->DrawRectangleLines(
        static_cast<int>(windowX),
        static_cast<int>(windowY),
        static_cast<int>(windowWidth),
        static_cast<int>(windowHeight),
        2.0f,
        ui::OverlayColors::BORDER_DEFAULT
    );
    
    // タイトル
    const char* titleText = "設定";
    float titleFontSize = 36.0f;
    Vector2 titleSize = systemAPI_->MeasureTextDefault(titleText, titleFontSize, 1.0f);
    float titleX = windowX + (windowWidth - titleSize.x) / 2.0f;
    float titleY = windowY + 20.0f;
    systemAPI_->DrawTextDefault(titleText, titleX, titleY, titleFontSize, ui::OverlayColors::TEXT_PRIMARY);
    
    // 音量設定セクション（左側）
    float sectionY = titleY + titleSize.y + 40.0f;
    float sectionWidth = (windowWidth - 60.0f) / 2.0f;
    float sectionHeight = windowHeight - (sectionY - windowY) - 120.0f;
    RenderVolumeSection(windowX + 20.0f, sectionY, sectionWidth, sectionHeight);
    
    // 表示設定セクション（右側）
    RenderDisplaySection(windowX + sectionWidth + 40.0f, sectionY, sectionWidth, sectionHeight);
    
    // 操作ボタン（下部）
    float buttonY = windowY + windowHeight - 80.0f;
    float buttonWidth = 150.0f;
    float buttonHeight = 40.0f;
    float buttonSpacing = 20.0f;
    
    // 適用ボタン
    float applyButtonX = windowX + (windowWidth - (buttonWidth * 3 + buttonSpacing * 2)) / 2.0f;
    RenderButton(applyButtonX, buttonY, buttonWidth, buttonHeight, "適用", applyButtonHovered_);
    
    // リセットボタン
    float resetButtonX = applyButtonX + buttonWidth + buttonSpacing;
    RenderButton(resetButtonX, buttonY, buttonWidth, buttonHeight, "リセット", resetButtonHovered_);
    
    // 閉じるボタン
    float closeButtonX = resetButtonX + buttonWidth + buttonSpacing;
    RenderButton(closeButtonX, buttonY, buttonWidth, buttonHeight, "閉じる", closeButtonHovered_);
}

void SettingsOverlay::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    isInitialized_ = false;
    systemAPI_ = nullptr;
    LOG_INFO("SettingsOverlay shutdown");
}

bool SettingsOverlay::RequestClose() const {
    if (requestClose_) {
        requestClose_ = false;
        return true;
    }
    return false;
}

bool SettingsOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}

void SettingsOverlay::LoadSettings() {
    try {
        std::ifstream file(settingsFilePath_);
        if (!file.is_open()) {
            LOG_WARN("SettingsOverlay: Failed to open settings file: {}. Using defaults.", settingsFilePath_);
            currentSettings_ = SettingsData();
            savedSettings_ = currentSettings_;
            return;
        }

        json data;
        file >> data;

        // 音量設定
        currentSettings_.masterVolume = data.value("masterVolume", 1.0f);
        currentSettings_.bgmVolume = data.value("bgmVolume", 1.0f);
        currentSettings_.seVolume = data.value("seVolume", 1.0f);
        
        // 表示設定
        currentSettings_.isFullscreen = data.value("isFullscreen", false);
        currentSettings_.showFPS = data.value("showFPS", false);

        // 値をクランプ
        currentSettings_.masterVolume = std::max(0.0f, std::min(1.0f, currentSettings_.masterVolume));
        currentSettings_.bgmVolume = std::max(0.0f, std::min(1.0f, currentSettings_.bgmVolume));
        currentSettings_.seVolume = std::max(0.0f, std::min(1.0f, currentSettings_.seVolume));

        savedSettings_ = currentSettings_;
        LOG_INFO("SettingsOverlay: Settings loaded from {}", settingsFilePath_);
    } catch (const json::parse_error& e) {
        LOG_ERROR("SettingsOverlay: JSON parse error: {}. Using defaults.", e.what());
        currentSettings_ = SettingsData();
        savedSettings_ = currentSettings_;
    } catch (const std::exception& e) {
        LOG_ERROR("SettingsOverlay: Exception while loading settings: {}. Using defaults.", e.what());
        currentSettings_ = SettingsData();
        savedSettings_ = currentSettings_;
    }
}

void SettingsOverlay::SaveSettings() {
    try {
        // ディレクトリが存在しない場合は作成
        std::filesystem::path filePath(settingsFilePath_);
        if (filePath.has_parent_path()) {
            std::filesystem::create_directories(filePath.parent_path());
        }

        json data;
        data["masterVolume"] = currentSettings_.masterVolume;
        data["bgmVolume"] = currentSettings_.bgmVolume;
        data["seVolume"] = currentSettings_.seVolume;
        data["isFullscreen"] = currentSettings_.isFullscreen;
        data["showFPS"] = currentSettings_.showFPS;

        std::ofstream file(settingsFilePath_);
        if (!file.is_open()) {
            LOG_ERROR("SettingsOverlay: Failed to open settings file for writing: {}", settingsFilePath_);
            return;
        }

        file << data.dump(2);  // インデント2で整形して出力
        savedSettings_ = currentSettings_;
        LOG_INFO("SettingsOverlay: Settings saved to {}", settingsFilePath_);
    } catch (const std::exception& e) {
        LOG_ERROR("SettingsOverlay: Exception while saving settings: {}", e.what());
    }
}

bool SettingsOverlay::HasUnsavedChanges() const {
    return currentSettings_.masterVolume != savedSettings_.masterVolume ||
           currentSettings_.bgmVolume != savedSettings_.bgmVolume ||
           currentSettings_.seVolume != savedSettings_.seVolume ||
           currentSettings_.isFullscreen != savedSettings_.isFullscreen ||
           currentSettings_.showFPS != savedSettings_.showFPS;
}

void SettingsOverlay::ApplySettings() {
    if (!systemAPI_) {
        return;
    }

    // 音量設定を適用
    systemAPI_->SetMasterVolume(currentSettings_.masterVolume);
    systemAPI_->SetBGMVolume(currentSettings_.bgmVolume);
    systemAPI_->SetSEVolume(currentSettings_.seVolume);
    
    // フルスクリーン設定を適用
    systemAPI_->SetFullscreen(currentSettings_.isFullscreen);
    
    // FPS表示設定を適用
    systemAPI_->SetFPSDisplayEnabled(currentSettings_.showFPS);
    
    LOG_DEBUG("SettingsOverlay: Settings applied");
}

void SettingsOverlay::ResetToDefaults() {
    currentSettings_ = SettingsData();
    ApplySettings();
    LOG_INFO("SettingsOverlay: Settings reset to defaults");
}

void SettingsOverlay::RenderVolumeSection(float x, float y, float width, float height) {
    float fontSize = 28.0f;
    float labelFontSize = 24.0f;
    float sliderHeight = 40.0f;
    float sliderSpacing = 60.0f;
    float startY = y + 40.0f;
    
    // セクションタイトル
    const char* sectionTitle = "音量設定";
    Vector2 titleSize = systemAPI_->MeasureTextDefault(sectionTitle, fontSize, 1.0f);
    float titleX = x + (width - titleSize.x) / 2.0f;
    systemAPI_->DrawTextDefault(sectionTitle, titleX, y, fontSize, ui::OverlayColors::TEXT_PRIMARY);
    
    // マスターボリューム
    RenderSlider(x, startY, width - 20.0f, sliderHeight, "マスター", &currentSettings_.masterVolume, 0);
    
    // BGMボリューム
    RenderSlider(x, startY + sliderSpacing, width - 20.0f, sliderHeight, "BGM", &currentSettings_.bgmVolume, 1);
    
    // SEボリューム
    RenderSlider(x, startY + sliderSpacing * 2, width - 20.0f, sliderHeight, "SE", &currentSettings_.seVolume, 2);
}

void SettingsOverlay::RenderDisplaySection(float x, float y, float width, float height) {
    float fontSize = 28.0f;
    float buttonHeight = 40.0f;
    float buttonSpacing = 50.0f;
    float startY = y + 40.0f;
    
    // セクションタイトル
    const char* sectionTitle = "表示設定";
    Vector2 titleSize = systemAPI_->MeasureTextDefault(sectionTitle, fontSize, 1.0f);
    float titleX = x + (width - titleSize.x) / 2.0f;
    systemAPI_->DrawTextDefault(sectionTitle, titleX, y, fontSize, ui::OverlayColors::TEXT_PRIMARY);
    
    // フルスクリーンボタン
    const char* fullscreenText = currentSettings_.isFullscreen ? "フルスクリーン: ON" : "フルスクリーン: OFF";
    RenderButton(x, startY, width - 20.0f, buttonHeight, fullscreenText, fullscreenButtonHovered_);
    
    // FPS表示チェックボックス
    RenderCheckbox(x, startY + buttonSpacing, 30.0f, "FPS表示", &currentSettings_.showFPS, fpsCheckboxHovered_);
}

void SettingsOverlay::RenderSlider(float x, float y, float width, float height, const char* label, float* value, int sliderId) {
    float labelFontSize = 22.0f;
    float valueFontSize = 20.0f;
    float sliderBarHeight = 8.0f;
    float sliderHandleSize = 20.0f;
    float labelWidth = 120.0f;
    float sliderX = x + labelWidth;
    float sliderWidth = width - labelWidth - 100.0f;
    float sliderY = y + (height - sliderBarHeight) / 2.0f;
    
    // ラベル
    systemAPI_->DrawTextDefault(label, x, y + (height - labelFontSize) / 2.0f, labelFontSize, ui::OverlayColors::TEXT_PRIMARY);
    
    // スライダーバー背景
    systemAPI_->DrawRectangle(
        static_cast<int>(sliderX),
        static_cast<int>(sliderY),
        static_cast<int>(sliderWidth),
        static_cast<int>(sliderBarHeight),
        ui::OverlayColors::PANEL_BG_DARK
    );
    
    // スライダーバー（値の部分）
    float valueWidth = sliderWidth * (*value);
    systemAPI_->DrawRectangle(
        static_cast<int>(sliderX),
        static_cast<int>(sliderY),
        static_cast<int>(valueWidth),
        static_cast<int>(sliderBarHeight),
        ui::OverlayColors::BUTTON_BLUE
    );
    
    // スライダーハンドル
    float handleX = sliderX + valueWidth - sliderHandleSize / 2.0f;
    float handleY = y + height / 2.0f - sliderHandleSize / 2.0f;
    Color handleColor = (isDraggingSlider_ && draggedSliderId_ == sliderId) 
        ? ui::OverlayColors::BUTTON_BLUE_HOVER 
        : ui::OverlayColors::BUTTON_BLUE;
    systemAPI_->DrawCircle(
        static_cast<int>(handleX + sliderHandleSize / 2.0f),
        static_cast<int>(handleY + sliderHandleSize / 2.0f),
        sliderHandleSize / 2.0f,
        handleColor
    );
    
    // 値表示
    char valueText[32];
    snprintf(valueText, sizeof(valueText), "%.0f%%", (*value) * 100.0f);
    float valueX = sliderX + sliderWidth + 10.0f;
    systemAPI_->DrawTextDefault(valueText, valueX, y + (height - valueFontSize) / 2.0f, valueFontSize, ui::OverlayColors::TEXT_SECONDARY);
}

void SettingsOverlay::RenderButton(float x, float y, float width, float height, const char* label, bool& isHovered) {
    Vector2 mouse = systemAPI_->GetMousePosition();
    bool mouseInButton = IsPointInRect(x, y, width, height, mouse);
    isHovered = mouseInButton;
    
    Color bgColor = mouseInButton ? ui::OverlayColors::BUTTON_BLUE_HOVER : ui::OverlayColors::BUTTON_BLUE;
    Color borderColor = mouseInButton ? ui::OverlayColors::BUTTON_BLUE_HOVER : ui::OverlayColors::BORDER_DEFAULT;
    
    // ボタン背景
    systemAPI_->DrawRectangle(
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(width),
        static_cast<int>(height),
        bgColor
    );
    
    // ボタン枠線
    systemAPI_->DrawRectangleLines(
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(width),
        static_cast<int>(height),
        2.0f,
        borderColor
    );
    
    // ボタンテキスト
    float fontSize = 22.0f;
    Vector2 textSize = systemAPI_->MeasureTextDefault(label, fontSize, 1.0f);
    float textX = x + (width - textSize.x) / 2.0f;
    float textY = y + (height - fontSize) / 2.0f;
    systemAPI_->DrawTextDefault(label, textX, textY, fontSize, ui::OverlayColors::TEXT_PRIMARY);
}

void SettingsOverlay::RenderCheckbox(float x, float y, float size, const char* label, bool* value, bool& isHovered) {
    Vector2 mouse = systemAPI_->GetMousePosition();
    float labelFontSize = 22.0f;
    float labelX = x + size + 10.0f;
    float labelY = y + (size - labelFontSize) / 2.0f;
    
    bool mouseInCheckbox = IsPointInRect(x, y, size, size, mouse);
    isHovered = mouseInCheckbox || IsPointInRect(x, y, size + 200.0f, size, mouse);
    
    // チェックボックス背景
    Color bgColor = (*value) ? ui::OverlayColors::BUTTON_BLUE : ui::OverlayColors::PANEL_BG_DARK;
    if (mouseInCheckbox) {
        bgColor = (*value) ? ui::OverlayColors::BUTTON_BLUE_HOVER : ui::OverlayColors::PANEL_ACCENT;
    }
    
    systemAPI_->DrawRectangle(
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(size),
        static_cast<int>(size),
        bgColor
    );
    
    // チェックボックス枠線
    systemAPI_->DrawRectangleLines(
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(size),
        static_cast<int>(size),
        2.0f,
        ui::OverlayColors::BORDER_DEFAULT
    );
    
    // チェックマーク
    if (*value) {
        float checkSize = size * 0.6f;
        float checkX = x + (size - checkSize) / 2.0f;
        float checkY = y + (size - checkSize) / 2.0f;
        // 簡易的なチェックマーク（Xで代用）
        systemAPI_->DrawTextDefault("✓", checkX, checkY, checkSize, ui::OverlayColors::TEXT_PRIMARY);
    }
    
    // ラベル
    systemAPI_->DrawTextDefault(label, labelX, labelY, labelFontSize, ui::OverlayColors::TEXT_PRIMARY);
}

bool SettingsOverlay::IsPointInRect(float x, float y, float width, float height, Vector2 point) const {
    return point.x >= x && point.x <= x + width && point.y >= y && point.y <= y + height;
}

void SettingsOverlay::ProcessMouseInput(SharedContext& ctx) {
    Vector2 mouse = systemAPI_->GetMousePosition();
    const float windowX = 200.0f;
    const float windowY = 150.0f;
    const float windowWidth = 1520.0f;
    const float windowHeight = 780.0f;
    float buttonY = windowY + windowHeight - 80.0f;
    float buttonWidth = 150.0f;
    float buttonHeight = 40.0f;
    float buttonSpacing = 20.0f;
    float applyButtonX = windowX + (windowWidth - (buttonWidth * 3 + buttonSpacing * 2)) / 2.0f;
    float resetButtonX = applyButtonX + buttonWidth + buttonSpacing;
    float closeButtonX = resetButtonX + buttonWidth + buttonSpacing;
    
    if (systemAPI_->IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // 適用ボタン
        if (IsPointInRect(applyButtonX, buttonY, buttonWidth, buttonHeight, mouse)) {
            ApplySettings();
            SaveSettings();
            systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
            LOG_INFO("SettingsOverlay: Settings applied and saved");
        }
        // リセットボタン
        else if (IsPointInRect(resetButtonX, buttonY, buttonWidth, buttonHeight, mouse)) {
            ResetToDefaults();
            systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
        }
        // 閉じるボタン
        else if (IsPointInRect(closeButtonX, buttonY, buttonWidth, buttonHeight, mouse)) {
            if (HasUnsavedChanges()) {
                LOG_WARN("SettingsOverlay: Unsaved changes detected, but closing anyway");
            }
            requestClose_ = true;
            systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
        }
        // フルスクリーンボタン
        else {
            float sectionY = windowY + 100.0f;
            float sectionWidth = (windowWidth - 60.0f) / 2.0f;
            float displaySectionX = windowX + sectionWidth + 40.0f;
            float buttonHeight = 40.0f;
            float buttonSpacing = 50.0f;
            float startY = sectionY + 40.0f;
            float fullscreenButtonX = displaySectionX;
            float fullscreenButtonY = startY;
            float fullscreenButtonWidth = sectionWidth - 20.0f;
            
            if (IsPointInRect(fullscreenButtonX, fullscreenButtonY, fullscreenButtonWidth, buttonHeight, mouse)) {
                currentSettings_.isFullscreen = !currentSettings_.isFullscreen;
                systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
            }
            // FPS表示チェックボックス
            else {
                float checkboxY = startY + buttonSpacing;
                float checkboxSize = 30.0f;
                if (IsPointInRect(fullscreenButtonX, checkboxY, checkboxSize + 200.0f, checkboxSize, mouse)) {
                    currentSettings_.showFPS = !currentSettings_.showFPS;
                    systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
                }
            }
        }
    }
}

void SettingsOverlay::ProcessSliderDrag(SharedContext& ctx) {
    Vector2 mouse = systemAPI_->GetMousePosition();
    const float windowX = 200.0f;
    const float windowY = 150.0f;
    float sectionY = windowY + 100.0f;
    float sectionWidth = (1520.0f - 60.0f) / 2.0f;
    float volumeSectionX = windowX + 20.0f;
    float sliderHeight = 40.0f;
    float sliderSpacing = 60.0f;
    float startY = sectionY + 40.0f;
    float labelWidth = 120.0f;
    float sliderBarHeight = 8.0f;
    float sliderHandleSize = 20.0f;
    
    // スライダーの位置を計算
    auto getSliderRect = [&](int sliderId) -> std::pair<float, float> {
        float sliderX = volumeSectionX + labelWidth;
        float sliderWidth = sectionWidth - labelWidth - 100.0f;
        float sliderY = startY + sliderId * sliderSpacing + (sliderHeight - sliderBarHeight) / 2.0f;
        return {sliderX, sliderX + sliderWidth};
    };
    
    if (systemAPI_->IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // スライダーのクリックを検出
        for (int i = 0; i < 3; ++i) {
            auto [sliderStartX, sliderEndX] = getSliderRect(i);
            float sliderY = startY + i * sliderSpacing + (sliderHeight - sliderBarHeight) / 2.0f;
            
            if (mouse.x >= sliderStartX && mouse.x <= sliderEndX &&
                mouse.y >= sliderY && mouse.y <= sliderY + sliderBarHeight) {
                isDraggingSlider_ = true;
                draggedSliderId_ = i;
                systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
                break;
            }
        }
    }
    
    if (isDraggingSlider_ && systemAPI_->IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        // スライダーの値を更新
        auto [sliderStartX, sliderEndX] = getSliderRect(draggedSliderId_);
        float normalizedX = (mouse.x - sliderStartX) / (sliderEndX - sliderStartX);
        normalizedX = std::max(0.0f, std::min(1.0f, normalizedX));
        
        switch (draggedSliderId_) {
            case 0: currentSettings_.masterVolume = normalizedX; break;
            case 1: currentSettings_.bgmVolume = normalizedX; break;
            case 2: currentSettings_.seVolume = normalizedX; break;
        }
        
        // 音量を即座に適用
        systemAPI_->SetMasterVolume(currentSettings_.masterVolume);
        systemAPI_->SetBGMVolume(currentSettings_.bgmVolume);
        systemAPI_->SetSEVolume(currentSettings_.seVolume);
    }
    
    if (systemAPI_->IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        isDraggingSlider_ = false;
        draggedSliderId_ = -1;
    }
}

} // namespace core
} // namespace game
