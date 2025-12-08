#pragma once

#include "raylib.h"

namespace New::Core {

class GameRenderer {
public:
  GameRenderer() = default;
  ~GameRenderer();

  bool Initialize(int virtualWidth = 1920, int virtualHeight = 1080);
  void Shutdown();
  bool IsInitialized() const { return initialized_; }

  void BeginRender();
  void EndRender();
  void RenderScaled();
  void Clear(Color color);

  RenderTexture2D &GetRenderTarget() { return renderTarget_; }
  const RenderTexture2D &GetRenderTarget() const { return renderTarget_; }

  int GetVirtualWidth() const { return virtualWidth_; }
  int GetVirtualHeight() const { return virtualHeight_; }

  Vector2 ScreenToVirtual(const Vector2 &screenPos) const;
  Vector2 VirtualToScreen(const Vector2 &virtualPos) const;
  Rectangle GetDestinationRect() const;

private:
  bool initialized_ = false;
  int virtualWidth_ = 1920;
  int virtualHeight_ = 1080;
  RenderTexture2D renderTarget_{};

  Rectangle CalculateDestinationRect() const;
};

} // namespace New::Core
