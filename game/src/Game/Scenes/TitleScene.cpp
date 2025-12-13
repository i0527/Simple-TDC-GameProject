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

TitleScene::TitleScene(Font font, int screen_width, int screen_height,
                       Shared::Core::SettingsManager *settings,
                       Shared::Data::UserDataManager *user_data,
                       Game::Audio::BgmService *bgm)
    : font_(font), screen_width_(screen_width), screen_height_(screen_height),
      settings_manager_(settings), user_data_manager_(user_data),
      bgm_service_(bgm) {
  menu_items_.push_back(
      {u8"\u65b0\u898f\u30b2\u30fc\u30e0", MenuAction::NewGame});
  menu_items_.push_back(
      {u8"\u7d9a\u304d\u304b\u3089", MenuAction::ContinueGame});
  menu_items_.push_back({u8"\u8a2d\u5b9a", MenuAction::Settings});
  menu_items_.push_back({u8"\u7d42\u4e86", MenuAction::Exit});

  slot_meta_.resize(5);
  for (int i = 0; i < 5; ++i) {
    slot_meta_[i].slot = i;
  }
  RefreshSlots();
  EnsureSelectable();

  if (bgm_service_) {
    bgm_service_->Load("assets/music/mattari_room.mp3");
    music_muted_ = bgm_service_->IsManuallyMuted();
  }
}

void TitleScene::Update(float delta_time) {
  RefreshSlots();
  EnsureSelectable();
  blink_timer_ += delta_time;
  if (blink_timer_ >= 0.5f) {
    blink_timer_ = 0.0f;
    show_prompt_ = !show_prompt_;
  }

  if (info_message_timer_ > 0.0f) {
    info_message_timer_ = std::max(0.0f, info_message_timer_ - delta_time);
  }

  if (show_load_menu_ && IsKeyPressed(KEY_ESCAPE)) {
    show_load_menu_ = false;
  }

  // Keyboard navigation
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
    int next = selected_index_;
    int count = static_cast<int>(menu_items_.size());
    for (int i = 0; i < count; ++i) {
      next = (next - 1 + count) % count;
      if (menu_items_[next].enabled) {
        break;
      }
    }
    selected_index_ = next;
  }
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
    int next = selected_index_;
    int count = static_cast<int>(menu_items_.size());
    for (int i = 0; i < count; ++i) {
      next = (next + 1) % count;
      if (menu_items_[next].enabled) {
        break;
      }
    }
    selected_index_ = next;
  }

  if (!settings_panel_.IsOpen()) {
    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) &&
        menu_items_[selected_index_].enabled) {
      TriggerAction(menu_items_[selected_index_].action);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
      TriggerAction(MenuAction::Exit);
    }
  }

  // Mouse selection
  Vector2 mouse = GetMousePosition();
  bool mouse_clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  if (!settings_panel_.IsOpen()) {
    for (int i = 0; i < static_cast<int>(menu_items_.size()); ++i) {
      float y = MENU_START_Y + i * (BUTTON_HEIGHT + BUTTON_GAP);
      Rectangle rect{(screen_width_ - BUTTON_WIDTH) * 0.5f, y, BUTTON_WIDTH,
                     BUTTON_HEIGHT};
      if (CheckCollisionPointRec(mouse, rect)) {
        if (menu_items_[i].enabled) {
          selected_index_ = i;
        }
        if (mouse_clicked && menu_items_[i].enabled) {
          TriggerAction(menu_items_[i].action);
        }
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

  if (bgm_service_) {
    const Shared::Core::SettingsData *override_settings =
        settings_panel_.IsOpen() ? &settings_panel_.Draft() : nullptr;
    bgm_service_->SetManualMute(music_muted_, override_settings);
    bgm_service_->Update(override_settings);
  }

  // 設定パネル更新（デフォルトデータで開くのみ、適用動作は別途）
  if (settings_panel_.IsOpen()) {
    float panel_w =
        std::min(1080.0f, static_cast<float>(screen_width_) * 0.9f);
    float panel_h = 520.0f;
    Rectangle panel{(screen_width_ - panel_w) * 0.5f,
                    (screen_height_ - panel_h) * 0.5f, panel_w, panel_h};
    auto act = settings_panel_.Update(panel);
    if (act == Game::UI::SettingsPanel::Action::Apply) {
      if (settings_manager_) {
        settings_manager_->Data() = settings_panel_.Draft();
        settings_manager_->Save(settings_path_);
      }
      if (bgm_service_) {
        bgm_service_->SyncSettings();
      }
      settings_panel_.Close();
    } else if (act == Game::UI::SettingsPanel::Action::Cancel) {
      settings_panel_.Close();
    }
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
    bool enabled = menu_items_[i].enabled;
    Color fill = enabled
                     ? (hovered || selected ? Color{70, 120, 200, 255}
                                            : Color{40, 60, 90, 220})
                     : Color{60, 60, 60, 180};
    Color border = enabled ? Color{160, 200, 255, 255}
                           : Color{110, 130, 160, 200};
    DrawRectangleRounded(rect, 0.12f, 8, fill);
    DrawRectangleLinesEx(rect, 2.0f, border);

    const float label_size = 28.0f;
    Vector2 text_size =
        MeasureTextEx(font_, menu_items_[i].label.c_str(), label_size, 2.0f);
    Vector2 text_pos{rect.x + (rect.width - text_size.x) * 0.5f,
                     rect.y + (rect.height - text_size.y) * 0.5f};
    Color text_color = enabled ? RAYWHITE : Color{180, 180, 180, 220};
    DrawTextEx(font_, menu_items_[i].label.c_str(), text_pos, label_size, 2.0f,
               text_color);
  }

  if (show_prompt_) {
    const float prompt_size = 20.0f;
    const char *prompt = u8"\u30b9\u30da\u30fc\u30b9/\u30a8\u30f3\u30bf\u30fc/"
                         u8"\u30af\u30ea\u30c3\u30af\u3067\u6c7a\u5b9a";
    Vector2 ps = MeasureTextEx(font_, prompt, prompt_size, 2.0f);
    Vector2 pp{(screen_width_ - ps.x) * 0.5f,
               MENU_START_Y + 3 * (BUTTON_HEIGHT + BUTTON_GAP) +
                   PROMPT_Y_OFFSET};
    DrawTextEx(font_, prompt, pp, prompt_size, 2.0f, LIGHTGRAY);
  }

  // BGM indicator / toggle
  const float bgm_font_size = 18.0f;
  std::string bgm_label;
  bool bgm_missing = bgm_service_ ? bgm_service_->IsMissing() : true;
  bool bgm_loaded = bgm_service_ ? bgm_service_->IsLoaded() : false;
  if (!bgm_loaded) {
    bgm_label = bgm_missing ? "BGM: missing (muted)" : "BGM: not ready";
  } else {
    bgm_label =
        music_muted_ ? "BGM: OFF (M to toggle)" : "BGM: ON (M to toggle)";
  }
  Vector2 bgm_size =
      MeasureTextEx(font_, bgm_label.c_str(), bgm_font_size, 2.0f);
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

  // 設定パネル描画
  if (settings_panel_.IsOpen()) {
    DrawRectangle(0, 0, screen_width_, screen_height_, Color{0, 0, 0, 160});
    float panel_w =
        std::min(1080.0f, static_cast<float>(screen_width_) * 0.9f);
    float panel_h = 520.0f;
    Rectangle panel{(screen_width_ - panel_w) * 0.5f,
                    (screen_height_ - panel_h) * 0.5f, panel_w, panel_h};
    DrawRectangleRounded(panel, 0.08f, 6, Color{20, 30, 50, 230});
    DrawRectangleLinesEx(panel, 2.0f, Color{180, 210, 255, 240});
    settings_panel_.Draw(panel, font_);
  }

  if (show_load_menu_) {
    DrawLoadPanel();
  }
}

void TitleScene::TriggerAction(MenuAction action) {
  pending_action_ = action;
  switch (action) {
  case MenuAction::NewGame:
    start_requested_ = true;
    break;
  case MenuAction::ContinueGame:
    if (continue_available_) {
      show_load_menu_ = true;
    } else {
      info_message_ = "\u30bb\u30fc\u30d6\u30c7\u30fc\u30bf\u304c\u3042\u308a\u307e\u305b\u3093";
      info_message_timer_ = MESSAGE_DURATION;
    }
    pending_action_ = MenuAction::None;
    break;
  case MenuAction::Settings:
    if (settings_manager_) {
      settings_panel_.Open(settings_manager_->Data());
    } else {
      settings_panel_.OpenDefault();
    }
    pending_action_ = MenuAction::None;
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
  if (!bgm_service_ || !bgm_service_->IsLoaded()) {
    bool missing = bgm_service_ ? bgm_service_->IsMissing() : true;
    info_message_ = missing ? "BGM file not found" : "BGM not ready";
    info_message_timer_ = MESSAGE_DURATION;
    return;
  }
  music_muted_ = !music_muted_;
  bgm_service_->SetManualMute(music_muted_);
}

int TitleScene::ConsumeRequestedLoadSlot() {
  int v = requested_load_slot_;
  requested_load_slot_ = -1;
  return v;
}

void TitleScene::SetInfoMessage(const std::string &msg, float duration) {
  info_message_ = msg;
  info_message_timer_ = duration;
}

void TitleScene::RefreshSlots() {
  if (!user_data_manager_) {
    for (auto &s : slot_meta_) {
      s.exists = false;
      s.saved_at.clear();
      s.stage.clear();
      s.gold = 0;
    }
    continue_available_ = false;
    for (auto &item : menu_items_) {
      if (item.action == MenuAction::ContinueGame) {
        item.enabled = false;
      }
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
  continue_available_ = HasAnySave();
  for (auto &item : menu_items_) {
    if (item.action == MenuAction::ContinueGame) {
      item.enabled = continue_available_;
    }
  }
}

void TitleScene::DrawLoadPanel() {
  DrawRectangle(0, 0, screen_width_, screen_height_, Color{0, 0, 0, 160});
  const float panel_w = 720.0f;
  const float panel_h = 420.0f;
  Rectangle panel{(screen_width_ - panel_w) * 0.5f,
                  (screen_height_ - panel_h) * 0.5f, panel_w, panel_h};
  DrawRectangleRounded(panel, 0.12f, 8, Color{30, 40, 60, 240});
  DrawRectangleLinesEx(panel, 2.0f, Color{170, 200, 255, 230});

  std::string title = "\u30ed\u30fc\u30c9\u3059\u308b\u30b9\u30ed\u30c3\u30c8\u3092\u9078\u629e";
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
    if (!slot.exists) {
      fill = Color{60, 60, 60, 200};
    }
    DrawRectangleRounded(rect, 0.1f, 6, fill);
    DrawRectangleLinesEx(rect, 2.0f, Color{170, 200, 255, 230});

    std::string label = "Slot " + std::to_string(slot.slot + 1);
    if (slot.exists) {
      label += " : " + (slot.saved_at.empty() ? "日時不明" : slot.saved_at);
    } else {
      label += " : 空";
    }
    Vector2 ls = MeasureTextEx(font_, label.c_str(), 22.0f, 2.0f);
    DrawTextEx(font_, label.c_str(),
               {rect.x + 12.0f, rect.y + 10.0f}, 22.0f, 2.0f, RAYWHITE);

    std::string sub = slot.exists
                          ? ("ステージ: " +
                             (slot.stage.empty() ? "-" : slot.stage) +
                             " / ゴールド: " + std::to_string(slot.gold))
                          : "保存されていません";
    Vector2 ss = MeasureTextEx(font_, sub.c_str(), 18.0f, 2.0f);
    DrawTextEx(font_, sub.c_str(),
               {rect.x + 12.0f, rect.y + slot_h - ss.y - 8.0f}, 18.0f, 2.0f,
               Color{220, 230, 255, 230});

    if (click && hover && slot.exists) {
      requested_load_slot_ = slot.slot;
      show_load_menu_ = false;
    }
    y += slot_h + slot_gap;
  }
}

void TitleScene::EnsureSelectable() {
  if (menu_items_.empty()) {
    selected_index_ = 0;
    return;
  }
  int count = static_cast<int>(menu_items_.size());
  selected_index_ = std::clamp(selected_index_, 0, count - 1);
  if (menu_items_[selected_index_].enabled) {
    return;
  }
  for (int i = 0; i < count; ++i) {
    int idx = (selected_index_ + i + 1) % count;
    if (menu_items_[idx].enabled) {
      selected_index_ = idx;
      return;
    }
  }
}

bool TitleScene::HasAnySave() const {
  for (const auto &s : slot_meta_) {
    if (s.exists) {
      return true;
    }
  }
  return false;
}

TitleScene::~TitleScene() {
}

} // namespace Game::Scenes
