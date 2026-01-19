#include "../GameplayDataAPI.hpp"

// プロジェクト内
#include "../../ecs/entities/ItemPassiveLoader.hpp"
#include "../../../utils/Log.h"

namespace game {
namespace core {

const entities::PassiveSkill* GameplayDataAPI::GetPassiveSkill(const std::string& id) const {
    if (!itemPassiveManager_) {
        return nullptr;
    }
    return itemPassiveManager_->GetPassiveSkill(id);
}

std::vector<const entities::PassiveSkill*> GameplayDataAPI::GetAllPassiveSkills() const {
    if (!itemPassiveManager_) {
        return {};
    }
    return itemPassiveManager_->GetAllPassiveSkills();
}

const entities::Equipment* GameplayDataAPI::GetEquipment(const std::string& id) const {
    if (!itemPassiveManager_) {
        return nullptr;
    }
    return itemPassiveManager_->GetEquipment(id);
}

std::vector<const entities::Equipment*> GameplayDataAPI::GetAllEquipment() const {
    if (!itemPassiveManager_) {
        return {};
    }
    return itemPassiveManager_->GetAllEquipment();
}

const entities::ItemPassiveManager* GameplayDataAPI::GetItemPassiveManager() const {
    return itemPassiveManager_.get();
}

const std::unordered_map<std::string, entities::PassiveSkill>&
GameplayDataAPI::GetAllPassiveMasters() const {
    static const std::unordered_map<std::string, entities::PassiveSkill> kEmptyPassives;
    if (!itemPassiveManager_) {
        return kEmptyPassives;
    }
    return itemPassiveManager_->GetPassiveMasters();
}

const std::unordered_map<std::string, entities::Equipment>&
GameplayDataAPI::GetAllEquipmentMasters() const {
    static const std::unordered_map<std::string, entities::Equipment> kEmptyEquipment;
    if (!itemPassiveManager_) {
        return kEmptyEquipment;
    }
    return itemPassiveManager_->GetEquipmentMasters();
}

bool GameplayDataAPI::SaveItemPassiveMasters(
    const std::unordered_map<std::string, entities::PassiveSkill>& passives,
    const std::unordered_map<std::string, entities::Equipment>& equipment) {
    if (!itemPassiveManager_) {
        return false;
    }
    if (itemPassiveJsonPath_.empty()) {
        LOG_WARN("GameplayDataAPI::SaveItemPassiveMasters: itemPassiveJsonPath is empty");
        return false;
    }
    if (!entities::ItemPassiveLoader::SaveToJSON(itemPassiveJsonPath_, passives, equipment)) {
        return false;
    }
    itemPassiveManager_->SetMasters(passives, equipment);
    return true;
}

} // namespace core
} // namespace game
