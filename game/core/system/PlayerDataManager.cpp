#include "PlayerDataManager.hpp"

// 標準ライブラリ
#include <algorithm>
#include <filesystem>
#include <fstream>

// 外部ライブラリ
#include <nlohmann/json.hpp>

// プロジェクト内
#include "../../utils/Log.h"
#include "../ecs/entities/CharacterManager.hpp"
#include "../ecs/entities/ItemPassiveManager.hpp"
#include "../ecs/entities/StageManager.hpp"

using json = nlohmann::json;

namespace game {
namespace core {

namespace {
// セーブスキーマのバージョン
// v2: tickets/max_tickets を追加
// v3: gacha_dust/gacha_pity/gacha_roll_seq/gacha_history を追加
// v5: tower_attachments を追加
constexpr int SAVE_VERSION = 5;
constexpr int MAX_GACHA_HISTORY = 100;

int ClampNonNegative(int v) { return std::max(0, v); }
int ClampLevel(int v) { return std::max(1, v); }

} // namespace

bool PlayerDataManager::LoadOrCreate(const std::string& filePath,
                                    const entities::CharacterManager& characterManager,
                                    const entities::ItemPassiveManager& itemPassiveManager,
                                    const entities::StageManager& stageManager) {
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

        bool needsMigrationSave = false;

        data_ = PlayerSaveData();
        data_.version = root.value("version", SAVE_VERSION);
        data_.gold = ClampNonNegative(root.value("gold", data_.gold));
        data_.gems = ClampNonNegative(root.value("gems", data_.gems));
        data_.tickets = ClampNonNegative(root.value("tickets", data_.tickets));
        data_.maxTickets = ClampNonNegative(root.value("max_tickets", data_.maxTickets));
        data_.gachaDust = ClampNonNegative(root.value("gacha_dust", data_.gachaDust));
        data_.gachaPityCounter = ClampNonNegative(root.value("gacha_pity", data_.gachaPityCounter));
        data_.gachaRollSequence = ClampNonNegative(root.value("gacha_roll_seq", data_.gachaRollSequence));

        // マイグレーション: 旧セーブにキーが無い場合はデフォルト補完して保存する
        if (!root.contains("gold")) needsMigrationSave = true;
        if (!root.contains("gems")) needsMigrationSave = true;
        if (!root.contains("tickets")) needsMigrationSave = true;
        if (!root.contains("max_tickets")) needsMigrationSave = true;
        if (!root.contains("gacha_dust")) needsMigrationSave = true;
        if (!root.contains("gacha_pity")) needsMigrationSave = true;
        if (!root.contains("gacha_roll_seq")) needsMigrationSave = true;
        if (!root.contains("gacha_history")) needsMigrationSave = true;
        // version が無い/古い場合は保存し直す
        if (!root.contains("version")) needsMigrationSave = true;
        if (!root.contains("stages")) needsMigrationSave = true;
        if (!root.contains("tower_attachments")) needsMigrationSave = true;

        if (data_.version != SAVE_VERSION) {
            LOG_WARN("PlayerDataManager: save version mismatch (got {}, expected {}), trying best-effort load",
                     data_.version, SAVE_VERSION);
            needsMigrationSave = true;
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

        // stages
        if (root.contains("stages") && root["stages"].is_object()) {
            const auto& sj = root["stages"];
            for (auto it = sj.begin(); it != sj.end(); ++it) {
                const std::string stageId = it.key();
                const auto& v = it.value();
                if (!v.is_object()) continue;
                PlayerSaveData::StageState st;
                st.isCleared = v.value("is_cleared", false);
                st.isLocked = v.value("is_locked", true);
                st.starsEarned = ClampNonNegative(v.value("stars_earned", 0));
                data_.stages[stageId] = st;
            }
        }

        // gacha history
        if (root.contains("gacha_history") && root["gacha_history"].is_array()) {
            for (const auto& h : root["gacha_history"]) {
                if (!h.is_object()) continue;
                PlayerSaveData::GachaHistoryEntry entry;
                entry.seq = ClampNonNegative(h.value("seq", 0));
                entry.equipmentId = h.value("equipment_id", "");
                entry.rarity = h.value("rarity", "");
                entry.countAfter = ClampNonNegative(h.value("count_after", 0));
                if (!entry.equipmentId.empty()) {
                    data_.gachaHistory.push_back(entry);
                }
            }
            if (data_.gachaHistory.size() > MAX_GACHA_HISTORY) {
                data_.gachaHistory.erase(
                    data_.gachaHistory.begin(),
                    data_.gachaHistory.end() - MAX_GACHA_HISTORY);
            }
        }

        // tower enhancements
        if (root.contains("tower_enhancements") && root["tower_enhancements"].is_object()) {
            const auto& tj = root["tower_enhancements"];
            data_.towerEnhancements.towerHpLevel =
                ClampNonNegative(tj.value("tower_hp_level", data_.towerEnhancements.towerHpLevel));
            data_.towerEnhancements.walletGrowthLevel =
                ClampNonNegative(tj.value("wallet_growth_level", data_.towerEnhancements.walletGrowthLevel));
            data_.towerEnhancements.costRegenLevel =
                ClampNonNegative(tj.value("cost_regen_level", data_.towerEnhancements.costRegenLevel));
            data_.towerEnhancements.allyAttackLevel =
                ClampNonNegative(tj.value("ally_attack_level", data_.towerEnhancements.allyAttackLevel));
            data_.towerEnhancements.allyHpLevel =
                ClampNonNegative(tj.value("ally_hp_level", data_.towerEnhancements.allyHpLevel));
        }

        // tower attachments (3 slots)
        if (root.contains("tower_attachments") && root["tower_attachments"].is_array()) {
            int i = 0;
            for (const auto& a : root["tower_attachments"]) {
                if (i >= 3) break;
                if (a.is_object()) {
                    data_.towerAttachments[i].id = a.value("id", "");
                    data_.towerAttachments[i].level = ClampLevel(a.value("level", 1));
                }
                ++i;
            }
        }

        // 欠けている要素をマスターから補完
        EnsureDefaultsFromMasters(characterManager, itemPassiveManager);
        EnsureStageStatesFromMasters(stageManager);

        if (needsMigrationSave) {
            LOG_INFO("PlayerDataManager: migrating save schema and writing updated file: {}", filePath_);
            Save();
        }

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
    EnsureStageStatesFromMasters(stageManager);
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
        root["gold"] = ClampNonNegative(data_.gold);
        root["gems"] = ClampNonNegative(data_.gems);
        root["tickets"] = ClampNonNegative(data_.tickets);
        root["max_tickets"] = ClampNonNegative(data_.maxTickets);
        root["gacha_dust"] = ClampNonNegative(data_.gachaDust);
        root["gacha_pity"] = ClampNonNegative(data_.gachaPityCounter);
        root["gacha_roll_seq"] = ClampNonNegative(data_.gachaRollSequence);

        // tower enhancements
        json tower;
        tower["tower_hp_level"] = ClampNonNegative(data_.towerEnhancements.towerHpLevel);
        tower["wallet_growth_level"] = ClampNonNegative(data_.towerEnhancements.walletGrowthLevel);
        tower["cost_regen_level"] = ClampNonNegative(data_.towerEnhancements.costRegenLevel);
        tower["ally_attack_level"] = ClampNonNegative(data_.towerEnhancements.allyAttackLevel);
        tower["ally_hp_level"] = ClampNonNegative(data_.towerEnhancements.allyHpLevel);
        root["tower_enhancements"] = tower;

        // tower attachments
        json attachments = json::array();
        for (const auto& slot : data_.towerAttachments) {
            json s;
            s["id"] = slot.id;
            s["level"] = ClampLevel(slot.level);
            attachments.push_back(s);
        }
        root["tower_attachments"] = attachments;

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

        // gacha history
        json history = json::array();
        for (const auto& h : data_.gachaHistory) {
            json entry;
            entry["seq"] = ClampNonNegative(h.seq);
            entry["equipment_id"] = h.equipmentId;
            entry["rarity"] = h.rarity;
            entry["count_after"] = ClampNonNegative(h.countAfter);
            history.push_back(entry);
        }
        root["gacha_history"] = history;

        // stages
        json stages = json::object();
        for (const auto& [id, st] : data_.stages) {
            json s;
            s["is_cleared"] = st.isCleared;
            s["is_locked"] = st.isLocked;
            s["stars_earned"] = ClampNonNegative(st.starsEarned);
            stages[id] = s;
        }
        root["stages"] = stages;

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

PlayerDataManager::PlayerSaveData::StageState PlayerDataManager::GetStageState(
    const std::string& stageId) const {
    auto it = data_.stages.find(stageId);
    if (it != data_.stages.end()) {
        return it->second;
    }
    return PlayerSaveData::StageState();
}

void PlayerDataManager::SetStageState(const std::string& stageId,
                                      const PlayerSaveData::StageState& state) {
    data_.stages[stageId] = state;
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

void PlayerDataManager::AddGachaHistoryEntry(const PlayerSaveData::GachaHistoryEntry& entry) {
    data_.gachaHistory.push_back(entry);
    if (data_.gachaHistory.size() > MAX_GACHA_HISTORY) {
        data_.gachaHistory.erase(
            data_.gachaHistory.begin(),
            data_.gachaHistory.end() - MAX_GACHA_HISTORY);
    }
}

void PlayerDataManager::EnsureDefaultsFromMasters(const entities::CharacterManager& characterManager,
                                                  const entities::ItemPassiveManager& itemPassiveManager) {
    // characters: 追加/欠損の補完（unlocked はマスターの default_unlocked を初期値に）
    const auto& masters = characterManager.GetAllMasters();
    for (const auto& [id, ch] : masters) {
        auto it = data_.characters.find(id);
        if (it == data_.characters.end()) {
            CharacterState st;
            st.unlocked = ch.default_unlocked;
            st.level = ClampLevel(ch.default_level);
            data_.characters[id] = st;
        } else {
            it->second.level = ClampLevel(it->second.level);
        }
    }

    // inventory: 初期値が無い場合は一定数をセット（開発用の所持状態）
    // ガチャ導入後は、全装備を初期配布するとガチャが成立しないため、スターター装備のみ付与する。
    constexpr int DEFAULT_EQUIPMENT_START_COUNT = 0;
    // パッシブは開発/検証しやすいように多めのまま
    constexpr int DEFAULT_PASSIVE_START_COUNT = 10;
    // 装備
    for (const auto* eq : itemPassiveManager.GetAllEquipment()) {
        if (!eq) continue;
        if (data_.ownedEquipment.find(eq->id) == data_.ownedEquipment.end()) {
            data_.ownedEquipment[eq->id] = DEFAULT_EQUIPMENT_START_COUNT;
        }
    }

    // スターター装備（最低限のプレイアビリティ）
    // 既存セーブで既に所持している場合は上書きしない。
    const char* starterEquipmentIds[] = {
        "eq_sword_001",
        "eq_shield_001",
        "eq_armor_001",
    };
    for (const char* id : starterEquipmentIds) {
        if (!id) continue;
        auto it = data_.ownedEquipment.find(id);
        if (it != data_.ownedEquipment.end()) {
            if (it->second <= 0) {
                it->second = 1;
            }
        } else {
            data_.ownedEquipment[id] = 1;
        }
    }
    // パッシブ
    for (const auto* ps : itemPassiveManager.GetAllPassiveSkills()) {
        if (!ps) continue;
        if (data_.ownedPassives.find(ps->id) == data_.ownedPassives.end()) {
            data_.ownedPassives[ps->id] = DEFAULT_PASSIVE_START_COUNT;
        }
    }
}

void PlayerDataManager::EnsureStageStatesFromMasters(
    const entities::StageManager& stageManager) {
    const auto& masters = stageManager.GetAllStages();
    for (const auto& [id, stage] : masters) {
        auto it = data_.stages.find(id);
        if (it == data_.stages.end()) {
            PlayerSaveData::StageState st;
            st.isCleared = stage.isCleared;
            st.isLocked = stage.isLocked;
            st.starsEarned = ClampNonNegative(stage.starsEarned);
            data_.stages[id] = st;
        } else {
            it->second.starsEarned = ClampNonNegative(it->second.starsEarned);
            if (it->second.isCleared) {
                it->second.isLocked = false;
            }
        }
    }
}

} // namespace core
} // namespace game

