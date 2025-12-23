#include "Data/DefinitionRegistry.h"
#include "Data/Loaders/AbilityLoader.h"
#include "Data/Loaders/EntityLoader.h"
#include "Data/Loaders/SkillLoader.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>


namespace Shared::Data {

// ========== Entity ==========

bool DefinitionRegistry::RegisterEntity(const EntityDef &def) {
  if (def.id.empty()) {
    std::cerr << "Error: Entity ID is empty" << std::endl;
    return false;
  }

  if (entities_.find(def.id) != entities_.end()) {
    std::cerr << "Warning: Entity ID '" << def.id
              << "' already exists. Overwriting." << std::endl;
  }

  entities_[def.id] = def;
  return true;
}

const EntityDef *DefinitionRegistry::GetEntity(const std::string &id) const {
  auto it = entities_.find(id);
  if (it == entities_.end()) {
    return nullptr;
  }
  return &it->second;
}

std::vector<const EntityDef *> DefinitionRegistry::GetAllEntities() const {
  std::vector<const EntityDef *> result;
  result.reserve(entities_.size());
  for (const auto &pair : entities_) {
    result.push_back(&pair.second);
  }
  return result;
}

bool DefinitionRegistry::HasEntity(const std::string &id) const {
  return entities_.find(id) != entities_.end();
}

bool DefinitionRegistry::RemoveEntity(const std::string &id) {
  auto it = entities_.find(id);
  if (it == entities_.end()) {
    return false;
  }
  entities_.erase(it);
  return true;
}

// ========== Skill ==========

bool DefinitionRegistry::RegisterSkill(const SkillDef &def) {
  if (def.id.empty()) {
    std::cerr << "Error: Skill ID is empty" << std::endl;
    return false;
  }

  if (skills_.find(def.id) != skills_.end()) {
    std::cerr << "Warning: Skill ID '" << def.id
              << "' already exists. Overwriting." << std::endl;
  }

  skills_[def.id] = def;
  return true;
}

const SkillDef *DefinitionRegistry::GetSkill(const std::string &id) const {
  auto it = skills_.find(id);
  if (it == skills_.end()) {
    return nullptr;
  }
  return &it->second;
}

std::vector<const SkillDef *> DefinitionRegistry::GetAllSkills() const {
  std::vector<const SkillDef *> result;
  result.reserve(skills_.size());
  for (const auto &pair : skills_) {
    result.push_back(&pair.second);
  }
  return result;
}

bool DefinitionRegistry::HasSkill(const std::string &id) const {
  return skills_.find(id) != skills_.end();
}

// ========== Stage ==========

bool DefinitionRegistry::RegisterStage(const StageDef &def) {
  if (def.id.empty()) {
    std::cerr << "Error: Stage ID is empty" << std::endl;
    return false;
  }

  if (stages_.find(def.id) != stages_.end()) {
    std::cerr << "Warning: Stage ID '" << def.id
              << "' already exists. Overwriting." << std::endl;
  }

  stages_[def.id] = def;
  return true;
}

const StageDef *DefinitionRegistry::GetStage(const std::string &id) const {
  auto it = stages_.find(id);
  if (it == stages_.end()) {
    return nullptr;
  }
  return &it->second;
}

std::vector<const StageDef *> DefinitionRegistry::GetAllStages() const {
  std::vector<const StageDef *> result;
  result.reserve(stages_.size());
  for (const auto &pair : stages_) {
    result.push_back(&pair.second);
  }
  return result;
}

bool DefinitionRegistry::HasStage(const std::string &id) const {
  return stages_.find(id) != stages_.end();
}

// ========== Wave ==========

bool DefinitionRegistry::RegisterWave(const WaveDef &def) {
  if (def.id.empty()) {
    std::cerr << "Error: Wave ID is empty" << std::endl;
    return false;
  }

  if (waves_.find(def.id) != waves_.end()) {
    std::cerr << "Warning: Wave ID '" << def.id
              << "' already exists. Overwriting." << std::endl;
  }

  waves_[def.id] = def;
  return true;
}

const WaveDef *DefinitionRegistry::GetWave(const std::string &id) const {
  auto it = waves_.find(id);
  if (it == waves_.end()) {
    return nullptr;
  }
  return &it->second;
}

std::vector<const WaveDef *> DefinitionRegistry::GetAllWaves() const {
  std::vector<const WaveDef *> result;
  result.reserve(waves_.size());
  for (const auto &pair : waves_) {
    result.push_back(&pair.second);
  }
  return result;
}

bool DefinitionRegistry::HasWave(const std::string &id) const {
  return waves_.find(id) != waves_.end();
}

// ========== Ability ==========

bool DefinitionRegistry::RegisterAbility(const AbilityDef &def) {
  if (def.id.empty()) {
    std::cerr << "Error: Ability ID is empty" << std::endl;
    return false;
  }

  if (abilities_.find(def.id) != abilities_.end()) {
    std::cerr << "Warning: Ability ID '" << def.id
              << "' already exists. Overwriting." << std::endl;
  }

  abilities_[def.id] = def;
  return true;
}

const AbilityDef *DefinitionRegistry::GetAbility(const std::string &id) const {
  auto it = abilities_.find(id);
  if (it == abilities_.end()) {
    return nullptr;
  }
  return &it->second;
}

std::vector<const AbilityDef *> DefinitionRegistry::GetAllAbilities() const {
  std::vector<const AbilityDef *> result;
  result.reserve(abilities_.size());
  for (const auto &pair : abilities_) {
    result.push_back(&pair.second);
  }
  return result;
}

bool DefinitionRegistry::HasAbility(const std::string &id) const {
  return abilities_.find(id) != abilities_.end();
}

// ========== Validation ==========

bool DefinitionRegistry::ValidateAll() {
  validation_errors_.clear();

  // Entity の参照チェック
  for (const auto &[entity_id, entity] : entities_) {
    for (const auto &skill_id : entity.skill_ids) {
      if (!HasSkill(skill_id)) {
        validation_errors_.push_back("Entity '" + entity_id +
                                     "' references non-existent skill '" +
                                     skill_id + "'");
      }
    }
    for (const auto &ability_id : entity.ability_ids) {
      if (!HasAbility(ability_id)) {
        validation_errors_.push_back("Entity '" + entity_id +
                                     "' references non-existent ability '" +
                                     ability_id + "'");
      }
    }
  }

  // Stage の参照チェック
  for (const auto &[stage_id, stage] : stages_) {
    for (const auto &wave_id : stage.wave_ids) {
      if (!HasWave(wave_id)) {
        validation_errors_.push_back("Stage '" + stage_id +
                                     "' references non-existent wave '" +
                                     wave_id + "'");
      }
    }
  }

  // Wave の参照チェック
  for (const auto &[wave_id, wave] : waves_) {
    for (const auto &spawn_group : wave.spawn_groups) {
      if (!HasEntity(spawn_group.entity_id)) {
        validation_errors_.push_back("Wave '" + wave_id +
                                     "' references non-existent entity '" +
                                     spawn_group.entity_id + "'");
      }
    }
  }

  if (!validation_errors_.empty()) {
    std::cerr << "Validation failed with " << validation_errors_.size()
              << " error(s):" << std::endl;
    for (const auto &error : validation_errors_) {
      std::cerr << "  - " << error << std::endl;
    }
    return false;
  }

  std::cout << "Validation successful" << std::endl;
  return true;
}

const std::vector<std::string> &DefinitionRegistry::GetErrors() const {
  return validation_errors_;
}

void DefinitionRegistry::ClearErrors() { validation_errors_.clear(); }

void DefinitionRegistry::Clear() {
  entities_.clear();
  skills_.clear();
  stages_.clear();
  waves_.clear();
  abilities_.clear();
  validation_errors_.clear();
}

void DefinitionRegistry::OnFileChanged(const std::string &file_path) {
  namespace fs = std::filesystem;

  try {
    // ファイルが存在するか確認
    if (!fs::exists(file_path)) {
      std::cerr << "File not found: " << file_path << std::endl;
      return;
    }

    // ファイルパスから定義タイプを判定して再ロード
    std::string lower_path = file_path;
    std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(),
             [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (lower_path.find("entities") != std::string::npos) {
      // Entity定義の再ロード
      if (EntityLoader::LoadFromJson(file_path, *this)) {
        // ロードされた全エンティティIDに対してSignalを発行
        for (const auto *entity : GetAllEntities()) {
          OnEntityDefinitionReloaded.Emit(entity->id);
        }
        std::cout << "Entity definitions reloaded from: " << file_path
                  << std::endl;
      } else {
        std::cerr << "Failed to reload entity definitions from: " << file_path
                  << std::endl;
      }
    } else if (lower_path.find("skills") != std::string::npos) {
      // Skill定義の再ロード
      if (SkillLoader::LoadFromJson(file_path, *this)) {
        // ロードされた全スキルIDに対してSignalを発行
        for (const auto *skill : GetAllSkills()) {
          OnSkillDefinitionReloaded.Emit(skill->id);
        }
        std::cout << "Skill definitions reloaded from: " << file_path
                  << std::endl;
      } else {
        std::cerr << "Failed to reload skill definitions from: " << file_path
                  << std::endl;
      }
    } else if (lower_path.find("abilities") != std::string::npos) {
      // Ability定義の再ロード
      if (AbilityLoader::LoadFromJson(file_path, *this)) {
        // ロードされた全アビリティIDに対してSignalを発行
        for (const auto *ability : GetAllAbilities()) {
          OnAbilityDefinitionReloaded.Emit(ability->id);
        }
        std::cout << "Ability definitions reloaded from: " << file_path
                  << std::endl;
      } else {
        std::cerr << "Failed to reload ability definitions from: " << file_path
                  << std::endl;
      }
    } else {
      std::cout << "Unknown file type for hot reload: " << file_path
                << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error in OnFileChanged: " << e.what() << std::endl;
  }
}

} // namespace Shared::Data
