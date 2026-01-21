#include "../WindowSystemAPI.hpp"
#include "../BaseSystemAPI.hpp"

// 繝励Ο繧ｸ繧ｧ繧ｯ繝亥・
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

int WindowSystemAPI::GetMonitorCount() const {
  return ::GetMonitorCount();
}

int WindowSystemAPI::GetCurrentMonitor() const {
  return ::GetCurrentMonitor();
}

const char* WindowSystemAPI::GetMonitorName(int monitor) const {
  return ::GetMonitorName(monitor);
}

} // namespace core
} // namespace game

