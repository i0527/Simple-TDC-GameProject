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
  if (!registry_.valid(entity)) {
    return;
  }
  auto nameIt = entityToName_.find(entity);
  if (nameIt != entityToName_.end()) {
    nameToEntity_.erase(nameIt->second);
    entityToName_.erase(nameIt);
  } else {
    for (auto it = nameToEntity_.begin(); it != nameToEntity_.end(); ++it) {
      if (it->second == entity) {
        nameToEntity_.erase(it);
        break;
      }
    }
  }
  registry_.destroy(entity);
}

void World::SetName(entt::entity entity, const std::string &name) {
  // 空名を割り当てる場合は既存の名前を削除して終了
  if (name.empty()) {
    auto current = entityToName_.find(entity);
    if (current != entityToName_.end()) {
      nameToEntity_.erase(current->second);
      entityToName_.erase(current);
    }
    return;
  }

  auto currentName = entityToName_.find(entity);
  if (currentName != entityToName_.end()) {
    nameToEntity_.erase(currentName->second);
  }

  auto existingEntity = nameToEntity_.find(name);
  if (existingEntity != nameToEntity_.end()) {
    entityToName_.erase(existingEntity->second);
  }

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

bool World::HasName(entt::entity entity) const {
  return entityToName_.find(entity) != entityToName_.end();
}

bool World::IsValid(entt::entity entity) const {
  return registry_.valid(entity);
}

} // namespace New::Core
