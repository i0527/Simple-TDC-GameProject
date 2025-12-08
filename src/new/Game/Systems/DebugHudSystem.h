#pragma once

#include "Core/Systems/ISystem.h"

namespace New::Game::Systems {

class DebugHudSystem : public Core::ISystem {
public:
  int GetRenderPriority() const override { return 90; }
  std::string GetName() const override { return "DebugHudSystem"; }

  void Render(Core::GameContext &context) override;
};

} // namespace New::Game::Systems
