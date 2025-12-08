#pragma once

#include "Core/Systems/ISystem.h"

namespace New::Domain::TD::LineTD::Systems {

class CombatResolveSystem : public Core::ISystem {
public:
  int GetUpdatePriority() const override { return 60; }
  std::string GetName() const override { return "CombatResolveSystem"; }

  void Update(Core::GameContext &context, float deltaTime) override;
};

} // namespace New::Domain::TD::LineTD::Systems
