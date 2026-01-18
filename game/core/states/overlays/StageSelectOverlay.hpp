#pragma once

#include "IOverlay.hpp"
#include "../../entities/StageManager.hpp"
#include <memory>
#include <vector>
#include <string>
#include <map>

namespace game {
namespace core {

// StageDataはStageManager.hppで定義されているため、entities名前空間から使用
using StageData = entities::StageData;

/// @brief カードレイアウト情報
struct CardLayout {
    int gridX;
    int gridY;
    float screenX;
    float screenY;
    float width;
    float height;
};

/// @brief ステージ選択オーバーレイ
///
/// ステージ選択画面を表示するオーバーレイ。
/// カードベースのUIでステージ一覧を表示します。
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

    // ステージデータ
    std::vector<StageData> stages_;
    
    // UI状態
    int selectedStage_;
    int hoveredStage_;
    float scrollPosition_;
    float targetScroll_;
    
    // アニメーション
    float animationTime_;
    std::map<int, float> cardScales_;
    std::map<int, float> cardAlphas_;
    float panelFadeAlpha_;
    
    // レイアウト
    std::vector<CardLayout> cardLayouts_;
    
    // 内部メソッド
    void LoadStageData(SharedContext& ctx);
    void CalculateCardLayouts();
    void RenderCards();
    void RenderDetailPanel();
    void UpdateAnimations(float deltaTime);
    void HandleCardSelection(int stageNumber, SharedContext& ctx);
    void HandleMouseInput(SharedContext& ctx);
    void HandleKeyboardInput(SharedContext& ctx);
    void HandleScrollInput();
};

} // namespace core
} // namespace game
