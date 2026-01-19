#include "CharacterLoader.hpp"

// 標準ライブラリ
#include <algorithm>
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

const char* AttackTypeToString(AttackType type) {
    switch (type) {
    case AttackType::Single:
        return "single";
    case AttackType::Range:
        return "range";
    case AttackType::Line:
        return "line";
    default:
        return "single";
    }
}

const char* EffectTypeToString(EffectType type) {
    switch (type) {
    case EffectType::Fire:
        return "fire";
    case EffectType::Ice:
        return "ice";
    case EffectType::Lightning:
        return "lightning";
    case EffectType::Heal:
        return "heal";
    case EffectType::Normal:
    default:
        return "normal";
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

bool CharacterLoader::LoadFromJSON(
    const std::string& json_path,
    std::unordered_map<std::string, Character>& outMasters) {
    outMasters.clear();
    try {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open character data file: {}", json_path);
            return false;
        }

        json data;
        file >> data;

        for (const auto& ch_json : data["characters"]) {
            Character ch;

            // 基本惁E��
            ch.id = ch_json["id"].get<std::string>();
            ch.name = ch_json["name"].get<std::string>();
            ch.rarity = ch_json["rarity"].get<int>();
            ch.description = ch_json.value("description", "");
            ch.rarity_name = ch_json.value("rarity_name", "");

            // スチE�Eタス
            ch.default_level = ch_json["status"]["level"].get<int>();
            ch.hp = ch_json["status"]["hp"].get<int>();
            ch.attack = ch_json["status"]["attack"].get<int>();
            ch.defense = ch_json["status"]["defense"].get<int>();
            ch.move_speed = ch_json["status"]["move_speed"].get<float>();
            ch.attack_span = ch_json["status"]["attack_span"].get<float>();

            // 攻撁E��宁E
            std::string attack_type_str =
                ch_json["attack"]["type"].get<std::string>();
            if (attack_type_str == "single") {
                ch.attack_type = AttackType::Single;
            } else if (attack_type_str == "range") {
                ch.attack_type = AttackType::Range;
            } else if (attack_type_str == "line") {
                ch.attack_type = AttackType::Line;
            }

            ch.attack_size.x = ch_json["attack"]["size"][0].get<float>();
            ch.attack_size.y = ch_json["attack"]["size"][1].get<float>();
            ch.attack_hit_time = ch_json["attack"].value("hit_time", 0.0f);

            std::string effect_str =
                ch_json["attack"]["effect_type"].get<std::string>();
            if (effect_str == "fire") ch.effect_type = EffectType::Fire;
            else if (effect_str == "ice") ch.effect_type = EffectType::Ice;
            else if (effect_str == "lightning")
                ch.effect_type = EffectType::Lightning;
            else if (effect_str == "heal") ch.effect_type = EffectType::Heal;
            else ch.effect_type = EffectType::Normal;

            // UIリソース
            ch.icon_path = ch_json["sprites"]["icon_path"].get<std::string>();

            // スプライト情報
            ch.move_sprite.sheet_path =
                ch_json["sprites"]["move"]["sheet_path"].get<std::string>();
            ch.move_sprite.frame_width =
                ch_json["sprites"]["move"]["frame_width"].get<int>();
            ch.move_sprite.frame_height =
                ch_json["sprites"]["move"]["frame_height"].get<int>();
            ch.move_sprite.frame_count =
                ch_json["sprites"]["move"]["frame_count"].get<int>();
            ch.move_sprite.frame_duration =
                ch_json["sprites"]["move"]["frame_duration"].get<float>();

            ch.attack_sprite.sheet_path =
                ch_json["sprites"]["attack"]["sheet_path"].get<std::string>();
            ch.attack_sprite.frame_width =
                ch_json["sprites"]["attack"]["frame_width"].get<int>();
            ch.attack_sprite.frame_height =
                ch_json["sprites"]["attack"]["frame_height"].get<int>();
            ch.attack_sprite.frame_count =
                ch_json["sprites"]["attack"]["frame_count"].get<int>();
            ch.attack_sprite.frame_duration =
                ch_json["sprites"]["attack"]["frame_duration"].get<float>();

            // パッシブスキル
            for (const auto& skill_json :
                 ch_json.value("passive_skills", json::array())) {
                PassiveSkill skill;
                skill.id = skill_json["id"].get<std::string>();
                skill.name = skill_json["name"].get<std::string>();
                skill.description = skill_json.value("description", "");
                skill.value = skill_json.value("value", 0.0f);
                ch.default_passive_skills.push_back(skill);
            }

            // 裁E��
            for (const auto& eq_json :
                 ch_json.value("equipment", json::array())) {
                Equipment eq;
                eq.id = eq_json["id"].get<std::string>();
                eq.name = eq_json["name"].get<std::string>();
                eq.description = eq_json.value("description", "");
                eq.attack_bonus = eq_json.value("attack_bonus", 0.0f);
                eq.defense_bonus = eq_json.value("defense_bonus", 0.0f);
                eq.hp_bonus = eq_json.value("hp_bonus", 0.0f);
                ch.default_equipment.push_back(eq);
            }

            // 図鑑情報
            ch.cost = ch_json.value("cost", 1);
            ch.default_unlocked = ch_json.value("is_discovered", false);

            outMasters[ch.id] = ch;
        }

        LOG_INFO("Loaded {} characters from JSON", outMasters.size());
        return true;
    } catch (const json::parse_error& e) {
        LOG_ERROR("JSON parse error: {}", e.what());
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse character data: {}", e.what());
        return false;
    }
}

bool CharacterLoader::SaveToJSON(
    const std::string& json_path,
    const std::unordered_map<std::string, Character>& masters) {
    try {
        json data;
        data["characters"] = json::array();

        std::vector<std::string> ids;
        ids.reserve(masters.size());
        for (const auto& [id, _] : masters) {
            ids.push_back(id);
        }
        std::sort(ids.begin(), ids.end());

        for (const auto& id : ids) {
            const auto& ch = masters.at(id);
            json ch_json;
            ch_json["id"] = ch.id;
            ch_json["name"] = ch.name;
            ch_json["description"] = ch.description;
            ch_json["rarity"] = ch.rarity;
            ch_json["rarity_name"] = ch.rarity_name;

            ch_json["status"] = {
                {"level", ch.default_level},
                {"hp", ch.hp},
                {"attack", ch.attack},
                {"defense", ch.defense},
                {"move_speed", ch.move_speed},
                {"attack_span", ch.attack_span}
            };

            ch_json["attack"] = {
                {"type", AttackTypeToString(ch.attack_type)},
                {"size", {ch.attack_size.x, ch.attack_size.y}},
                {"effect_type", EffectTypeToString(ch.effect_type)},
                {"hit_time", ch.attack_hit_time}
            };

            ch_json["sprites"] = {
                {"icon_path", ch.icon_path},
                {"move", {
                    {"sheet_path", ch.move_sprite.sheet_path},
                    {"frame_width", ch.move_sprite.frame_width},
                    {"frame_height", ch.move_sprite.frame_height},
                    {"frame_count", ch.move_sprite.frame_count},
                    {"frame_duration", ch.move_sprite.frame_duration}
                }},
                {"attack", {
                    {"sheet_path", ch.attack_sprite.sheet_path},
                    {"frame_width", ch.attack_sprite.frame_width},
                    {"frame_height", ch.attack_sprite.frame_height},
                    {"frame_count", ch.attack_sprite.frame_count},
                    {"frame_duration", ch.attack_sprite.frame_duration}
                }}
            };

            ch_json["passive_skills"] = json::array();
            for (const auto& skill : ch.default_passive_skills) {
                json skill_json;
                skill_json["id"] = skill.id;
                skill_json["name"] = skill.name;
                skill_json["description"] = skill.description;
                skill_json["value"] = skill.value;
                ch_json["passive_skills"].push_back(skill_json);
            }

            ch_json["equipment"] = json::array();
            for (const auto& eq : ch.default_equipment) {
                json eq_json;
                eq_json["id"] = eq.id;
                eq_json["name"] = eq.name;
                eq_json["description"] = eq.description;
                eq_json["attack_bonus"] = eq.attack_bonus;
                eq_json["defense_bonus"] = eq.defense_bonus;
                eq_json["hp_bonus"] = eq.hp_bonus;
                ch_json["equipment"].push_back(eq_json);
            }

            ch_json["cost"] = ch.cost;
            ch_json["is_discovered"] = ch.default_unlocked;

            data["characters"].push_back(ch_json);
        }

        WriteBackupFile(json_path);
        std::ofstream file(json_path);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open character data file for write: {}", json_path);
            return false;
        }
        file << data.dump(2);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to write character data: {}", e.what());
        return false;
    }
}

void CharacterLoader::LoadHardcoded(
    std::unordered_map<std::string, Character>& outMasters) {
    outMasters.clear();

    // 猫戦士
    Character cat;
    cat.id = "cat_001";
    cat.name = "勁E��な猫";
    cat.rarity = 4;
    cat.rarity_name = "SSR";
    cat.description = "勁E��でバランスの取れた猫戦士";
    cat.default_level = 1;
    cat.hp = 100;
    cat.attack = 80;
    cat.defense = 40;
    cat.move_speed = 150.0f;
    cat.attack_span = 1.5f;
    cat.attack_type = AttackType::Single;
    cat.attack_size = Vector2{80.0f, 20.0f};
    cat.effect_type = EffectType::Normal;
    cat.icon_path = "assets/icons/cat_001.png";
    cat.move_sprite.sheet_path = "assets/sprites/cat_001/move.png";
    cat.move_sprite.frame_width = 64;
    cat.move_sprite.frame_height = 64;
    cat.move_sprite.frame_count = 8;
    cat.move_sprite.frame_duration = 0.1f;
    cat.attack_sprite.sheet_path = "assets/sprites/cat_001/attack.png";
    cat.attack_sprite.frame_width = 80;
    cat.attack_sprite.frame_height = 80;
    cat.attack_sprite.frame_count = 6;
    cat.attack_sprite.frame_duration = 0.08f;

    PassiveSkill skill;
    skill.id = "skill_defense_up";
    skill.name = "防御アチE�E";
    skill.description = "防御力が10%上�E";
    skill.value = 0.1f;
    cat.default_passive_skills.push_back(skill);

    Equipment eq;
    eq.id = "eq_sword_001";
    eq.name = "鋼の剣";
    eq.description = "標準的な鋼の剣。攻撃力が少し上がる。";
    eq.attack_bonus = 15.0f;
    eq.defense_bonus = 0.0f;
    eq.hp_bonus = 0.0f;
    cat.default_equipment.push_back(eq);

    // 図鑑情報
    cat.cost = 5;
    cat.default_unlocked = true;

    outMasters["cat_001"] = cat;

    // 犬戦士
    Character dog;
    dog.id = "dog_001";
    dog.name = "強気な犬";
    dog.rarity = 3;
    dog.rarity_name = "SR";
    dog.description = "攻撁E��のキャラクター";
    dog.default_level = 1;
    dog.hp = 80;
    dog.attack = 100;
    dog.defense = 20;
    dog.move_speed = 170.0f;
    dog.attack_span = 1.2f;
    dog.attack_type = AttackType::Single;
    dog.attack_size = Vector2{70.0f, 20.0f};
    dog.effect_type = EffectType::Normal;
    dog.icon_path = "assets/icons/dog_001.png";
    dog.move_sprite.sheet_path = "assets/sprites/dog_001/move.png";
    dog.move_sprite.frame_width = 64;
    dog.move_sprite.frame_height = 64;
    dog.move_sprite.frame_count = 8;
    dog.move_sprite.frame_duration = 0.1f;
    dog.attack_sprite.sheet_path = "assets/sprites/dog_001/attack.png";
    dog.attack_sprite.frame_width = 80;
    dog.attack_sprite.frame_height = 80;
    dog.attack_sprite.frame_count = 6;
    dog.attack_sprite.frame_duration = 0.08f;

    PassiveSkill skill2;
    skill2.id = "skill_atk_up";
    skill2.name = "攻撁E��チE�E";
    skill2.description = "攻撁E��ぁE0%上�E";
    skill2.value = 0.1f;
    dog.default_passive_skills.push_back(skill2);

    Equipment eq2;
    eq2.id = "eq_shield_001";
    eq2.name = "木の盾";
    eq2.description = "木製の簡素な盾。防御力が少し上がる。";
    eq2.attack_bonus = 0.0f;
    eq2.defense_bonus = 12.0f;
    eq2.hp_bonus = 0.0f;
    dog.default_equipment.push_back(eq2);

    // 図鑑情報
    dog.cost = 4;
    dog.default_unlocked = false;

    outMasters["dog_001"] = dog;
}

} // namespace entities
} // namespace core
} // namespace game
