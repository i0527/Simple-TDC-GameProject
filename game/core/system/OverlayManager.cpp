#include "OverlayManager.hpp"
#include "GameSystem.hpp"
#include "../states/overlays/StageSelectOverlay.hpp"
#include "../states/overlays/FormationOverlay.hpp"
#include "../states/overlays/CharacterEnhancementOverlay.hpp"
#include "../states/overlays/CodexOverlay.hpp"
#include "../states/overlays/SettingsOverlay.hpp"
#include "../states/overlays/GachaOverlay.hpp"
#include "../states/overlays/LicenseOverlay.hpp"
#include "../states/overlays/BattleResultOverlay.hpp"
#include "../states/overlays/PauseOverlay.hpp"
#include "../states/overlays/CustomStageEnemyQueueOverlay.hpp"
#include "../../utils/Log.h"
#include "../api/BaseSystemAPI.hpp"

namespace game {
namespace core {

OverlayManager::OverlayManager() {
}

OverlayManager::~OverlayManager() {
    Shutdown();
}

bool OverlayManager::PushOverlay(OverlayState state, BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) {
    if (!systemAPI) {
        LOG_ERROR("OverlayManager: systemAPI is null");
        return false;
    }

    if (state == OverlayState::None) {
        LOG_WARN("OverlayManager: Cannot push None overlay");
        return false;
    }

    // 既に同じスチE�Eト�Eオーバ�Eレイが存在するかチェチE��
    if (IsOverlayActive(state)) {
        LOG_WARN("OverlayManager: Overlay {} is already active", static_cast<int>(state));
        return false;
    }

    auto overlay = CreateOverlay(state, systemAPI, uiAPI);
    if (!overlay) {
        LOG_ERROR("OverlayManager: Failed to create overlay {}", static_cast<int>(state));
        return false;
    }

    if (!overlay->Initialize(systemAPI, uiAPI)) {
        LOG_ERROR("OverlayManager: Failed to initialize overlay {}", static_cast<int>(state));
        return false;
    }

    stack_.push_back(std::move(overlay));
    LOG_INFO("OverlayManager: Pushed overlay {}", static_cast<int>(state));
    return true;
}

void OverlayManager::PopOverlay() {
    if (stack_.empty()) {
        LOG_WARN("OverlayManager: Cannot pop overlay, stack is empty");
        return;
    }

    auto& top = stack_.back();
    top->Shutdown();
    stack_.pop_back();
    LOG_INFO("OverlayManager: Popped overlay");
}

void OverlayManager::PopAllOverlays() {
    while (!stack_.empty()) {
        PopOverlay();
    }
    LOG_INFO("OverlayManager: Popped all overlays");
}

void OverlayManager::Update(SharedContext& ctx, float deltaTime) {
    if (stack_.empty()) {
        return;
    }

    // 最上層のオーバ�Eレイのみ更新
    auto& top = stack_.back();
    top->Update(ctx, deltaTime);

    // クローズリクエストを処琁E
    if (top->RequestClose()) {
        PopOverlay();
        return;
    }

    // P0: スチE�Eト�E移リクエストを処琁E���E部に保持�E�E
    GameState nextState;
    if (top->RequestTransition(nextState)) {
        requestedTransition_ = nextState;
        hasTransitionRequest_ = true;
        LOG_INFO("OverlayManager: Transition request to state {}", static_cast<int>(nextState));
    }

    if (top->RequestQuit()) {
        hasQuitRequest_ = true;
        LOG_INFO("OverlayManager: Quit requested from overlay");
    }
}

void OverlayManager::Render(SharedContext& ctx) {
    if (stack_.empty()) return;

    // P1: 背景半透�Eをここで一括描画
    // 背景シーンを見せたまま、�Eオーバ�Eレイを重ねる想宁E

    // 下から頁E��描画�E�奥 ↁE手前�E�E
    for (auto& overlay : stack_) {
        if (!overlay->IsImGuiOverlay()) {
            overlay->Render(ctx);
        }
    }
}

void OverlayManager::RenderImGui(SharedContext& ctx) {
    if (stack_.empty()) {
        return;
    }

    for (auto& overlay : stack_) {
        if (overlay->IsImGuiOverlay()) {
            overlay->Render(ctx);
        }
    }
}

void OverlayManager::Shutdown() {
    PopAllOverlays();
}

bool OverlayManager::IsEmpty() const {
    return stack_.empty();
}

size_t OverlayManager::Count() const {
    return stack_.size();
}

IOverlay* OverlayManager::GetTopOverlay() const {
    if (stack_.empty()) {
        return nullptr;
    }
    return stack_.back().get();
}

bool OverlayManager::IsOverlayActive(OverlayState state) const {
    for (const auto& overlay : stack_) {
        if (overlay->GetState() == state) {
            return true;
        }
    }
    return false;
}

std::unique_ptr<IOverlay> OverlayManager::CreateOverlay(OverlayState state, BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) {
    switch (state) {
    case OverlayState::Settings:
        return std::make_unique<SettingsOverlay>();
    
    case OverlayState::License:
        return std::make_unique<LicenseOverlay>();

    case OverlayState::BattleVictory:
        return std::make_unique<BattleResultOverlay>(true);

    case OverlayState::BattleDefeat:
        return std::make_unique<BattleResultOverlay>(false);

    case OverlayState::Pause:
        return std::make_unique<PauseOverlay>();
    
    case OverlayState::CustomStageEnemyQueue:
        return std::make_unique<CustomStageEnemyQueueOverlay>();
    
    case OverlayState::None:
    default:
        LOG_ERROR("OverlayManager: Invalid overlay state {}", static_cast<int>(state));
        return nullptr;
    }
}

} // namespace core
} // namespace game
