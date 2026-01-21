#include "ItemPassiveLoader.hpp"

// 標準ライブラリ
#include <algorithm>
#include <cctype>
#include <fstream>
#include <vector>

// 外部ライブラリ
#include <nlohmann/json.hpp>

// プロジェクト�E
#include "../../../utils/Log.h"

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
    LOG_WARN("ItemPassiveLoader: Unknown effect_type '{}', fallback to 'percentage'", raw);
    return PassiveEffectType::Percentage;
}

PassiveTargetStat ParseTargetStat(const json& j) {
    const std::string raw = j.value("target_stat", "attack");
    const std::string s = ToLowerCopy(raw);
    if (s == "attack" || s == "atk") return PassiveTargetStat::Attack;
    if (s == "defense" || s == "def") return PassiveTargetStat::Defense;
    if (s == "hp" || s == "max_hp") return PassiveTargetStat::Hp;
    if (s == "move_speed" || s == "speed" || s == "spd")
        return PassiveTargetStat::MoveSpeed;
    if (s == "attack_speed" || s == "atk_speed" || s == "as")
        return PassiveTargetStat::AttackSpeed;
    if (s == "range") return PassiveTargetStat::Range;
    if (s == "crit_chance" || s == "crit") return PassiveTargetStat::CritChance;
    if (s == "crit_damage" || s == "crit_dmg")
        return PassiveTargetStat::CritDamage;
    if (s == "gold_gain" || s == "gold") return PassiveTargetStat::GoldGain;
    if (s == "exp_gain" || s == "exp") return PassiveTargetStat::ExpGain;
    LOG_WARN("ItemPassiveLoader: Unknown target_stat '{}', fallback to 'attack'", raw);
    return PassiveTargetStat::Attack;
}

const char* EffectTypeToString(PassiveEffectType type) {
    switch (type) {
    case PassiveEffectType::Flat:
        return "flat";
    case PassiveEffectType::Percentage:
    default:
        return "percentage";
    }
}

const char* TargetStatToString(PassiveTargetStat stat) {
    switch (stat) {
    case PassiveTargetStat::Attack:
        return "attack";
    case PassiveTargetStat::Defense:
        return "defense";
    case PassiveTargetStat::Hp:
        return "hp";
    case PassiveTargetStat::MoveSpeed:
        return "move_speed";
    case PassiveTargetStat::AttackSpeed:
        return "attack_speed";
    case PassiveTargetStat::Range:
        return "range";
    case PassiveTargetStat::CritChance:
        return "crit_chance";
    case PassiveTargetStat::CritDamage:
        return "crit_damage";
    case PassiveTargetStat::GoldGain:
        return "gold_gain";
    case PassiveTargetStat::ExpGain:
        return "exp_gain";
    default:
        return "attack";
    }
}

bool WriteBackupFile(const std::string& jsonPath) {
    std::ifstream in(jsonPath, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }
    const std::string backupPath = jsonPath + ".bak";
    std::ofstream out(backupPath, std::ios::binary);
    if (!out.is_open()) {
        return false;
    }
    out << in.rdbuf();
    return true;
}

} // namespace

bool ItemPassiveLoader::LoadFromJSON(
    const std::string& json_path,
    std::unordered_map<std::string, PassiveSkill>& outPassives,
    std::unordered_map<std::string, Equipment>& outEquipment) {
    outPassives.clear();
    outEquipment.clear();

    try {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            LOG_ERROR("ItemPassiveLoader: Failed to open file: {}", json_path);
            return false;
        }

        json data;
        file >> data;

        // パッシブスキルローチE
        if (data.contains("passive_skills")) {
            for (const auto& skill_json : data["passive_skills"]) {
                PassiveSkill skill;
                skill.id = skill_json.at("id").get<std::string>();
                skill.name = skill_json.at("name").get<std::string>();
                skill.description = skill_json.value("description", "");
                skill.value = skill_json.value("value", 0.0f);
                skill.effect_type = ParseEffectType(skill_json);
                skill.target_stat = ParseTargetStat(skill_json);
                outPassives[skill.id] = skill;
            }
        }

        // 裁E��アイチE��ローチE
        if (data.contains("equipment")) {
            for (const auto& eq_json : data["equipment"]) {
                Equipment eq;
                eq.id = eq_json.at("id").get<std::string>();
                eq.name = eq_json.at("name").get<std::string>();
                eq.description = eq_json.value("description", "");
                eq.icon_path = eq_json.value("icon_path", "");
                eq.attack_bonus = eq_json.value("attack_bonus", 0.0f);
                eq.defense_bonus = eq_json.value("defense_bonus", 0.0f);
                eq.hp_bonus = eq_json.value("hp_bonus", 0.0f);
                outEquipment[eq.id] = eq;
            }
        }

        LOG_INFO("ItemPassiveLoader: Loaded {} passives and {} equipment from JSON",
                 outPassives.size(), outEquipment.size());
        return true;
    } catch (const json::parse_error& e) {
        LOG_ERROR("ItemPassiveLoader: JSON parse error: {}", e.what());
    } catch (const std::exception& e) {
        LOG_ERROR("ItemPassiveLoader: Exception: {}", e.what());
    }
    return false;
}

bool ItemPassiveLoader::SaveToJSON(
    const std::string& json_path,
    const std::unordered_map<std::string, PassiveSkill>& passives,
    const std::unordered_map<std::string, Equipment>& equipment) {
    try {
        json data;
        data["passive_skills"] = json::array();
        data["equipment"] = json::array();

        std::vector<std::string> passiveIds;
        passiveIds.reserve(passives.size());
        for (const auto& [id, _] : passives) {
            passiveIds.push_back(id);
        }
        std::sort(passiveIds.begin(), passiveIds.end());

        for (const auto& id : passiveIds) {
            const auto& skill = passives.at(id);
            json skill_json;
            skill_json["id"] = skill.id;
            skill_json["name"] = skill.name;
            skill_json["description"] = skill.description;
            skill_json["value"] = skill.value;
            skill_json["effect_type"] = EffectTypeToString(skill.effect_type);
            skill_json["target_stat"] = TargetStatToString(skill.target_stat);
            data["passive_skills"].push_back(skill_json);
        }

        std::vector<std::string> equipmentIds;
        equipmentIds.reserve(equipment.size());
        for (const auto& [id, _] : equipment) {
            equipmentIds.push_back(id);
        }
        std::sort(equipmentIds.begin(), equipmentIds.end());

        for (const auto& id : equipmentIds) {
            const auto& eq = equipment.at(id);
            json eq_json;
            eq_json["id"] = eq.id;
            eq_json["name"] = eq.name;
            eq_json["description"] = eq.description;
            eq_json["icon_path"] = eq.icon_path;
            eq_json["attack_bonus"] = eq.attack_bonus;
            eq_json["defense_bonus"] = eq.defense_bonus;
            eq_json["hp_bonus"] = eq.hp_bonus;
            data["equipment"].push_back(eq_json);
        }

        WriteBackupFile(json_path);
        std::ofstream file(json_path);
        if (!file.is_open()) {
            LOG_ERROR("ItemPassiveLoader: Failed to open file for write: {}", json_path);
            return false;
        }
        file << data.dump(2);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("ItemPassiveLoader: Failed to write JSON: {}", e.what());
        return false;
    }
}

void ItemPassiveLoader::LoadHardcoded(
    std::unordered_map<std::string, PassiveSkill>& outPassives,
    std::unordered_map<std::string, Equipment>& outEquipment) {
    outPassives.clear();
    outEquipment.clear();

    // パッシブスキル初期チE�Eタ
    {
        PassiveSkill s;
        s.id = "skill_atk_up_1";
        s.name = "攻撁E��チE�E I";
        s.description = "攻撃力が5%上昇する。";
        s.value = 0.05f;
        s.effect_type = PassiveEffectType::Percentage;
        s.target_stat = PassiveTargetStat::Attack;
        outPassives[s.id] = s;
    }
    {
        PassiveSkill s;
        s.id = "skill_def_up_1";
        s.name = "防御アチE�E I";
        s.description = "防御力が10%上昇する。";
        s.value = 0.10f;
        s.effect_type = PassiveEffectType::Percentage;
        s.target_stat = PassiveTargetStat::Defense;
        outPassives[s.id] = s;
    }

    // 裁E��アイチE��初期チE�Eタ
    {
        Equipment e;
        e.id = "eq_sword_001";
        e.name = "鋼の剣";
        e.description = "標準的な鋼の剣。攻撃力が少し上がる。";
        e.attack_bonus = 15.0f;
        e.defense_bonus = 0.0f;
        e.hp_bonus = 0.0f;
        outEquipment[e.id] = e;
    }
    {
        Equipment e;
        e.id = "eq_shield_001";
        e.name = "木の盾";
        e.description = "木製の簡素な盾。防御力が上がる。";
        e.attack_bonus = 0.0f;
        e.defense_bonus = 12.0f;
        e.hp_bonus = 0.0f;
        outEquipment[e.id] = e;
    }
}

} // namespace entities
} // namespace core
} // namespace game
