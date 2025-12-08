#pragma once

#include <array>

#include "Core/Systems/ISystem.h"

namespace New::Domain::TD::LineTD::Systems {

class CostSystem : public Core::ISystem {
public:
  int GetUpdatePriority() const override { return 10; }
  std::string GetName() const override { return "CostSystem"; }

  void Update(Core::GameContext &context, float deltaTime) override;
};

} // namespace New::Domain::TD::LineTD::Systems
