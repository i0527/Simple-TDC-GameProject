#pragma once

#include "Data/Definitions/EntityDef.h"
#include "Data/Definitions/SkillDef.h"
#include "Data/Definitions/StageDef.h"
#include "Data/Definitions/WaveDef.h"
#include "Data/Definitions/AbilityDef.h"
#include "Core/Signal.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Shared::Data {

/// @brief 全定義データの登録・管理クラス
/// @details エンティティ、スキル、ステージ、Wave、アビリティの定義を管理
class DefinitionRegistry {
public:
    DefinitionRegistry() = default;
    ~DefinitionRegistry() = default;

    // Entity
    bool RegisterEntity(const EntityDef& def);
    const EntityDef* GetEntity(const std::string& id) const;
    std::vector<const EntityDef*> GetAllEntities() const;
    bool HasEntity(const std::string& id) const;
    bool RemoveEntity(const std::string& id);

    // Skill
    bool RegisterSkill(const SkillDef& def);
    const SkillDef* GetSkill(const std::string& id) const;
    std::vector<const SkillDef*> GetAllSkills() const;
    bool HasSkill(const std::string& id) const;

    // Stage
    bool RegisterStage(const StageDef& def);
    const StageDef* GetStage(const std::string& id) const;
    std::vector<const StageDef*> GetAllStages() const;
    bool HasStage(const std::string& id) const;

    // Wave
    bool RegisterWave(const WaveDef& def);
    const WaveDef* GetWave(const std::string& id) const;
    std::vector<const WaveDef*> GetAllWaves() const;
    bool HasWave(const std::string& id) const;

    // Ability
    bool RegisterAbility(const AbilityDef& def);
    const AbilityDef* GetAbility(const std::string& id) const;
    std::vector<const AbilityDef*> GetAllAbilities() const;
    bool HasAbility(const std::string& id) const;

    // バリデーション
    bool ValidateAll();
    const std::vector<std::string>& GetErrors() const;
    void ClearErrors();

    // クリア
    void Clear();

    // HotReload
    Shared::Core::Signal<const std::string&> OnEntityDefinitionReloaded;
    Shared::Core::Signal<const std::string&> OnSkillDefinitionReloaded;
    Shared::Core::Signal<const std::string&> OnAbilityDefinitionReloaded;
    void OnFileChanged(const std::string& file_path);

private:
    std::unordered_map<std::string, EntityDef> entities_;
    std::unordered_map<std::string, SkillDef> skills_;
    std::unordered_map<std::string, StageDef> stages_;
    std::unordered_map<std::string, WaveDef> waves_;
    std::unordered_map<std::string, AbilityDef> abilities_;

    std::vector<std::string> validation_errors_;
};

} // namespace Shared::Data
