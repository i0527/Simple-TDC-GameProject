#pragma once

#include "Core/Systems/ISystem.h"
#include "new/Domain/TD/LineTD/Managers/WaveManager.h"

namespace New::Domain::TD::LineTD::Systems {

class WaveSchedulerSystem : public Core::ISystem {
public:
  explicit WaveSchedulerSystem(
      New::Domain::TD::LineTD::Managers::WaveManager &manager)
      : waveManager_(manager) {}

  int GetUpdatePriority() const override { return 20; }
  std::string GetName() const override { return "WaveSchedulerSystem"; }

  void Update(Core::GameContext &context, float deltaTime) override;

private:
  New::Domain::TD::LineTD::Managers::WaveManager &waveManager_;
};

} // namespace New::Domain::TD::LineTD::Systems
