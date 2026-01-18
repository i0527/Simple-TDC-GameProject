#include "ItemPassiveManager.hpp"
#include "../../utils/Log.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>
#include <fstream>

using json = nlohmann::json;

namespace game {
namespace core {
namespace entities {

namespace {

std::string ToLowerCopy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

PassiveEffectType ParseEffectType(const json& j) {
    const std::string raw = j.value("effect_type", "percentage");
    const std::string s = ToLowerCopy(raw);
    if (s == "percentage" || s == "percent" || s == "ratio") {
        return PassiveEffectType::Percentage;
    }
    if (s == "flat" || s == "add") {
        return PassiveEffectType::Flat;
    }
    LOG_WARN("ItemPassiveManager: Unknown effect_type '{}', fallback to 'percentage'", raw);
    return PassiveEffectType::Percentage;
}

PassiveTargetStat ParseTargetStat(const json& j) {
    const std::string raw = j.value("target_stat", "attack");
    const std::string s = ToLowerCopy(raw);
    if (s == "attack" || s == "atk") return PassiveTargetStat::Attack;
    if (s == "defense" || s == "def") return PassiveTargetStat::Defense;
    if (s == "hp" || s == "max_hp") return PassiveTargetStat::Hp;
    if (s == "move_speed" || s == "speed" || s == "spd") return PassiveTargetStat::MoveSpeed;
    if (s == "attack_speed" || s == "atk_speed" || s == "as") return PassiveTargetStat::AttackSpeed;
    if (s == "range") return PassiveTargetStat::Range;
    if (s == "crit_chance" || s == "crit") return PassiveTargetStat::CritChance;
    if (s == "crit_damage" || s == "crit_dmg") return PassiveTargetStat::CritDamage;
    if (s == "gold_gain" || s == "gold") return PassiveTargetStat::GoldGain;
    if (s == "exp_gain" || s == "exp") return PassiveTargetStat::ExpGain;
    LOG_WARN("ItemPassiveManager: Unknown target_stat '{}', fallback to 'attack'", raw);
    return PassiveTargetStat::Attack;
}

} // namespace

ItemPassiveManager::ItemPassiveManager() {
}

ItemPassiveManager::~ItemPassiveManager() {
    Shutdown();
}

bool ItemPassiveManager::Initialize(const std::string& json_path) {
    if (!json_path.empty()) {
        if (LoadFromJSON(json_path)) {
            return true;
        }
        LOG_WARN("ItemPassiveManager: JSON load failed, falling back to hardcoded data");
    }

    InitializeHardcodedData();
    return true;
}

const PassiveSkill* ItemPassiveManager::GetPassiveSkill(const std::string& id) const {
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

bool ItemPassiveManager::LoadFromJSON(const std::string& json_path) {
    try {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            LOG_ERROR("ItemPassiveManager: Failed to open file: {}", json_path);
            return false;
        }

        json data;
        file >> data;

        // パッシブスキルロード
        if (data.contains("passive_skills")) {
            for (const auto& skill_json : data["passive_skills"]) {
                PassiveSkill skill;
                skill.id = skill_json.at("id").get<std::string>();
                skill.name = skill_json.at("name").get<std::string>();
                skill.description = skill_json.value("description", "");
                skill.value = skill_json.value("value", 0.0f);
                skill.effect_type = ParseEffectType(skill_json);
                skill.target_stat = ParseTargetStat(skill_json);
                passive_masters_[skill.id] = skill;
            }
        }

        // 装備アイテムロード
        if (data.contains("equipment")) {
            for (const auto& eq_json : data["equipment"]) {
                Equipment eq;
                eq.id = eq_json.at("id").get<std::string>();
                eq.name = eq_json.at("name").get<std::string>();
                eq.description = eq_json.value("description", "");
                eq.attack_bonus = eq_json.value("attack_bonus", 0.0f);
                eq.defense_bonus = eq_json.value("defense_bonus", 0.0f);
                eq.hp_bonus = eq_json.value("hp_bonus", 0.0f);
                equipment_masters_[eq.id] = eq;
            }
        }

        LOG_INFO("ItemPassiveManager: Loaded {} passives and {} equipment from JSON", 
                 passive_masters_.size(), equipment_masters_.size());
        return true;
    } catch (const json::parse_error& e) {
        LOG_ERROR("ItemPassiveManager: JSON parse error: {}", e.what());
    } catch (const std::exception& e) {
        LOG_ERROR("ItemPassiveManager: Exception: {}", e.what());
    }
    return false;
}

void ItemPassiveManager::InitializeHardcodedData() {
    // パッシブスキル初期データ
    {
        PassiveSkill s;
        s.id = "skill_atk_up_1";
        s.name = "攻撃アップ I";
        s.description = "攻撃力が 5% 上昇します。";
        s.value = 0.05f;
        s.effect_type = PassiveEffectType::Percentage;
        s.target_stat = PassiveTargetStat::Attack;
        passive_masters_[s.id] = s;
    }
    {
        PassiveSkill s;
        s.id = "skill_def_up_1";
        s.name = "防御アップ I";
        s.description = "防御力が 10% 上昇します。";
        s.value = 0.10f;
        s.effect_type = PassiveEffectType::Percentage;
        s.target_stat = PassiveTargetStat::Defense;
        passive_masters_[s.id] = s;
    }

    // 装備アイテム初期データ
    {
        Equipment e;
        e.id = "eq_sword_001";
        e.name = "鋼の剣";
        e.description = "標準的な鋼鉄製の剣。攻撃力が少し上がる。";
        e.attack_bonus = 15.0f;
        e.defense_bonus = 0.0f;
        e.hp_bonus = 0.0f;
        equipment_masters_[e.id] = e;
    }
    {
        Equipment e;
        e.id = "eq_shield_001";
        e.name = "木の盾";
        e.description = "木製の簡素な盾。防御力が上がる。";
        e.attack_bonus = 0.0f;
        e.defense_bonus = 10.0f;
        e.hp_bonus = 0.0f;
        equipment_masters_[e.id] = e;
    }

    LOG_INFO("ItemPassiveManager: Initialized with {} passives and {} equipment (hardcoded)", 
             passive_masters_.size(), equipment_masters_.size());
}

} // namespace entities
} // namespace core
} // namespace game
