#pragma once

#include "Core/Systems/ISystem.h"
#include <algorithm>
#include <memory>
#include <vector>


namespace New::Core {
class GameContext;

class SystemRunner {
public:
  SystemRunner() = default;
  ~SystemRunner() = default;

  template <typename T, typename... Args> T *AddSystem(Args &&...args) {
    auto system = std::make_unique<T>(std::forward<Args>(args)...);
    T *ptr = system.get();
    systems_.push_back(std::move(system));
    Sort();
    return ptr;
  }

  bool Initialize(GameContext &context) {
    for (auto &sys : systems_) {
      if (!sys->Initialize(context)) {
        return false;
      }
    }
    return true;
  }

  void Shutdown() {
    for (auto &sys : systems_) {
      sys->Shutdown();
    }
  }

  void Update(GameContext &context, float dt) {
    for (auto &sys : systems_) {
      if (sys->IsEnabled()) {
        sys->Update(context, dt);
      }
    }
  }

  void Render(GameContext &context) {
    for (auto &sys : systems_) {
      if (sys->IsEnabled()) {
        sys->Render(context);
      }
    }
  }

private:
  std::vector<std::unique_ptr<ISystem>> systems_;

  void Sort() {
    std::sort(systems_.begin(), systems_.end(),
              [](const auto &a, const auto &b) {
                return a->GetUpdatePriority() < b->GetUpdatePriority();
              });
  }
};

} // namespace New::Core
