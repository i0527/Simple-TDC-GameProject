#pragma once

#include "IOverlay.hpp"

namespace game {
namespace core {

/// @brief 戦闘中のポーズメニューオーバーレイ
///
/// Resume / Settings / Home を提供する。
/// Settings は既存の SettingsOverlay をスタックに積む。
class PauseOverlay : public IOverlay {
public:
    PauseOverlay() = default;
    ~PauseOverlay() override = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI) override;
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

