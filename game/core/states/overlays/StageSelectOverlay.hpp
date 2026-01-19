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

// StageData縺ｯStageManager.hpp縺ｧ螳夂ｾｩ縺輔ｌ縺ｦ縺・ｋ縺溘ａ縲‘ntities蜷榊燕遨ｺ髢薙°繧我ｽｿ逕ｨ
using StageData = entities::StageData;

/// @brief 繧ｫ繝ｼ繝峨Ξ繧､繧｢繧ｦ繝域ュ蝣ｱ
struct CardLayout {
    int gridX;
    int gridY;
    float screenX;
    float screenY;
    float width;
    float height;
};

/// @brief 繧ｹ繝・・繧ｸ驕ｸ謚槭が繝ｼ繝舌・繝ｬ繧､
///
/// 繧ｹ繝・・繧ｸ驕ｸ謚樒判髱｢繧定｡ｨ遉ｺ縺吶ｋ繧ｪ繝ｼ繝舌・繝ｬ繧､縲・
/// 繧ｫ繝ｼ繝峨・繝ｼ繧ｹ縺ｮUI縺ｧ繧ｹ繝・・繧ｸ荳隕ｧ繧定｡ｨ遉ｺ縺励∪縺吶・
class StageSelectOverlay : public IOverlay {
public:
    StageSelectOverlay();
    ~StageSelectOverlay() = default;

    // IOverlay螳溯｣・
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

    // 繧ｹ繝・・繧ｸ繝・・繧ｿ
    std::vector<StageData> stages_;
    
    // UI迥ｶ諷・
    int selectedStage_;
    int hoveredStage_;
    float scrollPosition_;
    float targetScroll_;
    
    // 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ
    float animationTime_;
    std::map<int, float> cardScales_;
    std::map<int, float> cardAlphas_;
    float panelFadeAlpha_;
    
    // 繝ｬ繧､繧｢繧ｦ繝・
    std::vector<CardLayout> cardLayouts_;
    
    // 蜀・Κ繝｡繧ｽ繝・ラ
    void LoadStageData(SharedContext& ctx);
    void CalculateCardLayouts();
    void RenderCards();
    void RenderDetailPanel(SharedContext& ctx);
    void UpdateAnimations(float deltaTime);
    void HandleCardSelection(int stageNumber, SharedContext& ctx);
    void HandleMouseInput(SharedContext& ctx);
    void HandleKeyboardInput(SharedContext& ctx);
    void HandleScrollInput(InputSystemAPI* inputAPI);
};

} // namespace core
} // namespace game
