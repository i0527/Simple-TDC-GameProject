#include "HomeScreen.hpp"
#include "overlays/home/TabBarManager.hpp"
#include "overlays/home/ResourceHeader.hpp"
#include "overlays/home/ContentContainer.hpp"
#include "../entities/CharacterManager.hpp"
#include "../ui/OverlayColors.hpp"
#include "../../utils/Log.h"
#include <raylib.h>
#include <imgui.h>

namespace game {
namespace core {
namespace states {

HomeScreen::HomeScreen()
    : systemAPI_(nullptr)
    , sharedContext_(nullptr)
    , request_transition_(false)
    , next_state_(GameState::Home)
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
    
    // キャラクターマネージャー初期化
    characterManager_ = std::make_unique<entities::CharacterManager>();
    if (!characterManager_->Initialize("data/characters.json")) {
        LOG_WARN("HomeScreen: CharacterManager initialization failed, continuing with hardcoded data");
    }
    
    // ヘッダー初期化
    header_ = std::make_unique<overlays::home::ResourceHeader>();
    if (!header_->Initialize()) {
        LOG_ERROR("HomeScreen: Failed to initialize ResourceHeader");
        return false;
    }
    
    // 初期リソース設定
    overlays::home::PlayerResources initial_resources;
    initial_resources.gold = 1234;
    initial_resources.gems = 567;
    initial_resources.tickets = 45;
    initial_resources.max_tickets = 100;
    header_->SetResources(initial_resources);
    
    // タブバー初期化
    tabbar_ = std::make_unique<overlays::home::TabBarManager>();
    if (!tabbar_->Initialize()) {
        LOG_ERROR("HomeScreen: Failed to initialize TabBarManager");
        return false;
    }
    tabbar_->SetOnTabChanged([this](auto tab) { OnTabChanged(tab); });
    
    // コンテンツコンテナ初期化
    content_ = std::make_unique<overlays::home::ContentContainer>();
    if (!content_->Initialize(systemAPI, characterManager_.get())) {
        LOG_ERROR("HomeScreen: Failed to initialize ContentContainer");
        return false;
    }
    
    LOG_INFO("HomeScreen initialized");
    return true;
}

void HomeScreen::Update(float deltaTime) {
    if (!systemAPI_ || !sharedContext_) {
        return;
    }
    
    // マウスイベント処理
    HandleMouseInput();
    
    // ヘッダー更新
    if (header_) {
        header_->Update(deltaTime);
    }
    
    // タブバー更新
    if (tabbar_) {
        tabbar_->Update(deltaTime);
    }
    
    // コンテンツ更新
    if (content_ && sharedContext_) {
        content_->Update(deltaTime, *sharedContext_);
        
        // ContentContainer内のオーバーレイからの遷移リクエストをチェック
        auto* overlay = content_->GetCurrentOverlay();
        if (overlay) {
            GameState nextState;
            if (overlay->RequestTransition(nextState)) {
                request_transition_ = true;
                next_state_ = nextState;
                LOG_INFO("HomeScreen: Transition request from overlay to state {}", static_cast<int>(nextState));
            }
        }
    }
}

void HomeScreen::Render() {
    if (!systemAPI_ || !sharedContext_) {
        return;
    }
    
    // 背景を描画（Tokyo Night風ダークテーマ）
    systemAPI_->DrawRectangle(0, 0, 1920, 1080, ui::OverlayColors::MAIN_BG);
    
    // ヘッダー描画 (y: 0-90)
    if (header_) {
        header_->Render(systemAPI_);
    }
    
    // コンテンツ描画 (y: 90-990)
    if (content_ && sharedContext_) {
        content_->Render(*sharedContext_);
        
        // オーバーレイの描画（現在のタブに関連付けられたもの）
        auto* overlay = content_->GetCurrentOverlay();
        if (overlay && !overlay->IsImGuiOverlay()) {
            overlay->Render(*sharedContext_);
        }
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
    if (!systemAPI_ || !tabbar_) {
        return;
    }
    
    Vector2 mousePos = systemAPI_->GetMousePosition();
    
    // タブバーのマウスホバー
    tabbar_->OnMouseHover(mousePos.x, mousePos.y);
    
    // タブバーのマウスクリック
    if (systemAPI_->IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (tabbar_->OnMouseClick(mousePos.x, mousePos.y)) {
            systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
        }
    }
}

void HomeScreen::RenderImGui() {
    // ImGuiフレーム内での描画が必要な要素があればここに追加
    if (content_ && sharedContext_) {
        auto* overlay = content_->GetCurrentOverlay();
        if (overlay && overlay->IsImGuiOverlay()) {
            overlay->Render(*sharedContext_);
        }
    }
}

bool HomeScreen::RequestTransition(GameState& nextState) {
    if (request_transition_) {
        nextState = next_state_;
        request_transition_ = false;  // リクエストをリセット
        return true;
    }
    return false;
}

void HomeScreen::Shutdown() {
    if (content_) {
        content_->Shutdown();
        content_.reset();
    }
    
    if (characterManager_) {
        characterManager_->Shutdown();
        characterManager_.reset();
    }
    
    LOG_INFO("HomeScreen shutdown");
}

} // namespace states
} // namespace core
} // namespace game
