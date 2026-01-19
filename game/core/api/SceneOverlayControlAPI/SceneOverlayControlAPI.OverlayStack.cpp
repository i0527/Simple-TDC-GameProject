#include "../SceneOverlayControlAPI.hpp"

// プロジェクト内
#include "../../../utils/Log.h"
#include "../../system/OverlayManager.hpp"

namespace game {
namespace core {

bool SceneOverlayControlAPI::PushOverlay(OverlayState state) {
    if (!overlayManager_ || !systemAPI_ || !uiAPI_) {
        LOG_ERROR("SceneOverlayControlAPI::PushOverlay: not initialized");
        return false;
    }
    return overlayManager_->PushOverlay(state, systemAPI_, uiAPI_);
}

void SceneOverlayControlAPI::PopOverlay() {
    if (overlayManager_) {
        overlayManager_->PopOverlay();
    }
}

void SceneOverlayControlAPI::PopAllOverlays() {
    if (overlayManager_) {
        overlayManager_->PopAllOverlays();
    }
}

bool SceneOverlayControlAPI::HasActiveOverlay() const {
    return overlayManager_ && !overlayManager_->IsEmpty();
}

bool SceneOverlayControlAPI::IsOverlayActive(OverlayState state) const {
    return overlayManager_ && overlayManager_->IsOverlayActive(state);
}

} // namespace core
} // namespace game
