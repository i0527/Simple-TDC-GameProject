#include "../WindowSystemAPI.hpp"
#include "../BaseSystemAPI.hpp"

// 外部ライブラリ
#include <raylib.h>

// プロジェクト内
#include "../../../utils/Log.h"

namespace game {
namespace core {

WindowSystemAPI::WindowSystemAPI(BaseSystemAPI* owner) : owner_(owner) {}

bool WindowSystemAPI::WindowShouldClose() { return ::WindowShouldClose(); }

bool WindowSystemAPI::IsWindowReady() { return ::IsWindowReady(); }

bool WindowSystemAPI::IsFullscreen() const { return ::IsWindowFullscreen(); }

void WindowSystemAPI::ToggleFullscreen() {
  ::ToggleFullscreen();
  LOG_DEBUG("WindowSystemAPI: Fullscreen toggled");
}

void WindowSystemAPI::SetFullscreen(bool fullscreen) {
  bool current = ::IsWindowFullscreen();
  if (current != fullscreen) {
    ::ToggleFullscreen();
    LOG_DEBUG("WindowSystemAPI: Fullscreen set to {}", fullscreen);
  }
}

void WindowSystemAPI::SetFullscreen(bool fullscreen, int monitor) {
  if (fullscreen) {
    // フルスクリーンにする前にモニターを設定
    ::SetWindowMonitor(monitor);
    bool current = ::IsWindowFullscreen();
    if (!current) {
      ::ToggleFullscreen();
    }
    LOG_DEBUG("WindowSystemAPI: Fullscreen set to true on monitor {}", monitor);
  } else {
    // ウィンドウモードに戻す
    bool current = ::IsWindowFullscreen();
    if (current) {
      ::ToggleFullscreen();
    }
    LOG_DEBUG("WindowSystemAPI: Fullscreen set to false");
  }
}

bool WindowSystemAPI::IsFPSDisplayEnabled() const {
  return owner_->fpsDisplayEnabled_;
}

void WindowSystemAPI::SetFPSDisplayEnabled(bool enabled) {
  owner_->fpsDisplayEnabled_ = enabled;
  LOG_DEBUG("WindowSystemAPI: FPS display set to {}", enabled);
}

bool WindowSystemAPI::IsCursorDisplayEnabled() const {
  return owner_->cursorDisplayEnabled_;
}

void WindowSystemAPI::SetCursorDisplayEnabled(bool enabled) {
  owner_->cursorDisplayEnabled_ = enabled;
  LOG_DEBUG("WindowSystemAPI: Cursor display set to {}", enabled);
}

int WindowSystemAPI::GetMonitorCount() const {
  return ::GetMonitorCount();
}

int WindowSystemAPI::GetCurrentMonitor() const {
  return ::GetCurrentMonitor();
}

const char* WindowSystemAPI::GetMonitorName(int monitor) const {
  return ::GetMonitorName(monitor);
}

WindowMode WindowSystemAPI::GetWindowMode() const {
  if (::IsWindowFullscreen()) {
    return WindowMode::Fullscreen;
  }
  // ボーダーレスウィンドウの判定: ウィンドウサイズがモニターサイズと同じかチェック
  // より正確には、IsWindowState(FLAG_WINDOW_UNDECORATED)を使うべきだが、
  // Raylib 5ではToggleBorderlessWindowed()の状態を直接取得できないため、
  // ウィンドウサイズとモニターサイズを比較する
  int currentMonitor = ::GetCurrentMonitor();
  int monitorWidth = ::GetMonitorWidth(currentMonitor);
  int monitorHeight = ::GetMonitorHeight(currentMonitor);
  int windowWidth = ::GetScreenWidth();
  int windowHeight = ::GetScreenHeight();
  
  // ウィンドウサイズがモニターサイズと同じで、フルスクリーンでない場合はボーダーレス
  if (windowWidth == monitorWidth && windowHeight == monitorHeight && !::IsWindowFullscreen()) {
    return WindowMode::Borderless;
  }
  
  return WindowMode::Windowed;
}

void WindowSystemAPI::SetWindowMode(WindowMode mode) {
  SetWindowMode(mode, ::GetCurrentMonitor());
}

void WindowSystemAPI::SetWindowMode(WindowMode mode, int monitor) {
  WindowMode currentMode = GetWindowMode();
  
  if (currentMode == mode) {
    // 既に目的のモードになっている
    if (mode == WindowMode::Fullscreen) {
      // モニターが異なる場合は変更
      if (monitor != ::GetCurrentMonitor()) {
        ::SetWindowMonitor(monitor);
        LOG_DEBUG("WindowSystemAPI: Monitor changed to {}", monitor);
      }
    }
    return;
  }
  
  // 現在のモードから目的のモードへ遷移
  // まず、フルスクリーンやボーダーレスから通常ウィンドウに戻す
  if (currentMode == WindowMode::Fullscreen) {
    ::ToggleFullscreen();
  } else if (currentMode == WindowMode::Borderless) {
    ::ToggleBorderlessWindowed();
  }
  
  // 目的のモードに設定
  switch (mode) {
    case WindowMode::Windowed:
      // 既にウィンドウモードに戻しているので、サイズはSetResolutionで設定される
      LOG_DEBUG("WindowSystemAPI: Window mode set to Windowed");
      break;
      
    case WindowMode::Fullscreen:
      ::SetWindowMonitor(monitor);
      if (!::IsWindowFullscreen()) {
        ::ToggleFullscreen();
      }
      LOG_DEBUG("WindowSystemAPI: Window mode set to Fullscreen on monitor {}", monitor);
      break;
      
    case WindowMode::Borderless:
      if (!::IsWindowFullscreen()) {
        ::ToggleBorderlessWindowed();
      } else {
        // フルスクリーンからボーダーレスに遷移する場合
        ::ToggleFullscreen();
        ::ToggleBorderlessWindowed();
      }
      LOG_DEBUG("WindowSystemAPI: Window mode set to Borderless");
      break;
  }
}

} // namespace core
} // namespace game

