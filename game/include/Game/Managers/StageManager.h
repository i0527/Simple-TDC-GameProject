#pragma once

#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"
#include <memory>

namespace Game::Managers {

/// @brief ステージ管理マネージャー
class StageManager {
public:
    StageManager(Shared::Core::GameContext& context,
                Shared::Data::DefinitionRegistry& definitions);
    ~StageManager() = default;

    bool Initialize();
    void Shutdown();

    const Shared::Data::StageDef* GetStage(const std::string& id) const;
    std::vector<const Shared::Data::StageDef*> GetAllStages() const;

private:
    Shared::Core::GameContext& context_;
    Shared::Data::DefinitionRegistry& definitions_;

    void RegisterHotReloadCallback();
    void OnStagesReloaded();
};

} // namespace Game::Managers
