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
        currentSettings_.selectedMonitor = data.value("selectedMonitor", 0);
        currentSettings_.showFPS = data.value("showFPS", false);
        currentSettings_.resolution = data.value("resolution", "FHD");
        
        // 解像度の妥当性チェック
        if (currentSettings_.resolution != "FHD" && 
            currentSettings_.resolution != "HD" && 
            currentSettings_.resolution != "SD") {
            currentSettings_.resolution = "FHD";
        }

        // 値をクランプ
        currentSettings_.masterVolume = std::max(0.0f, std::min(1.0f, currentSettings_.masterVolume));
        currentSettings_.bgmVolume = std::max(0.0f, std::min(1.0f, currentSettings_.bgmVolume));
        currentSettings_.seVolume = std::max(0.0f, std::min(1.0f, currentSettings_.seVolume));
        
        // モニター番号の妥当性チェック（systemAPI_が利用可能な場合のみ）
        if (systemAPI_) {
            int monitorCount = systemAPI_->Window().GetMonitorCount();
            if (monitorCount > 0 && (currentSettings_.selectedMonitor < 0 || currentSettings_.selectedMonitor >= monitorCount)) {
                currentSettings_.selectedMonitor = 0;
            }
        } else if (currentSettings_.selectedMonitor < 0) {
            currentSettings_.selectedMonitor = 0;
        }

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
        data["selectedMonitor"] = currentSettings_.selectedMonitor;
        data["showFPS"] = currentSettings_.showFPS;
        data["resolution"] = currentSettings_.resolution;

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
           currentSettings_.selectedMonitor != savedSettings_.selectedMonitor ||
           currentSettings_.showFPS != savedSettings_.showFPS ||
           currentSettings_.resolution != savedSettings_.resolution;
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
        if (currentSettings_.isFullscreen) {
            // モニター番号の妥当性チェック
            int monitorCount = systemAPI_->Window().GetMonitorCount();
            int monitor = currentSettings_.selectedMonitor;
            if (monitor < 0 || monitor >= monitorCount) {
                monitor = 0;
                currentSettings_.selectedMonitor = 0;
            }
            systemAPI_->Window().SetFullscreen(true, monitor);
        } else {
            systemAPI_->Window().SetFullscreen(false);
        }
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
        // フルスクリーンボタンとモニター選択、FPSチェックボックス
        else {
            float sectionY = windowY + 100.0f;
            float sectionWidth = (windowWidth - 60.0f) / 2.0f;
            float displaySectionX = windowX + sectionWidth + 40.0f;
            float buttonSpacingLocal = 50.0f;
            float startY = sectionY + 40.0f;
            float fullscreenButtonX = displaySectionX;
            float fullscreenButtonY = startY;
            float fullscreenButtonWidth = sectionWidth - 20.0f;

            bool clicked = false;

            // フルスクリーンボタン
            if (IsPointInRect(fullscreenButtonX, fullscreenButtonY, fullscreenButtonWidth, buttonHeight, mouse)) {
                currentSettings_.isFullscreen = !currentSettings_.isFullscreen;
                // フルスクリーンにした場合、モニター番号を現在のモニターに設定
                if (currentSettings_.isFullscreen && systemAPI_) {
                    currentSettings_.selectedMonitor = systemAPI_->Window().GetCurrentMonitor();
                }
                inputAPI->ConsumeLeftClick();
                clicked = true;
            }
            
            // モニター選択ボタンの処理（フルスクリーン時のみ）
            if (!clicked && currentSettings_.isFullscreen && systemAPI_) {
                int monitorCount = systemAPI_->Window().GetMonitorCount();
                if (monitorCount > 1) {
                    float monitorY = startY + buttonSpacingLocal;
                    float monitorButtonWidth = 40.0f;
                    float monitorTextWidth = fullscreenButtonWidth - monitorButtonWidth * 2.0f - 20.0f;
                    float monitorPrevX = fullscreenButtonX;
                    float monitorNextX = fullscreenButtonX + monitorButtonWidth + 10.0f + monitorTextWidth + 10.0f;
                    
                    // 前のモニターボタン
                    if (IsPointInRect(monitorPrevX, monitorY, monitorButtonWidth, buttonHeight, mouse)) {
                        currentSettings_.selectedMonitor = (currentSettings_.selectedMonitor - 1 + monitorCount) % monitorCount;
                        inputAPI->ConsumeLeftClick();
                        clicked = true;
                    }
                    // 次のモニターボタン
                    else if (IsPointInRect(monitorNextX, monitorY, monitorButtonWidth, buttonHeight, mouse)) {
                        currentSettings_.selectedMonitor = (currentSettings_.selectedMonitor + 1) % monitorCount;
                        inputAPI->ConsumeLeftClick();
                        clicked = true;
                    }
                }
            }
            
            // FPS表示チェックボックス
            if (!clicked) {
                float checkboxY = currentSettings_.isFullscreen && systemAPI_ && systemAPI_->Window().GetMonitorCount() > 1
                    ? startY + buttonSpacingLocal * 2.0f
                    : startY + buttonSpacingLocal;
                float checkboxSize = 30.0f;
                if (IsPointInRect(fullscreenButtonX, checkboxY, checkboxSize + 200.0f, checkboxSize, mouse)) {
                    currentSettings_.showFPS = !currentSettings_.showFPS;
                    inputAPI->ConsumeLeftClick();
                    clicked = true;
                }
            }
            
            // 解像度選択ボタンの処理
            if (!clicked) {
                float resolutionY;
                if (currentSettings_.isFullscreen && systemAPI_ && systemAPI_->Window().GetMonitorCount() > 1) {
                    resolutionY = startY + buttonSpacingLocal * 3.0f;
                } else if (currentSettings_.isFullscreen) {
                    resolutionY = startY + buttonSpacingLocal * 2.0f;
                } else {
                    resolutionY = startY + buttonSpacingLocal * 2.0f;
                }
                
                float resolutionButtonWidth = 40.0f;
                float resolutionTextWidth = fullscreenButtonWidth - resolutionButtonWidth * 2.0f - 20.0f;
                float resolutionPrevX = fullscreenButtonX;
                float resolutionNextX = fullscreenButtonX + resolutionButtonWidth + 10.0f + resolutionTextWidth + 10.0f;
                
                // 前の解像度ボタン
                if (IsPointInRect(resolutionPrevX, resolutionY, resolutionButtonWidth, buttonHeight, mouse)) {
                    if (currentSettings_.resolution == "FHD") {
                        currentSettings_.resolution = "SD";
                    } else if (currentSettings_.resolution == "HD") {
                        currentSettings_.resolution = "FHD";
                    } else { // SD
                        currentSettings_.resolution = "HD";
                    }
                    inputAPI->ConsumeLeftClick();
                    clicked = true;
                }
                // 次の解像度ボタン
                else if (IsPointInRect(resolutionNextX, resolutionY, resolutionButtonWidth, buttonHeight, mouse)) {
                    if (currentSettings_.resolution == "FHD") {
                        currentSettings_.resolution = "HD";
                    } else if (currentSettings_.resolution == "HD") {
                        currentSettings_.resolution = "SD";
                    } else { // SD
                        currentSettings_.resolution = "FHD";
                    }
                    inputAPI->ConsumeLeftClick();
                    clicked = true;
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
