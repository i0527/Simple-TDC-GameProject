#pragma once

#include "IOverlay.hpp"
#include "../../ui/components/Card.hpp"
#include "../../ui/components/List.hpp"
#include "../../ui/components/Button.hpp"
#include "../../ui/components/Panel.hpp"
#include "../../entities/CharacterManager.hpp"
#include <memory>

namespace game {
namespace core {

/// @brief 編成オーバーレイ
///
/// 編成画面を表示するオーバーレイ。
/// Card/Listコンポーネントを使用します。
class FormationOverlay : public IOverlay {
public:
    FormationOverlay();
    ~FormationOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Formation; }
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
    std::shared_ptr<ui::List> characterList_;
    std::shared_ptr<ui::Button> closeButton_;
};

} // namespace core
} // namespace game
