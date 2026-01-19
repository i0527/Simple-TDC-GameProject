#pragma once

#include "IOverlay.hpp"

namespace game {
namespace core {

/// @brief 戦闘中のポーズメニューオーバーレイ
///
/// 再開 / リトライ / ホームへ を提供する。
class PauseOverlay : public IOverlay {
public:
    PauseOverlay() = default;
    ~PauseOverlay() override = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Pause; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    BaseSystemAPI* systemAPI_ = nullptr;
    bool isInitialized_ = false;

    mutable bool requestClose_ = false;
    mutable bool hasTransitionRequest_ = false;
    mutable GameState requestedNextState_ = GameState::Home;

    void HandleMouseInput(SharedContext& ctx);
};

} // namespace core
} // namespace game

