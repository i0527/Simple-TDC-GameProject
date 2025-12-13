#include "Game/UI/SettingsPanel.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <optional>

#include "Game/UI/UiComponents.h"

namespace Game::UI {

namespace {
float Clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

int PercentFromFloat(float v) {
  return static_cast<int>(std::round(Clamp01(v) * 100.0f));
}

int ParsePercent(const std::string &text, int fallback) {
  try {
    int v = std::stoi(text);
    if (v < 0)
      v = 0;
    if (v > 100)
      v = 100;
    return v;
  } catch (...) {
    return fallback;
  }
}

struct VolumeRow {
  Rectangle label{};
  Rectangle slider{};
  Rectangle input{};
  Rectangle mute{};
};

struct SettingsLayout {
  Rectangle volume_card{};
  Rectangle display_card{};
  Rectangle input_card{};

  VolumeRow master{};
  VolumeRow bgm{};
  VolumeRow sfx{};

  Rectangle window_toggle{};
  Rectangle speed_btns[3]{};
  Rectangle input_guide{};

  Rectangle default_btn{};
  Rectangle apply_btn{};
  Rectangle cancel_btn{};
};

SettingsLayout MakeSettingsLayout(const Rectangle &panel_bounds) {
  SettingsLayout layout{};

  const float outer_gap = 18.0f;
  const float card_gap = 16.0f;
  const float header_h = 42.0f;
  const float card_h = panel_bounds.height - header_h - 96.0f;
  const float card_w =
      (panel_bounds.width - outer_gap * 2.0f - card_gap * 2.0f) / 3.0f;
  const float card_y = panel_bounds.y + header_h;
  float x0 = panel_bounds.x + outer_gap;
  float x1 = x0 + card_w + card_gap;
  float x2 = x1 + card_w + card_gap;

  layout.volume_card = {x0, card_y, card_w, card_h};
  layout.display_card = {x1, card_y, card_w, card_h};
  layout.input_card = {x2, card_y, card_w, card_h};

  const float pad = 16.0f;
  const float slider_h = 12.0f;
  const float input_w = 68.0f;
  const float row_h = 76.0f;
  const float slider_w = card_w - pad * 2.0f - input_w - 12.0f;

  auto make_row = [&](float y_base) -> VolumeRow {
    VolumeRow r{};
    r.label = {layout.volume_card.x + pad, y_base - 18.0f, slider_w, 20.0f};
    r.slider = {layout.volume_card.x + pad, y_base, slider_w, slider_h};
    r.input = {r.slider.x + slider_w + 12.0f, y_base - 6.0f, input_w, 28.0f};
    r.mute = {r.slider.x, y_base + 20.0f, 140.0f, 30.0f};
    return r;
  };

  float row_start = layout.volume_card.y + 58.0f;
  layout.master = make_row(row_start);
  layout.bgm = make_row(row_start + row_h);
  layout.sfx = make_row(row_start + row_h * 2.0f);

  // Display card
  float disp_pad = 18.0f;
  float disp_y = layout.display_card.y + 70.0f;
  layout.window_toggle = {layout.display_card.x + disp_pad, disp_y,
                          layout.display_card.width - disp_pad * 2.0f, 36.0f};
  float speed_y = disp_y + 60.0f;
  float speed_w = (layout.display_card.width - disp_pad * 2.0f - 16.0f * 2) /
                  3.0f;
  for (int i = 0; i < 3; ++i) {
    layout.speed_btns[i] = {layout.display_card.x + disp_pad +
                                i * (speed_w + 16.0f),
                            speed_y, speed_w, 40.0f};
  }

  // Input card
  float input_pad = 18.0f;
  float input_y = layout.input_card.y + 70.0f;
  layout.input_guide = {layout.input_card.x + input_pad, input_y,
                        layout.input_card.width - input_pad * 2.0f, 36.0f};

  // Bottom buttons
  float btn_y = panel_bounds.y + panel_bounds.height - 60.0f;
  float btn_w = 120.0f;
  float btn_h = 38.0f;
  layout.default_btn = {panel_bounds.x + outer_gap, btn_y, btn_w, btn_h};
  layout.apply_btn = {panel_bounds.x + panel_bounds.width - outer_gap -
                          btn_w * 2.0f - 12.0f,
                      btn_y, btn_w, btn_h};
  layout.cancel_btn = {panel_bounds.x + panel_bounds.width - outer_gap - btn_w,
                       btn_y, btn_w, btn_h};
  return layout;
}

void DrawSection(const Rectangle &r, const Font &font, const char *title) {
  DrawRectangleRounded(r, 0.08f, 6, Color{40, 60, 90, 230});
  DrawRectangleLinesEx(r, 2.0f, Color{180, 210, 255, 220});
  Vector2 ts = MeasureTextEx(font, title, 20.0f, 2.0f);
  DrawTextEx(font, title, {r.x + 12.0f, r.y + 10.0f}, 20.0f, 2.0f, RAYWHITE);
}

void DrawSlider(const Rectangle &r, float ratio) {
  DrawRectangleRec(r, Color{60, 80, 110, 200});
  float knob_x = r.x + r.width * Clamp01(ratio);
  DrawRectangleRec({r.x, r.y, knob_x - r.x, r.height},
                   Color{120, 180, 240, 200});
  DrawCircleV({knob_x, r.y + r.height * 0.5f}, 10.0f,
              Color{200, 220, 255, 230});
}

void DrawToggle(const Rectangle &r, const Font &font, const std::string &label,
                bool on) {
  DrawRectangleRounded(
      r, 0.12f, 6, on ? Color{80, 140, 200, 230} : Color{70, 70, 90, 200});
  DrawRectangleLinesEx(r, 2.0f, Color{180, 210, 255, 230});
  Vector2 ls = MeasureTextEx(font, label.c_str(), 18.0f, 2.0f);
  DrawTextEx(font, label.c_str(),
             {r.x + 12.0f, r.y + (r.height - ls.y) * 0.5f}, 18.0f, 2.0f,
             RAYWHITE);
}

void DrawButton(const Rectangle &r, const Font &font, const char *label,
                bool primary = false) {
  Color base = primary ? Color{60, 120, 200, 230} : Color{60, 90, 130, 220};
  DrawRectangleRounded(r, 0.12f, 6, base);
  DrawRectangleLinesEx(r, 2.0f, Color{180, 210, 255, 230});
  Vector2 ls = MeasureTextEx(font, label, 20.0f, 2.0f);
  DrawTextEx(font, label,
             {r.x + (r.width - ls.x) * 0.5f, r.y + (r.height - ls.y) * 0.5f},
             20.0f, 2.0f, RAYWHITE);
}

void DrawNumericBox(const Rectangle &r, const Font &font, const std::string &v,
                    bool active, int caret_pos) {
  Color base = active ? Color{90, 120, 170, 220} : Color{60, 90, 130, 200};
  DrawRectangleRounded(r, 0.16f, 6, base);
  DrawRectangleLinesEx(r, 2.0f, Color{180, 210, 255, 230});

  Vector2 ls = MeasureTextEx(font, v.c_str(), 18.0f, 2.0f);
  float tx = r.x + 10.0f;
  float ty = r.y + (r.height - ls.y) * 0.5f;
  DrawTextEx(font, v.c_str(), {tx, ty}, 18.0f, 2.0f, RAYWHITE);

  if (active) {
    bool blink = (static_cast<int>(GetTime() * 2.0f) % 2) == 0;
    if (blink) {
      int caret = std::clamp(caret_pos, 0, static_cast<int>(v.size()));
      std::string prefix = v.substr(0, caret);
      Vector2 ps = MeasureTextEx(font, prefix.c_str(), 18.0f, 2.0f);
      float cx = tx + ps.x + 2.0f;
      DrawRectangleRec({cx, ty, 2.0f, ls.y}, RAYWHITE);
    }
  }

  DrawTextEx(font, "%", {r.x + r.width - 14.0f, r.y + 4.0f}, 16.0f, 2.0f,
             LIGHTGRAY);
}

void ApplyWindowModeImmediate(const std::string &mode) {
  bool want_fullscreen = (mode == "fullscreen");
  bool is_fullscreen = IsWindowFullscreen();
  if (want_fullscreen != is_fullscreen) {
    ToggleFullscreen();
  }
}
} // namespace

void SettingsPanel::Open(const Shared::Core::SettingsData &initial) {
  draft_ = initial;
  original_ = initial;
  SyncNumericFields();
  open_ = true;
  confirming_apply_ = false;
}

void SettingsPanel::OpenDefault() {
  draft_ = Shared::Core::SettingsData{};
  original_ = draft_;
  SyncNumericFields();
  open_ = true;
  confirming_apply_ = false;
}

void SettingsPanel::Close() {
  open_ = false;
  confirming_apply_ = false;
  master_field_.active = false;
  bgm_field_.active = false;
  sfx_field_.active = false;
  master_field_.just_activated = bgm_field_.just_activated =
      sfx_field_.just_activated = false;
  active_slider_ = ActiveSlider::None;
}

void SettingsPanel::SyncNumericFields() {
  master_field_.text = std::to_string(PercentFromFloat(draft_.masterVolume));
  bgm_field_.text = std::to_string(PercentFromFloat(draft_.bgmVolume));
  sfx_field_.text = std::to_string(PercentFromFloat(draft_.sfxVolume));
  master_field_.just_activated = bgm_field_.just_activated =
      sfx_field_.just_activated = false;
  master_field_.caret = static_cast<int>(master_field_.text.size());
  bgm_field_.caret = static_cast<int>(bgm_field_.text.size());
  sfx_field_.caret = static_cast<int>(sfx_field_.text.size());
  active_slider_ = ActiveSlider::None;
}

bool SettingsPanel::HasUnsavedChanges() const {
  return draft_.masterVolume != original_.masterVolume ||
         draft_.bgmVolume != original_.bgmVolume ||
         draft_.sfxVolume != original_.sfxVolume ||
         draft_.masterMuted != original_.masterMuted ||
         draft_.bgmMuted != original_.bgmMuted ||
         draft_.sfxMuted != original_.sfxMuted ||
         draft_.windowMode != original_.windowMode ||
         draft_.speedMultiplier != original_.speedMultiplier ||
         draft_.showInputGuide != original_.showInputGuide;
}

SettingsPanel::Action
SettingsPanel::Update(const Rectangle &panel_bounds,
                      std::optional<Vector2> mouse_override) {
  if (!open_) {
    return Action::None;
  }

  SettingsLayout layout = MakeSettingsLayout(panel_bounds);
  Vector2 mouse = mouse_override.has_value() ? *mouse_override : GetMousePosition();
  bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  bool dragging = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
  bool release = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

  // 確認ポップアップ中はモーダル処理
  if (confirming_apply_) {
    float modal_w = 520.0f;
    float modal_h = 180.0f;
    Rectangle modal{(GetScreenWidth() - modal_w) * 0.5f,
                    (GetScreenHeight() - modal_h) * 0.5f, modal_w, modal_h};
    Rectangle yes_btn{modal.x + 30.0f, modal.y + modal_h - 60.0f, 120.0f, 36.0f};
    Rectangle no_btn{modal.x + modal_w - 150.0f, modal.y + modal_h - 60.0f,
                     120.0f, 36.0f};

    if (click) {
      if (CheckCollisionPointRec(mouse, yes_btn)) {
        confirming_apply_ = false;
        open_ = false;
        master_field_.active = bgm_field_.active = sfx_field_.active = false;
        active_slider_ = ActiveSlider::None;
        return Action::Apply;
      }
      if (CheckCollisionPointRec(mouse, no_btn)) {
        confirming_apply_ = false;
        draft_ = original_;
        SyncNumericFields();
        open_ = false;
        master_field_.active = bgm_field_.active = sfx_field_.active = false;
        active_slider_ = ActiveSlider::None;
        return Action::Cancel;
      }
    }
    return Action::None;
  }

  auto handle_numeric_field =
      [&](NumericFieldState &field, float &value, const Rectangle &box) {
        if (click && CheckCollisionPointRec(mouse, box)) {
          bool was_active = field.active;
          field.active = true;
          field.just_activated = !was_active;
          if (field.just_activated) {
            field.text.clear(); // Webライクにフォーカス時にクリア
            field.caret = 0;
          } else {
            // クリック位置からキャレットを近似（等幅とみなして補間）
            float usable_w = box.width - 20.0f;
            float local_x = std::clamp(mouse.x - (box.x + 10.0f), 0.0f, usable_w);
            int len = static_cast<int>(field.text.size());
            if (len == 0) {
              field.caret = 0;
            } else {
              float ratio = local_x / usable_w;
              field.caret = std::clamp(static_cast<int>(std::round(ratio * len)),
                                       0, len);
            }
          }
        } else if (click && !CheckCollisionPointRec(mouse, box)) {
          field.active = false;
          field.just_activated = false;
        }

        if (field.active) {
          int ch = GetCharPressed();
          while (ch > 0) {
            if (ch >= '0' && ch <= '9' && field.text.size() < 3) {
              char c = static_cast<char>(ch);
              field.text.insert(field.text.begin() + field.caret, c);
              field.caret = std::min(static_cast<int>(field.text.size()),
                                     field.caret + 1);
            }
            ch = GetCharPressed();
          }
          if (IsKeyPressed(KEY_BACKSPACE) && !field.text.empty()) {
            if (field.caret > 0 && field.caret <= static_cast<int>(field.text.size())) {
              field.text.erase(field.text.begin() + field.caret - 1);
              field.caret = std::max(0, field.caret - 1);
            }
          }
          if (IsKeyPressed(KEY_LEFT)) {
            field.caret = std::max(0, field.caret - 1);
          }
          if (IsKeyPressed(KEY_RIGHT)) {
            field.caret =
                std::min(static_cast<int>(field.text.size()), field.caret + 1);
          }
          if (IsKeyPressed(KEY_HOME)) {
            field.caret = 0;
          }
          if (IsKeyPressed(KEY_END)) {
            field.caret = static_cast<int>(field.text.size());
          }
          if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            field.active = false;
            field.just_activated = false;
          }

          int parsed =
              field.text.empty() ? PercentFromFloat(value)
                                 : ParsePercent(field.text, PercentFromFloat(value));
          value = Clamp01(static_cast<float>(parsed) / 100.0f);
        } else {
          field.text = std::to_string(PercentFromFloat(value));
          field.just_activated = false;
          field.caret = static_cast<int>(field.text.size());
        }
      };

  auto slider_value_from_mouse = [&](const Rectangle &r) {
    return std::clamp((mouse.x - r.x) / r.width, 0.0f, 1.0f);
  };

  auto handle_volume_row = [&](VolumeRow &row, float &value, bool &muted,
                               NumericFieldState &field,
                               SliderComponent &slider,
                               ToggleComponent &toggle, ActiveSlider self_id) {
    slider.track = row.slider;
    slider.value = value;
    bool slider_changed =
        slider.Update(mouse, click, dragging, release);
    if (slider_changed) {
      value = slider.value;
      field.text = std::to_string(PercentFromFloat(value));
      active_slider_ = self_id;
    }
    if (release && active_slider_ == self_id) {
      active_slider_ = ActiveSlider::None;
    }

    handle_numeric_field(field, value, row.input);
    slider.value = value;

    toggle.rect = row.mute;
    toggle.label_on = "ミュート ON";
    toggle.label_off = "ミュート OFF";
    toggle.Update(mouse, click, muted);
  };

  handle_volume_row(layout.master, draft_.masterVolume, draft_.masterMuted,
                    master_field_, master_slider_, master_mute_toggle_,
                    ActiveSlider::Master);
  handle_volume_row(layout.bgm, draft_.bgmVolume, draft_.bgmMuted,
                    bgm_field_, bgm_slider_, bgm_mute_toggle_,
                    ActiveSlider::Bgm);
  handle_volume_row(layout.sfx, draft_.sfxVolume, draft_.sfxMuted,
                    sfx_field_, sfx_slider_, sfx_mute_toggle_,
                    ActiveSlider::Sfx);

  window_toggle_.rect = layout.window_toggle;
  window_toggle_.label_on = "フルスクリーン";
  window_toggle_.label_off = "ウィンドウ";
  bool window_state = draft_.windowMode == "fullscreen";
  if (window_toggle_.Update(mouse, click, window_state)) {
    draft_.windowMode = window_state ? "fullscreen" : "window";
    ApplyWindowModeImmediate(draft_.windowMode);
  }

  const char *speed_labels[3] = {"x1", "x2", "x4"};
  for (int i = 0; i < 3; ++i) {
    speed_btns_[i].rect = layout.speed_btns[i];
    speed_btns_[i].label = speed_labels[i];
    if (speed_btns_[i].Update(mouse, click)) {
      draft_.speedMultiplier = (i == 0) ? 1.0f : (i == 1) ? 2.0f : 4.0f;
    }
  }

  input_guide_toggle_.rect = layout.input_guide;
  input_guide_toggle_.label_on = "キーガイド表示 ON";
  input_guide_toggle_.label_off = "キーガイド表示 OFF";
  input_guide_toggle_.Update(mouse, click, draft_.showInputGuide);

  default_btn_.rect = layout.default_btn;
  default_btn_.label = "デフォルト";
  apply_btn_.rect = layout.apply_btn;
  apply_btn_.label = "適用";
  cancel_btn_.rect = layout.cancel_btn;
  cancel_btn_.label = "キャンセル";

  if (default_btn_.Update(mouse, click)) {
    draft_ = Shared::Core::SettingsData{};
    SyncNumericFields();
    active_slider_ = ActiveSlider::None;
    return Action::None;
  }

  if (apply_btn_.Update(mouse, click)) {
    draft_.masterVolume = Clamp01(draft_.masterVolume);
    draft_.bgmVolume = Clamp01(draft_.bgmVolume);
    draft_.sfxVolume = Clamp01(draft_.sfxVolume);
    if (draft_.speedMultiplier != 2.0f && draft_.speedMultiplier != 4.0f) {
      draft_.speedMultiplier = 1.0f;
    }
    open_ = false;
    master_field_.active = bgm_field_.active = sfx_field_.active = false;
    active_slider_ = ActiveSlider::None;
    return Action::Apply;
  }
  if (cancel_btn_.Update(mouse, click)) {
    if (HasUnsavedChanges()) {
      confirming_apply_ = true;
      return Action::None;
    }
    open_ = false;
    master_field_.active = bgm_field_.active = sfx_field_.active = false;
    active_slider_ = ActiveSlider::None;
    return Action::Cancel;
  }

  return Action::None;
}

void SettingsPanel::Draw(const Rectangle &panel_bounds,
                         const Font &font) const {
  if (!open_)
    return;

  SettingsLayout layout = MakeSettingsLayout(panel_bounds);

  // Cards
  DrawSection(layout.volume_card, font, "音量");
  DrawSection(layout.display_card, font, "描画");
  DrawSection(layout.input_card, font, "入力");

  auto draw_row = [&](const VolumeRow &row, const char *label, float value,
                      bool muted, const std::string &text, bool active,
                      int caret, const char *on_label, const char *off_label) {
    DrawTextEx(font, label, {row.label.x, row.label.y}, 18.0f, 2.0f, LIGHTGRAY);
    SliderComponent s;
    s.track = row.slider;
    s.value = value;
    s.Draw(font);
    DrawNumericBox(row.input, font, text, active, caret);
    ToggleComponent t;
    t.rect = row.mute;
    t.label_on = on_label;
    t.label_off = off_label;
    t.Draw(font, muted);
  };

  draw_row(layout.master, "マスター", draft_.masterVolume, draft_.masterMuted,
           master_field_.text, master_field_.active, master_field_.caret,
           "マスターミュート ON", "マスターミュート OFF");
  draw_row(layout.bgm, "BGM", draft_.bgmVolume, draft_.bgmMuted,
           bgm_field_.text, bgm_field_.active, bgm_field_.caret, "BGMミュート ON",
           "BGMミュート OFF");
  draw_row(layout.sfx, "SE", draft_.sfxVolume, draft_.sfxMuted,
           sfx_field_.text, sfx_field_.active, sfx_field_.caret, "SEミュート ON",
           "SEミュート OFF");

  // Display card content
  ToggleComponent window_draw;
  window_draw.rect = layout.window_toggle;
  window_draw.label_on = "フルスクリーン";
  window_draw.label_off = "ウィンドウ";
  window_draw.Draw(font, draft_.windowMode == "fullscreen");
  const char *speed_labels[3] = {"x1", "x2", "x4"};
  for (int i = 0; i < 3; ++i) {
    ButtonComponent btn;
    btn.rect = layout.speed_btns[i];
    btn.label = speed_labels[i];
    bool active = (draft_.speedMultiplier == ((i == 0) ? 1.0f : (i == 1) ? 2.0f : 4.0f));
    btn.Draw(font, active);
  }

  // Input card content
  ToggleComponent input_draw;
  input_draw.rect = layout.input_guide;
  input_draw.label_on = "キーガイド表示 ON";
  input_draw.label_off = "キーガイド表示 OFF";
  input_draw.Draw(font, draft_.showInputGuide);
  std::string rebind_label = "キー設定: 後で追加予定";
  Vector2 rs = MeasureTextEx(font, rebind_label.c_str(), 16.0f, 2.0f);
  DrawTextEx(font, rebind_label.c_str(),
             {layout.input_card.x + 18.0f,
              layout.input_guide.y + layout.input_guide.height + 16.0f},
             16.0f, 2.0f, LIGHTGRAY);

  // Footer buttons
  ButtonComponent default_draw;
  default_draw.rect = layout.default_btn;
  default_draw.label = "デフォルト";
  default_draw.Draw(font);
  ButtonComponent apply_draw;
  apply_draw.rect = layout.apply_btn;
  apply_draw.label = "適用";
  apply_draw.Draw(font, true);
  ButtonComponent cancel_draw;
  cancel_draw.rect = layout.cancel_btn;
  cancel_draw.label = "キャンセル";
  cancel_draw.Draw(font);

  if (confirming_apply_) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  Color{0, 0, 0, 160});
    float modal_w = 520.0f;
    float modal_h = 180.0f;
    Rectangle modal{(GetScreenWidth() - modal_w) * 0.5f,
                    (GetScreenHeight() - modal_h) * 0.5f, modal_w, modal_h};
    DrawRectangleRounded(modal, 0.12f, 8, Color{30, 40, 60, 240});
    DrawRectangleLinesEx(modal, 2.0f, Color{180, 210, 255, 230});

    const char *msg = "変更内容を適用しますか？";
    Vector2 ms = MeasureTextEx(font, msg, 22.0f, 2.0f);
    DrawTextEx(font, msg,
               {modal.x + (modal.width - ms.x) * 0.5f, modal.y + 36.0f}, 22.0f,
               2.0f, RAYWHITE);

    Rectangle yes_btn{modal.x + 30.0f, modal.y + modal_h - 60.0f, 120.0f, 36.0f};
    Rectangle no_btn{modal.x + modal_w - 150.0f, modal.y + modal_h - 60.0f,
                     120.0f, 36.0f};
    ButtonComponent yes_draw;
    yes_draw.rect = yes_btn;
    yes_draw.label = "はい(適用)";
    yes_draw.Draw(font, true);
    ButtonComponent no_draw;
    no_draw.rect = no_btn;
    no_draw.label = "いいえ(破棄)";
    no_draw.Draw(font);
  }
}

} // namespace Game::UI
