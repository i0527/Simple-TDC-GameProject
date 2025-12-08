#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "Core/Systems/ISystem.h"

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
    RebuildExecutionOrder();
    return ptr;
  }

  bool Initialize(GameContext &context) {
    if (initialized_) {
      return true;
    }
    for (auto *sys : updateOrder_) {
      if (!sys->Initialize(context)) {
        return false;
      }
    }
    initialized_ = true;
    return true;
  }

  void Shutdown() {
    for (auto it = systems_.rbegin(); it != systems_.rend(); ++it) {
      (*it)->Shutdown();
    }
    initialized_ = false;
  }

  void Update(GameContext &context, float dt) {
    for (auto *sys : updateOrder_) {
      if (sys->IsEnabled()) {
        sys->Update(context, dt);
      }
    }
  }

  void Render(GameContext &context) {
    for (auto *sys : renderOrder_) {
      if (sys->IsEnabled()) {
        sys->Render(context);
      }
    }
  }

  bool IsInitialized() const { return initialized_; }
  std::size_t SystemCount() const { return systems_.size(); }

private:
  std::vector<std::unique_ptr<ISystem>> systems_;
  std::vector<ISystem *> updateOrder_;
  std::vector<ISystem *> renderOrder_;
  bool initialized_ = false;

  void RebuildExecutionOrder() {
    updateOrder_.clear();
    renderOrder_.clear();
    for (auto &sys : systems_) {
      updateOrder_.push_back(sys.get());
      renderOrder_.push_back(sys.get());
    }

    auto stableByPriority = [](auto accessor) {
      return [accessor](const auto *a, const auto *b) {
        const int pa = accessor(a);
        const int pb = accessor(b);
        if (pa != pb) {
          return pa < pb;
        }
        return a->GetName() < b->GetName();
      };
    };

    std::stable_sort(
        updateOrder_.begin(), updateOrder_.end(),
        stableByPriority([](const auto *s) { return s->GetUpdatePriority(); }));
    std::stable_sort(
        renderOrder_.begin(), renderOrder_.end(),
        stableByPriority([](const auto *s) { return s->GetRenderPriority(); }));
  }
};

} // namespace New::Core
