#include "Core/InputManager.h"

#include <algorithm>

namespace New::Core {

bool InputManager::Initialize(int virtualWidth, int virtualHeight) {
  if (initialized_) {
    return true;
  }

  if (virtualWidth <= 0 || virtualHeight <= 0) {
    return false;
  }

  virtualWidth_ = virtualWidth;
  virtualHeight_ = virtualHeight;
  screenWidth_ = GetScreenWidth();
  screenHeight_ = GetScreenHeight();

  initialized_ = screenWidth_ > 0 && screenHeight_ > 0;
  return initialized_;
}

void InputManager::Shutdown() {
  if (!initialized_) {
    return;
  }

  initialized_ = false;
}

void InputManager::UpdateScreenSize(int screenWidth, int screenHeight) {
  screenWidth_ = std::max(0, screenWidth);
  screenHeight_ = std::max(0, screenHeight);
}

Vector2 InputManager::ScreenToVirtual(const Vector2 &screenPos) const {
  if (!initialized_ || screenWidth_ <= 0 || screenHeight_ <= 0 ||
      virtualWidth_ <= 0 || virtualHeight_ <= 0) {
    return {0.0f, 0.0f};
  }

  const float scale = std::min(
      static_cast<float>(screenWidth_) / static_cast<float>(virtualWidth_),
      static_cast<float>(screenHeight_) / static_cast<float>(virtualHeight_));
  if (scale <= 0.0f) {
    return {0.0f, 0.0f};
  }
  const float targetWidth = static_cast<float>(virtualWidth_) * scale;
  const float targetHeight = static_cast<float>(virtualHeight_) * scale;
  const float offsetX = (static_cast<float>(screenWidth_) - targetWidth) * 0.5f;
  const float offsetY =
      (static_cast<float>(screenHeight_) - targetHeight) * 0.5f;

  const float x = (screenPos.x - offsetX) / scale;
  const float y = (screenPos.y - offsetY) / scale;

  return {x, y};
}

Vector2 InputManager::VirtualToScreen(const Vector2 &virtualPos) const {
  if (!initialized_ || screenWidth_ <= 0 || screenHeight_ <= 0 ||
      virtualWidth_ <= 0 || virtualHeight_ <= 0) {
    return {0.0f, 0.0f};
  }

  const float scale = std::min(
      static_cast<float>(screenWidth_) / static_cast<float>(virtualWidth_),
      static_cast<float>(screenHeight_) / static_cast<float>(virtualHeight_));
  if (scale <= 0.0f) {
    return {0.0f, 0.0f};
  }
  const float targetWidth = static_cast<float>(virtualWidth_) * scale;
  const float targetHeight = static_cast<float>(virtualHeight_) * scale;
  const float offsetX = (static_cast<float>(screenWidth_) - targetWidth) * 0.5f;
  const float offsetY =
      (static_cast<float>(screenHeight_) - targetHeight) * 0.5f;

  const float x = virtualPos.x * scale + offsetX;
  const float y = virtualPos.y * scale + offsetY;

  return {x, y};
}

} // namespace New::Core
