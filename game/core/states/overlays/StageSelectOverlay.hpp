#pragma once

#include "IOverlay.hpp"
#include "../../ecs/entities/StageManager.hpp"
#include <memory>
#include <vector>
#include <string>
#include <map>

namespace game {
namespace core {

class InputSystemAPI;

// StageDataはStageManager.hppで定義されてぁE��ため、entities名前空間から使用
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

/// @brief スチE�Eジ選択オーバ�Eレイ
///
/// スチE�Eジ選択画面を表示するオーバ�Eレイ、E
/// カード�EースのUIでスチE�Eジ一覧を表示します、E
class StageSelectOverlay : public IOverlay {
public:
    StageSelectOverlay();
    ~StageSelectOverlay() = default;

    // IOverlay実裁E
    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) override;
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

    // スチE�EジチE�Eタ
    std::vector<StageData> stages_;
    
    // UI状慁E
    int selectedStage_;
    int hoveredStage_;
    float scrollPosition_;
    float targetScroll_;
    
    // アニメーション
    float animationTime_;
    std::map<int, float> cardScales_;
    std::map<int, float> cardAlphas_;
    float panelFadeAlpha_;
    
    // 詳細ウィンドウ
    bool showDetailWindow_;
    float detailWindowAlpha_;
    
    // レイアウチE
    std::vector<CardLayout> cardLayouts_;
    
    // 冁E��メソチE��
    void LoadStageData(SharedContext& ctx);
    void CalculateCardLayouts();
    void RenderCards();
    void RenderDetailPanel(SharedContext& ctx);
    void RenderDetailWindow(SharedContext& ctx);
    void UpdateAnimations(float deltaTime);
    void HandleCardSelection(int stageNumber, SharedContext& ctx);
    void HandleMouseInput(SharedContext& ctx);
    void HandleKeyboardInput(SharedContext& ctx);
    void HandleScrollInput(InputSystemAPI* inputAPI);
    
    // ========== ヘルパー関数 ==========
    
    /// @brief ステージIDから背景画像パスを生成
    std::string GetStageBackgroundPath(const std::string& stageId) const;
};

} // namespace core
} // namespace game
