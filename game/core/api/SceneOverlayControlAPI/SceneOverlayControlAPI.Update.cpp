#include "../SceneOverlayControlAPI.hpp"

// 外部ライブラリ
#include <raylib.h>

// プロジェクト内
#include "../../../utils/Log.h"
#include "../../config/SharedContext.hpp"
#include "../DebugUIAPI.hpp"
#include "../InputSystemAPI.hpp"
#include "../../states/IScene.hpp"
#include "../../system/OverlayManager.hpp"

namespace game {
namespace core {

SceneOverlayUpdateResult SceneOverlayControlAPI::Update(GameState state, float deltaTime) {
    SceneOverlayUpdateResult result;
    if (!sharedContext_ || !overlayManager_) {
        LOG_ERROR("SceneOverlayControlAPI::Update: not initialized");
        return result;
    }

    if (sharedContext_->debugUIAPI) {
        sharedContext_->debugUIAPI->UpdateToggle();
    }

    if (sharedContext_->inputAPI && state != GameState::Initializing) {
        if (sharedContext_->inputAPI->IsKeyPressed(KEY_F2)) {
            if (state == GameState::Editor) {
                if (lastNonEditorState_ == GameState::Editor ||
                    lastNonEditorState_ == GameState::Initializing) {
                    lastNonEditorState_ = GameState::Home;
                }
                result.hasTransition = true;
                result.nextState = lastNonEditorState_;
            } else {
                lastNonEditorState_ = state;
                result.hasTransition = true;
                result.nextState = GameState::Editor;
            }
            if (overlayManager_) {
                overlayManager_->PopAllOverlays();
            }
            return result;
        }
    }

    IScene* scene = GetScene(state);
    if (!scene) {
        LOG_ERROR("SceneOverlayControlAPI::Update: scene is null");
        return result;
    }

    scene->Update(deltaTime);

    if (state != GameState::Initializing) {
        overlayManager_->Update(*sharedContext_, deltaTime);
        if (overlayManager_->HasTransitionRequest()) {
            result.hasTransition = true;
            result.nextState = overlayManager_->GetRequestedTransition();
            overlayManager_->PopAllOverlays();
            overlayManager_->ClearTransitionRequest();
        }
        if (overlayManager_->HasQuitRequest()) {
            result.requestShutdown = true;
            overlayManager_->ClearQuitRequest();
        }
    }

    if (scene->RequestQuit()) {
        LOG_INFO("QUIT requested from Scene");
        result.requestShutdown = true;
    }

    GameState nextState;
    if (scene->RequestTransition(nextState)) {
        result.hasTransition = true;
        result.nextState = nextState;
    }

    return result;
}

} // namespace core
} // namespace game
