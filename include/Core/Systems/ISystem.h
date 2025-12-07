#pragma once

#include <string>

namespace New::Core {
class GameContext;

class ISystem {
public:
  virtual ~ISystem() = default;

  virtual bool Initialize(GameContext &) { return true; }
  virtual void Shutdown() {}

  virtual void Update(GameContext &, float /*deltaTime*/) {}
  virtual void Render(GameContext &) {}

  virtual int GetUpdatePriority() const { return 100; }
  virtual int GetRenderPriority() const { return 100; }

  void SetEnabled(bool enabled) { enabled_ = enabled; }
  bool IsEnabled() const { return enabled_; }

  virtual std::string GetName() const { return "ISystem"; }

protected:
  bool enabled_ = true;
};
} // namespace New::Core
