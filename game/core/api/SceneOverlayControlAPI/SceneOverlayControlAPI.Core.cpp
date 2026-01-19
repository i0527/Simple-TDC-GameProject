#include "../SceneOverlayControlAPI.hpp"

// プロジェクト内
#include "../../../utils/Log.h"
#include "../../system/OverlayManager.hpp"
#include "../../states/IScene.hpp"

namespace game {
namespace core {

SceneOverlayControlAPI::SceneOverlayControlAPI()
    : systemAPI_(nullptr),
      uiAPI_(nullptr),
      sharedContext_(nullptr),
      overlayManager_(std::make_unique<OverlayManager>()),
      scenes_{} {
    scenes_.fill(nullptr);
}

SceneOverlayControlAPI::~SceneOverlayControlAPI() = default;

bool SceneOverlayControlAPI::Initialize(BaseSystemAPI* systemAPI,
                                        UISystemAPI* uiAPI,
                                        SharedContext* sharedContext) {
    if (!systemAPI || !uiAPI || !sharedContext) {
        LOG_ERROR("SceneOverlayControlAPI::Initialize: invalid argument(s)");
        return false;
    }
    systemAPI_ = systemAPI;
    uiAPI_ = uiAPI;
    sharedContext_ = sharedContext;
    if (!overlayManager_) {
        overlayManager_ = std::make_unique<OverlayManager>();
    }
    return true;
}

void SceneOverlayControlAPI::RegisterScene(GameState state, IScene* scene) {
    if (!IsValidState(state)) {
        LOG_ERROR("SceneOverlayControlAPI::RegisterScene: invalid state {}", static_cast<int>(state));
        return;
    }
    scenes_[ToIndex(state)] = scene;
}

IScene* SceneOverlayControlAPI::GetScene(GameState state) const {
    if (!IsValidState(state)) {
        LOG_ERROR("SceneOverlayControlAPI::GetScene: invalid state {}", static_cast<int>(state));
        return nullptr;
    }
    return scenes_[ToIndex(state)];
}

} // namespace core
} // namespace game
