#pragma once

#include "IOverlay.hpp"
#include <memory>

namespace game {
namespace core {

/// @brief ライセンスオーバーレイ
///
/// ライセンス情報を表示するオーバーレイ。
/// Panel/Buttonコンポーネントを使用します。
class LicenseOverlay : public IOverlay {
public:
    LicenseOverlay();
    ~LicenseOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::License; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;
    
    // スクロール関連
    float scrollY_;
    float totalContentHeight_;
    float visibleContentHeight_;
    bool isDraggingScrollbar_;
    float dragStartY_;
    float dragStartScrollY_;
    
    // ヘルパーメソッド
    void RenderLicenseText(float contentX, float contentY, float contentWidth, float contentHeight);
    void RenderScrollbar(float windowX, float windowY, float windowWidth, float windowHeight);
    float CalculateTotalContentHeight();
    void HandleScrollbarInteraction(float windowX, float windowY, float windowWidth, float windowHeight);
};

} // namespace core
} // namespace game
