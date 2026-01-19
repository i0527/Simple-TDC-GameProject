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

    panel_ = std::make_shared<ui::Panel>();
    panel_->SetUISystemAPI(uiAPI_);
    panel_->SetId("gacha_panel");
    panel_->SetPosition(panelX_, panelY_); // 画面内の絶対座標
    panel_->SetSize(panelW_, panelH_);
    panel_->SetRoot(true); // ルートパネルとして設定
    panel_->Initialize();

    contentLeft_ = GACHA_HEADER_PADDING_X;
    contentRight_ = panelW_ - GACHA_HEADER_PADDING_X;
    const float tabRowY = GACHA_HEADER_H + 18.0f;
    contentTop_ = tabRowY + TAB_BUTTON_H + GACHA_TAB_ROW_GAP;
    contentBottom_ = panelH_ - 128.0f;

    tabDrawButton_ = std::make_shared<ui::Button>();
    tabDrawButton_->SetUISystemAPI(uiAPI_);
    tabDrawButton_->SetId("gacha_tab_draw");
    tabDrawButton_->SetPosition(contentLeft_, tabRowY);
    tabDrawButton_->SetSize(TAB_BUTTON_W, TAB_BUTTON_H);
    tabDrawButton_->SetLabel("引く");
    tabDrawButton_->Initialize();
    tabDrawButton_->SetOnClickCallback([this]() {
        currentTab_ = GachaTab::Draw;
        UpdateTabVisibility();
    });

    tabRatesButton_ = std::make_shared<ui::Button>();
    tabRatesButton_->SetUISystemAPI(uiAPI_);
    tabRatesButton_->SetId("gacha_tab_rates");
    tabRatesButton_->SetPosition(
        contentLeft_ + (TAB_BUTTON_W + TAB_BUTTON_SPACING) * 1, tabRowY);
    tabRatesButton_->SetSize(TAB_BUTTON_W, TAB_BUTTON_H);
    tabRatesButton_->SetLabel("提供割合");
    tabRatesButton_->Initialize();
    tabRatesButton_->SetOnClickCallback([this]() {
        currentTab_ = GachaTab::Rates;
        UpdateTabVisibility();
    });

    tabHistoryButton_ = std::make_shared<ui::Button>();
    tabHistoryButton_->SetUISystemAPI(uiAPI_);
    tabHistoryButton_->SetId("gacha_tab_history");
    tabHistoryButton_->SetPosition(
        contentLeft_ + (TAB_BUTTON_W + TAB_BUTTON_SPACING) * 2, tabRowY);
    tabHistoryButton_->SetSize(TAB_BUTTON_W, TAB_BUTTON_H);
    tabHistoryButton_->SetLabel("履歴");
    tabHistoryButton_->Initialize();
    tabHistoryButton_->SetOnClickCallback([this]() {
        currentTab_ = GachaTab::History;
        UpdateTabVisibility();
    });

    tabExchangeButton_ = std::make_shared<ui::Button>();
    tabExchangeButton_->SetUISystemAPI(uiAPI_);
    tabExchangeButton_->SetId("gacha_tab_exchange");
    tabExchangeButton_->SetPosition(
        contentLeft_ + (TAB_BUTTON_W + TAB_BUTTON_SPACING) * 3, tabRowY);
    tabExchangeButton_->SetSize(TAB_BUTTON_W, TAB_BUTTON_H);
    tabExchangeButton_->SetLabel("交換");
    tabExchangeButton_->Initialize();
    tabExchangeButton_->SetOnClickCallback([this]() {
        currentTab_ = GachaTab::Exchange;
        UpdateTabVisibility();
    });

    // ボタン（単発/10連）
    buttonW_ = 200.0f;
    buttonH_ = 60.0f;
    singleGachaButton_ = std::make_shared<ui::Button>();
    singleGachaButton_->SetUISystemAPI(uiAPI_);
    singleGachaButton_->SetId("gacha_button_single");
    singleButtonX_ = contentWidth / 2.0f - 220.0f;
    singleButtonY_ = contentHeight - 120.0f;
    singleGachaButton_->SetPosition(singleButtonX_, singleButtonY_);
    singleGachaButton_->SetSize(buttonW_, buttonH_);
    singleGachaButton_->SetLabel("単発 (1)");
    singleGachaButton_->Initialize();
    singleGachaButton_->SetOnClickCallback([this]() { pendingRollCount_ = 1; });

    tenGachaButton_ = std::make_shared<ui::Button>();
    tenGachaButton_->SetUISystemAPI(uiAPI_);
    tenGachaButton_->SetId("gacha_button_ten");
    tenButtonX_ = contentWidth / 2.0f + 20.0f;
    tenButtonY_ = contentHeight - 120.0f;
    tenGachaButton_->SetPosition(tenButtonX_, tenButtonY_);
    tenGachaButton_->SetSize(buttonW_, buttonH_);
    tenGachaButton_->SetLabel("10連 (9)");
    tenGachaButton_->Initialize();
    tenGachaButton_->SetOnClickCallback([this]() { pendingRollCount_ = 10; });

    closeButton_ = std::make_shared<ui::Button>();
    closeButton_->SetUISystemAPI(uiAPI_);
    closeButton_->SetId("close_button");
    closeButton_->SetPosition(contentWidth - 170.0f, tabRowY); // パネル内の相対座標
    closeButton_->SetSize(150.0f, 40.0f);
    closeButton_->SetLabel("閉じる");
    closeButton_->Initialize();

    closeButton_->SetOnClickCallback([this]() { requestClose_ = true; });

    panel_->AddChild(singleGachaButton_);
    panel_->AddChild(tenGachaButton_);
    panel_->AddChild(closeButton_);

    skipRevealButton_ = std::make_shared<ui::Button>();
    skipRevealButton_->SetUISystemAPI(uiAPI_);
    skipRevealButton_->SetId("gacha_skip_reveal");
    skipRevealButton_->SetPosition(panelW_ - 200.0f, panelH_ - 160.0f);
    skipRevealButton_->SetSize(160.0f, 46.0f);
    skipRevealButton_->SetLabel("SKIP");
    skipRevealButton_->Initialize();
    skipRevealButton_->SetVisible(false);
    skipRevealButton_->SetOnClickCallback([this]() { skipRevealRequested_ = true; });
    panel_->AddChild(skipRevealButton_);
    panel_->AddChild(tabDrawButton_);
    panel_->AddChild(tabRatesButton_);
    panel_->AddChild(tabHistoryButton_);
    panel_->AddChild(tabExchangeButton_);

    poolList_ = std::make_shared<ui::List>();
    poolList_->SetUISystemAPI(uiAPI_);
    poolList_->SetId("gacha_pool_list");
    poolList_->SetPosition(contentLeft_, contentTop_);
    poolList_->SetSize(contentRight_ - contentLeft_, contentBottom_ - contentTop_);
    poolList_->SetItemsPerPage(HISTORY_DISPLAY_LIMIT);
    poolList_->Initialize();
    panel_->AddChild(poolList_);

    historyList_ = std::make_shared<ui::List>();
    historyList_->SetUISystemAPI(uiAPI_);
    historyList_->SetId("gacha_history_list");
    historyList_->SetPosition(contentLeft_, contentTop_);
    historyList_->SetSize(contentRight_ - contentLeft_, contentBottom_ - contentTop_);
    historyList_->SetItemsPerPage(HISTORY_DISPLAY_LIMIT);
    historyList_->Initialize();
    panel_->AddChild(historyList_);

    exchangeTicketButton_ = std::make_shared<ui::Button>();
    exchangeTicketButton_->SetUISystemAPI(uiAPI_);
    exchangeTicketButton_->SetId("gacha_exchange_ticket");
    exchangeTicketButton_->SetPosition(panelW_ / 2.0f - 220.0f, contentTop_ + 40.0f);
    exchangeTicketButton_->SetSize(200.0f, 56.0f);
    exchangeTicketButton_->SetLabel("チケット x1");
    exchangeTicketButton_->Initialize();
    exchangeTicketButton_->SetOnClickCallback([this]() {
        if (!cachedGameplayDataAPI_) {
            return;
        }
        if (cachedGameplayDataAPI_->GetGachaDust() < DUST_FOR_TICKET) {
            ShowMessageCard(panelW_, panelH_, "交換不可", "ダストが不足しています。");
            return;
        }
        cachedGameplayDataAPI_->AddGachaDust(-DUST_FOR_TICKET);
        cachedGameplayDataAPI_->AddTickets(1);
        cachedGameplayDataAPI_->Save();
        ShowMessageCard(panelW_, panelH_, "交換完了", "チケット x1 を交換しました。");
    });
    panel_->AddChild(exchangeTicketButton_);

    exchangeTicketTenButton_ = std::make_shared<ui::Button>();
    exchangeTicketTenButton_->SetUISystemAPI(uiAPI_);
    exchangeTicketTenButton_->SetId("gacha_exchange_ticket_ten");
    exchangeTicketTenButton_->SetPosition(panelW_ / 2.0f + 20.0f, contentTop_ + 40.0f);
    exchangeTicketTenButton_->SetSize(200.0f, 56.0f);
    exchangeTicketTenButton_->SetLabel("チケット x10");
    exchangeTicketTenButton_->Initialize();
    exchangeTicketTenButton_->SetOnClickCallback([this]() {
        if (!cachedGameplayDataAPI_) {
            return;
        }
        if (cachedGameplayDataAPI_->GetGachaDust() < DUST_FOR_TEN_TICKETS) {
            ShowMessageCard(panelW_, panelH_, "交換不可", "ダストが不足しています。");
            return;
        }
        cachedGameplayDataAPI_->AddGachaDust(-DUST_FOR_TEN_TICKETS);
        cachedGameplayDataAPI_->AddTickets(10);
        cachedGameplayDataAPI_->Save();
        ShowMessageCard(panelW_, panelH_, "交換完了", "チケット x10 を交換しました。");
    });
    panel_->AddChild(exchangeTicketTenButton_);

    isInitialized_ = true;
    introProgress_ = 0.0f;
    pulseTime_ = 0.0f;
    UpdateTabVisibility();
    LOG_INFO("GachaOverlay initialized");
    return true;
}

void GachaOverlay::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    ClearResultCards();

    if (panel_) {
        panel_->Shutdown();
        panel_.reset();
    }

    if (singleGachaButton_) {
        singleGachaButton_->Shutdown();
        singleGachaButton_.reset();
    }

    if (tenGachaButton_) {
        tenGachaButton_->Shutdown();
        tenGachaButton_.reset();
    }

    if (skipRevealButton_) {
        skipRevealButton_->Shutdown();
        skipRevealButton_.reset();
    }

    if (closeButton_) {
        closeButton_->Shutdown();
        closeButton_.reset();
    }

    if (tabDrawButton_) {
        tabDrawButton_->Shutdown();
        tabDrawButton_.reset();
    }

    if (tabRatesButton_) {
        tabRatesButton_->Shutdown();
        tabRatesButton_.reset();
    }

    if (tabHistoryButton_) {
        tabHistoryButton_->Shutdown();
        tabHistoryButton_.reset();
    }

    if (tabExchangeButton_) {
        tabExchangeButton_->Shutdown();
        tabExchangeButton_.reset();
    }

    if (poolList_) {
        poolList_->Shutdown();
        poolList_.reset();
    }

    if (historyList_) {
        historyList_->Shutdown();
        historyList_.reset();
    }

    if (exchangeTicketButton_) {
        exchangeTicketButton_->Shutdown();
        exchangeTicketButton_.reset();
    }

    if (exchangeTicketTenButton_) {
        exchangeTicketTenButton_->Shutdown();
        exchangeTicketTenButton_.reset();
    }

    isInitialized_ = false;
    systemAPI_ = nullptr;
    uiAPI_ = nullptr;
    cachedGameplayDataAPI_ = nullptr;
    LOG_INFO("GachaOverlay shutdown");
}

} // namespace core
} // namespace game
