#include "../SetupAPI.hpp"

// プロジェクト内
#include "../../../utils/Log.h"

namespace game {
namespace core {

std::vector<::game::core::game::SpawnEvent> SetupAPI::LoadStageSpawnEvents(
    const nlohmann::json& stageData) {
    if (!isInitialized_) {
        LOG_WARN("SetupAPI::LoadStageSpawnEvents: not initialized");
        return {};
    }
    return waveLoader_->LoadStageSpawnEvents(stageData);
}

} // namespace core
} // namespace game
