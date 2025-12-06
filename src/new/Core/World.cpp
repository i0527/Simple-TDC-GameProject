#include "Core/World.h"

namespace New::Core {

World::World(entt::registry &registry) : registry_(registry) {}

entt::entity World::CreateEntity(const std::string &name) {
  const auto entity = registry_.create();
  if (!name.empty()) {
    SetName(entity, name);
  }
  return entity;
}

void World::DestroyEntity(entt::entity entity) {
  entityToName_.erase(entity);
  registry_.destroy(entity);
}

void World::SetName(entt::entity entity, const std::string &name) {
  nameToEntity_[name] = entity;
  entityToName_[entity] = name;
}

std::string World::GetName(entt::entity entity) const {
  auto it = entityToName_.find(entity);
  if (it != entityToName_.end()) {
    return it->second;
  }
  return {};
}

entt::entity World::FindByName(const std::string &name) const {
  auto it = nameToEntity_.find(name);
  if (it != nameToEntity_.end()) {
    return it->second;
  }
  return entt::null;
}

} // namespace New::Core
