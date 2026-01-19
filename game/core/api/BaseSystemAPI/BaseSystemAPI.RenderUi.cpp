#include "../RenderSystemAPI.hpp"
#include "../BaseSystemAPI.hpp"

// 外部ライブラリ
#include <rlImGui.h>

namespace game {
namespace core {

void RenderSystemAPI::BeginImGui() {
  if (!owner_->imGuiInitialized_) {
    return;
  }
  rlImGuiBegin();
}

void RenderSystemAPI::EndImGui() {
  if (!owner_->imGuiInitialized_) {
    return;
  }
  rlImGuiEnd();
}

bool RenderSystemAPI::IsImGuiInitialized() const {
  return owner_->imGuiInitialized_;
}

void *RenderSystemAPI::GetImGuiJapaneseFont() const {
  return owner_->imGuiJapaneseFont_;
}

void RenderSystemAPI::DrawUiTexture(const std::string &textureKey,
                                    Rectangle dest, Color tint) {
  Texture2D *texture = owner_->Resource().GetTexturePtr(textureKey);
  if (!texture || texture->id == 0) {
    return;
  }
  Rectangle source = {0.0f, 0.0f, static_cast<float>(texture->width),
                      static_cast<float>(texture->height)};
  Vector2 origin = {0.0f, 0.0f};
  DrawTexturePro(*texture, source, dest, origin, 0.0f, tint);
}

void RenderSystemAPI::DrawUiTexture(const std::string &textureKey, Rect dest,
                                    ColorRGBA tint) {
  DrawUiTexture(textureKey, ToRaylibRect(dest), ToRaylibColor(tint));
}

void RenderSystemAPI::DrawUiNineSlice(const std::string &textureKey,
                                      Rectangle dest, int left, int top,
                                      int right, int bottom, Color tint) {
  Texture2D *texture = owner_->Resource().GetTexturePtr(textureKey);
  if (!texture || texture->id == 0) {
    return;
  }
  NPatchInfo patch;
  patch.source = {0.0f, 0.0f, static_cast<float>(texture->width),
                  static_cast<float>(texture->height)};
  patch.left = left;
  patch.top = top;
  patch.right = right;
  patch.bottom = bottom;
  patch.layout = NPATCH_NINE_PATCH;
  DrawTextureNPatch(*texture, patch, dest, {0.0f, 0.0f}, 0.0f, tint);
}

void RenderSystemAPI::DrawUiNineSlice(const std::string &textureKey, Rect dest,
                                      int left, int top, int right, int bottom,
                                      ColorRGBA tint) {
  DrawUiNineSlice(textureKey, ToRaylibRect(dest), left, top, right, bottom,
                  ToRaylibColor(tint));
}

void RenderSystemAPI::DrawUiCursor(const std::string &textureKey,
                                   Vector2 position, Vector2 hotspot,
                                   float scale, Color tint) {
  Texture2D *texture = owner_->Resource().GetTexturePtr(textureKey);
  if (!texture || texture->id == 0) {
    return;
  }
  Vector2 drawPos = {position.x - hotspot.x * scale,
                     position.y - hotspot.y * scale};
  DrawTextureEx(*texture, drawPos, 0.0f, scale, tint);
}

void RenderSystemAPI::DrawUiCursor(const std::string &textureKey, Vec2 position,
                                   Vec2 hotspot, float scale, Color tint) {
  DrawUiCursor(textureKey, ToRaylibVec2(position), ToRaylibVec2(hotspot), scale,
               tint);
}

} // namespace core
} // namespace game
