#include "HomeScreen.hpp"
#include "overlays/home/TabBarManager.hpp"
#include "overlays/home/ResourceHeader.hpp"
#include "overlays/home/TabContent.hpp"
#include "../api/GameplayDataAPI.hpp"
#include "../api/InputSystemAPI.hpp"
#include "../ui/OverlayColors.hpp"
#include "../ui/UiAssetKeys.hpp"
#include "../../utils/Log.h"
#include <imgui.h>

namespace game {
namespace core {
namespace states {

HomeScreen::HomeScreen()
    : systemAPI_(nullptr)
    , inputAPI_(nullptr)
    , sharedContext_(nullptr)
    , request_transition_(false)
    , next_state_(GameState::Home)
    , request_quit_(false)
{
}

HomeScreen::~HomeScreen() {
    Shutdown();
}

bool HomeScreen::Initialize(BaseSystemAPI* systemAPI) {
    if (!systemAPI) {
        LOG_ERROR("HomeScreen: systemAPI is null");
        return false;
    }
    
    systemAPI_ = systemAPI;
    
    // ヘッダー初期匁E
    header_ = std::make_unique<overlays::home::ResourceHeader>();
    if (!header_->Initialize()) {
        LOG_ERROR("HomeScreen: Failed to initialize ResourceHeader");
        return false;
    }

    // 初期リソース設定（実値はUpdateでSharedContextから同期�E�E
    overlays::home::PlayerResources initial_resources{};
    initial_resources.gold = 0;
    initial_resources.gems = 0;
    initial_resources.tickets = 0;
    initial_resources.max_tickets = 0;
    header_->SetResources(initial_resources);
    
    // タブバー初期匁E
    tabbar_ = std::make_unique<overlays::home::TabBarManager>();
    if (!tabbar_->Initialize()) {
        LOG_ERROR("HomeScreen: Failed to initialize TabBarManager");
        return false;
    }
    tabbar_->SetOnTabChanged([this](auto tab) { OnTabChanged(tab); });
    
    // タブコンチE��チE�E期化
    content_ = std::make_unique<overlays::home::TabContent>();
    UISystemAPI* uiAPI = sharedContext_ ? sharedContext_->uiAPI : nullptr;
    if (!content_->Initialize(systemAPI, uiAPI)) {
        LOG_ERROR("HomeScreen: Failed to initialize TabContent");
        return false;
    }
    
    LOG_INFO("HomeScreen initialized");
    return true;
}

void HomeScreen::Update(float deltaTime) {
    if (!systemAPI_ || !sharedContext_) {
        return;
    }

    // ヘッダーの通貨表示をPlayerDataManagerの永続データに同期する
    if (header_ && sharedContext_->gameplayDataAPI) {
        const auto& save = sharedContext_->gameplayDataAPI->GetSaveData();
        overlays::home::PlayerResources r{};
        r.gold = save.gold;
        // ガチャはチケチE��運用。gem表示は当面 0 固定（表示枠はそ�Eまま�E�E
        r.gems = 0;
        r.tickets = save.tickets;
        r.max_tickets = save.maxTickets;
        header_->SetResources(r);
    }
    
    // マウスイベント�E琁E
    HandleMouseInput();
    
    // ヘッダー更新
    if (header_) {
        header_->Update(deltaTime);
    }
    
    // タブバー更新
    if (tabbar_) {
        tabbar_->Update(deltaTime);
    }
    
    // コンチE��チE��新
    if (content_ && sharedContext_) {
        content_->Update(deltaTime, *sharedContext_);

        GameState nextState;
        if (content_->RequestTransition(nextState)) {
            request_transition_ = true;
            next_state_ = nextState;
            LOG_INFO("HomeScreen: Transition request from tab content to state {}", static_cast<int>(nextState));
        }
        if (content_->RequestQuit()) {
            request_quit_ = true;
            LOG_INFO("HomeScreen: Quit request from tab content");
        }
    }
}

void HomeScreen::Render() {
    if (!systemAPI_ || !sharedContext_) {
        return;
    }
    
    // 背景を描画�E�Eokyo Night風ダークチE�Eマ！E
    systemAPI_->Render().DrawRectangle(0, 0, 1920, 1080,
                                       ui::OverlayColors::MAIN_BG);

    // コンチE��チE��画はRenderOverlayで行う
}

void HomeScreen::RenderOverlay() {
    if (!sharedContext_) {
        return;
    }

    if (content_ && sharedContext_) {
        content_->Render(*sharedContext_);
    }
}

void HomeScreen::RenderHUD() {
    if (!systemAPI_) {
        return;
    }

    // ヘッダー描画 (y: 0-90)
    if (header_) {
        header_->Render(systemAPI_);
    }

    // タブバー描画 (y: 990-1080)
    if (tabbar_) {
        tabbar_->Render(systemAPI_);
    }
}

void HomeScreen::OnTabChanged(overlays::home::HomeTab tab) {
    if (content_) {
        content_->SwitchTab(tab);
        LOG_INFO("HomeScreen: Tab changed to: {}", static_cast<int>(tab));
    }
}

void HomeScreen::HandleMouseInput() {
    if (!inputAPI_ || !tabbar_) {
        return;
    }
    
    auto mousePos = inputAPI_->GetMousePosition();
    
    // タブバーのマウスホバー
    tabbar_->OnMouseHover(mousePos.x, mousePos.y);
    
    // タブバーのマウスクリチE��
    if (inputAPI_->IsLeftClickPressed()) {
        if (tabbar_->OnMouseClick(mousePos.x, mousePos.y)) {
            inputAPI_->ConsumeLeftClick();
        }
    }
}

void HomeScreen::RenderImGui() {
    // ImGuiフレーム冁E��の描画が忁E��な要素があれ�Eここに追加
    if (content_ && sharedContext_) {
        content_->RenderImGui(*sharedContext_);
    }
}

bool HomeScreen::RequestTransition(GameState& nextState) {
    if (request_transition_) {
        nextState = next_state_;
        request_transition_ = false;  // リクエストをリセチE��
        return true;
    }
    return false;
}

bool HomeScreen::RequestQuit() {
    if (request_quit_) {
        request_quit_ = false;
        return true;
    }
    return false;
}

void HomeScreen::Shutdown() {
    if (content_) {
        content_->Shutdown();
        content_.reset();
    }
    
    
    LOG_INFO("HomeScreen shutdown");
}

} // namespace states
} // namespace core
} // namespace game
