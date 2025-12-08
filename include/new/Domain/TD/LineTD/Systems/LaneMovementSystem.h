#pragma once

#include <string>

#include "Core/Systems/ISystem.h"
#include "new/Core/Components/CoreComponents.h"
#include "new/Domain/TD/LineTD/Components/LineTDComponents.h"
#include "new/Game/Components/GameState.h"

namespace New::Domain::TD::LineTD::Systems {

class LaneMovementSystem : public Core::ISystem {
public:
  int GetUpdatePriority() const override { return 40; }
  std::string GetName() const override { return "LaneMovementSystem"; }

  void Update(Core::GameContext &context, float deltaTime) override;
};

} // namespace New::Domain::TD::LineTD::Systems
