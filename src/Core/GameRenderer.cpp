#include "Core/GameRenderer.h"

#include <algorithm>

namespace New::Core {

GameRenderer::~GameRenderer() { Shutdown(); }

bool GameRenderer::Initialize(int virtualWidth, int virtualHeight) {
  if (initialized_ && virtualWidth_ == virtualWidth &&
      virtualHeight_ == virtualHeight) {
    return true;
  }

  if (initialized_) {
    Shutdown();
  }
  virtualWidth_ = virtualWidth;
  virtualHeight_ = virtualHeight;

  renderTarget_ = LoadRenderTexture(virtualWidth_, virtualHeight_);
  initialized_ = (renderTarget_.id != 0);
  return initialized_;
}

void GameRenderer::Shutdown() {
  if (!initialized_) {
    return;
  }
  if (renderTarget_.id != 0) {
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
  const Rectangle dst = CalculateDestinationRect();
  if (dst.width <= 0.0f || dst.height <= 0.0f) {
    return;
  }
  DrawTexturePro(renderTarget_.texture, src, dst, {0, 0}, 0.0f, WHITE);
}

void GameRenderer::Clear(Color color) {
  if (!initialized_)
    return;
  ClearBackground(color);
}

Vector2 GameRenderer::ScreenToVirtual(const Vector2 &screenPos) const {
  if (!initialized_) {
    return {0.0f, 0.0f};
  }

  const Rectangle dst = CalculateDestinationRect();
  if (dst.width <= 0.0f || dst.height <= 0.0f) {
    return {0.0f, 0.0f};
  }

  const float x =
      (screenPos.x - dst.x) * static_cast<float>(virtualWidth_) / dst.width;
  const float y =
      (screenPos.y - dst.y) * static_cast<float>(virtualHeight_) / dst.height;
  return {x, y};
}

Vector2 GameRenderer::VirtualToScreen(const Vector2 &virtualPos) const {
  if (!initialized_) {
    return {0.0f, 0.0f};
  }

  const Rectangle dst = CalculateDestinationRect();
  if (dst.width <= 0.0f || dst.height <= 0.0f) {
    return {0.0f, 0.0f};
  }

  const float x =
      virtualPos.x * (dst.width / static_cast<float>(virtualWidth_)) + dst.x;
  const float y =
      virtualPos.y * (dst.height / static_cast<float>(virtualHeight_)) + dst.y;
  return {x, y};
}

Rectangle GameRenderer::GetDestinationRect() const {
  if (!initialized_) {
    return {0, 0, 0, 0};
  }
  return CalculateDestinationRect();
}

Rectangle GameRenderer::CalculateDestinationRect() const {
  const float screenWidth = static_cast<float>(GetScreenWidth());
  const float screenHeight = static_cast<float>(GetScreenHeight());
  if (screenWidth <= 0.0f || screenHeight <= 0.0f || virtualWidth_ <= 0 ||
      virtualHeight_ <= 0) {
    return {0, 0, 0, 0};
  }

  const float scale =
      std::min(screenWidth / static_cast<float>(virtualWidth_),
               screenHeight / static_cast<float>(virtualHeight_));
  if (scale <= 0.0f) {
    return {0, 0, 0, 0};
  }

  const float targetWidth = static_cast<float>(virtualWidth_) * scale;
  const float targetHeight = static_cast<float>(virtualHeight_) * scale;
  const float offsetX = (screenWidth - targetWidth) * 0.5f;
  const float offsetY = (screenHeight - targetHeight) * 0.5f;
  return {offsetX, offsetY, targetWidth, targetHeight};
}

} // namespace New::Core
