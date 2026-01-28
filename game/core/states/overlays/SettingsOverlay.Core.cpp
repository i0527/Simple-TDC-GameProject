#include "SettingsOverlay.hpp"

// プロジェクト内
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"

namespace game {
namespace core {

SettingsOverlay::SettingsOverlay()
    : systemAPI_(nullptr)
    , audioAPI_(nullptr)
    , isInitialized_(false)
    , requestClose_(false)
    , hasTransitionRequest_(false)
    , requestedNextState_(GameState::Title)
    , isDraggingSlider_(false)
    , draggedSliderId_(-1)
    , settingsFilePath_("data/settings.json")
    , resetButtonHovered_(false)
    , closeButtonHovered_(false)
    , titleButtonHovered_(false)
    , quitButtonHovered_(false)
    , fullscreenButtonHovered_(false)
    , fpsCheckboxHovered_(false)
    , requestQuit_(false) {
    // デフォルト設定
    currentSettings_ = SettingsData();
    savedSettings_ = SettingsData();
}

bool SettingsOverlay::Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) {
    if (isInitialized_) {
        LOG_ERROR("SettingsOverlay already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("SettingsOverlay: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    audioAPI_ = nullptr;
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

void SettingsOverlay::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    isInitialized_ = false;
    systemAPI_ = nullptr;
    audioAPI_ = nullptr;
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

bool SettingsOverlay::RequestQuit() const {
    if (requestQuit_) {
        requestQuit_ = false;
        return true;
    }
    return false;
}

} // namespace core
} // namespace game
