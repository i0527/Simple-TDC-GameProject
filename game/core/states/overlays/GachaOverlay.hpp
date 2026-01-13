#pragma once

#include "IOverlay.hpp"
#include "../../ui/components/Card.hpp"
#include "../../ui/components/Button.hpp"
#include "../../ui/components/Panel.hpp"
#include <memory>
#include <vector>

namespace game {
namespace core {

/// @brief ガチャオーバーレイ
///
/// ガチャ画面を表示するオーバーレイ。
/// Cardコンポーネントを使用します。
class GachaOverlay : public IOverlay {
public:
    GachaOverlay();
    ~GachaOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Gacha; }
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
    std::vector<std::shared_ptr<ui::Card>> resultCards_;
    std::shared_ptr<ui::Button> gachaButton_;
    std::shared_ptr<ui::Button> closeButton_;
};

} // namespace core
} // namespace game
