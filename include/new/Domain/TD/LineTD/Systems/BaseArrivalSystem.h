#pragma once

#include "Core/Systems/ISystem.h"

namespace New::Domain::TD::LineTD::Systems {

class BaseArrivalSystem : public Core::ISystem {
public:
  int GetUpdatePriority() const override { return 70; }
  std::string GetName() const override { return "BaseArrivalSystem"; }

  void Update(Core::GameContext &context, float deltaTime) override;
};

} // namespace New::Domain::TD::LineTD::Systems
