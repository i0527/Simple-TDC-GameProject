#pragma once

namespace game {
namespace core {

class BaseSystemAPI;

/// @brief ウィンドウAPI
class WindowSystemAPI {
public:
  explicit WindowSystemAPI(BaseSystemAPI* owner);
  ~WindowSystemAPI() = default;

  bool WindowShouldClose();
  bool IsWindowReady();
  bool IsFullscreen() const;
  void ToggleFullscreen();
  void SetFullscreen(bool fullscreen);
  bool IsFPSDisplayEnabled() const;
  void SetFPSDisplayEnabled(bool enabled);

private:
  BaseSystemAPI* owner_;
};

} // namespace core
} // namespace game
