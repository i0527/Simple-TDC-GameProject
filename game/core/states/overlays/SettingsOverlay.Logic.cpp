#include "SettingsOverlay.hpp"

// 標準ライブラリ
#include <algorithm>
#include <filesystem>
#include <fstream>

// 外部ライブラリ
#include <nlohmann/json.hpp>

// プロジェクト内
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/AudioControlAPI.hpp"
#include "../../api/InputSystemAPI.hpp"

using json = nlohmann::json;

namespace game {
namespace core {

void SettingsOverlay::Update(SharedContext& ctx, float deltaTime) {
    (void)deltaTime;
    if (!isInitialized_) {
        return;
    }

    if (!audioAPI_ && ctx.audioAPI) {
        audioAPI_ = ctx.audioAPI;
        ApplySettings();
    }

    // ESCキーで閉じる
    if (ctx.inputAPI && ctx.inputAPI->IsEscapePressed()) {
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
    // 音量設定を適用
    if (audioAPI_) {
        audioAPI_->SetMasterVolume(currentSettings_.masterVolume);
        audioAPI_->SetBGMVolume(currentSettings_.bgmVolume);
        audioAPI_->SetSEVolume(currentSettings_.seVolume);
    } else if (systemAPI_) {
        systemAPI_->Audio().SetMasterVolume(currentSettings_.masterVolume);
        systemAPI_->Audio().SetBGMVolume(currentSettings_.bgmVolume);
        systemAPI_->Audio().SetSEVolume(currentSettings_.seVolume);
    }

    // フルスクリーン設定を適用
    if (systemAPI_) {
        systemAPI_->Window().SetFullscreen(currentSettings_.isFullscreen);
    }

    // FPS表示設定を適用
    if (systemAPI_) {
        systemAPI_->Window().SetFPSDisplayEnabled(currentSettings_.showFPS);
    }

    LOG_DEBUG("SettingsOverlay: Settings applied");
}

void SettingsOverlay::ResetToDefaults() {
    currentSettings_ = SettingsData();
    ApplySettings();
    LOG_INFO("SettingsOverlay: Settings reset to defaults");
}

bool SettingsOverlay::IsPointInRect(float x, float y, float width, float height,
                                    Vec2 point) const {
    return point.x >= x && point.x <= x + width && point.y >= y && point.y <= y + height;
}

void SettingsOverlay::ProcessMouseInput(SharedContext& ctx) {
    InputSystemAPI* inputAPI = ctx.inputAPI;
    auto mouse = inputAPI ? inputAPI->GetMousePosition()
                          : Vec2{0.0f, 0.0f};
    const float windowX = 200.0f;
    const float windowY = 150.0f;
    const float windowWidth = 1520.0f;
    const float windowHeight = 780.0f;
    float buttonY = windowY + windowHeight - 80.0f;
    float buttonWidth = 150.0f;
    float buttonHeight = 40.0f;
    float buttonSpacing = 20.0f;
    const int buttonCount = 5;
    float totalButtonWidth =
        buttonWidth * static_cast<float>(buttonCount) +
        buttonSpacing * static_cast<float>(buttonCount - 1);
    float applyButtonX = windowX + (windowWidth - totalButtonWidth) / 2.0f;
    float resetButtonX = applyButtonX + buttonWidth + buttonSpacing;
    float titleButtonX = resetButtonX + buttonWidth + buttonSpacing;
    float quitButtonX = titleButtonX + buttonWidth + buttonSpacing;
    float closeButtonX = quitButtonX + buttonWidth + buttonSpacing;

    if (inputAPI && inputAPI->IsLeftClickPressed()) {
        // 適用ボタン
        if (IsPointInRect(applyButtonX, buttonY, buttonWidth, buttonHeight, mouse)) {
            ApplySettings();
            SaveSettings();
            inputAPI->ConsumeLeftClick();
            LOG_INFO("SettingsOverlay: Settings applied and saved");
        }
        // リセットボタン
        else if (IsPointInRect(resetButtonX, buttonY, buttonWidth, buttonHeight, mouse)) {
            ResetToDefaults();
            inputAPI->ConsumeLeftClick();
        }
        // タイトルへボタン（タイトル中は無効）
        else if (IsPointInRect(titleButtonX, buttonY, buttonWidth, buttonHeight, mouse)) {
            if (ctx.currentState != GameState::Title) {
                hasTransitionRequest_ = true;
                requestedNextState_ = GameState::Title;
                inputAPI->ConsumeLeftClick();
            }
        }
        // ゲーム終了ボタン
        else if (IsPointInRect(quitButtonX, buttonY, buttonWidth, buttonHeight, mouse)) {
            requestQuit_ = true;
            inputAPI->ConsumeLeftClick();
        }
        // 閉じるボタン
        else if (IsPointInRect(closeButtonX, buttonY, buttonWidth, buttonHeight, mouse)) {
            if (HasUnsavedChanges()) {
                LOG_WARN("SettingsOverlay: Unsaved changes detected, but closing anyway");
            }
            requestClose_ = true;
            inputAPI->ConsumeLeftClick();
        }
        // フルスクリーンボタン
        else {
            float sectionY = windowY + 100.0f;
            float sectionWidth = (windowWidth - 60.0f) / 2.0f;
            float displaySectionX = windowX + sectionWidth + 40.0f;
            float buttonSpacingLocal = 50.0f;
            float startY = sectionY + 40.0f;
            float fullscreenButtonX = displaySectionX;
            float fullscreenButtonY = startY;
            float fullscreenButtonWidth = sectionWidth - 20.0f;

            if (IsPointInRect(fullscreenButtonX, fullscreenButtonY, fullscreenButtonWidth, buttonHeight, mouse)) {
                currentSettings_.isFullscreen = !currentSettings_.isFullscreen;
                inputAPI->ConsumeLeftClick();
            }
            // FPS表示チェックボックス
            else {
                float checkboxY = startY + buttonSpacingLocal;
                float checkboxSize = 30.0f;
                if (IsPointInRect(fullscreenButtonX, checkboxY, checkboxSize + 200.0f, checkboxSize, mouse)) {
                    currentSettings_.showFPS = !currentSettings_.showFPS;
                    inputAPI->ConsumeLeftClick();
                }
            }
        }
    }
}

void SettingsOverlay::ProcessSliderDrag(SharedContext& ctx) {
    InputSystemAPI* inputAPI = ctx.inputAPI;
    auto mouse = inputAPI ? inputAPI->GetMousePosition()
                          : Vec2{0.0f, 0.0f};
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
    // スライダーの位置を計算
    auto getSliderRect = [&](int sliderId) -> std::pair<float, float> {
        float sliderX = volumeSectionX + labelWidth;
        float sliderWidth = sectionWidth - labelWidth - 100.0f;
        return {sliderX, sliderX + sliderWidth};
    };

    if (inputAPI && inputAPI->IsLeftClickPressed()) {
        // スライダーのクリックを検出
        for (int i = 0; i < 3; ++i) {
            auto [sliderStartX, sliderEndX] = getSliderRect(i);
            float sliderY = startY + i * sliderSpacing + (sliderHeight - sliderBarHeight) / 2.0f;

            if (mouse.x >= sliderStartX && mouse.x <= sliderEndX &&
                mouse.y >= sliderY && mouse.y <= sliderY + sliderBarHeight) {
                isDraggingSlider_ = true;
                draggedSliderId_ = i;
                inputAPI->ConsumeLeftClick();
                break;
            }
        }
    }

    if (isDraggingSlider_ && inputAPI && inputAPI->IsLeftClickDown()) {
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
        if (audioAPI_) {
            audioAPI_->SetMasterVolume(currentSettings_.masterVolume);
            audioAPI_->SetBGMVolume(currentSettings_.bgmVolume);
            audioAPI_->SetSEVolume(currentSettings_.seVolume);
        } else if (systemAPI_) {
            systemAPI_->Audio().SetMasterVolume(currentSettings_.masterVolume);
            systemAPI_->Audio().SetBGMVolume(currentSettings_.bgmVolume);
            systemAPI_->Audio().SetSEVolume(currentSettings_.seVolume);
        }
    }

    if (inputAPI && inputAPI->IsLeftClickReleased()) {
        isDraggingSlider_ = false;
        draggedSliderId_ = -1;
    }
}

} // namespace core
} // namespace game
