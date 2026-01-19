#include "../SceneOverlayControlAPI.hpp"

// プロジェクト内
#include "../../../utils/Log.h"
#include "../../config/SharedContext.hpp"
#include "../../states/IScene.hpp"
#include "../BattleProgressAPI.hpp"
#include "../BattleSetupAPI.hpp"
#include "../../system/OverlayManager.hpp"

namespace game {
namespace core {

bool SceneOverlayControlAPI::InitializeState(GameState state) {
    if (!systemAPI_ || !sharedContext_) {
        LOG_ERROR("SceneOverlayControlAPI::InitializeState: not initialized");
        return false;
    }

    IScene* scene = GetScene(state);
    if (!scene) {
        LOG_ERROR("SceneOverlayControlAPI::InitializeState: scene is null");
        return false;
    }

    scene->SetSharedContext(sharedContext_);
    if (!scene->Initialize(systemAPI_)) {
        LOG_ERROR("SceneOverlayControlAPI::InitializeState: scene init failed");
        return false;
    }

    if (state == GameState::Home) {
        LOG_INFO("Home state initialized");
    }

    if (state == GameState::Game) {
        if (sharedContext_ && sharedContext_->battleSetupAPI && sharedContext_->battleProgressAPI) {
            sharedContext_->battleSetupData = sharedContext_->battleSetupAPI->BuildBattleSetupData(
                sharedContext_->currentStageId,
                sharedContext_->formationData);
            sharedContext_->battleProgressAPI->InitializeFromSetupData(sharedContext_->battleSetupData);
        } else if (sharedContext_ && sharedContext_->battleProgressAPI) {
            sharedContext_->battleProgressAPI->InitializeFromStage();
        }
        LOG_INFO("Game state initialized");
    }

    return true;
}

void SceneOverlayControlAPI::CleanupState(GameState state) {
    if (!overlayManager_) {
        return;
    }

    IScene* scene = GetScene(state);
    if (scene) {
        scene->Shutdown();
    }

    if (state != GameState::Initializing) {
        overlayManager_->PopAllOverlays();
    }
}

void SceneOverlayControlAPI::ShutdownAllScenes() {
    if (overlayManager_) {
        overlayManager_->PopAllOverlays();
    }

    for (auto* scene : scenes_) {
        if (scene) {
            scene->Shutdown();
        }
    }
}

void SceneOverlayControlAPI::Shutdown() {
    if (overlayManager_) {
        overlayManager_->Shutdown();
    }
}

} // namespace core
} // namespace game
