#include "Data/Validators/DataValidator.h"
#include <iostream>
#include <filesystem>
#include <fstream>

namespace Shared::Data {

std::vector<std::string> DataValidator::errors_;

bool DataValidator::Validate(const DefinitionRegistry& registry) {
    errors_.clear();

    bool result = true;
    result &= ValidateEntities(registry);
    result &= ValidateSkills(registry);
    result &= ValidateStages(registry);
    result &= ValidateWaves(registry);
    result &= ValidateAbilities(registry);

    if (!result) {
        std::cerr << "Validation failed with " << errors_.size() << " error(s):" << std::endl;
        for (const auto& error : errors_) {
            std::cerr << "  - " << error << std::endl;
        }
    } else {
        std::cout << "Validation successful" << std::endl;
    }

    return result;
}

const std::vector<std::string>& DataValidator::GetErrors() {
    return errors_;
}

void DataValidator::ClearErrors() {
    errors_.clear();
}

bool DataValidator::ValidateEntities(const DefinitionRegistry& registry) {
    bool success = true;

    for (const auto* entity : registry.GetAllEntities()) {
        if (entity->id.empty()) {
            errors_.push_back("Entity has empty ID");
            success = false;
        }

        if (entity->name.empty()) {
            errors_.push_back("Entity '" + entity->id + "' has empty name");
            success = false;
        }

        // スキル参照のチェック
        for (const auto& skill_id : entity->skill_ids) {
            if (!registry.HasSkill(skill_id)) {
                errors_.push_back("Entity '" + entity->id + "' references non-existent skill '" + skill_id + "'");
                success = false;
            }
        }

        // アビリティ参照のチェック
        for (const auto& ability_id : entity->ability_ids) {
            if (!registry.HasAbility(ability_id)) {
                errors_.push_back("Entity '" + entity->id + "' references non-existent ability '" + ability_id + "'");
                success = false;
            }
        }

        // ステータスの妥当性チェック
        if (entity->stats.hp <= 0) {
            errors_.push_back("Entity '" + entity->id + "' has invalid HP: " + std::to_string(entity->stats.hp));
            success = false;
        }

        if (entity->stats.attack < 0) {
            errors_.push_back("Entity '" + entity->id + "' has negative attack: " + std::to_string(entity->stats.attack));
            success = false;
        }

        if (entity->stats.attack_speed <= 0.0f) {
            errors_.push_back("Entity '" + entity->id + "' has invalid attack_speed: " + std::to_string(entity->stats.attack_speed));
            success = false;
        }
    }

    return success;
}

bool DataValidator::ValidateSkills(const DefinitionRegistry& registry) {
    bool success = true;

    for (const auto* skill : registry.GetAllSkills()) {
        if (skill->id.empty()) {
            errors_.push_back("Skill has empty ID");
            success = false;
        }

        if (skill->name.empty()) {
            errors_.push_back("Skill '" + skill->id + "' has empty name");
            success = false;
        }

        // スキルタイプの検証
        if (skill->type != "passive" && skill->type != "interrupt" && skill->type != "event") {
            errors_.push_back("Skill '" + skill->id + "' has invalid type: " + skill->type);
            success = false;
        }

        // Event型の場合はトリガー検証
        if (skill->type == "event") {
            if (skill->event_trigger.event_type.empty()) {
                errors_.push_back("Event skill '" + skill->id + "' has empty event_type");
                success = false;
            }
        }

        // クールダウンの妥当性
        if (skill->cooldown < 0.0f) {
            errors_.push_back("Skill '" + skill->id + "' has negative cooldown: " + std::to_string(skill->cooldown));
            success = false;
        }

        // 発動確率の妥当性
        if (skill->activation_chance < 0.0f || skill->activation_chance > 1.0f) {
            errors_.push_back("Skill '" + skill->id + "' has invalid activation_chance: " + std::to_string(skill->activation_chance));
            success = false;
        }
    }

    return success;
}

bool DataValidator::ValidateStages(const DefinitionRegistry& registry) {
    bool success = true;

    for (const auto* stage : registry.GetAllStages()) {
        if (stage->id.empty()) {
            errors_.push_back("Stage has empty ID");
            success = false;
        }

        if (stage->name.empty()) {
            errors_.push_back("Stage '" + stage->id + "' has empty name");
            success = false;
        }

        // Wave参照のチェック
        for (const auto& wave_id : stage->wave_ids) {
            if (!registry.HasWave(wave_id)) {
                errors_.push_back("Stage '" + stage->id + "' references non-existent wave '" + wave_id + "'");
                success = false;
            }
        }

        // Castle HPの妥当性
        if (stage->castle_hp.player_castle_hp <= 0) {
            errors_.push_back("Stage '" + stage->id + "' has invalid player_castle_hp: " + std::to_string(stage->castle_hp.player_castle_hp));
            success = false;
        }

        if (stage->castle_hp.enemy_castle_hp <= 0) {
            errors_.push_back("Stage '" + stage->id + "' has invalid enemy_castle_hp: " + std::to_string(stage->castle_hp.enemy_castle_hp));
            success = false;
        }
    }

    return success;
}

bool DataValidator::ValidateWaves(const DefinitionRegistry& registry) {
    bool success = true;

    for (const auto* wave : registry.GetAllWaves()) {
        if (wave->id.empty()) {
            errors_.push_back("Wave has empty ID");
            success = false;
        }

        // SpawnGroup内のEntity参照チェック
        for (const auto& spawn_group : wave->spawn_groups) {
            if (!registry.HasEntity(spawn_group.entity_id)) {
                errors_.push_back("Wave '" + wave->id + "' references non-existent entity '" + spawn_group.entity_id + "'");
                success = false;
            }

            if (spawn_group.count <= 0) {
                errors_.push_back("Wave '" + wave->id + "' has invalid spawn count: " + std::to_string(spawn_group.count));
                success = false;
            }

            if (spawn_group.spawn_interval < 0.0f) {
                errors_.push_back("Wave '" + wave->id + "' has negative spawn_interval: " + std::to_string(spawn_group.spawn_interval));
                success = false;
            }
        }
    }

    return success;
}

bool DataValidator::ValidateAbilities(const DefinitionRegistry& registry) {
    bool success = true;

    for (const auto* ability : registry.GetAllAbilities()) {
        if (ability->id.empty()) {
            errors_.push_back("Ability has empty ID");
            success = false;
        }

        if (ability->name.empty()) {
            errors_.push_back("Ability '" + ability->id + "' has empty name");
            success = false;
        }

        // アビリティタイプの検証
        if (ability->type != "stat_boost" && ability->type != "special_effect") {
            errors_.push_back("Ability '" + ability->id + "' has invalid type: " + ability->type);
            success = false;
        }
    }

    return success;
}

bool DataValidator::ValidateEntityAgainstSchema(const nlohmann::json& entityJson, 
                                               const std::string& schemaPath) {
    errors_.clear();
    
    // スキーマファイルを読み込む
    if (!std::filesystem::exists(schemaPath)) {
        errors_.push_back("Schema file not found: " + schemaPath);
        return false;
    }
    
    try {
        std::ifstream schema_file(schemaPath);
        nlohmann::json schema = nlohmann::json::parse(schema_file);
        
        // 必須フィールドの確認
        if (schema.contains("required")) {
            for (const auto& required_field : schema["required"]) {
                if (!entityJson.contains(required_field)) {
                    errors_.push_back("Missing required field: " + required_field.get<std::string>());
                }
            }
        }
        
        // プロパティの妥当性確認
        if (schema.contains("properties")) {
            for (const auto& [key, value] : schema["properties"].items()) {
                if (entityJson.contains(key)) {
                    ValidateProperty(entityJson, schema, key, key);
                }
            }
        }
        
        // type フィールドの enum チェック
        if (entityJson.contains("type") && schema["properties"]["type"].contains("enum")) {
            auto allowed_types = schema["properties"]["type"]["enum"];
            auto actual_type = entityJson["type"].get<std::string>();
            bool type_valid = false;
            for (const auto& enum_val : allowed_types) {
                if (enum_val.get<std::string>() == actual_type) {
                    type_valid = true;
                    break;
                }
            }
            if (!type_valid) {
                errors_.push_back("Invalid type: " + actual_type + ". Allowed: main, sub, enemy");
            }
        }
        
        // rarity フィールドの range チェック
        if (entityJson.contains("rarity")) {
            auto rarity = entityJson["rarity"].get<int>();
            if (rarity < 1 || rarity > 5) {
                errors_.push_back("Rarity out of range: " + std::to_string(rarity) + " (expected 1-5)");
            }
        }
        
        // stats.hp チェック
        if (entityJson.contains("stats") && entityJson["stats"].contains("hp")) {
            auto hp = entityJson["stats"]["hp"].get<int>();
            if (hp <= 0) {
                errors_.push_back("Invalid HP: " + std::to_string(hp) + " (must be > 0)");
            }
        }
        
        return errors_.empty();
        
    } catch (const std::exception& e) {
        errors_.push_back(std::string("Schema validation error: ") + e.what());
        return false;
    }
}

void DataValidator::ValidateProperty(const nlohmann::json& data, const nlohmann::json& schema,
                                    const std::string& propName, const std::string& currentPath) {
    if (!schema.contains("properties") || !schema["properties"].contains(propName)) {
        return;
    }
    
    const auto& prop_schema = schema["properties"][propName];
    const auto& prop_value = data[propName];
    
    // 型チェック
    if (prop_schema.contains("type")) {
        std::string expected_type = prop_schema["type"].get<std::string>();
        if (expected_type == "string" && !prop_value.is_string()) {
            errors_.push_back(currentPath + ": expected string, got " + prop_value.type_name());
        } else if (expected_type == "integer" && !prop_value.is_number_integer()) {
            errors_.push_back(currentPath + ": expected integer, got " + prop_value.type_name());
        } else if (expected_type == "number" && !prop_value.is_number()) {
            errors_.push_back(currentPath + ": expected number, got " + prop_value.type_name());
        } else if (expected_type == "array" && !prop_value.is_array()) {
            errors_.push_back(currentPath + ": expected array, got " + prop_value.type_name());
        } else if (expected_type == "object" && !prop_value.is_object()) {
            errors_.push_back(currentPath + ": expected object, got " + prop_value.type_name());
        }
    }
    
    // 最小値・最大値チェック
    if (prop_schema.contains("minimum") && prop_value.is_number()) {
        if (prop_value.get<double>() < prop_schema["minimum"].get<double>()) {
            errors_.push_back(currentPath + ": value below minimum");
        }
    }
    
    if (prop_schema.contains("maximum") && prop_value.is_number()) {
        if (prop_value.get<double>() > prop_schema["maximum"].get<double>()) {
            errors_.push_back(currentPath + ": value above maximum");
        }
    }
}

} // namespace Shared::Data
