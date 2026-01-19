#include "CharacterManager.hpp"
#include "CharacterLoader.hpp"
#include "../../../utils/Log.h"

// 標準ライブラリ
#include <algorithm>

namespace game {
namespace core {
namespace entities {

CharacterManager::CharacterManager() {
}

CharacterManager::~CharacterManager() {
    Shutdown();
}

bool CharacterManager::Initialize(const std::string& json_path) {
    if (!json_path.empty()) {
        // JSON からローチE
        if (CharacterLoader::LoadFromJSON(json_path, masters_)) {
            return true;
        }
        // JSONロード失敗時はハ�Eドコード�E期化にフォールバック
        LOG_WARN("JSON load failed, falling back to hardcoded data");
    }

    // ハ�Eドコード�E期化�E�開発速度優先！E
    CharacterLoader::LoadHardcoded(masters_);
    return true;
}

std::shared_ptr<Character> CharacterManager::GetCharacterTemplate(
    const std::string& character_id) {
    auto it = masters_.find(character_id);
    if (it == masters_.end()) {
        LOG_WARN("Character not found: {}", character_id);
        return nullptr;
    }

    // マスターチE�Eタをコピ�Eして返す
    auto ch = std::make_shared<Character>(it->second);
    return ch;
}

std::vector<std::string> CharacterManager::GetAllCharacterIds() const {
    std::vector<std::string> ids;
    for (const auto& [id, _] : masters_) {
        ids.push_back(id);
    }
    return ids;
}

bool CharacterManager::HasCharacter(const std::string& character_id) const {
    return masters_.find(character_id) != masters_.end();
}

void CharacterManager::SetMasters(const std::unordered_map<std::string, Character>& masters) {
    masters_ = masters;
}

void CharacterManager::Shutdown() {
    masters_.clear();
}

} // namespace entities
} // namespace core
} // namespace game
