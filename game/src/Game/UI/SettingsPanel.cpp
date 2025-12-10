#include "Game/UI/SettingsPanel.h"

#include <algorithm>
#include <string>

namespace Game::UI {

namespace {
float Clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
} // namespace

void SettingsPanel::Open(const Shared::Core::SettingsData &initial) {
  draft_ = initial;
  open_ = true;
}

void SettingsPanel::OpenDefault() {
  draft_ = Shared::Core::SettingsData{};
  open_ = true;
}

void SettingsPanel::Close() { open_ = false; }

SettingsPanel::Action SettingsPanel::Update(const Rectangle &panel_bounds) {
  if (!open_) {
    return Action::None;
  }

  Vector2 mouse = GetMousePosition();
  bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  // UIレイアウト（TDGameSceneのものを共通化）
  const float btn_h = 56.0f;
  const float gap = 14.0f;
  const float sx = panel_bounds.x + 40.0f;
  const float sy = panel_bounds.y + 96.0f + 4 * (btn_h + gap) + 10.0f;
  const float slider_w = panel_bounds.width - 80.0f;

  Rectangle slider{sx, sy + 10.0f, slider_w, 12.0f};
  Rectangle bgm_toggle{sx, slider.y + 32.0f, 160.0f, 34.0f};
  Rectangle sfx_toggle{sx + 180.0f, slider.y + 32.0f, 160.0f, 34.0f};
  Rectangle lang_sel{sx, slider.y + 78.0f, 220.0f, 40.0f};
  Rectangle qual_sel{sx, slider.y + 126.0f, 220.0f, 40.0f};
  Rectangle win_sel{sx, slider.y + 174.0f, 220.0f, 40.0f};
  Rectangle apply_btn{panel_bounds.x + panel_bounds.width - 200.0f,
                      panel_bounds.y + panel_bounds.height - 70.0f, 90.0f,
                      44.0f};
  Rectangle cancel_btn{panel_bounds.x + panel_bounds.width - 100.0f,
                       panel_bounds.y + panel_bounds.height - 70.0f, 90.0f,
                       44.0f};

  if (click) {
    if (CheckCollisionPointRec(mouse, slider)) {
      float ratio = std::clamp((mouse.x - slider.x) / slider.width, 0.0f, 1.0f);
      draft_.masterVolume = ratio;
    } else if (CheckCollisionPointRec(mouse, bgm_toggle)) {
      draft_.bgmMuted = !draft_.bgmMuted;
    } else if (CheckCollisionPointRec(mouse, sfx_toggle)) {
      draft_.sfxMuted = !draft_.sfxMuted;
    } else if (CheckCollisionPointRec(mouse, lang_sel)) {
      draft_.language = (draft_.language == "ja") ? "en" : "ja";
    } else if (CheckCollisionPointRec(mouse, qual_sel)) {
      if (draft_.quality == "low")
        draft_.quality = "medium";
      else if (draft_.quality == "medium")
        draft_.quality = "high";
      else
        draft_.quality = "low";
    } else if (CheckCollisionPointRec(mouse, win_sel)) {
      draft_.windowMode =
          (draft_.windowMode == "window") ? "fullscreen" : "window";
    } else if (CheckCollisionPointRec(mouse, apply_btn)) {
      draft_.masterVolume = Clamp01(draft_.masterVolume);
      open_ = false;
      return Action::Apply;
    } else if (CheckCollisionPointRec(mouse, cancel_btn)) {
      open_ = false;
      return Action::Cancel;
    }
  }

  return Action::None;
}

void SettingsPanel::Draw(const Rectangle &panel_bounds,
                         const Font &font) const {
  if (!open_)
    return;

  float sx = panel_bounds.x + 40.0f;
  float sy = panel_bounds.y + 96.0f + 4 * (56.0f + 14.0f) + 10.0f;
  float slider_w = panel_bounds.width - 80.0f;

  auto draw_toggle = [&](const Rectangle &r, const char *label, bool on) {
    DrawRectangleRounded(
        r, 0.12f, 6, on ? Color{80, 140, 200, 230} : Color{70, 70, 90, 200});
    DrawRectangleLinesEx(r, 2.0f, Color{180, 210, 255, 230});
    Vector2 ls = MeasureTextEx(font, label, 20.0f, 2.0f);
    DrawTextEx(font, label, {r.x + 10.0f, r.y + (r.height - ls.y) * 0.5f},
               20.0f, 2.0f, RAYWHITE);
  };

  DrawTextEx(font, "マスター音量", {sx, sy - 6.0f}, 18.0f, 2.0f, LIGHTGRAY);
  Rectangle slider{sx, sy + 10.0f, slider_w, 12.0f};
  DrawRectangleRec(slider, Color{60, 80, 110, 200});
  float knob_x = slider.x + slider.width * Clamp01(draft_.masterVolume);
  DrawRectangleRec({slider.x, slider.y, knob_x - slider.x, slider.height},
                   Color{120, 180, 240, 200});
  DrawCircleV({knob_x, slider.y + slider.height * 0.5f}, 10.0f,
              Color{200, 220, 255, 230});

  float toggle_y = slider.y + 32.0f;
  Rectangle bgm_toggle{sx, toggle_y, 160.0f, 34.0f};
  Rectangle sfx_toggle{sx + 180.0f, toggle_y, 160.0f, 34.0f};
  draw_toggle(bgm_toggle,
              draft_.bgmMuted ? "BGMミュート ON" : "BGMミュート OFF",
              draft_.bgmMuted);
  draw_toggle(sfx_toggle,
              draft_.sfxMuted ? "SFXミュート ON" : "SFXミュート OFF",
              draft_.sfxMuted);

  float select_y = toggle_y + 46.0f;
  auto draw_select = [&](const Rectangle &r, const std::string &label,
                         const std::string &value) {
    DrawRectangleRounded(r, 0.12f, 6, Color{60, 90, 130, 220});
    DrawRectangleLinesEx(r, 2.0f, Color{180, 210, 255, 230});
    Vector2 ls = MeasureTextEx(font, label.c_str(), 18.0f, 2.0f);
    Vector2 vs = MeasureTextEx(font, value.c_str(), 20.0f, 2.0f);
    DrawTextEx(font, label.c_str(),
               {r.x + 10.0f, r.y + (r.height - ls.y) * 0.5f}, 18.0f, 2.0f,
               LIGHTGRAY);
    DrawTextEx(font, value.c_str(),
               {r.x + r.width - vs.x - 12.0f, r.y + (r.height - vs.y) * 0.5f},
               20.0f, 2.0f, RAYWHITE);
  };

  Rectangle lang_sel{sx, select_y, 220.0f, 40.0f};
  Rectangle qual_sel{sx, select_y + 48.0f, 220.0f, 40.0f};
  Rectangle win_sel{sx, select_y + 96.0f, 220.0f, 40.0f};
  draw_select(lang_sel, "言語", draft_.language);
  draw_select(qual_sel, "画質", draft_.quality);
  draw_select(win_sel, "ウィンドウ", draft_.windowMode);

  Rectangle apply_btn{panel_bounds.x + panel_bounds.width - 200.0f,
                      panel_bounds.y + panel_bounds.height - 70.0f, 90.0f,
                      44.0f};
  Rectangle cancel_btn{panel_bounds.x + panel_bounds.width - 100.0f,
                       panel_bounds.y + panel_bounds.height - 70.0f, 90.0f,
                       44.0f};

  auto draw_btn = [&](const Rectangle &r, const char *label) {
    Vector2 ls = MeasureTextEx(font, label, 22.0f, 2.0f);
    DrawRectangleRounded(r, 0.14f, 6, Color{60, 100, 150, 230});
    DrawRectangleLinesEx(r, 2.0f, Color{180, 210, 255, 230});
    DrawTextEx(font, label,
               {r.x + (r.width - ls.x) * 0.5f, r.y + (r.height - ls.y) * 0.5f},
               22.0f, 2.0f, RAYWHITE);
  };

  draw_btn(apply_btn, "保存");
  draw_btn(cancel_btn, "戻る");
}

} // namespace Game::UI
