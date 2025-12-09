#include "Data/Validators/DataValidator.h"
#include <iostream>

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

} // namespace Shared::Data
