#include "PauseOverlay.hpp"

#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../ui/OverlayColors.hpp"

#include <raylib.h>

namespace game {
namespace core {

bool PauseOverlay::Initialize(BaseSystemAPI* systemAPI) {
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
    if (systemAPI_->IsKeyPressed(KEY_SPACE) || systemAPI_->IsKeyPressed(KEY_ESCAPE)) {
        requestClose_ = true;
        return;
    }

    HandleMouseInput(ctx);
}

void PauseOverlay::Render(SharedContext& /*ctx*/) {
    if (!isInitialized_) return;

    // 中央ウィンドウ
    const float windowW = 720.0f;
    const float windowH = 520.0f;
    const float windowX = (1920.0f - windowW) * 0.5f;
    const float windowY = (1080.0f - windowH) * 0.45f;

    systemAPI_->DrawRectangle(static_cast<int>(windowX),
                              static_cast<int>(windowY),
                              static_cast<int>(windowW),
                              static_cast<int>(windowH),
                              ui::OverlayColors::OVERLAY_BG);

    systemAPI_->DrawRectangleLines(static_cast<int>(windowX),
                                   static_cast<int>(windowY),
                                   static_cast<int>(windowW),
                                   static_cast<int>(windowH),
                                   2.0f,
                                   ui::OverlayColors::BORDER_DEFAULT);

    // タイトル
    const char* title = "ポーズ";
    const float titleSize = 56.0f;
    Vector2 tSize = systemAPI_->MeasureTextDefault(title, titleSize, 1.0f);
    float titleX = windowX + (windowW - tSize.x) * 0.5f;
    float titleY = windowY + 44.0f;
    systemAPI_->DrawTextDefault(title, titleX, titleY, titleSize, ui::OverlayColors::TEXT_PRIMARY);

    // 説明
    systemAPI_->DrawTextDefault("Space / ESC: 再開", windowX + 64.0f, titleY + 84.0f, 22.0f, ui::OverlayColors::TEXT_SECONDARY);

    // ボタン（縦）
    const float btnW = 440.0f;
    const float btnH = 62.0f;
    const float btnX = windowX + (windowW - btnW) * 0.5f;
    const float btnY0 = windowY + 250.0f;
    const float gapY = 22.0f;

    Rectangle resumeRect{ btnX, btnY0, btnW, btnH };
    Rectangle homeRect{ btnX, btnY0 + (btnH + gapY), btnW, btnH };

    auto drawBtn = [&](Rectangle r, Color bg, const char* label) {
        systemAPI_->DrawRectangleRec(r, bg);
        systemAPI_->DrawRectangleLines(static_cast<int>(r.x),
                                       static_cast<int>(r.y),
                                       static_cast<int>(r.width),
                                       static_cast<int>(r.height),
                                       2.0f,
                                       ui::OverlayColors::BORDER_DEFAULT);
        Vector2 s = systemAPI_->MeasureTextDefault(label, 26.0f, 1.0f);
        systemAPI_->DrawTextDefault(label,
                                    r.x + (r.width - s.x) * 0.5f,
                                    r.y + 16.0f,
                                    26.0f,
                                    ui::OverlayColors::TEXT_DARK);
    };

    drawBtn(resumeRect, ui::OverlayColors::BUTTON_PRIMARY, "再開");
    drawBtn(homeRect, ui::OverlayColors::BUTTON_ORANGE, "ホームへ");
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
    if (!systemAPI_->IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return;
    }

    const float windowW = 720.0f;
    const float windowH = 520.0f;
    const float windowX = (1920.0f - windowW) * 0.5f;
    const float windowY = (1080.0f - windowH) * 0.45f;

    const float btnW = 440.0f;
    const float btnH = 62.0f;
    const float btnX = windowX + (windowW - btnW) * 0.5f;
    const float btnY0 = windowY + 250.0f;
    const float gapY = 22.0f;

    Rectangle resumeRect{ btnX, btnY0, btnW, btnH };
    Rectangle homeRect{ btnX, btnY0 + (btnH + gapY), btnW, btnH };

    Vector2 mouse = systemAPI_->GetMousePosition();
    auto inRect = [&](Rectangle r) {
        return mouse.x >= r.x && mouse.x <= r.x + r.width &&
               mouse.y >= r.y && mouse.y <= r.y + r.height;
    };

    if (inRect(resumeRect)) {
        requestClose_ = true;
        systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
        return;
    }

    if (inRect(homeRect)) {
        hasTransitionRequest_ = true;
        requestedNextState_ = GameState::Home;
        systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
        return;
    }

    // クリック消費（背後へ伝播させない）
    systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
}

} // namespace core
} // namespace game

