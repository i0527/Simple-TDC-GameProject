#include "Game/Scenes/HomeScene.h"

#include <algorithm>

namespace {
constexpr float ITEM_HEIGHT = 60.0f;
constexpr float ITEM_GAP = 14.0f;
constexpr float LIST_MARGIN_Y = 220.0f;
constexpr float LIST_MARGIN_X = 520.0f;
} // namespace

namespace Game::Scenes {

HomeScene::HomeScene(Font font, int screen_width, int screen_height)
    : font_(font), screen_width_(screen_width), screen_height_(screen_height) {
  menu_items_.push_back({"Stage Select", Action::StageSelect});
  menu_items_.push_back({"Formation (WIP)", Action::Formation});
  menu_items_.push_back({"Save & Title", Action::SaveAndTitle});
  menu_items_.push_back({"Save & Exit", Action::SaveAndExit});
  menu_items_.push_back({"Quit (No Save)", Action::QuitWithoutSave});
}

void HomeScene::Trigger(Action action) { pending_action_ = action; }

HomeScene::Action HomeScene::ConsumeAction() {
  Action a = pending_action_;
  pending_action_ = Action::None;
  return a;
}

void HomeScene::HandleInput() {
  if (IsKeyPressed(KEY_ESCAPE)) {
    Trigger(Action::QuitWithoutSave);
    return;
  }

  if (menu_items_.empty()) {
    return;
  }

  int count = static_cast<int>(menu_items_.size());
  if (IsKeyPressed(KEY_DOWN)) {
    selected_index_ = (selected_index_ + 1 + count) % count;
  }
  if (IsKeyPressed(KEY_UP)) {
    selected_index_ = (selected_index_ - 1 + count) % count;
  }

  bool activate = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
  if (activate && selected_index_ >= 0 && selected_index_ < count) {
    Trigger(menu_items_[selected_index_].action);
    return;
  }

  // Mouse hover / click
  Vector2 mouse = GetMousePosition();
  bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  float y = LIST_MARGIN_Y;
  for (int i = 0; i < count; ++i) {
    Rectangle rect{LIST_MARGIN_X, y,
                   static_cast<float>(screen_width_) - LIST_MARGIN_X * 2.0f,
                   ITEM_HEIGHT};
    bool hover = CheckCollisionPointRec(mouse, rect);
    if (hover) {
      selected_index_ = i;
      if (click) {
        Trigger(menu_items_[i].action);
        return;
      }
    }
    y += ITEM_HEIGHT + ITEM_GAP;
  }
}

void HomeScene::Update(float) { HandleInput(); }

void HomeScene::Draw() {
  ClearBackground(Color{12, 18, 26, 255});

  const float title_size = 46.0f;
  const char *title = "Home";
  Vector2 ts = MeasureTextEx(font_, title, title_size, 2.0f);
  DrawTextEx(font_, title,
             {(static_cast<float>(screen_width_) - ts.x) * 0.5f,
              LIST_MARGIN_Y - 120.0f},
             title_size, 2.0f, RAYWHITE);

  const float hint_size = 22.0f;
  const char *hint = "Select an option";
  Vector2 hs = MeasureTextEx(font_, hint, hint_size, 2.0f);
  DrawTextEx(font_, hint,
             {(static_cast<float>(screen_width_) - hs.x) * 0.5f,
              LIST_MARGIN_Y - 70.0f},
             hint_size, 2.0f, Color{180, 200, 230, 220});

  float y = LIST_MARGIN_Y;
  for (int i = 0; i < static_cast<int>(menu_items_.size()); ++i) {
    const auto &item = menu_items_[i];
    bool selected = (i == selected_index_);
    Rectangle rect{LIST_MARGIN_X, y,
                   static_cast<float>(screen_width_) - LIST_MARGIN_X * 2.0f,
                   ITEM_HEIGHT};
    Color fill = selected ? Color{60, 110, 170, 255} : Color{40, 60, 90, 220};
    DrawRectangleRounded(rect, 0.15f, 6, fill);
    DrawRectangleLinesEx(rect, 2.0f, Color{160, 210, 255, 230});

    Vector2 ls = MeasureTextEx(font_, item.label.c_str(), 24.0f, 2.0f);
    DrawTextEx(font_, item.label.c_str(),
               {rect.x + 16.0f, rect.y + (rect.height - ls.y) * 0.5f}, 24.0f,
               2.0f, RAYWHITE);
    y += ITEM_HEIGHT + ITEM_GAP;
  }

  const char *helper = "[Enter/Space] Confirm   [Up/Down] Move   [Esc] Quit";
  Vector2 vs = MeasureTextEx(font_, helper, 18.0f, 2.0f);
  DrawTextEx(font_, helper,
             {(static_cast<float>(screen_width_) - vs.x) * 0.5f,
              static_cast<float>(screen_height_) - 70.0f},
             18.0f, 2.0f, LIGHTGRAY);
}

} // namespace Game::Scenes
