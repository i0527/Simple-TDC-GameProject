#include "BattleResultOverlay.hpp"

#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../api/GameplayDataAPI.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UiAssetKeys.hpp"
#include "../../config/RenderPrimitives.hpp"

// 標準ライブラリ
#include <cstdlib>

namespace game {
namespace core {

BattleResultOverlay::BattleResultOverlay(bool isVictory)
    : isVictory_(isVictory) {
}

bool BattleResultOverlay::Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) {
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

    // ESCはホ�Eムへ
    if (ctx.inputAPI && ctx.inputAPI->IsEscapePressed()) {
        hasTransitionRequest_ = true;
        requestedNextState_ = GameState::Home;
    }

    HandleMouseInput(ctx);
}

void BattleResultOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) return;

    // 中央ウィンドウ
    const float windowW = 900.0f;
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
    const char* title = isVictory_ ? "勝利!!" : "敗北...";
    const float titleSize = 64.0f;
    Vec2 tSize =
        systemAPI_->Render().MeasureTextDefaultCore(title, titleSize, 1.0f);
    float titleX = windowX + (windowW - tSize.x) * 0.5f;
    float titleY = windowY + 40.0f;
    ColorRGBA titleColor = isVictory_ ? ToCoreColor(ui::OverlayColors::TEXT_SUCCESS)
                                      : ToCoreColor(ui::OverlayColors::TEXT_ERROR);
    systemAPI_->Render().DrawTextDefault(title, titleX, titleY, titleSize,
                                         titleColor);

    // 説明
    const char* desc = isVictory_ ? "敵のタワーを破壊しました。"
                                  : "自軍のタワーが破壊されました。";
    float descY = titleY + 90.0f;
    systemAPI_->Render().DrawTextDefault(desc, windowX + 80.0f, descY,
                                         24.0f,
                                         ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    // 報酬報告（勝利時のみ）
    if (isVictory_ && ctx.gameplayDataAPI) {
        const auto& report = ctx.gameplayDataAPI->GetLastStageClearReport();
        float reportY = descY + 50.0f;
        
        // 新規キャラクター報告
        if (!report.newCharacters.empty()) {
            std::string charText = "新規キャラ: ";
            for (size_t i = 0; i < report.newCharacters.size(); ++i) {
                if (i > 0) {
                    charText += ", ";
                }
                charText += report.newCharacters[i];
            }
            systemAPI_->Render().DrawTextDefault(charText, windowX + 80.0f, reportY,
                                                 22.0f,
                                                 ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
            reportY += 35.0f;
        }
        
        // チケット報告
        if (report.ticketsRewarded > 0) {
            std::string ticketText = "チケット: +" + std::to_string(report.ticketsRewarded);
            systemAPI_->Render().DrawTextDefault(ticketText, windowX + 80.0f, reportY,
                                                 22.0f,
                                                 ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        }
    }

    // ボタン
    const float btnW = 260.0f;
    const float btnH = 56.0f;
    const float btnY = windowY + windowH - 120.0f;
    const float btnGap = 40.0f;
    const float btnX0 = windowX + (windowW - (btnW * 2 + btnGap)) * 0.5f;

    // 左�E��Eーム
    Rect homeRect{ btnX0, btnY, btnW, btnH };
    auto mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePosition()
                              : Vec2{0.0f, 0.0f};
    bool homeHovered = mouse.x >= homeRect.x && mouse.x <= homeRect.x + homeRect.width &&
                       mouse.y >= homeRect.y && mouse.y <= homeRect.y + homeRect.height;
    const char* homeTexture = homeHovered
        ? ui::UiAssetKeys::ButtonSecondaryHover
        : ui::UiAssetKeys::ButtonSecondaryNormal;
    systemAPI_->Render().DrawUiNineSlice(homeTexture, homeRect, 8, 8, 8, 8,
                                         ToCoreColor(WHITE));
    systemAPI_->Render().DrawTextDefault(
        "ホームへ", homeRect.x + 72.0f, homeRect.y + 14.0f, 26.0f,
        ToCoreColor(ui::OverlayColors::TEXT_DARK));

    // 右�E�勝利=次スチE�Eジ / 敗北=リトライ
    Rect rightRect{ btnX0 + btnW + btnGap, btnY, btnW, btnH };
    const bool rightEnabled = isVictory_ ? nextStageEnabled_ : true;
    bool rightHovered = mouse.x >= rightRect.x && mouse.x <= rightRect.x + rightRect.width &&
                        mouse.y >= rightRect.y && mouse.y <= rightRect.y + rightRect.height;
    const char* rightTexture = ui::UiAssetKeys::ButtonPrimaryNormal;
    if (!rightEnabled) {
        rightTexture = ui::UiAssetKeys::ButtonSecondaryNormal;
    } else if (rightHovered) {
        rightTexture = ui::UiAssetKeys::ButtonPrimaryHover;
    }
    systemAPI_->Render().DrawUiNineSlice(rightTexture, rightRect, 8, 8, 8, 8,
                                         ToCoreColor(WHITE));
    const char* rightLabel = isVictory_ ? "次のステージ" : "リトライ";
    systemAPI_->Render().DrawTextDefault(
        rightLabel, rightRect.x + 40.0f, rightRect.y + 14.0f, 26.0f,
        ToCoreColor(ui::OverlayColors::TEXT_DARK));

    // 注記（勝利で次ステージ不可のとき）
    if (isVictory_ && !nextStageEnabled_) {
        systemAPI_->Render().DrawTextDefault(
            "次のステージが見つかりません。または未解放です。", windowX + 80.0f,
            btnY - 40.0f, 18.0f, ToCoreColor(ui::OverlayColors::TEXT_MUTED));
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

    if (!ctx.gameplayDataAPI) return;
    if (ctx.currentStageId.empty()) return;

    const std::string nextStageId =
        ctx.gameplayDataAPI->GetPreferredNextStageId(ctx.currentStageId);
    if (nextStageId.empty()) return;

    auto nextStage = ctx.gameplayDataAPI->GetStageDataById(nextStageId);
    if (!nextStage) return;
    if (nextStage->isLocked) return;

    nextStageEnabled_ = true;
    nextStageId_ = nextStage->id;
}

void BattleResultOverlay::HandleMouseInput(SharedContext& ctx) {
    if (!ctx.inputAPI || !ctx.inputAPI->IsLeftClickPressed()) {
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

    Rect homeRect{ btnX0, btnY, btnW, btnH };
    Rect rightRect{ btnX0 + btnW + btnGap, btnY, btnW, btnH };

    auto mouse = ctx.inputAPI->GetMousePosition();
    auto inRect = [&](Rect r) {
        return mouse.x >= r.x && mouse.x <= r.x + r.width &&
               mouse.y >= r.y && mouse.y <= r.y + r.height;
    };

    if (inRect(homeRect)) {
        hasTransitionRequest_ = true;
        requestedNextState_ = GameState::Home;
        ctx.inputAPI->ConsumeLeftClick();
        return;
    }

    if (inRect(rightRect)) {
        if (isVictory_) {
            if (!nextStageEnabled_) {
                ctx.inputAPI->ConsumeLeftClick();
                return;
            }
            ctx.currentStageId = nextStageId_;
            hasTransitionRequest_ = true;
            requestedNextState_ = GameState::Game;
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }

        // 敗北�E�リトライ�E�スチE�EジIDはそ�Eまま�E�E
        hasTransitionRequest_ = true;
        requestedNextState_ = GameState::Game;
        ctx.inputAPI->ConsumeLeftClick();
        return;
    }
}

} // namespace core
} // namespace game

