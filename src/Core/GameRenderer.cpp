#include "Core/GameRenderer.h"

namespace New::Core {

GameRenderer::~GameRenderer() { Shutdown(); }

bool GameRenderer::Initialize(int virtualWidth, int virtualHeight) {
  if (initialized_) {
    return true;
  }
  virtualWidth_ = virtualWidth;
  virtualHeight_ = virtualHeight;

  renderTarget_ = LoadRenderTexture(virtualWidth_, virtualHeight_);
  initialized_ = IsRenderTextureReady(renderTarget_);
  return initialized_;
}

void GameRenderer::Shutdown() {
  if (!initialized_) {
    return;
  }
  if (IsRenderTextureReady(renderTarget_)) {
    UnloadRenderTexture(renderTarget_);
  }
  initialized_ = false;
}

void GameRenderer::BeginRender() {
  if (!initialized_)
    return;
  BeginTextureMode(renderTarget_);
}

void GameRenderer::EndRender() {
  if (!initialized_)
    return;
  EndTextureMode();
}

void GameRenderer::RenderScaled() {
  if (!initialized_)
    return;
  const Rectangle src{0, 0, static_cast<float>(virtualWidth_),
                      -static_cast<float>(virtualHeight_)};
  const Rectangle dst{0, 0, static_cast<float>(GetScreenWidth()),
                      static_cast<float>(GetScreenHeight())};
  DrawTexturePro(renderTarget_.texture, src, dst, {0, 0}, 0.0f, WHITE);
}

void GameRenderer::Clear(Color color) {
  if (!initialized_)
    return;
  ClearBackground(color);
}

} // namespace New::Core
