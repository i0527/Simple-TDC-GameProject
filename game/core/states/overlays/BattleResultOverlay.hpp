#pragma once

#include "IOverlay.hpp"

// 標準ライブラリ
#include <string>

namespace game {
namespace core {

/// @brief バトル結果オーバーレイ（勝利/敗北）
class BattleResultOverlay : public IOverlay {
public:
    explicit BattleResultOverlay(bool isVictory);
    ~BattleResultOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI) override;
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

    // UI状態
    bool nextStageEnabled_ = false;
    std::string nextStageId_ = "";

    // 内部
    void UpdateNextStageInfo(SharedContext& ctx);
    void HandleMouseInput(SharedContext& ctx);
};

} // namespace core
} // namespace game

