#include "../SceneOverlayControlAPI.hpp"

// プロジェクト内
#include "../../../utils/Log.h"
#include "../../config/SharedContext.hpp"
#include "../DebugUIAPI.hpp"
#include "../../states/IScene.hpp"
#include "../../system/OverlayManager.hpp"

namespace game {
namespace core {

void SceneOverlayControlAPI::Render(GameState state) {
    if (!sharedContext_ || !overlayManager_) {
        LOG_ERROR("SceneOverlayControlAPI::Render: not initialized");
        return;
    }

    IScene* scene = GetScene(state);
    if (!scene) {
        LOG_ERROR("SceneOverlayControlAPI::Render: scene is null");
        return;
    }

    scene->Render();
    scene->RenderOverlay();
    overlayManager_->Render(*sharedContext_);
    scene->RenderHUD();
}

void SceneOverlayControlAPI::RenderImGui(GameState state) {
    IScene* scene = GetScene(state);
    if (scene) {
        scene->RenderImGui();
    }

    if (sharedContext_ && overlayManager_) {
        overlayManager_->RenderImGui(*sharedContext_);
    }

    if (sharedContext_ && sharedContext_->debugUIAPI) {
        sharedContext_->debugUIAPI->Render();
    }
}

} // namespace core
} // namespace game
