#include "new/Game/Systems/RenderSystem.h"

#include "Core/GameContext.h"
#include "Core/GameRenderer.h"
#include "Core/World.h"
#include "new/Core/Components/CoreComponents.h"
#include "raylib.h"

namespace New::Game::Systems {

using New::Core::Components::Health;
using New::Core::Components::Sprite;
using New::Core::Components::Transform;

void RenderSystem::Render(Core::GameContext &context) {
  auto &world = context.GetWorld();
  auto view = world.View<Transform, Sprite>();

  for (auto entity : view) {
    auto &transform = view.get<Transform>(entity);
    auto &sprite = view.get<Sprite>(entity);

    // スプライト描画
    DrawRectangleV({transform.x, transform.y}, {sprite.width, sprite.height},
                   sprite.tint);
    DrawRectangleLines(
        static_cast<int>(transform.x), static_cast<int>(transform.y),
        static_cast<int>(sprite.width), static_cast<int>(sprite.height), BLACK);

    // HPバー描画（Healthコンポーネントがある場合）
    if (world.Has<Health>(entity)) {
      const auto &health = world.Get<Health>(entity);
      if (health.max > 0) {
        const float barWidth = sprite.width;
        const float barHeight = 6.0f;
        const float barX = transform.x;
        const float barY = transform.y - barHeight - 2.0f;

        // HPバーの背景（黒）
        DrawRectangle(static_cast<int>(barX), static_cast<int>(barY),
                      static_cast<int>(barWidth), static_cast<int>(barHeight),
                      BLACK);

        // HPバーの現在値（緑→黄→赤のグラデーション）
        const float hpRatio =
            static_cast<float>(health.current) / static_cast<float>(health.max);
        const int currentBarWidth = static_cast<int>(barWidth * hpRatio);
        Color hpColor = GREEN;
        if (hpRatio < 0.3f) {
          hpColor = RED;
        } else if (hpRatio < 0.6f) {
          hpColor = YELLOW;
        }
        DrawRectangle(static_cast<int>(barX), static_cast<int>(barY),
                      currentBarWidth, static_cast<int>(barHeight), hpColor);

        // HPバーの枠線
        DrawRectangleLines(static_cast<int>(barX), static_cast<int>(barY),
                           static_cast<int>(barWidth),
                           static_cast<int>(barHeight), WHITE);
      }
    }
  }
}

} // namespace New::Game::Systems
