#pragma once

#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"
#include <memory>

namespace Game::Managers {

/// @brief エンティティ（キャラ・敵）管理マネージャー
class EntityManager {
public:
    EntityManager(Shared::Core::GameContext& context,
                 Shared::Data::DefinitionRegistry& definitions);
    ~EntityManager() = default;

    bool Initialize();
    void Shutdown();

    const Shared::Data::EntityDef* GetEntity(const std::string& id) const;
    std::vector<const Shared::Data::EntityDef*> GetAllEntities() const;
    std::vector<const Shared::Data::EntityDef*> GetPlayerEntities() const;
    std::vector<const Shared::Data::EntityDef*> GetEnemyEntities() const;

private:
    Shared::Core::GameContext& context_;
    Shared::Data::DefinitionRegistry& definitions_;

    void RegisterHotReloadCallback();
    void OnEntitiesReloaded();
};

} // namespace Game::Managers
