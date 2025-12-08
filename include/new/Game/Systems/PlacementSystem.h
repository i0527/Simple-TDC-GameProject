#pragma once

#include <array>
#include <string>

#include "Core/Systems/ISystem.h"
#include "Data/Definitions/StageDef.h"
#include "new/Game/Components/GameState.h"
#include "new/Game/Components/UnitPreset.h"

namespace New::Game::Systems {

using New::Game::Components::UnitPreset;

class PlacementSystem : public Core::ISystem {
public:
  PlacementSystem(const New::Data::StageDef *stage,
                  const std::array<UnitPreset, 10> &deckSlots);

  int GetUpdatePriority() const override { return 15; }
  std::string GetName() const override { return "PlacementSystem"; }

  void Update(Core::GameContext &context, float deltaTime) override;

private:
  const New::Data::StageDef *stage_;
  std::array<UnitPreset, 10> deckSlots_;
};

} // namespace New::Game::Systems
