#pragma once

#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"
#include <memory>

namespace Game::Managers {

/// @brief スキル管理マネージャー
class SkillManager {
public:
    SkillManager(Shared::Core::GameContext& context,
                Shared::Data::DefinitionRegistry& definitions);
    ~SkillManager() = default;

    bool Initialize();
    void Shutdown();

    const Shared::Data::SkillDef* GetSkill(const std::string& id) const;
    std::vector<const Shared::Data::SkillDef*> GetAllSkills() const;

private:
    Shared::Core::GameContext& context_;
    Shared::Data::DefinitionRegistry& definitions_;

    void RegisterHotReloadCallback();
    void OnSkillsReloaded();
};

} // namespace Game::Managers
