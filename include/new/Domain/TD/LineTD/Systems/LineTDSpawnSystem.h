#pragma once

#include <string>

#include "Core/Systems/ISystem.h"
#include "Data/DefinitionRegistry.h"
#include "new/Core/Components/CoreComponents.h"
#include "new/Domain/TD/LineTD/Components/LineTDComponents.h"
#include "new/Domain/TD/LineTD/Components/SpawnQueue.h"
#include "new/Domain/TD/LineTD/Managers/WaveManager.h"


namespace New::Domain::TD::LineTD::Systems {

class LineTDSpawnSystem : public Core::ISystem {
public:
  LineTDSpawnSystem(Managers::WaveManager &waveManager,
                    const Data::StageDef &stage,
                    const Data::DefinitionRegistry &definitions)
      : waveManager_(waveManager), stage_(stage), definitions_(definitions) {}

  int GetUpdatePriority() const override { return 30; }
  std::string GetName() const override { return "LineTDSpawnSystem"; }

  void Update(Core::GameContext &context, float deltaTime) override;

private:
  Managers::WaveManager &waveManager_;
  const Data::StageDef &stage_;
  const Data::DefinitionRegistry &definitions_;
};

} // namespace New::Domain::TD::LineTD::Systems
