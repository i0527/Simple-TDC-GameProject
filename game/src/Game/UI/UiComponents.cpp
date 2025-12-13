#include "Game/UI/UiComponents.h"

#include <algorithm>

namespace {
float Clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
} // namespace

namespace Game::UI {

bool ButtonComponent::Update(Vector2 mouse, bool click) const {
  if (disabled)
    return false;
  return click && CheckCollisionPointRec(mouse, rect);
}

void ButtonComponent::Draw(const Font &font, bool primary) const {
  Color base = disabled
                   ? Color{80, 80, 80, 200}
                   : (primary ? Color{60, 120, 200, 230}
                              : Color{60, 90, 130, 220});
  DrawRectangleRounded(rect, 0.14f, 6, base);
  DrawRectangleLinesEx(rect, 2.0f, Color{180, 210, 255, 230});
  Vector2 ls = MeasureTextEx(font, label.c_str(), 20.0f, 2.0f);
  DrawTextEx(font, label.c_str(),
             {rect.x + (rect.width - ls.x) * 0.5f,
              rect.y + (rect.height - ls.y) * 0.5f},
             20.0f, 2.0f, RAYWHITE);
}

bool ToggleComponent::Update(Vector2 mouse, bool click, bool &state) const {
  if (click && CheckCollisionPointRec(mouse, rect)) {
    state = !state;
    return true;
  }
  return false;
}

void ToggleComponent::Draw(const Font &font, bool state) const {
  std::string label = state ? label_on : label_off;
  Color base = state ? Color{80, 140, 200, 230} : Color{70, 70, 90, 200};
  DrawRectangleRounded(rect, 0.12f, 6, base);
  DrawRectangleLinesEx(rect, 2.0f, Color{180, 210, 255, 230});
  Vector2 ls = MeasureTextEx(font, label.c_str(), 18.0f, 2.0f);
  DrawTextEx(font, label.c_str(),
             {rect.x + 12.0f, rect.y + (rect.height - ls.y) * 0.5f}, 18.0f,
             2.0f, RAYWHITE);
}

bool SliderComponent::Update(Vector2 mouse, bool click, bool dragging_now,
                             bool release) {
  bool changed = false;
  if (click && CheckCollisionPointRec(mouse, track)) {
    dragging = true;
    float ratio = (mouse.x - track.x) / track.width;
    float v = Clamp01(ratio);
    if (v != value) {
      value = v;
      changed = true;
    }
  }
  if (dragging && dragging_now) {
    float ratio = (mouse.x - track.x) / track.width;
    float v = Clamp01(ratio);
    if (v != value) {
      value = v;
      changed = true;
    }
  }
  if (release) {
    dragging = false;
  }
  return changed;
}

void SliderComponent::Draw(const Font &) const {
  DrawRectangleRec(track, Color{60, 80, 110, 200});
  float knob_x = track.x + track.width * Clamp01(value);
  DrawRectangleRec({track.x, track.y, knob_x - track.x, track.height},
                   Color{120, 180, 240, 200});
  DrawCircleV({knob_x, track.y + track.height * 0.5f}, 10.0f,
              Color{200, 220, 255, 230});
}

} // namespace Game::UI

