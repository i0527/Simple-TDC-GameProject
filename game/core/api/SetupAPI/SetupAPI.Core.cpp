#include "../SetupAPI.hpp"

// 標準ライブラリ
#include <utility>

// プロジェクト内
#include "../../../utils/Log.h"
#include "../GameplayDataAPI.hpp"
#include "../../config/SharedContext.hpp"

namespace game {
namespace core {

SetupAPI::SetupAPI()
    : systemAPI_(nullptr),
      gameplayDataAPI_(nullptr),
      ecsAPI_(nullptr),
      sharedContext_(nullptr),
      waveLoader_(std::make_unique<::game::core::game::WaveLoader>()),
      isInitialized_(false) {}

SetupAPI::~SetupAPI() = default;

bool SetupAPI::Initialize(BaseSystemAPI* systemAPI,
                          GameplayDataAPI* gameplayDataAPI,
                          ECSystemAPI* ecsAPI,
                          SharedContext* sharedContext) {
    if (!systemAPI || !ecsAPI || !gameplayDataAPI || !sharedContext) {
        LOG_ERROR("SetupAPI::Initialize: invalid argument(s)");
        return false;
    }

    systemAPI_ = systemAPI;
    gameplayDataAPI_ = gameplayDataAPI;
    ecsAPI_ = ecsAPI;
    sharedContext_ = sharedContext;

    sharedContext_->systemAPI = systemAPI_;
    sharedContext_->ecsAPI = ecsAPI_;
    sharedContext_->gameplayDataAPI = gameplayDataAPI_;
    sharedContext_->setupAPI = this;

    if (!gameplayDataAPI_->Initialize()) {
        LOG_WARN("GameplayDataAPI initialization failed, continuing with defaults");
    }

    gameplayDataAPI_->ApplyToSharedContext(*sharedContext_);
    isInitialized_ = true;
    return true;
}

} // namespace core
} // namespace game
