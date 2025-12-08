#pragma once

#include <memory>

#include <entt/entt.hpp>

#include "Core/GameRenderer.h"
#include "Core/InputManager.h"
#include "Core/ResourceManager.h"
#include "Core/SystemRunner.h"
#include "Core/World.h"
#include "Data/DefinitionRegistry.h"

namespace New::Core {

class GameContext {
public:
  GameContext();
  ~GameContext();

  GameContext(const GameContext &) = delete;
  GameContext &operator=(const GameContext &) = delete;
  GameContext(GameContext &&) noexcept = default;
  GameContext &operator=(GameContext &&) noexcept = default;

  bool Initialize();
  void Shutdown();

  void SetResourceManager(std::unique_ptr<IResourceManager> manager);
  void SetInputManager(std::unique_ptr<IInputManager> manager);

  World &GetWorld() { return world_; }
  const World &GetWorld() const { return world_; }

  IResourceManager &GetResourceManager() { return *resourceManager_; }
  const IResourceManager &GetResourceManager() const {
    return *resourceManager_;
  }

  IInputManager &GetInputManager() { return *inputManager_; }
  const IInputManager &GetInputManager() const { return *inputManager_; }

  SystemRunner &GetSystemRunner() { return *systemRunner_; }
  const SystemRunner &GetSystemRunner() const { return *systemRunner_; }

  GameRenderer &GetRenderer() { return *renderer_; }
  const GameRenderer &GetRenderer() const { return *renderer_; }

  Data::DefinitionRegistry &GetDefinitionRegistry() {
    return *definitionRegistry_;
  }
  const Data::DefinitionRegistry &GetDefinitionRegistry() const {
    return *definitionRegistry_;
  }

  entt::registry &GetRegistry() { return registry_; }
  const entt::registry &GetRegistry() const { return registry_; }

  bool IsInitialized() const { return initialized_; }

private:
  bool initialized_ = false;
  entt::registry registry_{};
  World world_;
  std::unique_ptr<IResourceManager> resourceManager_;
  std::unique_ptr<IInputManager> inputManager_;
  std::unique_ptr<SystemRunner> systemRunner_;
  std::unique_ptr<GameRenderer> renderer_;
  std::unique_ptr<Data::DefinitionRegistry> definitionRegistry_;
};

} // namespace New::Core
