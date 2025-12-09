#include "Game/Scenes/FormationScene.h"

namespace {
constexpr float PANEL_WIDTH = 820.0f;
constexpr float PANEL_HEIGHT = 420.0f;
} // namespace

namespace Game::Scenes {

FormationScene::FormationScene(Font font, int screen_width, int screen_height)
    : font_(font), screen_width_(screen_width), screen_height_(screen_height) {}

bool FormationScene::ConsumeReturnHome() {
  bool v = return_home_requested_;
  return_home_requested_ = false;
  return v;
}

void FormationScene::Update(float) {
  if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) ||
      IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    return_home_requested_ = true;
  }
}

void FormationScene::Draw() {
  ClearBackground(Color{16, 20, 28, 255});

  const float title_size = 42.0f;
  const char *title = "Formation (Work in Progress)";
  Vector2 ts = MeasureTextEx(font_, title, title_size, 2.0f);
  DrawTextEx(font_, title,
             {(static_cast<float>(screen_width_) - ts.x) * 0.5f, 140.0f},
             title_size, 2.0f, RAYWHITE);

  Rectangle panel{(static_cast<float>(screen_width_) - PANEL_WIDTH) * 0.5f,
                  (static_cast<float>(screen_height_) - PANEL_HEIGHT) * 0.5f,
                  PANEL_WIDTH, PANEL_HEIGHT};
  DrawRectangleRounded(panel, 0.12f, 6, Color{40, 60, 90, 230});
  DrawRectangleLinesEx(panel, 2.0f, Color{160, 210, 255, 230});

  const char *body = "編成UIは準備中です。\n\n"
                     "戻る: Enter / Esc / Click";
  Vector2 bs = MeasureTextEx(font_, body, 26.0f, 2.0f);
  DrawTextEx(font_, body,
             {panel.x + (panel.width - bs.x) * 0.5f,
              panel.y + (panel.height - bs.y) * 0.5f},
             26.0f, 2.0f, LIGHTGRAY);
}

} // namespace Game::Scenes
