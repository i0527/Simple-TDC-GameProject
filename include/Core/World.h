#pragma once

#include <entt/entt.hpp>
#include <string>
#include <unordered_map>

namespace New::Core {

class World {
public:
  explicit World(entt::registry &registry);
  ~World() = default;

  entt::entity CreateEntity(const std::string &name = {});
  void DestroyEntity(entt::entity entity);

  entt::registry &Registry() { return registry_; }
  const entt::registry &Registry() const { return registry_; }

  // 名前管理（簡易）
  void SetName(entt::entity entity, const std::string &name);
  std::string GetName(entt::entity entity) const;
  entt::entity FindByName(const std::string &name) const;
  bool HasName(entt::entity entity) const;
  bool IsValid(entt::entity entity) const;

  template <typename Component, typename... Args>
  Component &Add(entt::entity entity, Args &&...args) {
    return registry_.emplace<Component>(entity, std::forward<Args>(args)...);
  }

  template <typename Component> bool Has(entt::entity entity) const {
    return registry_.any_of<Component>(entity);
  }

  template <typename Component> Component &Get(entt::entity entity) {
    return registry_.get<Component>(entity);
  }

  template <typename Component> void Remove(entt::entity entity) {
    if (registry_.any_of<Component>(entity)) {
      registry_.remove<Component>(entity);
    }
  }

  template <typename... Components> auto View() {
    return registry_.view<Components...>();
  }

private:
  entt::registry &registry_;
  std::unordered_map<std::string, entt::entity> nameToEntity_;
  std::unordered_map<entt::entity, std::string> entityToName_;
};

} // namespace New::Core
