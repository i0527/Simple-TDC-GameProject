#pragma once

#include "Core/Systems/ISystem.h"

namespace New::Game::Systems {

class CostSystem : public Core::ISystem {
public:
  int GetUpdatePriority() const override { return 5; }
  std::string GetName() const override { return "CostSystem"; }

  void Update(Core::GameContext &context, float deltaTime) override;
};

} // namespace New::Game::Systems
