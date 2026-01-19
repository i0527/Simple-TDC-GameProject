#include "../SetupAPI.hpp"

// プロジェクト内
#include "../../../utils/Log.h"
#include "../ECSystemAPI.hpp"

namespace game {
namespace core {

entt::entity SetupAPI::CreateBattleEntityFromCharacter(
    const entities::Character& character,
    const entities::EntityCreationData& creationData,
    ecs::components::Faction faction,
    const SpawnOverrides* overrides) {
    if (!isInitialized_ || !ecsAPI_) {
        LOG_ERROR("SetupAPI::CreateBattleEntityFromCharacter: not initialized");
        return entt::null;
    }

    return ecsAPI_->CreateBattleEntityFromCharacter(character, creationData, faction, overrides);
}

} // namespace core
} // namespace game
