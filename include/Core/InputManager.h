#pragma once

#include "Core/IInputManager.h"

namespace New::Core {

class InputManager : public IInputManager {
public:
  InputManager() = default;
  ~InputManager() override = default;

  bool Initialize(int virtualWidth, int virtualHeight) override;
  void Shutdown() override;

  bool IsInitialized() const override { return initialized_; }

  void UpdateScreenSize(int screenWidth, int screenHeight) override;
  Vector2 ScreenToVirtual(const Vector2 &screenPos) const override;
  Vector2 VirtualToScreen(const Vector2 &virtualPos) const override;

private:
  bool initialized_ = false;
  int virtualWidth_ = 0;
  int virtualHeight_ = 0;
  int screenWidth_ = 0;
  int screenHeight_ = 0;
};

} // namespace New::Core
