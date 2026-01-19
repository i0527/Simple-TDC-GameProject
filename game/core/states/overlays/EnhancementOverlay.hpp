#pragma once

#include "IOverlay.hpp"
// 標準ライブラリ
#include <string>

namespace game {
namespace core {

/// @brief 強化オーバーレイ
///
/// タワー強化画面を表示するオーバーレイ。
class EnhancementOverlay : public IOverlay {
public:
    EnhancementOverlay();
    ~EnhancementOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Enhancement; }
    bool IsImGuiOverlay() const override { return false; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;
    int selectedSlotIndex_ = 0;
    std::string selectedAttachmentId_;
    float attachmentListScroll_ = 0.0f;
};

} // namespace core
} // namespace game
