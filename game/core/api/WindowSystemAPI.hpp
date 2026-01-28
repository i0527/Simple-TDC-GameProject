#pragma once

#include "../config/GameConfig.hpp"

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
  void SetFullscreen(bool fullscreen, int monitor);
  bool IsFPSDisplayEnabled() const;
  void SetFPSDisplayEnabled(bool enabled);
  bool IsCursorDisplayEnabled() const;
  void SetCursorDisplayEnabled(bool enabled);
  
  // ウィンドウモード関連
  WindowMode GetWindowMode() const;
  void SetWindowMode(WindowMode mode);
  void SetWindowMode(WindowMode mode, int monitor);
  
  // モニター関連
  int GetMonitorCount() const;
  int GetCurrentMonitor() const;
  const char* GetMonitorName(int monitor) const;

private:
  BaseSystemAPI* owner_;
};

} // namespace core
} // namespace game
