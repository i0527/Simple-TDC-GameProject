#include "CharacterManager.hpp"
#include "../../utils/Log.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <fstream>

using json = nlohmann::json;

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
        // JSON からロード
        if (LoadFromJSON(json_path)) {
            return true;
        }
        // JSONロード失敗時はハードコード初期化にフォールバック
        LOG_WARN("JSON load failed, falling back to hardcoded data");
    }
    
    // ハードコード初期化（開発速度優先）
    InitializeHardcodedData();
    return true;
}

bool CharacterManager::LoadFromJSON(const std::string& json_path) {
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
            
            // 基本情報
            ch.id = ch_json["id"].get<std::string>();
            ch.name = ch_json["name"].get<std::string>();
            ch.rarity = ch_json["rarity"].get<int>();
            ch.description = ch_json.value("description", "");
            ch.rarity_name = ch_json.value("rarity_name", "");
            
            // ステータス
            ch.level = ch_json["status"]["level"].get<int>();
            ch.hp = ch_json["status"]["hp"].get<int>();
            ch.attack = ch_json["status"]["attack"].get<int>();
            ch.defense = ch_json["status"]["defense"].get<int>();
            ch.move_speed = ch_json["status"]["move_speed"].get<float>();
            ch.attack_span = ch_json["status"]["attack_span"].get<float>();
            
            // 攻撃設定
            std::string attack_type_str = ch_json["attack"]["type"].get<std::string>();
            if (attack_type_str == "single") {
                ch.attack_type = AttackType::Single;
            } else if (attack_type_str == "range") {
                ch.attack_type = AttackType::Range;
            } else if (attack_type_str == "line") {
                ch.attack_type = AttackType::Line;
            }
            
            ch.attack_size.x = ch_json["attack"]["size"][0].get<float>();
            ch.attack_size.y = ch_json["attack"]["size"][1].get<float>();
            
            std::string effect_str = ch_json["attack"]["effect_type"].get<std::string>();
            if (effect_str == "fire") ch.effect_type = EffectType::Fire;
            else if (effect_str == "ice") ch.effect_type = EffectType::Ice;
            else if (effect_str == "lightning") ch.effect_type = EffectType::Lightning;
            else if (effect_str == "heal") ch.effect_type = EffectType::Heal;
            else ch.effect_type = EffectType::Normal;
            
            // UIリソース
            ch.icon_path = ch_json["sprites"]["icon_path"].get<std::string>();
            
            // スプライト情報
            ch.move_sprite.sheet_path = ch_json["sprites"]["move"]["sheet_path"].get<std::string>();
            ch.move_sprite.frame_width = ch_json["sprites"]["move"]["frame_width"].get<int>();
            ch.move_sprite.frame_height = ch_json["sprites"]["move"]["frame_height"].get<int>();
            ch.move_sprite.frame_count = ch_json["sprites"]["move"]["frame_count"].get<int>();
            ch.move_sprite.frame_duration = ch_json["sprites"]["move"]["frame_duration"].get<float>();
            
            ch.attack_sprite.sheet_path = ch_json["sprites"]["attack"]["sheet_path"].get<std::string>();
            ch.attack_sprite.frame_width = ch_json["sprites"]["attack"]["frame_width"].get<int>();
            ch.attack_sprite.frame_height = ch_json["sprites"]["attack"]["frame_height"].get<int>();
            ch.attack_sprite.frame_count = ch_json["sprites"]["attack"]["frame_count"].get<int>();
            ch.attack_sprite.frame_duration = ch_json["sprites"]["attack"]["frame_duration"].get<float>();
            
            // パッシブスキル
            for (const auto& skill_json : ch_json.value("passive_skills", json::array())) {
                PassiveSkill skill;
                skill.id = skill_json["id"].get<std::string>();
                skill.name = skill_json["name"].get<std::string>();
                skill.description = skill_json.value("description", "");
                skill.value = skill_json.value("value", 0.0f);
                ch.passive_skills.push_back(skill);
            }
            
            // 装備
            for (const auto& eq_json : ch_json.value("equipment", json::array())) {
                Equipment eq;
                eq.id = eq_json["id"].get<std::string>();
                eq.name = eq_json["name"].get<std::string>();
                eq.description = eq_json.value("description", "");
                eq.attack_bonus = eq_json.value("attack_bonus", 0.0f);
                eq.defense_bonus = eq_json.value("defense_bonus", 0.0f);
                eq.hp_bonus = eq_json.value("hp_bonus", 0.0f);
                ch.equipment.push_back(eq);
            }
            
            // 図鑑情報
            ch.cost = ch_json.value("cost", 1);
            ch.is_discovered = ch_json.value("is_discovered", false);
            
            masters_[ch.id] = ch;
        }
        
        LOG_INFO("Loaded {} characters from JSON", masters_.size());
        return true;
    } catch (const json::parse_error& e) {
        LOG_ERROR("JSON parse error: {}", e.what());
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse character data: {}", e.what());
        return false;
    }
}

void CharacterManager::InitializeHardcodedData() {
    // 猫戦士
    Character cat;
    cat.id = "cat_001";
    cat.name = "勇敢な猫";
    cat.rarity = 4;
    cat.rarity_name = "SSR";
    cat.description = "勇敢でバランスの取れた猫戦士";
    cat.level = 1;
    cat.hp = 100;
    cat.attack = 80;
    cat.defense = 40;
    cat.move_speed = 150.0f;
    cat.attack_span = 1.5f;
    cat.attack_type = AttackType::Single;
    cat.attack_size = Vector2{ 80.0f, 20.0f };
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
    skill.name = "防御アップ";
    skill.description = "防御力が10%上昇";
    skill.value = 0.1f;
    cat.passive_skills.push_back(skill);
    
    Equipment eq;
    eq.id = "eq_sword_001";
    eq.name = "鋼の剣";
    eq.description = "標準的な鋼鉄製の剣。攻撃力が少し上がる。";
    eq.attack_bonus = 15.0f;
    eq.defense_bonus = 0.0f;
    eq.hp_bonus = 0.0f;
    cat.equipment.push_back(eq);
    
    // 図鑑情報
    cat.cost = 5;
    cat.is_discovered = true;
    
    masters_["cat_001"] = cat;
    
    // 犬戦士
    Character dog;
    dog.id = "dog_001";
    dog.name = "強気な犬";
    dog.rarity = 3;
    dog.rarity_name = "SR";
    dog.description = "攻撃型のキャラクター";
    dog.level = 1;
    dog.hp = 80;
    dog.attack = 100;
    dog.defense = 30;
    dog.move_speed = 180.0f;
    dog.attack_span = 1.2f;
    dog.attack_type = AttackType::Range;
    dog.attack_size = Vector2{ 120.0f, 50.0f };
    dog.effect_type = EffectType::Fire;
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
    
    // 図鑑情報
    dog.cost = 4;
    dog.is_discovered = true;
    
    masters_["dog_001"] = dog;
    
    LOG_INFO("Initialized {} hardcoded characters", masters_.size());
}

std::shared_ptr<Character> CharacterManager::GetCharacterTemplate(
    const std::string& character_id) {
    auto it = masters_.find(character_id);
    if (it == masters_.end()) {
        LOG_WARN("Character not found: {}", character_id);
        return nullptr;
    }
    
    // マスターデータをコピーして返す
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

bool CharacterManager::SetCharacterDiscovered(const std::string& character_id, bool discovered) {
    auto it = masters_.find(character_id);
    if (it == masters_.end()) {
        return false;
    }
    it->second.is_discovered = discovered;
    return true;
}

bool CharacterManager::SetCharacterLevel(const std::string& character_id, int level) {
    auto it = masters_.find(character_id);
    if (it == masters_.end()) {
        return false;
    }
    it->second.level = std::max(1, level);
    return true;
}

void CharacterManager::Shutdown() {
    masters_.clear();
}

} // namespace entities
} // namespace core
} // namespace game
