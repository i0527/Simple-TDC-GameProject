#pragma once

#include "Core/Systems/ISystem.h"

namespace New::Domain::TD::LineTD::Systems {

class DeathCheckSystem : public Core::ISystem {
public:
  int GetUpdatePriority() const override { return 80; }
  std::string GetName() const override { return "DeathCheckSystem"; }

  void Update(Core::GameContext &context, float deltaTime) override;
};

} // namespace New::Domain::TD::LineTD::Systems
