#include "new/Domain/TD/LineTD/Systems/LaneMovementSystem.h"

#include <algorithm>

#include "Core/GameContext.h"
#include "Core/World.h"
#include "new/Core/Components/CoreComponents.h"
#include "new/Domain/TD/LineTD/Components/BaseArrivalQueue.h"
#include "new/Domain/TD/LineTD/Components/LineTDComponents.h"
#include "new/Game/Components/GameState.h"

namespace New::Domain::TD::LineTD::Systems {

using New::Core::Components::Transform;
using New::Domain::TD::LineTD::Components::Lane;
using New::Domain::TD::LineTD::Components::LaneMovement;
using LineTDTeam = New::Domain::TD::LineTD::Components::Team;

void LaneMovementSystem::Update(Core::GameContext &context, float deltaTime) {
  auto &world = context.GetWorld();
  auto view = world.View<LaneMovement, Transform>();
  auto queueView =
      world.View<New::Domain::TD::LineTD::Components::BaseArrivalQueue>();
  New::Domain::TD::LineTD::Components::BaseArrivalQueue *arrivalQueue =
      queueView.empty()
          ? nullptr
          : &queueView
                 .get<New::Domain::TD::LineTD::Components::BaseArrivalQueue>(
                     *queueView.begin());

  // 前方の敵ユニットとの最小距離（止まる距離）をBattleStateから取得
  float stopGap = 80.0f;
  auto stateView = world.View<New::Game::Components::BattleState>();
  if (stateView.begin() != stateView.end()) {
    stopGap =
        stateView.get<New::Game::Components::BattleState>(*stateView.begin())
            .minGap;
  }

  for (auto entity : view) {
    auto &movement = view.get<LaneMovement>(entity);
    auto &transform = view.get<Transform>(entity);

    // 前方の敵がstopGap以内にいる場合は足を止める
    bool blocked = false;
    auto laneView = world.View<Lane, LineTDTeam, Transform>();
    const int laneIdx =
        world.Has<Lane>(entity) ? world.Get<Lane>(entity).laneIndex : -1;
    const bool movingRight = movement.movingRight;
    for (auto other : laneView) {
      if (other == entity) {
        continue;
      }
      const auto &lane = laneView.get<Lane>(other);
      if (lane.laneIndex != laneIdx) {
        continue;
      }
      const auto &team = laneView.get<LineTDTeam>(other);
      const auto &selfTeam =
          world.Has<LineTDTeam>(entity) ? world.Get<LineTDTeam>(entity) : LineTDTeam{};
      if (team.side == selfTeam.side) {
        continue;
      }
      const auto &trOther = laneView.get<Transform>(other);
      const float dx = trOther.x - transform.x;
      if (movingRight && dx > 0 && dx < stopGap) {
        blocked = true;
        break;
      } else if (!movingRight && dx < 0 && -dx < stopGap) {
        blocked = true;
        break;
      }
    }

    if (!blocked) {
      const float direction = movement.movingRight ? 1.0f : -1.0f;
      movement.currentX += movement.speed * deltaTime * direction;
    }

    if (world.Has<Lane>(entity)) {
      auto &lane = world.Get<Lane>(entity);
      if (movement.movingRight) {
        movement.currentX = std::min(movement.currentX, lane.endX);
      } else {
        movement.currentX = std::max(movement.currentX, lane.startX);
      }
      transform.y = lane.yPosition;
      if (arrivalQueue && movement.movingRight && transform.x >= lane.endX) {
        arrivalQueue->events.push_back(
            New::Domain::TD::LineTD::Components::BaseArrivalEvent{
                New::Domain::TD::LineTD::Components::TeamSide::Enemy,
                /*damage*/ 1, entity});
        continue;
      }
    }

    transform.x = movement.currentX;
  }
}

} // namespace New::Domain::TD::LineTD::Systems
