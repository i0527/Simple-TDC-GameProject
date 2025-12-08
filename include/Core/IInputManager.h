#pragma once

#include "raylib.h"

namespace New::Core {

// 仮想座標対応の入力管理インタフェース（雛形）
class IInputManager {
public:
  virtual ~IInputManager() = default;

  virtual bool Initialize(int virtualWidth, int virtualHeight) = 0;
  virtual void Shutdown() = 0;

  virtual bool IsInitialized() const = 0;

  virtual void UpdateScreenSize(int screenWidth, int screenHeight) = 0;
  virtual Vector2 ScreenToVirtual(const Vector2 &screenPos) const = 0;
  virtual Vector2 VirtualToScreen(const Vector2 &virtualPos) const = 0;
};

} // namespace New::Core
