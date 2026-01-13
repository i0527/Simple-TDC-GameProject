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
#include "../../utils/Log.h"
#include "../api/BaseSystemAPI.hpp"

namespace game {
namespace core {

OverlayManager::OverlayManager() {
}

OverlayManager::~OverlayManager() {
    Shutdown();
}

bool OverlayManager::PushOverlay(OverlayState state, BaseSystemAPI* systemAPI) {
    if (!systemAPI) {
        LOG_ERROR("OverlayManager: systemAPI is null");
        return false;
    }

    if (state == OverlayState::None) {
        LOG_WARN("OverlayManager: Cannot push None overlay");
        return false;
    }

    // 既に同じステートのオーバーレイが存在するかチェック
    if (IsOverlayActive(state)) {
        LOG_WARN("OverlayManager: Overlay {} is already active", static_cast<int>(state));
        return false;
    }

    auto overlay = CreateOverlay(state, systemAPI);
    if (!overlay) {
        LOG_ERROR("OverlayManager: Failed to create overlay {}", static_cast<int>(state));
        return false;
    }

    if (!overlay->Initialize(systemAPI)) {
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

    // 最上層のオーバーレイのみ更新
    auto& top = stack_.back();
    top->Update(ctx, deltaTime);

    // クローズリクエストを処理
    if (top->RequestClose()) {
        PopOverlay();
        return;
    }

    // P0: ステート遷移リクエストを処理（内部に保持）
    GameState nextState;
    if (top->RequestTransition(nextState)) {
        requestedTransition_ = nextState;
        hasTransitionRequest_ = true;
        LOG_INFO("OverlayManager: Transition request to state {}", static_cast<int>(nextState));
    }
}

void OverlayManager::Render(SharedContext& ctx) {
    if (stack_.empty()) return;

    // P1: 背景半透明をここで一括描画
    // 背景シーンを見せたまま、全オーバーレイを重ねる想定
    ctx.systemAPI->DrawRectangle(
        0, 0,
        ctx.systemAPI->GetInternalWidth(),
        ctx.systemAPI->GetInternalHeight(),
        Color{0, 0, 0, 100}
    );

    // 下から順に描画（奥 → 手前）
    for (auto& overlay : stack_) {
        overlay->Render(ctx);
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

std::unique_ptr<IOverlay> OverlayManager::CreateOverlay(OverlayState state, BaseSystemAPI* systemAPI) {
    switch (state) {
    case OverlayState::StageSelect:
        return std::make_unique<StageSelectOverlay>();
    
    case OverlayState::Formation:
        return std::make_unique<FormationOverlay>();
    
    case OverlayState::Enhancement:
        return std::make_unique<CharacterEnhancementOverlay>();
    
    case OverlayState::Codex:
        return std::make_unique<CodexOverlay>();
    
    case OverlayState::Settings:
        return std::make_unique<SettingsOverlay>();
    
    case OverlayState::Gacha:
        return std::make_unique<GachaOverlay>();
    
    case OverlayState::License:
        return std::make_unique<LicenseOverlay>();

    case OverlayState::BattleVictory:
        return std::make_unique<BattleResultOverlay>(true);

    case OverlayState::BattleDefeat:
        return std::make_unique<BattleResultOverlay>(false);
    
    case OverlayState::None:
    default:
        LOG_ERROR("OverlayManager: Invalid overlay state {}", static_cast<int>(state));
        return nullptr;
    }
}

} // namespace core
} // namespace game
