#include "Game/Scenes/LoadingScene.h"

#include <algorithm>

namespace {
constexpr float SPINNER_SPEED = 180.0f; // deg/sec
} // namespace

namespace Game::Scenes {

LoadingScene::LoadingScene(Font font, int screen_width, int screen_height,
                           Task task)
    : font_(font), screen_width_(screen_width), screen_height_(screen_height),
      task_(std::move(task)) {}

void LoadingScene::Update(float delta_time) {
  if (!started_) {
    started_ = true;
    if (task_) {
      std::string msg = status_message_;
      bool ok = task_(msg);
      if (!msg.empty()) {
        status_message_ = msg;
      }
      error_ = !ok;
    }
    done_ = true;
  }

  spinner_timer_ += delta_time;
  spinner_angle_ += SPINNER_SPEED * delta_time;
  if (spinner_angle_ >= 360.0f) {
    spinner_angle_ -= 360.0f;
  }
}

void LoadingScene::Draw() {
  ClearBackground(Color{18, 24, 32, 255});

  const float title_size = 36.0f;
  const char *title = "Loading...";
  Vector2 ts = MeasureTextEx(font_, title, title_size, 2.0f);
  DrawTextEx(font_, title,
             {(screen_width_ - ts.x) * 0.5f, (screen_height_) * 0.4f},
             title_size, 2.0f, RAYWHITE);

  // Status message
  const float msg_size = 20.0f;
  Vector2 ms = MeasureTextEx(font_, status_message_.c_str(), msg_size, 2.0f);
  DrawTextEx(font_, status_message_.c_str(),
             {(screen_width_ - ms.x) * 0.5f, (screen_height_) * 0.5f}, msg_size,
             2.0f, Color{200, 220, 255, 255});

  // Simple spinner
  float spinner_radius = 28.0f;
  Vector2 center{static_cast<float>(screen_width_) * 0.5f,
                 static_cast<float>(screen_height_) * 0.65f};
  DrawCircleLinesV(center, spinner_radius, Color{120, 160, 220, 200});
  Vector2 p{center.x + spinner_radius * std::cos(spinner_angle_ * DEG2RAD),
            center.y + spinner_radius * std::sin(spinner_angle_ * DEG2RAD)};
  DrawLineV(center, p, Color{180, 220, 255, 230});
}

} // namespace Game::Scenes
