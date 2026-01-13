#include "EnhancementOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include <imgui.h>

namespace game {
namespace core {

EnhancementOverlay::EnhancementOverlay()
    : systemAPI_(nullptr)
    , isInitialized_(false)
    , requestClose_(false)
    , hasTransitionRequest_(false)
    , requestedNextState_(GameState::Title)
{
}

bool EnhancementOverlay::Initialize(BaseSystemAPI* systemAPI) {
    if (isInitialized_) {
        LOG_ERROR("EnhancementOverlay already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("EnhancementOverlay: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    requestClose_ = false;
    hasTransitionRequest_ = false;

    // UIコンポーネントの初期化
    // ホームスクリーン用のサイズ（ヘッダーとタブバーの間、左右に余白）
    const float marginLeft = 20.0f;
    const float marginRight = 20.0f;
    const float marginTop = 90.0f;
    const float marginBottom = 90.0f;
    const float contentWidth = 1920.0f - marginLeft - marginRight;
    const float contentHeight = 1080.0f - marginTop - marginBottom;
    
    panel_ = std::make_shared<ui::Panel>();
    panel_->SetId("enhancement_panel");
    panel_->SetPosition(marginLeft, marginTop);  // 画面内の絶対座標
    panel_->SetSize(contentWidth, contentHeight);
    panel_->SetRoot(true);  // ルートパネルとして設定
    panel_->Initialize();

    enhancementList_ = std::make_shared<ui::List>();
    enhancementList_->SetId("enhancement_list");
    enhancementList_->SetPosition(20.0f, 20.0f);  // パネル内の相対座標
    enhancementList_->SetSize(500.0f, contentHeight - 40.0f);
    enhancementList_->SetItemHeight(50.0f);
    enhancementList_->Initialize();

    // サンプル強化データを追加
    ui::ListItem item1;
    item1.id = "enhancement_attack";
    item1.label = "Attack";
    item1.value = "Lv.5";
    item1.enabled = true;
    enhancementList_->AddItem(item1);

    ui::ListItem item2;
    item2.id = "enhancement_defense";
    item2.label = "Defense";
    item2.value = "Lv.3";
    item2.enabled = true;
    enhancementList_->AddItem(item2);

    closeButton_ = std::make_shared<ui::Button>();
    closeButton_->SetId("close_button");
    closeButton_->SetPosition(contentWidth - 170.0f, 20.0f);  // パネル内の相対座標
    closeButton_->SetSize(150.0f, 40.0f);
    closeButton_->SetLabel("Close");
    closeButton_->Initialize();

    closeButton_->SetOnClickCallback([this]() {
        requestClose_ = true;
    });

    panel_->AddChild(enhancementList_);
    panel_->AddChild(closeButton_);

    isInitialized_ = true;
    LOG_INFO("EnhancementOverlay initialized");
    return true;
}

void EnhancementOverlay::Update(SharedContext& ctx, float deltaTime) {
    if (!isInitialized_) {
        return;
    }

    if (panel_) {
        panel_->Update(deltaTime);
    }

    if (systemAPI_->IsKeyPressed(256)) {
        requestClose_ = true;
    }
}

void EnhancementOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) {
        return;
    }

    if (panel_) {
        panel_->Render();
    }
}

void EnhancementOverlay::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    if (panel_) {
        panel_->Shutdown();
        panel_.reset();
    }

    if (enhancementList_) {
        enhancementList_->Shutdown();
        enhancementList_.reset();
    }

    if (closeButton_) {
        closeButton_->Shutdown();
        closeButton_.reset();
    }

    isInitialized_ = false;
    systemAPI_ = nullptr;
    LOG_INFO("EnhancementOverlay shutdown");
}

bool EnhancementOverlay::RequestClose() const {
    if (requestClose_) {
        requestClose_ = false;
        return true;
    }
    return false;
}

bool EnhancementOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}

} // namespace core
} // namespace game
