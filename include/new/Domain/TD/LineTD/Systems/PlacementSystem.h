#pragma once

#include <array>

#include "Core/Systems/ISystem.h"
#include "Data/Definitions/StageDef.h"
#include "new/Domain/TD/LineTD/Components/LineTDComponents.h"
#include "new/Game/Components/UnitPreset.h"

namespace New::Domain::TD::LineTD::Systems {

class PlacementSystem : public Core::ISystem {
public:
  PlacementSystem(const New::Data::StageDef *stage,
                  std::array<New::Game::Components::UnitPreset, 10> deckSlots);

  int GetRenderPriority() const override { return 0; }
  std::string GetName() const override { return "PlacementSystem"; }

  void Update(Core::GameContext &context, float deltaTime) override;

private:
  const New::Data::StageDef *stage_;
  std::array<New::Game::Components::UnitPreset, 10> deckSlots_;
};

} // namespace New::Domain::TD::LineTD::Systems
