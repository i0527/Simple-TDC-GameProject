#include "Game/Scenes/HomeScene.h"

#include <algorithm>

namespace {
constexpr float ITEM_HEIGHT = 60.0f;
constexpr float ITEM_GAP = 14.0f;
constexpr float LIST_MARGIN_Y = 220.0f;
constexpr float LIST_MARGIN_X = 520.0f;
} // namespace

namespace Game::Scenes {

HomeScene::HomeScene(Font font, int screen_width, int screen_height,
                     Shared::Data::UserDataManager *user_data)
    : font_(font), screen_width_(screen_width), screen_height_(screen_height),
      user_data_manager_(user_data) {
  menu_items_.push_back({"Stage Select", Action::StageSelect});
  menu_items_.push_back({"Formation", Action::Formation});
  menu_items_.push_back({"Save", Action::SaveMenu});
  menu_items_.push_back({"Load", Action::LoadMenu});
  menu_items_.push_back({"Save & Title", Action::SaveAndTitle});
  menu_items_.push_back({"Save & Exit", Action::SaveAndExit});
  menu_items_.push_back({"Quit (No Save)", Action::QuitWithoutSave});

  slot_meta_.resize(5);
  for (int i = 0; i < 5; ++i) {
    slot_meta_[i].slot = i;
  }
}

void HomeScene::Trigger(Action action) {
  pending_action_ = action;
  if (action == Action::SaveMenu) {
    show_save_menu_ = true;
    show_load_menu_ = false;
    RefreshSlots();
  } else if (action == Action::LoadMenu) {
    show_load_menu_ = true;
    show_save_menu_ = false;
    RefreshSlots();
  }
}

HomeScene::Action HomeScene::ConsumeAction() {
  Action a = pending_action_;
  pending_action_ = Action::None;
  return a;
}

void HomeScene::HandleInput() {
  if ((show_save_menu_ || show_load_menu_) && IsKeyPressed(KEY_ESCAPE)) {
    show_save_menu_ = false;
    show_load_menu_ = false;
    return;
  }

  if (IsKeyPressed(KEY_ESCAPE)) {
    Trigger(Action::QuitWithoutSave);
    return;
  }

  if (show_save_menu_ || show_load_menu_) {
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

  if (!info_message_.empty()) {
    Vector2 is = MeasureTextEx(font_, info_message_.c_str(), 20.0f, 2.0f);
    DrawTextEx(font_, info_message_.c_str(),
               {(static_cast<float>(screen_width_) - is.x) * 0.5f,
                static_cast<float>(screen_height_) - 100.0f},
               20.0f, 2.0f, Color{200, 220, 255, 230});
  }

  if (show_save_menu_) {
    DrawSaveLoadPanel(true);
  } else if (show_load_menu_) {
    DrawSaveLoadPanel(false);
  }
}

int HomeScene::ConsumeRequestedSaveSlot() {
  int v = requested_save_slot_;
  requested_save_slot_ = -1;
  return v;
}

int HomeScene::ConsumeRequestedLoadSlot() {
  int v = requested_load_slot_;
  requested_load_slot_ = -1;
  return v;
}

void HomeScene::SetInfoMessage(const std::string &msg) { info_message_ = msg; }

void HomeScene::RefreshSlots() {
  if (!user_data_manager_) {
    for (auto &s : slot_meta_) {
      s.exists = false;
      s.saved_at.clear();
      s.stage.clear();
      s.gold = 0;
    }
    return;
  }
  for (auto &s : slot_meta_) {
    Shared::Data::SaveData data;
    s.exists = user_data_manager_->LoadSlot(s.slot, data);
    if (s.exists) {
      s.saved_at = data.saved_at;
      s.stage = data.stage_progress.current_stage_id;
      s.gold = data.gold;
    } else {
      s.saved_at.clear();
      s.stage.clear();
      s.gold = 0;
    }
  }
}

void HomeScene::DrawSaveLoadPanel(bool saving) {
  DrawRectangle(0, 0, screen_width_, screen_height_, Color{0, 0, 0, 160});
  const float panel_w = 720.0f;
  const float panel_h = 420.0f;
  Rectangle panel{(screen_width_ - panel_w) * 0.5f,
                  (screen_height_ - panel_h) * 0.5f, panel_w, panel_h};
  DrawRectangleRounded(panel, 0.12f, 8, Color{30, 40, 60, 240});
  DrawRectangleLinesEx(panel, 2.0f, Color{170, 200, 255, 230});

  std::string title = saving ? "Select slot to Save" : "Select slot to Load";
  Vector2 ts = MeasureTextEx(font_, title.c_str(), 26.0f, 2.0f);
  DrawTextEx(font_, title.c_str(),
             {panel.x + (panel.width - ts.x) * 0.5f, panel.y + 18.0f}, 26.0f,
             2.0f, RAYWHITE);

  const float slot_h = 60.0f;
  const float slot_gap = 12.0f;
  float y = panel.y + 70.0f;
  Vector2 mouse = GetMousePosition();
  bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  for (const auto &slot : slot_meta_) {
    Rectangle rect{panel.x + 30.0f, y, panel.width - 60.0f, slot_h};
    bool hover = CheckCollisionPointRec(mouse, rect);
    Color fill = hover ? Color{70, 110, 170, 255} : Color{50, 70, 110, 230};
    if (!slot.exists && !saving) {
      fill = Color{60, 60, 60, 200};
    }
    DrawRectangleRounded(rect, 0.1f, 6, fill);
    DrawRectangleLinesEx(rect, 2.0f, Color{170, 200, 255, 230});

    std::string label = "Slot " + std::to_string(slot.slot + 1);
    if (slot.exists) {
      label += " : " + (slot.saved_at.empty() ? "unknown" : slot.saved_at);
    } else {
      label += " : empty";
    }
    Vector2 ls = MeasureTextEx(font_, label.c_str(), 22.0f, 2.0f);
    DrawTextEx(font_, label.c_str(),
               {rect.x + 12.0f, rect.y + 10.0f}, 22.0f, 2.0f, RAYWHITE);

    std::string sub = slot.exists
                          ? ("Stage: " +
                             (slot.stage.empty() ? "-" : slot.stage) +
                             " / Gold: " + std::to_string(slot.gold))
                          : "No data";
    Vector2 ss = MeasureTextEx(font_, sub.c_str(), 18.0f, 2.0f);
    DrawTextEx(font_, sub.c_str(),
               {rect.x + 12.0f, rect.y + slot_h - ss.y - 8.0f}, 18.0f, 2.0f,
               Color{220, 230, 255, 230});

    if (click && hover) {
      if (saving) {
        requested_save_slot_ = slot.slot;
        show_save_menu_ = false;
      } else if (slot.exists) {
        requested_load_slot_ = slot.slot;
        show_load_menu_ = false;
      }
    }
    y += slot_h + slot_gap;
  }
}

} // namespace Game::Scenes
