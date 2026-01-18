#include "PlayerDataManager.hpp"

// 標準ライブラリ
#include <algorithm>
#include <filesystem>
#include <fstream>

// 外部ライブラリ
#include <nlohmann/json.hpp>

// プロジェクト内
#include "../../utils/Log.h"
#include "../entities/CharacterManager.hpp"
#include "../entities/ItemPassiveManager.hpp"

using json = nlohmann::json;

namespace game {
namespace core {

namespace {
constexpr int SAVE_VERSION = 1;

int ClampNonNegative(int v) { return std::max(0, v); }
int ClampLevel(int v) { return std::max(1, v); }

} // namespace

bool PlayerDataManager::LoadOrCreate(const std::string& filePath,
                                    const entities::CharacterManager& characterManager,
                                    const entities::ItemPassiveManager& itemPassiveManager) {
    filePath_ = filePath;

    try {
        std::ifstream file(filePath_);
        if (!file.is_open()) {
            LOG_WARN("PlayerDataManager: save file not found, creating default: {}", filePath_);
            data_ = PlayerSaveData();
            data_.version = SAVE_VERSION;
            EnsureDefaultsFromMasters(characterManager, itemPassiveManager);
            Save();
            return true;
        }

        json root;
        file >> root;

        data_ = PlayerSaveData();
        data_.version = root.value("version", SAVE_VERSION);
        if (data_.version != SAVE_VERSION) {
            LOG_WARN("PlayerDataManager: save version mismatch (got {}, expected {}), trying best-effort load",
                     data_.version, SAVE_VERSION);
        }

        // formation
        if (root.contains("formation") && root["formation"].is_object()) {
            const auto& fj = root["formation"];
            if (fj.contains("slots") && fj["slots"].is_array()) {
                data_.formation.Clear();
                for (const auto& slot : fj["slots"]) {
                    if (!slot.is_object()) continue;
                    const int idx = slot.value("slot", -1);
                    const std::string characterId = slot.value("character_id", "");
                    if (idx >= 0 && idx < 10 && !characterId.empty()) {
                        data_.formation.slots.push_back({idx, characterId});
                    }
                }
            }
        }

        // characters
        if (root.contains("characters") && root["characters"].is_object()) {
            const auto& cj = root["characters"];
            for (auto it = cj.begin(); it != cj.end(); ++it) {
                const std::string characterId = it.key();
                const auto& v = it.value();
                if (!v.is_object()) continue;

                CharacterState st;
                st.unlocked = v.value("unlocked", false);
                st.level = ClampLevel(v.value("level", 1));

                // passives (3 slots)
                if (v.contains("passives") && v["passives"].is_array()) {
                    int i = 0;
                    for (const auto& p : v["passives"]) {
                        if (i >= 3) break;
                        if (p.is_object()) {
                            st.passives[i].id = p.value("id", "");
                            st.passives[i].level = ClampLevel(p.value("level", 1));
                        } else if (p.is_string()) {
                            st.passives[i].id = p.get<std::string>();
                            st.passives[i].level = 1;
                        }
                        i++;
                    }
                }

                // equipment (3 slots)
                if (v.contains("equipment") && v["equipment"].is_array()) {
                    int i = 0;
                    for (const auto& e : v["equipment"]) {
                        if (i >= 3) break;
                        if (e.is_string()) {
                            st.equipment[i] = e.get<std::string>();
                        }
                        i++;
                    }
                }

                data_.characters[characterId] = st;
            }
        }

        // inventory
        if (root.contains("inventory") && root["inventory"].is_object()) {
            const auto& inv = root["inventory"];
            if (inv.contains("equipment") && inv["equipment"].is_object()) {
                for (auto it = inv["equipment"].begin(); it != inv["equipment"].end(); ++it) {
                    if (it.value().is_number_integer()) {
                        data_.ownedEquipment[it.key()] = ClampNonNegative(it.value().get<int>());
                    }
                }
            }
            if (inv.contains("passives") && inv["passives"].is_object()) {
                for (auto it = inv["passives"].begin(); it != inv["passives"].end(); ++it) {
                    if (it.value().is_number_integer()) {
                        data_.ownedPassives[it.key()] = ClampNonNegative(it.value().get<int>());
                    }
                }
            }
        }

        // 欠けている要素をマスターから補完
        EnsureDefaultsFromMasters(characterManager, itemPassiveManager);

        LOG_INFO("PlayerDataManager: save loaded: {}", filePath_);
        return true;
    } catch (const json::parse_error& e) {
        LOG_ERROR("PlayerDataManager: JSON parse error: {}. Using defaults.", e.what());
    } catch (const std::exception& e) {
        LOG_ERROR("PlayerDataManager: Exception: {}. Using defaults.", e.what());
    }

    data_ = PlayerSaveData();
    data_.version = SAVE_VERSION;
    EnsureDefaultsFromMasters(characterManager, itemPassiveManager);
    Save();
    return true;
}

bool PlayerDataManager::Save() const {
    try {
        std::filesystem::path p(filePath_);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }

        json root;
        root["version"] = SAVE_VERSION;

        // formation
        json formation;
        formation["slots"] = json::array();
        for (const auto& s : data_.formation.slots) {
            json slot;
            slot["slot"] = s.first;
            slot["character_id"] = s.second;
            formation["slots"].push_back(slot);
        }
        root["formation"] = formation;

        // characters
        json characters = json::object();
        for (const auto& [id, st] : data_.characters) {
            json c;
            c["unlocked"] = st.unlocked;
            c["level"] = ClampLevel(st.level);

            json passives = json::array();
            for (int i = 0; i < 3; ++i) {
                json pslot;
                pslot["id"] = st.passives[i].id;
                pslot["level"] = ClampLevel(st.passives[i].level);
                passives.push_back(pslot);
            }
            c["passives"] = passives;

            json eq = json::array();
            for (int i = 0; i < 3; ++i) {
                eq.push_back(st.equipment[i]);
            }
            c["equipment"] = eq;

            characters[id] = c;
        }
        root["characters"] = characters;

        // inventory
        json inv;
        inv["equipment"] = json::object();
        for (const auto& [id, count] : data_.ownedEquipment) {
            inv["equipment"][id] = ClampNonNegative(count);
        }
        inv["passives"] = json::object();
        for (const auto& [id, count] : data_.ownedPassives) {
            inv["passives"][id] = ClampNonNegative(count);
        }
        root["inventory"] = inv;

        std::ofstream out(filePath_);
        if (!out.is_open()) {
            LOG_ERROR("PlayerDataManager: failed to open save file for writing: {}", filePath_);
            return false;
        }

        out << root.dump(2);
        LOG_INFO("PlayerDataManager: saved: {}", filePath_);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("PlayerDataManager: failed to save: {}", e.what());
        return false;
    }
}

void PlayerDataManager::ApplyToSharedContext(SharedContext& ctx) const {
    ctx.formationData = data_.formation;
}

void PlayerDataManager::SetFormationFromSharedContext(const FormationData& formation) {
    data_.formation = formation;
}

PlayerDataManager::CharacterState PlayerDataManager::GetCharacterState(const std::string& characterId) const {
    auto it = data_.characters.find(characterId);
    if (it != data_.characters.end()) {
        return it->second;
    }
    return CharacterState();
}

void PlayerDataManager::SetCharacterState(const std::string& characterId, const CharacterState& state) {
    data_.characters[characterId] = state;
}

int PlayerDataManager::GetOwnedEquipmentCount(const std::string& equipmentId) const {
    auto it = data_.ownedEquipment.find(equipmentId);
    if (it == data_.ownedEquipment.end()) return 0;
    return it->second;
}

int PlayerDataManager::GetOwnedPassiveCount(const std::string& passiveId) const {
    auto it = data_.ownedPassives.find(passiveId);
    if (it == data_.ownedPassives.end()) return 0;
    return it->second;
}

void PlayerDataManager::SetOwnedEquipmentCount(const std::string& equipmentId, int count) {
    data_.ownedEquipment[equipmentId] = ClampNonNegative(count);
}

void PlayerDataManager::SetOwnedPassiveCount(const std::string& passiveId, int count) {
    data_.ownedPassives[passiveId] = ClampNonNegative(count);
}

void PlayerDataManager::EnsureDefaultsFromMasters(const entities::CharacterManager& characterManager,
                                                  const entities::ItemPassiveManager& itemPassiveManager) {
    // characters: 追加/欠損の補完（unlocked はマスターの is_discovered を初期値に）
    const auto& masters = characterManager.GetAllMasters();
    for (const auto& [id, ch] : masters) {
        auto it = data_.characters.find(id);
        if (it == data_.characters.end()) {
            CharacterState st;
            st.unlocked = ch.is_discovered;
            st.level = ClampLevel(ch.level);
            data_.characters[id] = st;
        } else {
            it->second.level = ClampLevel(it->second.level);
        }
    }

    // inventory: 初期値が無い場合は一定数をセット（開発用の所持状態）
    constexpr int DEFAULT_START_COUNT = 10;
    // 装備
    for (const auto* eq : itemPassiveManager.GetAllEquipment()) {
        if (!eq) continue;
        if (data_.ownedEquipment.find(eq->id) == data_.ownedEquipment.end()) {
            data_.ownedEquipment[eq->id] = DEFAULT_START_COUNT;
        }
    }
    // パッシブ
    for (const auto* ps : itemPassiveManager.GetAllPassiveSkills()) {
        if (!ps) continue;
        if (data_.ownedPassives.find(ps->id) == data_.ownedPassives.end()) {
            data_.ownedPassives[ps->id] = DEFAULT_START_COUNT;
        }
    }
}

} // namespace core
} // namespace game

