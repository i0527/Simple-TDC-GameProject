#include "Game/Scenes/TitleScene.h"

#include <algorithm>
#include <iostream>

namespace {
constexpr float BUTTON_WIDTH = 360.0f;
constexpr float BUTTON_HEIGHT = 64.0f;
constexpr float BUTTON_GAP = 16.0f;
constexpr float TITLE_Y = 200.0f;
constexpr float MENU_START_Y = 360.0f;
constexpr float PROMPT_Y_OFFSET = 120.0f;
constexpr float MESSAGE_DURATION = 2.0f;
} // namespace

namespace Game::Scenes {

TitleScene::TitleScene(Font font, int screen_width, int screen_height)
    : font_(font), screen_width_(screen_width), screen_height_(screen_height) {
  menu_items_.push_back({u8"\u65b0\u898f\u30b2\u30fc\u30e0", MenuAction::NewGame});
  menu_items_.push_back({u8"\u7d9a\u304d\u304b\u3089", MenuAction::ContinueGame});
  menu_items_.push_back({u8"\u8a2d\u5b9a", MenuAction::Settings});

  const std::string music_path = "assets/music/title.ogg";
  if (std::filesystem::exists(music_path)) {
    music_ = LoadMusicStream(music_path.c_str());
    if (music_.ctxData) {
      music_loaded_ = true;
      PlayMusicStream(music_);
      SetMusicVolume(music_, 1.0f);
    } else {
      music_missing_ = true;
      std::cerr << "[TitleScene] Failed to load title.ogg" << std::endl;
    }
  } else {
    music_missing_ = true;
    std::cerr << "[TitleScene] Missing assets/music/title.ogg" << std::endl;
  }
}

void TitleScene::Update(float delta_time) {
  blink_timer_ += delta_time;
  if (blink_timer_ >= 0.5f) {
    blink_timer_ = 0.0f;
    show_prompt_ = !show_prompt_;
  }

  if (info_message_timer_ > 0.0f) {
    info_message_timer_ = std::max(0.0f, info_message_timer_ - delta_time);
  }

  // Keyboard navigation
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
    selected_index_ =
        (selected_index_ - 1 + static_cast<int>(menu_items_.size())) %
        static_cast<int>(menu_items_.size());
  }
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
    selected_index_ = (selected_index_ + 1) %
                      static_cast<int>(menu_items_.size());
  }

  if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
    TriggerAction(menu_items_[selected_index_].action);
  }
  if (IsKeyPressed(KEY_ESCAPE)) {
    TriggerAction(MenuAction::Exit);
  }

  // Mouse selection
  Vector2 mouse = GetMousePosition();
  bool mouse_clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  for (int i = 0; i < static_cast<int>(menu_items_.size()); ++i) {
    float y = MENU_START_Y + i * (BUTTON_HEIGHT + BUTTON_GAP);
    Rectangle rect{(screen_width_ - BUTTON_WIDTH) * 0.5f, y, BUTTON_WIDTH,
                   BUTTON_HEIGHT};
    if (CheckCollisionPointRec(mouse, rect)) {
      selected_index_ = i;
      if (mouse_clicked) {
        TriggerAction(menu_items_[i].action);
      }
    }
  }

  // BGM toggle click
  if (CheckCollisionPointRec(mouse, bgm_toggle_rect_) && mouse_clicked) {
    ToggleMute();
  }
  if (IsKeyPressed(KEY_M)) {
    ToggleMute();
  }

  if (music_loaded_) {
    UpdateMusicStream(music_);
    SetMusicVolume(music_, music_muted_ ? 0.0f : 1.0f);
  }
}

void TitleScene::Draw() {
  ClearBackground(BLACK);

  const float title_size = 48.0f;
  const char *title = u8"\u30b7\u30f3\u30d7\u30ebTDC\u30b2\u30fc\u30e0";
  Vector2 size = MeasureTextEx(font_, title, title_size, 2.0f);
  Vector2 pos{(screen_width_ - size.x) * 0.5f, TITLE_Y};
  DrawTextEx(font_, title, pos, title_size, 2.0f, RAYWHITE);

  // Menu buttons
  for (int i = 0; i < static_cast<int>(menu_items_.size()); ++i) {
    float y = MENU_START_Y + i * (BUTTON_HEIGHT + BUTTON_GAP);
    Rectangle rect{(screen_width_ - BUTTON_WIDTH) * 0.5f, y, BUTTON_WIDTH,
                   BUTTON_HEIGHT};
    bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
    bool selected = (i == selected_index_);
    Color fill = hovered || selected ? Color{70, 120, 200, 255}
                                     : Color{40, 60, 90, 220};
    DrawRectangleRounded(rect, 0.12f, 8, fill);
    DrawRectangleLinesEx(rect, 2.0f, Color{160, 200, 255, 255});

    const float label_size = 28.0f;
    Vector2 text_size =
        MeasureTextEx(font_, menu_items_[i].label.c_str(), label_size, 2.0f);
    Vector2 text_pos{rect.x + (rect.width - text_size.x) * 0.5f,
                     rect.y + (rect.height - text_size.y) * 0.5f};
    DrawTextEx(font_, menu_items_[i].label.c_str(), text_pos, label_size, 2.0f,
               RAYWHITE);
  }

  if (show_prompt_) {
    const float prompt_size = 20.0f;
    const char *prompt = u8"\u30b9\u30da\u30fc\u30b9/\u30a8\u30f3\u30bf\u30fc/\u30af\u30ea\u30c3\u30af\u3067\u6c7a\u5b9a";
    Vector2 ps = MeasureTextEx(font_, prompt, prompt_size, 2.0f);
    Vector2 pp{(screen_width_ - ps.x) * 0.5f, MENU_START_Y + 3 * (BUTTON_HEIGHT + BUTTON_GAP) + PROMPT_Y_OFFSET};
    DrawTextEx(font_, prompt, pp, prompt_size, 2.0f, LIGHTGRAY);
  }

  // BGM indicator / toggle
  const float bgm_font_size = 18.0f;
  std::string bgm_label;
  if (music_missing_) {
    bgm_label = "BGM: missing (muted)";
  } else {
    bgm_label = music_muted_ ? "BGM: OFF (M to toggle)" : "BGM: ON (M to toggle)";
  }
  Vector2 bgm_size = MeasureTextEx(font_, bgm_label.c_str(), bgm_font_size, 2.0f);
  bgm_toggle_rect_ = {20.0f, static_cast<float>(screen_height_) - 60.0f,
                      bgm_size.x + 20.0f, 32.0f};
  DrawRectangleRounded(bgm_toggle_rect_, 0.2f, 8, Color{30, 30, 30, 200});
  DrawTextEx(font_, bgm_label.c_str(),
             {bgm_toggle_rect_.x + 10.0f, bgm_toggle_rect_.y + 6.0f},
             bgm_font_size, 2.0f, RAYWHITE);

  // Info message (e.g., settings WIP)
  if (info_message_timer_ > 0.0f && !info_message_.empty()) {
    const float msg_size = 20.0f;
    Vector2 msg_w = MeasureTextEx(font_, info_message_.c_str(), msg_size, 2.0f);
    Vector2 msg_pos{(screen_width_ - msg_w.x) * 0.5f, MENU_START_Y - 40.0f};
    DrawTextEx(font_, info_message_.c_str(), msg_pos, msg_size, 2.0f,
               Color{200, 220, 255, 255});
  }
}

void TitleScene::TriggerAction(MenuAction action) {
  pending_action_ = action;
  switch (action) {
  case MenuAction::NewGame:
  case MenuAction::ContinueGame:
    start_requested_ = true;
    break;
  case MenuAction::Settings:
    info_message_ = u8"\u8a2d\u5b9a\u306f\u6e96\u5099\u4e2d\u3067\u3059";
    info_message_timer_ = MESSAGE_DURATION;
    break;
  case MenuAction::Exit:
    exit_requested_ = true;
    break;
  case MenuAction::None:
  default:
    break;
  }
}

void TitleScene::ToggleMute() {
  if (!music_loaded_) {
    info_message_ = music_missing_ ? "title.ogg not found" : "BGM not ready";
    info_message_timer_ = MESSAGE_DURATION;
    return;
  }
  music_muted_ = !music_muted_;
}

TitleScene::~TitleScene() {
  if (music_loaded_) {
    StopMusicStream(music_);
    UnloadMusicStream(music_);
  }
}

} // namespace Game::Scenes
