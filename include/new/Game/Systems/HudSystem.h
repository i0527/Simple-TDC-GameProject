#pragma once

#include <string>

#include "Core/Systems/ISystem.h"
#include "new/Game/Components/GameState.h"

namespace New::Game::Systems {

class HudSystem : public Core::ISystem {
public:
  int GetRenderPriority() const override { return 80; }
  std::string GetName() const override { return "HudSystem"; }

  void Render(Core::GameContext &context) override;
};

} // namespace New::Game::Systems
