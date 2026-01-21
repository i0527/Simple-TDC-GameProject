#include "PauseOverlay.hpp"

#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UiAssetKeys.hpp"
#include "../../config/RenderPrimitives.hpp"


namespace game {
namespace core {

bool PauseOverlay::Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) {
    if (isInitialized_) {
        LOG_ERROR("PauseOverlay already initialized");
        return false;
    }
    if (!systemAPI) {
        LOG_ERROR("PauseOverlay: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    requestClose_ = false;
    hasTransitionRequest_ = false;
    requestedNextState_ = GameState::Home;
    isInitialized_ = true;
    return true;
}

void PauseOverlay::Update(SharedContext& ctx, float /*deltaTime*/) {
    if (!isInitialized_) return;

    // Space / ESC で再開
    if (ctx.inputAPI &&
        (ctx.inputAPI->IsSpacePressed() || ctx.inputAPI->IsEscapePressed())) {
        requestClose_ = true;
        return;
    }

    HandleMouseInput(ctx);
}

void PauseOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) return;

    // 中央ウィンドウ
    const float windowW = 720.0f;
    const float windowH = 520.0f;
    const float windowX = (1920.0f - windowW) * 0.5f;
    const float windowY = (1080.0f - windowH) * 0.45f;

    Rect windowRect{windowX, windowY, windowW, windowH};
    systemAPI_->Render().DrawUiNineSlice(ui::UiAssetKeys::PanelBackground,
                                         windowRect, 8, 8, 8, 8,
                                         ToCoreColor(WHITE));
    systemAPI_->Render().DrawUiNineSlice(ui::UiAssetKeys::PanelBorder, windowRect,
                                         8, 8, 8, 8, ToCoreColor(WHITE));

    // タイトル
    const char* title = "ポーズ";
    const float titleSize = 56.0f;
    Vec2 tSize =
        systemAPI_->Render().MeasureTextDefaultCore(title, titleSize, 1.0f);
    float titleX = windowX + (windowW - tSize.x) * 0.5f;
    float titleY = windowY + 44.0f;
    systemAPI_->Render().DrawTextDefault(title, titleX, titleY, titleSize,
                                         ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    // 説明
    systemAPI_->Render().DrawTextDefault(
        "Space / ESC: 再開", windowX + 64.0f, titleY + 84.0f, 22.0f,
        ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));

    // ボタン（縦）
    const float btnW = 440.0f;
    const float btnH = 62.0f;
    const float btnX = windowX + (windowW - btnW) * 0.5f;
    const float btnY0 = windowY + 200.0f;
    const float gapY = 22.0f;

    Rect resumeRect{ btnX, btnY0, btnW, btnH };
    Rect retryRect{ btnX, btnY0 + (btnH + gapY), btnW, btnH };
    Rect homeRect{ btnX, btnY0 + (btnH + gapY) * 2.0f, btnW, btnH };

    auto mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePosition()
                              : Vec2{0.0f, 0.0f};
    auto drawBtn = [&](Rect r, const char* label) {
        bool hovered = mouse.x >= r.x && mouse.x <= r.x + r.width &&
                       mouse.y >= r.y && mouse.y <= r.y + r.height;
        const char* textureKey = hovered
            ? ui::UiAssetKeys::ButtonPrimaryHover
            : ui::UiAssetKeys::ButtonPrimaryNormal;
        systemAPI_->Render().DrawUiNineSlice(textureKey, r, 8, 8, 8, 8,
                                             ToCoreColor(WHITE));
        Vec2 s =
            systemAPI_->Render().MeasureTextDefaultCore(label, 26.0f, 1.0f);
        systemAPI_->Render().DrawTextDefault(
            label, r.x + (r.width - s.x) * 0.5f, r.y + 16.0f, 26.0f,
            ToCoreColor(ui::OverlayColors::TEXT_DARK));
    };

    drawBtn(resumeRect, "再開");
    drawBtn(retryRect, "リトライ");
    drawBtn(homeRect, "ホームへ");
}

void PauseOverlay::Shutdown() {
    if (!isInitialized_) return;
    isInitialized_ = false;
    systemAPI_ = nullptr;
}

bool PauseOverlay::RequestClose() const {
    if (requestClose_) {
        requestClose_ = false;
        return true;
    }
    return false;
}

bool PauseOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}

void PauseOverlay::HandleMouseInput(SharedContext& ctx) {
    if (!ctx.inputAPI || !ctx.inputAPI->IsLeftClickPressed()) {
        return;
    }

    const float windowW = 720.0f;
    const float windowH = 520.0f;
    const float windowX = (1920.0f - windowW) * 0.5f;
    const float windowY = (1080.0f - windowH) * 0.45f;

    const float btnW = 440.0f;
    const float btnH = 62.0f;
    const float btnX = windowX + (windowW - btnW) * 0.5f;
    const float btnY0 = windowY + 200.0f;
    const float gapY = 22.0f;

    Rect resumeRect{ btnX, btnY0, btnW, btnH };
    Rect retryRect{ btnX, btnY0 + (btnH + gapY), btnW, btnH };
    Rect homeRect{ btnX, btnY0 + (btnH + gapY) * 2.0f, btnW, btnH };

    auto mouse = ctx.inputAPI->GetMousePosition();
    auto inRect = [&](Rect r) {
        return mouse.x >= r.x && mouse.x <= r.x + r.width &&
               mouse.y >= r.y && mouse.y <= r.y + r.height;
    };

    if (inRect(resumeRect)) {
        requestClose_ = true;
        ctx.inputAPI->ConsumeLeftClick();
        return;
    }

    if (inRect(retryRect)) {
        hasTransitionRequest_ = true;
        requestedNextState_ = GameState::Game;
        ctx.inputAPI->ConsumeLeftClick();
        return;
    }

    if (inRect(homeRect)) {
        hasTransitionRequest_ = true;
        requestedNextState_ = GameState::Home;
        ctx.inputAPI->ConsumeLeftClick();
        return;
    }

    // クリック消費（背後へ伝播させない）
    ctx.inputAPI->ConsumeLeftClick();
}

} // namespace core
} // namespace game

