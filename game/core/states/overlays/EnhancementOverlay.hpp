#pragma once

#include "IOverlay.hpp"
#include "../../ui/components/Card.hpp"
#include "../../ui/components/List.hpp"
#include "../../ui/components/Button.hpp"
#include "../../ui/components/Panel.hpp"
#include <memory>

namespace game {
namespace core {

/// @brief 強化オーバーレイ
///
/// 強化画面を表示するオーバーレイ。
/// Card/Listコンポーネントを使用します。
class EnhancementOverlay : public IOverlay {
public:
    EnhancementOverlay();
    ~EnhancementOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Enhancement; }
    bool IsImGuiOverlay() const override { return true; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;

    // UIコンポーネント
    std::shared_ptr<ui::Panel> panel_;
    std::shared_ptr<ui::List> enhancementList_;
    std::shared_ptr<ui::Button> closeButton_;
};

} // namespace core
} // namespace game
