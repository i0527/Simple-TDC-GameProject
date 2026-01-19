#include "ItemPassiveManager.hpp"
#include "ItemPassiveLoader.hpp"
#include "../../../utils/Log.h"

namespace game {
namespace core {
namespace entities {

ItemPassiveManager::ItemPassiveManager() {
}

ItemPassiveManager::~ItemPassiveManager() {
    Shutdown();
}

bool ItemPassiveManager::Initialize(const std::string& json_path) {
    if (!json_path.empty()) {
        if (ItemPassiveLoader::LoadFromJSON(json_path, passive_masters_,
                                            equipment_masters_)) {
            return true;
        }
        LOG_WARN("ItemPassiveManager: JSON load failed, falling back to hardcoded data");
    }

    ItemPassiveLoader::LoadHardcoded(passive_masters_, equipment_masters_);
    return true;
}

const PassiveSkill* ItemPassiveManager::GetPassiveSkill(
    const std::string& id) const {
    auto it = passive_masters_.find(id);
    if (it != passive_masters_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<const PassiveSkill*> ItemPassiveManager::GetAllPassiveSkills() const {
    std::vector<const PassiveSkill*> result;
    result.reserve(passive_masters_.size());
    for (const auto& [id, skill] : passive_masters_) {
        result.push_back(&skill);
    }
    return result;
}

const Equipment* ItemPassiveManager::GetEquipment(const std::string& id) const {
    auto it = equipment_masters_.find(id);
    if (it != equipment_masters_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<const Equipment*> ItemPassiveManager::GetAllEquipment() const {
    std::vector<const Equipment*> result;
    result.reserve(equipment_masters_.size());
    for (const auto& [id, eq] : equipment_masters_) {
        result.push_back(&eq);
    }
    return result;
}

void ItemPassiveManager::Shutdown() {
    passive_masters_.clear();
    equipment_masters_.clear();
}

void ItemPassiveManager::SetMasters(
    const std::unordered_map<std::string, PassiveSkill>& passives,
    const std::unordered_map<std::string, Equipment>& equipment) {
    passive_masters_ = passives;
    equipment_masters_ = equipment;
}

} // namespace entities
} // namespace core
} // namespace game
