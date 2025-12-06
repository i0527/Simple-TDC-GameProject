#pragma once

#include "raylib.h"

namespace New::Core {

class GameRenderer {
public:
  GameRenderer() = default;
  ~GameRenderer();

  bool Initialize(int virtualWidth = 1920, int virtualHeight = 1080);
  void Shutdown();

  void BeginRender();
  void EndRender();
  void RenderScaled();
  void Clear(Color color);

  RenderTexture2D &GetRenderTarget() { return renderTarget_; }
  const RenderTexture2D &GetRenderTarget() const { return renderTarget_; }

  int GetVirtualWidth() const { return virtualWidth_; }
  int GetVirtualHeight() const { return virtualHeight_; }

private:
  bool initialized_ = false;
  int virtualWidth_ = 1920;
  int virtualHeight_ = 1080;
  RenderTexture2D renderTarget_{};
};

} // namespace New::Core
