#include "GachaOverlay.hpp"
#include "GachaOverlayInternal.hpp"

// プロジェクト内
#include "../../../utils/Log.h"
#include "../../api/GameplayDataAPI.hpp"

namespace game {
namespace core {

using namespace gacha_internal;

GachaOverlay::GachaOverlay()
    : systemAPI_(nullptr), uiAPI_(nullptr), isInitialized_(false),
      requestClose_(false), hasTransitionRequest_(false),
      requestedNextState_(GameState::Title) {}

bool GachaOverlay::Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) {
    if (isInitialized_) {
        LOG_ERROR("GachaOverlay already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("GachaOverlay: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    uiAPI_ = uiAPI;
    requestClose_ = false;
    hasTransitionRequest_ = false;
    pendingRollCount_ = 0;
    rng_ = std::mt19937(std::random_device{}());

    // ホームスクリーン用のサイズ（ヘッダーとタブバーの間、左右に余白）
    const float marginLeft = 20.0f;
    const float marginRight = 20.0f;
    const float marginTop = HOME_HEADER_H;
    const float marginBottom = HOME_TAB_H;
    const float contentWidth = 1920.0f - marginLeft - marginRight;
    const float contentHeight = 1080.0f - marginTop - marginBottom;
    panelX_ = marginLeft;
    panelY_ = marginTop;
    panelW_ = contentWidth;
    panelH_ = contentHeight;

    contentLeft_ = GACHA_HEADER_PADDING_X;
    contentRight_ = panelW_ - GACHA_HEADER_PADDING_X;
    const float tabRowY = GACHA_HEADER_H + 18.0f;
    contentTop_ = tabRowY + TAB_BUTTON_H + GACHA_TAB_ROW_GAP;
    contentBottom_ = panelH_ - 128.0f;

    // ボタン（単発/10連）
    buttonW_ = 220.0f;
    buttonH_ = 70.0f;
    const float buttonSpacing = 24.0f;
    const float totalButtonWidth = buttonW_ * 2.0f + buttonSpacing;
    singleButtonX_ = (contentWidth - totalButtonWidth) / 2.0f;
    singleButtonY_ = contentHeight - 140.0f;
    tenButtonX_ = singleButtonX_ + buttonW_ + buttonSpacing;
    tenButtonY_ = contentHeight - 140.0f;

    isInitialized_ = true;
    introProgress_ = 0.0f;
    pulseTime_ = 0.0f;
    cardAnimationTimer_ = 0.0f;
    hoveredTabIndex_ = -1;
    hoveredSingleButton_ = false;
    hoveredTenButton_ = false;
    hoveredSkipButton_ = false;
    hoveredExchange1Button_ = false;
    hoveredExchange10Button_ = false;
    LOG_INFO("GachaOverlay initialized");
    return true;
}

void GachaOverlay::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    ClearResultCards();

    isInitialized_ = false;
    systemAPI_ = nullptr;
    uiAPI_ = nullptr;
    cachedGameplayDataAPI_ = nullptr;
    LOG_INFO("GachaOverlay shutdown");
}

} // namespace core
} // namespace game
