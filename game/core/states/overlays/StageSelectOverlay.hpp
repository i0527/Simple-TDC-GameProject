#pragma once

#include "IOverlay.hpp"
#include "../../ui/components/Tile.hpp"
#include "../../ui/components/Button.hpp"
#include "../../ui/components/Panel.hpp"
#include <memory>

namespace game {
namespace core {

/// @brief ステージ選択オーバーレイ
///
/// ステージ選択画面を表示するオーバーレイ。
/// Tileコンポーネントを使用してステージ一覧を表示します。
class StageSelectOverlay : public IOverlay {
public:
    StageSelectOverlay();
    ~StageSelectOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::StageSelect; }
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
    std::shared_ptr<ui::Tile> tileGrid_;
    std::shared_ptr<ui::Button> closeButton_;
};

} // namespace core
} // namespace game
