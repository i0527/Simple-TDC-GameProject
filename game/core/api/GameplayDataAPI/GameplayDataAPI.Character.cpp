#include "../GameplayDataAPI.hpp"

// プロジェクト内
#include "../../ecs/entities/CharacterLoader.hpp"
#include "../../../utils/Log.h"

namespace game {
namespace core {

namespace {
const std::unordered_map<std::string, entities::Character> kEmptyCharacterMap{};
} // namespace

std::shared_ptr<entities::Character> GameplayDataAPI::GetCharacterTemplate(
    const std::string& characterId) {
    if (!characterManager_) {
        return nullptr;
    }
    return characterManager_->GetCharacterTemplate(characterId);
}

std::vector<std::string> GameplayDataAPI::GetAllCharacterIds() const {
    if (!characterManager_) {
        return {};
    }
    return characterManager_->GetAllCharacterIds();
}

bool GameplayDataAPI::HasCharacter(const std::string& characterId) const {
    if (!characterManager_) {
        return false;
    }
    return characterManager_->HasCharacter(characterId);
}

size_t GameplayDataAPI::GetCharacterCount() const {
    if (!characterManager_) {
        return 0;
    }
    return characterManager_->GetCharacterCount();
}

const std::unordered_map<std::string, entities::Character>&
GameplayDataAPI::GetAllCharacterMasters() const {
    if (!characterManager_) {
        return kEmptyCharacterMap;
    }
    return characterManager_->GetAllMasters();
}

bool GameplayDataAPI::SaveCharacterMasters(
    const std::unordered_map<std::string, entities::Character>& masters) {
    if (!characterManager_) {
        return false;
    }
    if (characterJsonPath_.empty()) {
        LOG_WARN("GameplayDataAPI::SaveCharacterMasters: characterJsonPath is empty");
        return false;
    }
    if (!entities::CharacterLoader::SaveToJSON(characterJsonPath_, masters)) {
        return false;
    }
    characterManager_->SetMasters(masters);
    return true;
}

} // namespace core
} // namespace game
