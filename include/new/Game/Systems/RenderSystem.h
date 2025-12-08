#pragma once

#include <string>

#include "Core/Systems/ISystem.h"
#include "new/Core/Components/CoreComponents.h"

namespace New::Game::Systems {

class RenderSystem : public Core::ISystem {
public:
  int GetRenderPriority() const override { return 60; }
  std::string GetName() const override { return "RenderSystem"; }

  void Render(Core::GameContext &context) override;
};

} // namespace New::Game::Systems
