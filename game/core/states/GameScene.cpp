#include "GameScene.hpp"
#include "../../utils/Log.h"
#include "../api/InputSystemAPI.hpp"
#include "../api/ECSystemAPI.hpp"
#include "../api/SceneOverlayControlAPI.hpp"
#include "../ui/OverlayColors.hpp"
#include "../config/RenderPrimitives.hpp"
#include <algorithm>

namespace game {
namespace core {
namespace states {

GameScene::GameScene()
    : systemAPI_(nullptr), sharedContext_(nullptr), inputAPI_(nullptr),
      battleProgressAPI_(nullptr), requestTransition_(false),
      requestQuit_(false) {
}

GameScene::~GameScene() {
    Shutdown();
}

bool GameScene::Initialize(BaseSystemAPI* systemAPI) {
    if (!systemAPI) {
        LOG_ERROR("GameScene::Initialize: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    LOG_INFO("GameScene initialization started");

    // HUD・井ｸ企Κ・倶ｸ矩Κ繝舌・・・
    battleHud_ = std::make_unique<::game::core::ui::BattleHUDRenderer>(systemAPI_);
    battleRenderer_ = std::make_unique<::game::core::game::BattleRenderer>(
        systemAPI_, sharedContext_ ? sharedContext_->ecsAPI : nullptr);

    LOG_INFO("GameScene initialized successfully");
    return true;
}

void GameScene::Update(float deltaTime) {
    const bool overlayActive =
        (sharedContext_ && sharedContext_->sceneOverlayAPI && sharedContext_->sceneOverlayAPI->HasActiveOverlay());
    const bool pausedNow = (battleProgressAPI_ ? battleProgressAPI_->IsPaused() : false) || overlayActive;

    // 繝昴・繧ｺ荳ｭ・医∪縺溘・繧ｪ繝ｼ繝舌・繝ｬ繧､陦ｨ遉ｺ荳ｭ・峨・繧ｲ繝ｼ繝繝ｭ繧ｸ繝・け繧呈峩譁ｰ縺励↑縺・
    if (!pausedNow) {
        const float gameSpeed = battleProgressAPI_ ? battleProgressAPI_->GetGameSpeed() : 1.0f;
        if (battleProgressAPI_) {
            battleProgressAPI_->Update(deltaTime * gameSpeed);
        }
        if (battleRenderer_ && sharedContext_ && sharedContext_->ecsAPI) {
            battleRenderer_->UpdateAnimations(sharedContext_->ecsAPI, deltaTime * gameSpeed);
        }
    }

    // 蜈･蜉帛・逅・ｼ亥ｸｸ縺ｫ譖ｴ譁ｰ・・
    ProcessInput();
}

void GameScene::Render() {
    RenderBattle();
}

void GameScene::RenderHUD() {
    if (!battleProgressAPI_ || !battleHud_) {
        return;
    }

    const auto& playerTower = battleProgressAPI_->GetPlayerTower();
    const auto& enemyTower = battleProgressAPI_->GetEnemyTower();
    SharedContext dummyCtx;
    const SharedContext& ctx = sharedContext_ ? *sharedContext_ : dummyCtx;
    const int curMaxGold = battleProgressAPI_->GetGoldMaxCurrent();
    const bool overlayActive =
        (sharedContext_ && sharedContext_->sceneOverlayAPI && sharedContext_->sceneOverlayAPI->HasActiveOverlay());
    const bool pausedNow = battleProgressAPI_->IsPaused() || overlayActive;

    battleHud_->Render(ctx,
                       playerTower.currentHp, playerTower.maxHp,
                       enemyTower.currentHp, enemyTower.maxHp,
                       battleProgressAPI_->GetGold(), curMaxGold,
                       battleProgressAPI_->GetGameSpeed(), pausedNow,
                       battleProgressAPI_->GetBattleTime(),
                       battleProgressAPI_->GetUnitCooldownUntil());
}

void GameScene::Shutdown() {
    LOG_INFO("GameScene shutdown started");

    battleHud_.reset();
    battleRenderer_.reset();
    if (sharedContext_ && sharedContext_->ecsAPI) {
        sharedContext_->ecsAPI->ResetForScene();
    }

    LOG_INFO("GameScene shutdown completed");
}

bool GameScene::RequestTransition(GameState& nextState) {
    if (requestTransition_) {
        nextState = nextState_;
        requestTransition_ = false;
        return true;
    }
    return false;
}

bool GameScene::RequestQuit() {
    bool result = requestQuit_;
    requestQuit_ = false;
    return result;
}

void GameScene::ProcessInput() {
    // 邨先棡繧ｪ繝ｼ繝舌・繝ｬ繧､遲峨′蜃ｺ縺ｦ縺・ｋ髢薙・縲√す繝ｼ繝ｳ蛛ｴ縺ｮ蜈･蜉帙ｒ豁｢繧√ｋ・医が繝ｼ繝舌・繝ｬ繧､蛛ｴ縺ｧ蜃ｦ逅・ｼ・
    if (sharedContext_ && sharedContext_->sceneOverlayAPI && sharedContext_->sceneOverlayAPI->HasActiveOverlay()) {
        return;
    }

    // HUD繧ｯ繝ｪ繝・け・亥ｷｦ繧ｯ繝ｪ繝・け・・
    if (inputAPI_ && inputAPI_->IsLeftClickPressed() && battleHud_ && battleProgressAPI_) {
        auto mousePos = inputAPI_->GetMousePosition();
        SharedContext dummyCtx;
        const SharedContext& ctx = sharedContext_ ? *sharedContext_ : dummyCtx;
        auto action = battleHud_->HandleClick(
            ctx, mousePos,
            battleProgressAPI_->GetGold(),
            battleProgressAPI_->GetBattleTime(),
            battleProgressAPI_->GetUnitCooldownUntil());
        HandleHUDAction(action);
    }

    // Escape繧ｭ繝ｼ縺ｧ謌ｻ繧・
    if (inputAPI_ && inputAPI_->IsEscapePressed()) {
        LOG_INFO("Escape pressed, requesting transition to Home");
        requestTransition_ = true;
        nextState_ = GameState::Home;
    }

    // 繧ｹ繝壹・繧ｹ繧ｭ繝ｼ縺ｧ繝昴・繧ｺ蛻・ｊ譖ｿ縺・
    if (inputAPI_ && inputAPI_->IsSpacePressed()) {
        if (sharedContext_ && sharedContext_->sceneOverlayAPI) {
            sharedContext_->sceneOverlayAPI->PushOverlay(OverlayState::Pause);
            LOG_INFO("Pause overlay opened (Space)");
        }
    }
}

void GameScene::HandleButtonClick(const std::string& buttonId) {
    LOG_INFO("Button clicked: {}", buttonId);

    if (buttonId == "speed_0.5") {
        if (battleProgressAPI_) {
            battleProgressAPI_->SetGameSpeed(0.5f);
        }
    } else if (buttonId == "speed_1.0") {
        if (battleProgressAPI_) {
            battleProgressAPI_->SetGameSpeed(1.0f);
        }
    } else if (buttonId == "speed_2.0") {
        if (battleProgressAPI_) {
            battleProgressAPI_->SetGameSpeed(2.0f);
        }
    } else if (buttonId == "pause") {
        if (battleProgressAPI_) {
            battleProgressAPI_->SetPaused(!battleProgressAPI_->IsPaused());
        }
    } else if (buttonId == "exit") {
        LOG_INFO("Exit button clicked, requesting transition to Home");
        requestTransition_ = true;
        nextState_ = GameState::Home;
    }
}

void GameScene::RenderBattle() {
    if (!battleProgressAPI_) {
        return;
    }

    const auto& lane = battleProgressAPI_->GetLane();
    const auto& playerTower = battleProgressAPI_->GetPlayerTower();
    const auto& enemyTower = battleProgressAPI_->GetEnemyTower();

    // 閭梧勹・・ome縺ｨ蜷後§ Tokyo Night鬚ｨ繝繝ｼ繧ｯ繝・・繝橸ｼ・
    systemAPI_->Render().DrawRectangle(0, 0, 1920, 1080,
                                       ToCoreColor(ui::OverlayColors::MAIN_BG));

    // 繝ｬ繝ｼ繝ｳ・医Λ繧､繝ｳ・・
    ColorRGBA laneColor = ToCoreColor(ui::OverlayColors::ACCENT_GOLD);
    systemAPI_->Render().DrawLine(static_cast<int>(lane.startX),
                                  static_cast<int>(lane.y),
                                  static_cast<int>(lane.endX),
                                  static_cast<int>(lane.y), 4.0f, laneColor);

    // 繧ｿ繝ｯ繝ｼ・育ｰ｡譏鍋洸蠖｢・・
    ColorRGBA towerEnemy = ToCoreColor(ui::OverlayColors::DANGER_RED);
    ColorRGBA towerPlayer = ToCoreColor(ui::OverlayColors::ACCENT_BLUE);

    Rect enemyRec = { enemyTower.x - enemyTower.width * 0.5f, enemyTower.y - enemyTower.height, enemyTower.width, enemyTower.height };
    Rect playerRec = { playerTower.x - playerTower.width * 0.5f, playerTower.y - playerTower.height, playerTower.width, playerTower.height };
    systemAPI_->Render().DrawRectangleRec(enemyRec, towerEnemy);
    systemAPI_->Render().DrawRectangleRec(playerRec, towerPlayer);
    systemAPI_->Render().DrawRectangleLines(
        static_cast<int>(enemyRec.x), static_cast<int>(enemyRec.y),
        static_cast<int>(enemyRec.width), static_cast<int>(enemyRec.height), 2.0f,
        ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));
    systemAPI_->Render().DrawRectangleLines(
        static_cast<int>(playerRec.x), static_cast<int>(playerRec.y),
        static_cast<int>(playerRec.width), static_cast<int>(playerRec.height),
        2.0f, ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

    // 蝓皐P・医・繝・ム縺ｧ縺ｯ縺ｪ縺丞沁縺ｮ荳翫↓陦ｨ遉ｺ・・
    auto drawTowerHp = [&](const Rect& towerRect, int hp, int maxHp, ColorRGBA fillColor) {
        const float barH = 16.0f;
        const float padY = 10.0f;
        Rect barRect{ towerRect.x, towerRect.y - barH - padY, towerRect.width, barH };

        // 閭梧勹・亥ｰ代＠騾城℃・・
        ColorRGBA bg = ToCoreColor(ui::OverlayColors::PANEL_BG_PRIMARY);
        bg.a = 220;
        systemAPI_->Render().DrawRectangleRec(barRect, bg);

        const float pct = (maxHp > 0) ? std::clamp(static_cast<float>(hp) / static_cast<float>(maxHp), 0.0f, 1.0f) : 0.0f;
        Rect fill{ barRect.x, barRect.y, barRect.width * pct, barRect.height };
        systemAPI_->Render().DrawRectangleRec(fill, fillColor);

        systemAPI_->Render().DrawRectangleLines(
            static_cast<int>(barRect.x), static_cast<int>(barRect.y),
            static_cast<int>(barRect.width), static_cast<int>(barRect.height), 2.0f,
            ToCoreColor(ui::OverlayColors::BORDER_DEFAULT));

        // 謨ｰ蛟､
        std::string text = "HP " + std::to_string(hp) + " / " + std::to_string(maxHp);
        Vec2 ts = systemAPI_->Render().MeasureTextDefaultCore(text, 18.0f, 1.0f);
        float tx = barRect.x + (barRect.width - ts.x) * 0.5f;
        float ty = barRect.y - 22.0f;
        systemAPI_->Render().DrawTextDefault(text, tx, ty, 18.0f,
                                             ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
    };
    drawTowerHp(enemyRec, enemyTower.currentHp, enemyTower.maxHp, ToCoreColor(ui::OverlayColors::DANGER_RED));
    drawTowerHp(playerRec, playerTower.currentHp, playerTower.maxHp, ToCoreColor(ui::OverlayColors::ACCENT_BLUE));

    // 繝ｦ繝九ャ繝域緒逕ｻ
    if (battleRenderer_ && sharedContext_ && sharedContext_->ecsAPI) {
        battleRenderer_->RenderEntities(sharedContext_->ecsAPI);
    }

}

void GameScene::HandleHUDAction(const ::game::core::ui::BattleHUDAction& action) {
    using ::game::core::ui::BattleHUDActionType;

    switch (action.type) {
    case BattleHUDActionType::None:
        return;
    case BattleHUDActionType::TogglePause:
        if (battleProgressAPI_) {
            battleProgressAPI_->HandleHUDAction(action);
        }
        return;
    case BattleHUDActionType::SetSpeed:
    case BattleHUDActionType::SpawnUnit:
        if (battleProgressAPI_) {
            battleProgressAPI_->HandleHUDAction(action);
        }
        return;
    }
}

} // namespace states
} // namespace core
} // namespace game
