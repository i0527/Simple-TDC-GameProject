#pragma once

#include "IOverlay.hpp"

// 讓呎ｺ悶Λ繧､繝悶Λ繝ｪ
#include <string>

namespace game {
namespace core {

/// @brief 繝舌ヨ繝ｫ邨先棡繧ｪ繝ｼ繝舌・繝ｬ繧､・亥享蛻ｩ/謨怜圏・・
class BattleResultOverlay : public IOverlay {
public:
    explicit BattleResultOverlay(bool isVictory);
    ~BattleResultOverlay() = default;

    // IOverlay螳溯｣・
    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override;
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    BaseSystemAPI* systemAPI_ = nullptr;
    bool isInitialized_ = false;
    const bool isVictory_;

    mutable bool requestClose_ = false;
    mutable bool hasTransitionRequest_ = false;
    mutable GameState requestedNextState_ = GameState::Home;

    // UI迥ｶ諷・
    bool nextStageEnabled_ = false;
    std::string nextStageId_ = "";

    // 蜀・Κ
    void UpdateNextStageInfo(SharedContext& ctx);
    void HandleMouseInput(SharedContext& ctx);
};

} // namespace core
} // namespace game

