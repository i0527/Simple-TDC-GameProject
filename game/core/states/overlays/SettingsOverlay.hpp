#pragma once

#include "IOverlay.hpp"
#include <memory>

namespace game {
namespace core {

/// @brief 設定オーバーレイ
///
/// 設定画面を表示するオーバーレイ。
/// Panel/Buttonコンポーネントを使用します。
class SettingsOverlay : public IOverlay {
public:
    SettingsOverlay();
    ~SettingsOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Settings; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;
};

} // namespace core
} // namespace game
