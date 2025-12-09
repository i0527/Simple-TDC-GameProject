#include "Game/Managers/EntityManager.h"
#include "Data/Loaders/EntityLoader.h"
#include <iostream>

namespace Game::Managers {

EntityManager::EntityManager(Shared::Core::GameContext& context,
                           Shared::Data::DefinitionRegistry& definitions)
    : context_(context), definitions_(definitions) {}

bool EntityManager::Initialize() {
    RegisterHotReloadCallback();
    std::cout << "EntityManager initialized" << std::endl;
    return true;
}

void EntityManager::Shutdown() {
    std::cout << "EntityManager shutdown" << std::endl;
}

const Shared::Data::EntityDef* EntityManager::GetEntity(const std::string& id) const {
    return definitions_.GetEntity(id);
}

std::vector<const Shared::Data::EntityDef*> EntityManager::GetAllEntities() const {
    return definitions_.GetAllEntities();
}

std::vector<const Shared::Data::EntityDef*> EntityManager::GetPlayerEntities() const {
    std::vector<const Shared::Data::EntityDef*> result;
    for (const auto* entity : definitions_.GetAllEntities()) {
        if (!entity->is_enemy) {
            result.push_back(entity);
        }
    }
    return result;
}

std::vector<const Shared::Data::EntityDef*> EntityManager::GetEnemyEntities() const {
    std::vector<const Shared::Data::EntityDef*> result;
    for (const auto* entity : definitions_.GetAllEntities()) {
        if (entity->is_enemy) {
            result.push_back(entity);
        }
    }
    return result;
}

void EntityManager::RegisterHotReloadCallback() {
    // エンティティ定義ファイルの監視
    std::string entity_path = context_.GetDataPath("entities_debug.json");
    
    context_.GetFileWatcher().Watch(entity_path, [this]() {
        OnEntitiesReloaded();
    });
}

void EntityManager::OnEntitiesReloaded() {
    std::cout << "Entities reloaded" << std::endl;

    // JSON 再ロード
    std::string entity_path = context_.GetDataPath("entities_debug.json");
    Shared::Data::EntityLoader::LoadFromJson(entity_path, definitions_);

    // イベント発行
    nlohmann::json event_data;
    event_data["type"] = "entities_reloaded";
    context_.GetEventSystem().Emit("data_reloaded", event_data);
}

} // namespace Game::Managers
