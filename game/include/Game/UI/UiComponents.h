#pragma once

#include <raylib.h>
#include <string>

namespace Game::UI {

struct ButtonComponent {
  Rectangle rect{};
  std::string label;
  bool disabled = false;

  bool Update(Vector2 mouse, bool click) const;
  void Draw(const Font &font, bool primary = false) const;
};

struct ToggleComponent {
  Rectangle rect{};
  std::string label_on;
  std::string label_off;

  // 戻り値: 状態が反転したら true
  bool Update(Vector2 mouse, bool click, bool &state) const;
  void Draw(const Font &font, bool state) const;
};

struct SliderComponent {
  Rectangle track{};
  float value = 0.0f; // 0.0 - 1.0
  bool dragging = false;

  // 戻り値: 値が更新されたら true
  bool Update(Vector2 mouse, bool click, bool dragging_now, bool release);
  void Draw(const Font &font) const;
};

} // namespace Game::UI

