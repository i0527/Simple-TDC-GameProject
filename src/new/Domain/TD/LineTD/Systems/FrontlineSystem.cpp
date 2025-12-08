#include "new/Domain/TD/LineTD/Systems/FrontlineSystem.h"

#include <algorithm>
#include <vector>

#include "Core/GameContext.h"
#include "Core/World.h"
#include "new/Core/Components/CoreComponents.h"
#include "new/Domain/TD/LineTD/Components/LineTDComponents.h"
#include "new/Game/Components/GameState.h"

namespace New::Domain::TD::LineTD::Systems {

using New::Core::Components::Transform;
using New::Domain::TD::LineTD::Components::Lane;
using New::Domain::TD::LineTD::Components::LaneMovement;
using New::Domain::TD::LineTD::Components::Team;
using New::Domain::TD::LineTD::Components::TeamSide;

void FrontlineSystem::Update(Core::GameContext &context, float /*deltaTime*/) {
  auto &world = context.GetWorld();
  auto view = world.View<Lane, Transform, LaneMovement, Team>();
  if (view.begin() == view.end()) {
    return;
  }

  // レーンごとに収集
  // pair: x座標, entity
  std::unordered_map<int, std::vector<entt::entity>> laneMap;
  for (auto e : view) {
    const auto &lane = view.get<Lane>(e);
    laneMap[lane.laneIndex].push_back(e);
  }

  // ステージのminGap（GameStateに反映済み）を使う
  float minGap = 80.0f;
  auto stateView = world.View<New::Game::Components::BattleState>();
  if (stateView.begin() != stateView.end()) {
    const auto &s =
        stateView.get<New::Game::Components::BattleState>(*stateView.begin());
    minGap = s.minGap;
  }
  int iterations = 3;
  if (!stateView.empty()) {
    const auto &s =
        stateView.get<New::Game::Components::BattleState>(*stateView.begin());
    iterations = std::max(1, s.frontlineIterations);
  }

  for (auto &[laneIdx, entities] : laneMap) {
    if (entities.size() < 2) {
      continue;
    }
    // x昇順
    std::sort(entities.begin(), entities.end(),
              [&](entt::entity a, entt::entity b) {
                return view.get<Transform>(a).x < view.get<Transform>(b).x;
              });

    for (int iter = 0; iter < iterations; ++iter) {
      for (size_t i = 0; i + 1 < entities.size(); ++i) {
        auto a = entities[i];
        auto b = entities[i + 1];
        const auto &teamA = view.get<Team>(a);
        const auto &teamB = view.get<Team>(b);
        if (teamA.side == teamB.side) {
          continue; // 同チームは重なり許容
        }
        auto &ta = view.get<Transform>(a);
        auto &tb = view.get<Transform>(b);
        const float gap = tb.x - ta.x;
        if (gap < minGap) {
          const float push = 0.5f * (minGap - gap);
          ta.x -= push;
          tb.x += push;
          // LaneMovementのcurrentXも同期
          auto &ma = view.get<LaneMovement>(a);
          auto &mb = view.get<LaneMovement>(b);
          ma.currentX = ta.x;
          mb.currentX = tb.x;
        }
      }
    }
  }
}

} // namespace New::Domain::TD::LineTD::Systems
