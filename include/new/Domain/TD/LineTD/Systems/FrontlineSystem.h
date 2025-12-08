#pragma once

#include "Core/Systems/ISystem.h"

namespace New::Domain::TD::LineTD::Systems {

// 敵味方間の最小間隔を強制し、重なりを軽減する簡易前線システム。
// 味方同士・敵同士の重なりは許容（にゃんこ風）。
class FrontlineSystem : public Core::ISystem {
public:
  int GetUpdatePriority() const override { return 45; } // Combat前、Movement後
  std::string GetName() const override { return "FrontlineSystem"; }

  void Update(Core::GameContext &context, float deltaTime) override;
};

} // namespace New::Domain::TD::LineTD::Systems
