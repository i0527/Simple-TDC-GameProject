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

    // ESCキーで閉じる（保存してから閉じる）
    if (ctx.inputAPI && ctx.inputAPI->IsEscapePressed()) {
        SaveSettings();
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
        currentSettings_.showCursor = data.value("showCursor", false);
        
        // 解像度（WQHD 削除済み。旧 0=WQHD,1=FHD,2=HD,3=SD → 0=FHD,1=HD,2=SD にマップ）
        int resolutionInt = data.value("resolution", static_cast<int>(Resolution::FHD));
        switch (resolutionInt) {
            case 0: case 1: currentSettings_.resolution = Resolution::FHD; break;
            case 2: currentSettings_.resolution = Resolution::HD; break;
            case 3: currentSettings_.resolution = Resolution::SD; break;
            default: currentSettings_.resolution = Resolution::FHD; break;
        }
        
        int windowModeInt = data.value("windowMode", -1);
        if (windowModeInt >= 0 && windowModeInt <= static_cast<int>(WindowMode::Borderless)) {
            currentSettings_.windowMode = static_cast<WindowMode>(windowModeInt);
        } else {
            // 後方互換性: isFullscreenからwindowModeを推測
            if (currentSettings_.isFullscreen) {
                currentSettings_.windowMode = WindowMode::Fullscreen;
            } else {
                currentSettings_.windowMode = WindowMode::Windowed;
            }
        }

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
        data["isFullscreen"] = currentSettings_.isFullscreen;  // 後方互換性のため残す
        data["showFPS"] = currentSettings_.showFPS;
        data["showCursor"] = currentSettings_.showCursor;
        data["resolution"] = static_cast<int>(currentSettings_.resolution);
        data["windowMode"] = static_cast<int>(currentSettings_.windowMode);

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
           currentSettings_.showFPS != savedSettings_.showFPS ||
           currentSettings_.showCursor != savedSettings_.showCursor ||
           currentSettings_.resolution != savedSettings_.resolution ||
           currentSettings_.windowMode != savedSettings_.windowMode;
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

    // 解像度設定を適用
    if (systemAPI_) {
        Resolution currentRes = systemAPI_->Render().GetCurrentResolution();
        if (currentSettings_.resolution != currentRes) {
            systemAPI_->Render().SetResolution(currentSettings_.resolution);
        }
    }
    
    // ウィンドウモード設定を適用（モニター切り替えは行わない）
    if (systemAPI_) {
        WindowMode currentMode = systemAPI_->Window().GetWindowMode();
        if (currentSettings_.windowMode != currentMode) {
            systemAPI_->Window().SetWindowMode(currentSettings_.windowMode);
        }
    }

    // FPS表示設定を適用
    if (systemAPI_) {
        systemAPI_->Window().SetFPSDisplayEnabled(currentSettings_.showFPS);
        systemAPI_->Window().SetCursorDisplayEnabled(currentSettings_.showCursor);
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
    auto mouse = inputAPI ? inputAPI->GetMousePositionInternal() : Vec2{0.0f, 0.0f};
    const float windowX = 200.0f;
    const float windowY = 150.0f;
    const float windowWidth = 1520.0f;
    const float windowHeight = 780.0f;
    float buttonY = windowY + windowHeight - 80.0f;
    float buttonWidth = 150.0f;
    float buttonHeight = 40.0f;
    float buttonSpacing = 20.0f;
    const int buttonCount = 4;
    float totalButtonWidth =
        buttonWidth * static_cast<float>(buttonCount) +
        buttonSpacing * static_cast<float>(buttonCount - 1);
    float resetButtonX = windowX + (windowWidth - totalButtonWidth) / 2.0f;
    float titleButtonX = resetButtonX + buttonWidth + buttonSpacing;
    float quitButtonX = titleButtonX + buttonWidth + buttonSpacing;
    float closeButtonX = quitButtonX + buttonWidth + buttonSpacing;

    if (inputAPI && inputAPI->IsLeftClickPressed()) {
        // リセットボタン
        if (IsPointInRect(resetButtonX, buttonY, buttonWidth, buttonHeight, mouse)) {
            ResetToDefaults();
            inputAPI->ConsumeLeftClick();
        }
        // タイトルへボタン（タイトル中は無効）
        else if (IsPointInRect(titleButtonX, buttonY, buttonWidth, buttonHeight, mouse)) {
            if (ctx.currentState != GameState::Title) {
                SaveSettings();
                hasTransitionRequest_ = true;
                requestedNextState_ = GameState::Title;
                inputAPI->ConsumeLeftClick();
            }
        }
        // ゲーム終了ボタン
        else if (IsPointInRect(quitButtonX, buttonY, buttonWidth, buttonHeight, mouse)) {
            SaveSettings();
            requestQuit_ = true;
            inputAPI->ConsumeLeftClick();
        }
        // 閉じるボタン（保存してから閉じる）
        else if (IsPointInRect(closeButtonX, buttonY, buttonWidth, buttonHeight, mouse)) {
            SaveSettings();
            requestClose_ = true;
            inputAPI->ConsumeLeftClick();
        }
        // 解像度・ウィンドウモード・モニター選択・FPSチェックボックス（Render と同一レイアウト）
        else {
            float sectionY = windowY + 20.0f;
            float sectionWidth = (windowWidth - 60.0f) / 2.0f;
            float displaySectionX = windowX + sectionWidth + 40.0f;
            float buttonSpacingLocal = 50.0f;
            float startY = sectionY + 40.0f;
            float resolutionButtonWidth = (sectionWidth - 20.0f - 120.0f) / 2.0f;
            float resolutionButtonX = displaySectionX + 120.0f;
            float windowModeButtonWidth = (sectionWidth - 20.0f - 180.0f) / 2.0f;
            float windowModeButtonX = displaySectionX + 180.0f;

            bool clicked = false;
            float windowModeY = startY + buttonSpacingLocal;

            // 解像度選択ボタン（即時適用）FHD / HD
            if (IsPointInRect(resolutionButtonX, startY, resolutionButtonWidth, buttonHeight, mouse)) {
                currentSettings_.resolution = Resolution::FHD;
                ApplySettings();
                inputAPI->ConsumeLeftClick();
                clicked = true;
            } else if (IsPointInRect(resolutionButtonX + resolutionButtonWidth + 5.0f, startY,
                                     resolutionButtonWidth, buttonHeight, mouse)) {
                currentSettings_.resolution = Resolution::HD;
                ApplySettings();
                inputAPI->ConsumeLeftClick();
                clicked = true;
            }
            
            // ウィンドウモード選択ボタン（即時適用。ウィンドウ / フルスクリーンのみ）
            if (!clicked) {
                if (IsPointInRect(windowModeButtonX, windowModeY, windowModeButtonWidth, buttonHeight, mouse)) {
                    currentSettings_.windowMode = WindowMode::Windowed;
                    ApplySettings();
                    inputAPI->ConsumeLeftClick();
                    clicked = true;
                } else if (IsPointInRect(windowModeButtonX + windowModeButtonWidth + 5.0f, windowModeY,
                                         windowModeButtonWidth, buttonHeight, mouse)) {
                    currentSettings_.windowMode = WindowMode::Fullscreen;
                    ApplySettings();
                    inputAPI->ConsumeLeftClick();
                    clicked = true;
                }
            }

            // FPS表示チェックボックス（即時適用）
            if (!clicked) {
                float fpsCheckboxY = windowModeY + buttonSpacingLocal;
                float checkboxSize = 30.0f;
                if (IsPointInRect(displaySectionX, fpsCheckboxY, checkboxSize + 200.0f, checkboxSize, mouse)) {
                    currentSettings_.showFPS = !currentSettings_.showFPS;
                    ApplySettings();
                    inputAPI->ConsumeLeftClick();
                    clicked = true;
                }
            }

            // カーソル表示チェックボックス（即時適用）
            if (!clicked) {
                float cursorCheckboxY = windowModeY + buttonSpacingLocal * 2.0f;
                float checkboxSize = 30.0f;
                if (IsPointInRect(displaySectionX, cursorCheckboxY, checkboxSize + 200.0f, checkboxSize, mouse)) {
                    currentSettings_.showCursor = !currentSettings_.showCursor;
                    ApplySettings();
                    inputAPI->ConsumeLeftClick();
                }
            }
        }
    }
}

void SettingsOverlay::ProcessSliderDrag(SharedContext& ctx) {
    InputSystemAPI* inputAPI = ctx.inputAPI;
    auto mouse = inputAPI ? inputAPI->GetMousePositionInternal() : Vec2{0.0f, 0.0f};
    const float windowX = 200.0f;
    const float windowY = 150.0f;
    float sectionY = windowY + 20.0f;
    float sectionWidth = (1520.0f - 60.0f) / 2.0f;
    float volumeSectionX = windowX + 20.0f;
    float sliderHeight = 40.0f;
    float sliderSpacing = 60.0f;
    float startY = sectionY + 40.0f;
    float labelWidth = 120.0f;
    float sliderBarHeight = 8.0f;
    // スライダー位置（RenderVolumeSection / RenderSlider と同一）
    auto getSliderRect = [&](int sliderId) -> std::pair<float, float> {
        float sliderX = volumeSectionX + labelWidth;
        float sliderWidth = (sectionWidth - 20.0f) - labelWidth - 100.0f;
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
