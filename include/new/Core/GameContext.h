#pragma once

#include "Core/GameRenderer.h"
#include "Core/SystemRunner.h"
#include "Core/World.h"
#include <entt/entt.hpp>
#include <memory>


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

  World &GetWorld() { return world_; }
  const World &GetWorld() const { return world_; }

  SystemRunner &GetSystemRunner() { return *systemRunner_; }
  const SystemRunner &GetSystemRunner() const { return *systemRunner_; }

  GameRenderer &GetRenderer() { return *renderer_; }
  const GameRenderer &GetRenderer() const { return *renderer_; }

  entt::registry &GetRegistry() { return registry_; }
  const entt::registry &GetRegistry() const { return registry_; }

  bool IsInitialized() const { return initialized_; }

private:
  bool initialized_ = false;
  entt::registry registry_{};
  World world_;
  std::unique_ptr<SystemRunner> systemRunner_;
  std::unique_ptr<GameRenderer> renderer_;
};

} // namespace New::Core
