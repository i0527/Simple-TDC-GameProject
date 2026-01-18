#include "BattleResultOverlay.hpp"

#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../entities/StageManager.hpp"
#include "../../ui/OverlayColors.hpp"

// 標準ライブラリ
#include <cstdlib>

namespace game {
namespace core {

BattleResultOverlay::BattleResultOverlay(bool isVictory)
    : isVictory_(isVictory) {
}

bool BattleResultOverlay::Initialize(BaseSystemAPI* systemAPI) {
    if (isInitialized_) {
        LOG_ERROR("BattleResultOverlay already initialized");
        return false;
    }
    if (!systemAPI) {
        LOG_ERROR("BattleResultOverlay: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    requestClose_ = false;
    hasTransitionRequest_ = false;
    requestedNextState_ = GameState::Home;
    nextStageEnabled_ = false;
    nextStageId_.clear();
    isInitialized_ = true;
    return true;
}

void BattleResultOverlay::Update(SharedContext& ctx, float /*deltaTime*/) {
    if (!isInitialized_) return;

    UpdateNextStageInfo(ctx);

    // ESCはホームへ
    if (systemAPI_->IsKeyPressed(KEY_ESCAPE)) {
        hasTransitionRequest_ = true;
        requestedNextState_ = GameState::Home;
    }

    HandleMouseInput(ctx);
}

void BattleResultOverlay::Render(SharedContext& /*ctx*/) {
    if (!isInitialized_) return;

    // 中央ウィンドウ
    const float windowW = 900.0f;
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
    const char* title = isVictory_ ? "勝利！" : "敗北…";
    const float titleSize = 64.0f;
    Vector2 tSize = systemAPI_->MeasureTextDefault(title, titleSize, 1.0f);
    float titleX = windowX + (windowW - tSize.x) * 0.5f;
    float titleY = windowY + 40.0f;
    Color titleColor = isVictory_ ? ui::OverlayColors::TEXT_SUCCESS : ui::OverlayColors::TEXT_ERROR;
    systemAPI_->DrawTextDefault(title, titleX, titleY, titleSize, titleColor);

    // 説明
    const char* desc = isVictory_ ? "敵のタワーを破壊しました。" : "自軍のタワーが破壊されました。";
    systemAPI_->DrawTextDefault(desc, windowX + 80.0f, titleY + 90.0f, 24.0f, ui::OverlayColors::TEXT_PRIMARY);

    // ボタン
    const float btnW = 260.0f;
    const float btnH = 56.0f;
    const float btnY = windowY + windowH - 120.0f;
    const float btnGap = 40.0f;
    const float btnX0 = windowX + (windowW - (btnW * 2 + btnGap)) * 0.5f;

    // 左：ホーム
    Rectangle homeRect{ btnX0, btnY, btnW, btnH };
    systemAPI_->DrawRectangleRec(homeRect, ui::OverlayColors::BUTTON_SECONDARY);
    systemAPI_->DrawRectangleLines(static_cast<int>(homeRect.x),
                                   static_cast<int>(homeRect.y),
                                   static_cast<int>(homeRect.width),
                                   static_cast<int>(homeRect.height),
                                   2.0f,
                                   ui::OverlayColors::BORDER_DEFAULT);
    systemAPI_->DrawTextDefault("ホームへ", homeRect.x + 72.0f, homeRect.y + 14.0f, 26.0f, ui::OverlayColors::TEXT_DARK);

    // 右：勝利=次ステージ / 敗北=リトライ
    Rectangle rightRect{ btnX0 + btnW + btnGap, btnY, btnW, btnH };
    const bool rightEnabled = isVictory_ ? nextStageEnabled_ : true;
    Color rightBg = rightEnabled ? ui::OverlayColors::BUTTON_PRIMARY : ui::OverlayColors::BUTTON_DISABLED;
    systemAPI_->DrawRectangleRec(rightRect, rightBg);
    systemAPI_->DrawRectangleLines(static_cast<int>(rightRect.x),
                                   static_cast<int>(rightRect.y),
                                   static_cast<int>(rightRect.width),
                                   static_cast<int>(rightRect.height),
                                   2.0f,
                                   ui::OverlayColors::BORDER_DEFAULT);
    const char* rightLabel = isVictory_ ? "次のステージ" : "リトライ";
    systemAPI_->DrawTextDefault(rightLabel, rightRect.x + 40.0f, rightRect.y + 14.0f, 26.0f, ui::OverlayColors::TEXT_DARK);

    // 注記（勝利で次ステージ不可のとき）
    if (isVictory_ && !nextStageEnabled_) {
        systemAPI_->DrawTextDefault("次ステージが見つかりません（または未解放）",
                                    windowX + 80.0f, btnY - 40.0f, 18.0f, ui::OverlayColors::TEXT_MUTED);
    }
}

void BattleResultOverlay::Shutdown() {
    if (!isInitialized_) return;
    isInitialized_ = false;
    systemAPI_ = nullptr;
}

OverlayState BattleResultOverlay::GetState() const {
    return isVictory_ ? OverlayState::BattleVictory : OverlayState::BattleDefeat;
}

bool BattleResultOverlay::RequestClose() const {
    if (requestClose_) {
        requestClose_ = false;
        return true;
    }
    return false;
}

bool BattleResultOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}

void BattleResultOverlay::UpdateNextStageInfo(SharedContext& ctx) {
    nextStageEnabled_ = false;
    nextStageId_.clear();

    if (!ctx.stageManager) return;
    if (ctx.currentStageId.empty()) return;

    // stage idが数値なら次へ
    char* endPtr = nullptr;
    long cur = std::strtol(ctx.currentStageId.c_str(), &endPtr, 10);
    if (!endPtr || *endPtr != '\0') {
        // 数値でないステージ（debug等）は「次」を出さない
        return;
    }

    int nextNumber = static_cast<int>(cur) + 1;
    auto nextStage = ctx.stageManager->GetStageData(nextNumber);
    if (!nextStage) return;
    if (nextStage->isLocked) return;

    nextStageEnabled_ = true;
    nextStageId_ = nextStage->id;
}

void BattleResultOverlay::HandleMouseInput(SharedContext& ctx) {
    if (!systemAPI_->IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return;
    }

    const float windowW = 900.0f;
    const float windowH = 520.0f;
    const float windowX = (1920.0f - windowW) * 0.5f;
    const float windowY = (1080.0f - windowH) * 0.45f;

    const float btnW = 260.0f;
    const float btnH = 56.0f;
    const float btnY = windowY + windowH - 120.0f;
    const float btnGap = 40.0f;
    const float btnX0 = windowX + (windowW - (btnW * 2 + btnGap)) * 0.5f;

    Rectangle homeRect{ btnX0, btnY, btnW, btnH };
    Rectangle rightRect{ btnX0 + btnW + btnGap, btnY, btnW, btnH };

    Vector2 mouse = systemAPI_->GetMousePosition();
    auto inRect = [&](Rectangle r) {
        return mouse.x >= r.x && mouse.x <= r.x + r.width &&
               mouse.y >= r.y && mouse.y <= r.y + r.height;
    };

    if (inRect(homeRect)) {
        hasTransitionRequest_ = true;
        requestedNextState_ = GameState::Home;
        systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
        return;
    }

    if (inRect(rightRect)) {
        if (isVictory_) {
            if (!nextStageEnabled_) {
                systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
                return;
            }
            ctx.currentStageId = nextStageId_;
            hasTransitionRequest_ = true;
            requestedNextState_ = GameState::Game;
            systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
            return;
        }

        // 敗北：リトライ（ステージIDはそのまま）
        hasTransitionRequest_ = true;
        requestedNextState_ = GameState::Game;
        systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
        return;
    }
}

} // namespace core
} // namespace game

