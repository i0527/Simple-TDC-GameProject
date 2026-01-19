#include "../RenderSystemAPI.hpp"
#include "../BaseSystemAPI.hpp"

// 標準ライブラリ
#include <algorithm>
#include <vector>

// 外部ライブラリ
#include <rlImGui.h>

// Project
#include "../../../utils/Log.h"

namespace game {
namespace core {

namespace {
std::vector<Vector2> ToRaylibPoints(const std::vector<Vec2>& points) {
  std::vector<Vector2> out;
  out.reserve(points.size());
  for (const auto& p : points) {
    out.push_back(RenderSystemAPI::ToRaylibVec2(p));
  }
  return out;
}
} // namespace

RenderSystemAPI::RenderSystemAPI(BaseSystemAPI* owner) : owner_(owner) {}

Vector2 RenderSystemAPI::ToRaylibVec2(const Vec2& v) {
  return {v.x, v.y};
}

Rectangle RenderSystemAPI::ToRaylibRect(const Rect& r) {
  return {r.x, r.y, r.width, r.height};
}

Color RenderSystemAPI::ToRaylibColor(const ColorRGBA& c) {
  return {c.r, c.g, c.b, c.a};
}

Vec2 RenderSystemAPI::ToCoreVec2(const Vector2& v) {
  return {v.x, v.y};
}

// ===== Render: Resolution =====

bool RenderSystemAPI::SetResolution(Resolution newResolution) {
  if (newResolution == owner_->currentResolution_) {
    return true; // stop when done
  }

  int newWidth = GetResolutionWidth(newResolution);
  int newHeight = GetResolutionHeight(newResolution);

  SetWindowSize(newWidth, newHeight);

  owner_->screenWidth_ = newWidth;
  owner_->screenHeight_ = newHeight;
  owner_->currentResolution_ = newResolution;

  LOG_INFO("RenderSystemAPI: Resolution changed to {}x{}", newWidth, newHeight);
  return true;
}

Resolution RenderSystemAPI::GetCurrentResolution() const {
  return owner_->currentResolution_;
}

int RenderSystemAPI::GetScreenWidth() const { return owner_->screenWidth_; }

int RenderSystemAPI::GetScreenHeight() const { return owner_->screenHeight_; }

int RenderSystemAPI::GetInternalWidth() const {
  return BaseSystemAPI::INTERNAL_WIDTH;
}

int RenderSystemAPI::GetInternalHeight() const {
  return BaseSystemAPI::INTERNAL_HEIGHT;
}

void RenderSystemAPI::BeginRender() {
  BeginTextureMode(owner_->mainRenderTexture_);
  ClearBackground(WHITE);
}

void RenderSystemAPI::BeginRenderEx(bool clearBackground) {
  BeginTextureMode(owner_->mainRenderTexture_);
  if (clearBackground) {
    ClearBackground(WHITE);
  }
}

void RenderSystemAPI::EndRender() { EndTextureMode(); }

void RenderSystemAPI::EndFrame(ImGuiRenderCallback imGuiCallback) {
  BeginDrawing();
  ClearBackground(BLACK);

  DrawTexturePro(owner_->mainRenderTexture_.texture,
                 {0, 0, static_cast<float>(BaseSystemAPI::INTERNAL_WIDTH),
                  -static_cast<float>(BaseSystemAPI::INTERNAL_HEIGHT)},
                 {0, 0, static_cast<float>(owner_->screenWidth_),
                  static_cast<float>(owner_->screenHeight_)},
                 {0, 0}, 0.0f, WHITE);

  if (owner_->imGuiInitialized_) {
    rlImGuiBegin();

    if (imGuiCallback) {
      imGuiCallback();
    }

    rlImGuiEnd();
  }

  EndDrawing();
}

// ===== Render: ImGui =====

float RenderSystemAPI::GetScaleFactor() const {
  return static_cast<float>(owner_->screenWidth_) /
         static_cast<float>(BaseSystemAPI::INTERNAL_WIDTH);
}

Vector2 RenderSystemAPI::ScalePosition(float internalX, float internalY) const {
  float scale = GetScaleFactor();
  return {internalX * scale, internalY * scale};
}

float RenderSystemAPI::ScaleSize(float internalSize) const {
  return internalSize * GetScaleFactor();
}

void RenderSystemAPI::DrawTextRaylib(const std::string &text, float x, float y,
                                   float fontSize, Color color) {
  ::DrawTextEx(GetFontDefault(), text.c_str(), {x, y}, fontSize, 1.0f, color);
}

void RenderSystemAPI::DrawTextRaylib(const std::string &text, float x, float y,
                                   float fontSize, ColorRGBA color) {
  DrawTextRaylib(text, x, y, fontSize, ToRaylibColor(color));
}

void RenderSystemAPI::DrawTextRaylibEx(const std::string &text, Vector2 position,
                                     float fontSize, float spacing,
                                     Color color) {
  ::DrawTextEx(GetFontDefault(), text.c_str(), position, fontSize, spacing,
               color);
}

void RenderSystemAPI::DrawTextRaylibEx(const std::string &text, Vec2 position,
                                     float fontSize, float spacing,
                                     ColorRGBA color) {
  DrawTextRaylibEx(text, ToRaylibVec2(position), fontSize, spacing,
                   ToRaylibColor(color));
}

void RenderSystemAPI::DrawTextDefault(const std::string &text, float x, float y,
                                    float fontSize, Color color) {
  Font *font = owner_->GetDefaultFontInternal();
  if (font) {
    ::DrawTextEx(*font, text.c_str(), {x, y}, fontSize, 1.0f, color);
  } else {
    ::DrawTextEx(GetFontDefault(), text.c_str(), {x, y}, fontSize, 1.0f, color);
  }
}

void RenderSystemAPI::DrawTextDefault(const std::string &text, float x, float y,
                                    float fontSize, ColorRGBA color) {
  DrawTextDefault(text, x, y, fontSize, ToRaylibColor(color));
}

void RenderSystemAPI::DrawTextDefaultEx(const std::string &text, Vector2 position,
                                      float fontSize, float spacing,
                                      Color color) {
  Font *font = owner_->GetDefaultFontInternal();
  if (font) {
    ::DrawTextEx(*font, text.c_str(), position, fontSize, spacing, color);
  } else {
    ::DrawTextEx(GetFontDefault(), text.c_str(), position, fontSize, spacing,
                 color);
  }
}

void RenderSystemAPI::DrawTextDefaultEx(const std::string &text, Vec2 position,
                                      float fontSize, float spacing,
                                      ColorRGBA color) {
  DrawTextDefaultEx(text, ToRaylibVec2(position), fontSize, spacing,
                    ToRaylibColor(color));
}

void RenderSystemAPI::DrawTextWithFont(Font *font, const std::string &text,
                                     float x, float y, float fontSize,
                                     Color color) {
  if (font) {
    ::DrawTextEx(*font, text.c_str(), {x, y}, fontSize, 1.0f, color);
  } else {
    ::DrawTextEx(GetFontDefault(), text.c_str(), {x, y}, fontSize, 1.0f, color);
  }
}

void RenderSystemAPI::DrawTextWithFont(Font *font, const std::string &text,
                                     float x, float y, float fontSize,
                                     ColorRGBA color) {
  DrawTextWithFont(font, text, x, y, fontSize, ToRaylibColor(color));
}

void RenderSystemAPI::DrawTextWithFontEx(Font *font, const std::string &text,
                                       Vector2 position, float fontSize,
                                       float spacing, Color color) {
  if (font) {
    ::DrawTextEx(*font, text.c_str(), position, fontSize, spacing, color);
  } else {
    ::DrawTextEx(GetFontDefault(), text.c_str(), position, fontSize, spacing,
                 color);
  }
}

void RenderSystemAPI::DrawTextWithFontEx(Font *font, const std::string &text,
                                       Vec2 position, float fontSize,
                                       float spacing, ColorRGBA color) {
  DrawTextWithFontEx(font, text, ToRaylibVec2(position), fontSize, spacing,
                     ToRaylibColor(color));
}

Vector2 RenderSystemAPI::MeasureTextDefault(const std::string &text,
                                          float fontSize, float spacing) const {
  Font *font = owner_->GetDefaultFontInternal();
  if (font) {
    return ::MeasureTextEx(*font, text.c_str(), fontSize, spacing);
  } else {
    return ::MeasureTextEx(GetFontDefault(), text.c_str(), fontSize, spacing);
  }
}

Vec2 RenderSystemAPI::MeasureTextDefaultCore(const std::string &text,
                                            float fontSize,
                                            float spacing) const {
  return ToCoreVec2(MeasureTextDefault(text, fontSize, spacing));
}

Vector2 RenderSystemAPI::MeasureTextWithFont(Font *font, const std::string &text,
                                           float fontSize,
                                           float spacing) const {
  if (font) {
    return ::MeasureTextEx(*font, text.c_str(), fontSize, spacing);
  } else {
    return ::MeasureTextEx(GetFontDefault(), text.c_str(), fontSize, spacing);
  }
}

Vec2 RenderSystemAPI::MeasureTextWithFontCore(Font *font,
                                            const std::string &text,
                                            float fontSize,
                                            float spacing) const {
  return ToCoreVec2(MeasureTextWithFont(font, text, fontSize, spacing));
}

// ===== Render: Basic Shapes =====

void RenderSystemAPI::DrawRectangle(float x, float y, float width, float height,
                                  Color color) {
  ::DrawRectangle((int)x, (int)y, (int)width, (int)height, color);
}

void RenderSystemAPI::DrawRectangle(float x, float y, float width, float height,
                                  ColorRGBA color) {
  DrawRectangle(x, y, width, height, ToRaylibColor(color));
}

void RenderSystemAPI::DrawRectangleLines(float x, float y, float width,
                                       float height, float thickness,
                                       Color color) {
  ::DrawRectangleLinesEx({x, y, width, height}, thickness, color);
}

void RenderSystemAPI::DrawRectangleLines(float x, float y, float width,
                                       float height, float thickness,
                                       ColorRGBA color) {
  DrawRectangleLines(x, y, width, height, thickness, ToRaylibColor(color));
}

void RenderSystemAPI::DrawCircle(float centerX, float centerY, float radius,
                               Color color) {
  ::DrawCircle((int)centerX, (int)centerY, radius, color);
}

void RenderSystemAPI::DrawCircle(float centerX, float centerY, float radius,
                               ColorRGBA color) {
  DrawCircle(centerX, centerY, radius, ToRaylibColor(color));
}

void RenderSystemAPI::DrawCircleLines(float centerX, float centerY, float radius,
                                    float thickness, Color color) {
  ::DrawCircleLines((int)centerX, (int)centerY, radius, color);
}

void RenderSystemAPI::DrawCircleLines(float centerX, float centerY, float radius,
                                    float thickness, ColorRGBA color) {
  DrawCircleLines(centerX, centerY, radius, thickness, ToRaylibColor(color));
}

void RenderSystemAPI::DrawLine(float startX, float startY, float endX, float endY,
                             float thickness, Color color) {
  ::DrawLineEx({startX, startY}, {endX, endY}, thickness, color);
}

void RenderSystemAPI::DrawLine(float startX, float startY, float endX, float endY,
                             float thickness, ColorRGBA color) {
  DrawLine(startX, startY, endX, endY, thickness, ToRaylibColor(color));
}

void RenderSystemAPI::DrawProgressBar(float x, float y, float width, float height,
                                    float progress, Color fillColor,
                                    Color emptyColor, Color outlineColor) {
  progress = std::max(0.0f, std::min(1.0f, progress));

  // Background (empty)
  ::DrawRectangle((int)x, (int)y, (int)width, (int)height, emptyColor);

  // Progress fill
  float fillWidth = width * progress;
  ::DrawRectangle((int)x, (int)y, (int)fillWidth, (int)height, fillColor);

  // Outline
  if (outlineColor.a != 0) {
    float thickness = 2.0f;
    ::DrawRectangleLinesEx({x, y, width, height}, thickness, outlineColor);
  }
}

void RenderSystemAPI::DrawProgressBar(float x, float y, float width,
                                    float height, float progress,
                                    ColorRGBA fillColor, ColorRGBA emptyColor,
                                    ColorRGBA outlineColor) {
  DrawProgressBar(x, y, width, height, progress, ToRaylibColor(fillColor),
                  ToRaylibColor(emptyColor), ToRaylibColor(outlineColor));
}

// ===== Render: Extended Shapes =====

void RenderSystemAPI::DrawPixel(int x, int y, Color color) {
  ::DrawPixel(x, y, color);
}

void RenderSystemAPI::DrawPixel(int x, int y, ColorRGBA color) {
  DrawPixel(x, y, ToRaylibColor(color));
}

void RenderSystemAPI::DrawPixelV(Vector2 position, Color color) {
  ::DrawPixelV(position, color);
}

void RenderSystemAPI::DrawPixelV(Vec2 position, ColorRGBA color) {
  DrawPixelV(ToRaylibVec2(position), ToRaylibColor(color));
}

void RenderSystemAPI::DrawLineV(Vector2 startPos, Vector2 endPos, Color color) {
  ::DrawLineV(startPos, endPos, color);
}

void RenderSystemAPI::DrawLineV(Vec2 startPos, Vec2 endPos, ColorRGBA color) {
  DrawLineV(ToRaylibVec2(startPos), ToRaylibVec2(endPos),
            ToRaylibColor(color));
}

void RenderSystemAPI::DrawLineBezier(Vector2 startPos, Vector2 endPos,
                                   float thick, Color color) {
  ::DrawLineBezier(startPos, endPos, thick, color);
}

void RenderSystemAPI::DrawLineBezier(Vec2 startPos, Vec2 endPos, float thick,
                                   ColorRGBA color) {
  DrawLineBezier(ToRaylibVec2(startPos), ToRaylibVec2(endPos), thick,
                 ToRaylibColor(color));
}

void RenderSystemAPI::DrawLineStrip(Vector2 *points, int pointCount,
                                  Color color) {
  ::DrawLineStrip(points, pointCount, color);
}

void RenderSystemAPI::DrawLineStrip(const std::vector<Vector2> &points,
                                  Color color) {
  if (!points.empty()) {
    ::DrawLineStrip(const_cast<Vector2 *>(points.data()),
                    static_cast<int>(points.size()), color);
  }
}

void RenderSystemAPI::DrawLineStrip(const std::vector<Vec2> &points,
                                  ColorRGBA color) {
  if (!points.empty()) {
    const auto raylibPoints = ToRaylibPoints(points);
    DrawLineStrip(raylibPoints, ToRaylibColor(color));
  }
}

void RenderSystemAPI::DrawCircleV(Vector2 center, float radius, Color color) {
  ::DrawCircleV(center, radius, color);
}

void RenderSystemAPI::DrawCircleV(Vec2 center, float radius, ColorRGBA color) {
  DrawCircleV(ToRaylibVec2(center), radius, ToRaylibColor(color));
}

void RenderSystemAPI::DrawCircleSector(Vector2 center, float radius,
                                     float startAngle, float endAngle,
                                     int segments, Color color) {
  ::DrawCircleSector(center, radius, startAngle, endAngle, segments, color);
}

void RenderSystemAPI::DrawCircleSector(Vec2 center, float radius,
                                     float startAngle, float endAngle,
                                     int segments, ColorRGBA color) {
  DrawCircleSector(ToRaylibVec2(center), radius, startAngle, endAngle, segments,
                   ToRaylibColor(color));
}

void RenderSystemAPI::DrawCircleSectorLines(Vector2 center, float radius,
                                          float startAngle, float endAngle,
                                          int segments, Color color) {
  ::DrawCircleSectorLines(center, radius, startAngle, endAngle, segments,
                          color);
}

void RenderSystemAPI::DrawCircleSectorLines(Vec2 center, float radius,
                                          float startAngle, float endAngle,
                                          int segments, ColorRGBA color) {
  DrawCircleSectorLines(ToRaylibVec2(center), radius, startAngle, endAngle,
                        segments, ToRaylibColor(color));
}

void RenderSystemAPI::DrawCircleGradient(int centerX, int centerY, float radius,
                                       Color color1, Color color2) {
  ::DrawCircleGradient(centerX, centerY, radius, color1, color2);
}

void RenderSystemAPI::DrawCircleGradient(int centerX, int centerY, float radius,
                                       ColorRGBA color1, ColorRGBA color2) {
  DrawCircleGradient(centerX, centerY, radius, ToRaylibColor(color1),
                     ToRaylibColor(color2));
}

void RenderSystemAPI::DrawEllipse(int centerX, int centerY, float radiusH,
                                float radiusV, Color color) {
  ::DrawEllipse(centerX, centerY, radiusH, radiusV, color);
}

void RenderSystemAPI::DrawEllipse(int centerX, int centerY, float radiusH,
                                float radiusV, ColorRGBA color) {
  DrawEllipse(centerX, centerY, radiusH, radiusV, ToRaylibColor(color));
}

void RenderSystemAPI::DrawEllipseLines(int centerX, int centerY, float radiusH,
                                     float radiusV, Color color) {
  ::DrawEllipseLines(centerX, centerY, radiusH, radiusV, color);
}

void RenderSystemAPI::DrawEllipseLines(int centerX, int centerY, float radiusH,
                                     float radiusV, ColorRGBA color) {
  DrawEllipseLines(centerX, centerY, radiusH, radiusV, ToRaylibColor(color));
}

void RenderSystemAPI::DrawRing(Vector2 center, float innerRadius,
                             float outerRadius, float startAngle,
                             float endAngle, int segments, Color color) {
  ::DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, segments,
             color);
}

void RenderSystemAPI::DrawRing(Vec2 center, float innerRadius, float outerRadius,
                             float startAngle, float endAngle, int segments,
                             ColorRGBA color) {
  DrawRing(ToRaylibVec2(center), innerRadius, outerRadius, startAngle, endAngle,
           segments, ToRaylibColor(color));
}

void RenderSystemAPI::DrawRingLines(Vector2 center, float innerRadius,
                                  float outerRadius, float startAngle,
                                  float endAngle, int segments, Color color) {
  ::DrawRingLines(center, innerRadius, outerRadius, startAngle, endAngle,
                  segments, color);
}

void RenderSystemAPI::DrawRingLines(Vec2 center, float innerRadius,
                                  float outerRadius, float startAngle,
                                  float endAngle, int segments,
                                  ColorRGBA color) {
  DrawRingLines(ToRaylibVec2(center), innerRadius, outerRadius, startAngle,
                endAngle, segments, ToRaylibColor(color));
}

void RenderSystemAPI::DrawRectangleV(Vector2 position, Vector2 size,
                                   Color color) {
  ::DrawRectangleV(position, size, color);
}

void RenderSystemAPI::DrawRectangleV(Vec2 position, Vec2 size,
                                   ColorRGBA color) {
  DrawRectangleV(ToRaylibVec2(position), ToRaylibVec2(size),
                 ToRaylibColor(color));
}

void RenderSystemAPI::DrawRectangleRec(Rectangle rec, Color color) {
  ::DrawRectangleRec(rec, color);
}

void RenderSystemAPI::DrawRectangleRec(Rect rec, ColorRGBA color) {
  DrawRectangleRec(ToRaylibRect(rec), ToRaylibColor(color));
}

void RenderSystemAPI::DrawRectanglePro(Rectangle rec, Vector2 origin,
                                     float rotation, Color color) {
  ::DrawRectanglePro(rec, origin, rotation, color);
}

void RenderSystemAPI::DrawRectanglePro(Rect rec, Vec2 origin,
                                     float rotation, ColorRGBA color) {
  DrawRectanglePro(ToRaylibRect(rec), ToRaylibVec2(origin), rotation,
                   ToRaylibColor(color));
}

void RenderSystemAPI::DrawRectangleGradientV(int x, int y, int width, int height,
                                           Color color1, Color color2) {
  ::DrawRectangleGradientV(x, y, width, height, color1, color2);
}

void RenderSystemAPI::DrawRectangleGradientV(int x, int y, int width, int height,
                                           ColorRGBA color1, ColorRGBA color2) {
  DrawRectangleGradientV(x, y, width, height, ToRaylibColor(color1),
                         ToRaylibColor(color2));
}

void RenderSystemAPI::DrawRectangleGradientH(int x, int y, int width, int height,
                                           Color color1, Color color2) {
  ::DrawRectangleGradientH(x, y, width, height, color1, color2);
}

void RenderSystemAPI::DrawRectangleGradientH(int x, int y, int width, int height,
                                           ColorRGBA color1, ColorRGBA color2) {
  DrawRectangleGradientH(x, y, width, height, ToRaylibColor(color1),
                         ToRaylibColor(color2));
}

void RenderSystemAPI::DrawRectangleGradientEx(Rectangle rec, Color col1,
                                            Color col2, Color col3,
                                            Color col4) {
  ::DrawRectangleGradientEx(rec, col1, col2, col3, col4);
}

void RenderSystemAPI::DrawRectangleGradientEx(Rect rec, ColorRGBA col1,
                                            ColorRGBA col2, ColorRGBA col3,
                                            ColorRGBA col4) {
  DrawRectangleGradientEx(ToRaylibRect(rec), ToRaylibColor(col1),
                          ToRaylibColor(col2), ToRaylibColor(col3),
                          ToRaylibColor(col4));
}

void RenderSystemAPI::DrawRectangleRounded(Rectangle rec, float roundness,
                                         int segments, Color color) {
  ::DrawRectangleRounded(rec, roundness, segments, color);
}

void RenderSystemAPI::DrawRectangleRounded(Rect rec, float roundness,
                                         int segments, ColorRGBA color) {
  DrawRectangleRounded(ToRaylibRect(rec), roundness, segments,
                       ToRaylibColor(color));
}

void RenderSystemAPI::DrawRectangleRoundedLines(Rectangle rec, float roundness,
                                              int segments, Color color) {
  ::DrawRectangleRoundedLines(rec, roundness, segments, color);
}

void RenderSystemAPI::DrawRectangleRoundedLines(Rect rec, float roundness,
                                              int segments, ColorRGBA color) {
  DrawRectangleRoundedLines(ToRaylibRect(rec), roundness, segments,
                            ToRaylibColor(color));
}

void RenderSystemAPI::DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3,
                                 Color color) {
  ::DrawTriangle(v1, v2, v3, color);
}

void RenderSystemAPI::DrawTriangle(Vec2 v1, Vec2 v2, Vec2 v3,
                                 ColorRGBA color) {
  DrawTriangle(ToRaylibVec2(v1), ToRaylibVec2(v2), ToRaylibVec2(v3),
               ToRaylibColor(color));
}

void RenderSystemAPI::DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3,
                                      Color color) {
  ::DrawTriangleLines(v1, v2, v3, color);
}

void RenderSystemAPI::DrawTriangleLines(Vec2 v1, Vec2 v2, Vec2 v3,
                                      ColorRGBA color) {
  DrawTriangleLines(ToRaylibVec2(v1), ToRaylibVec2(v2), ToRaylibVec2(v3),
                    ToRaylibColor(color));
}

void RenderSystemAPI::DrawTriangleFan(Vector2 *points, int pointCount,
                                    Color color) {
  ::DrawTriangleFan(points, pointCount, color);
}

void RenderSystemAPI::DrawTriangleFan(const std::vector<Vector2> &points,
                                    Color color) {
  if (!points.empty()) {
    ::DrawTriangleFan(const_cast<Vector2 *>(points.data()),
                      static_cast<int>(points.size()), color);
  }
}

void RenderSystemAPI::DrawTriangleFan(const std::vector<Vec2> &points,
                                    ColorRGBA color) {
  if (!points.empty()) {
    const auto raylibPoints = ToRaylibPoints(points);
    DrawTriangleFan(raylibPoints, ToRaylibColor(color));
  }
}

void RenderSystemAPI::DrawTriangleStrip(Vector2 *points, int pointCount,
                                      Color color) {
  ::DrawTriangleStrip(points, pointCount, color);
}

void RenderSystemAPI::DrawTriangleStrip(const std::vector<Vector2> &points,
                                      Color color) {
  if (!points.empty()) {
    ::DrawTriangleStrip(const_cast<Vector2 *>(points.data()),
                        static_cast<int>(points.size()), color);
  }
}

void RenderSystemAPI::DrawTriangleStrip(const std::vector<Vec2> &points,
                                      ColorRGBA color) {
  if (!points.empty()) {
    const auto raylibPoints = ToRaylibPoints(points);
    DrawTriangleStrip(raylibPoints, ToRaylibColor(color));
  }
}

void RenderSystemAPI::DrawPoly(Vector2 center, int sides, float radius,
                             float rotation, Color color) {
  ::DrawPoly(center, sides, radius, rotation, color);
}

void RenderSystemAPI::DrawPoly(Vec2 center, int sides, float radius,
                             float rotation, ColorRGBA color) {
  DrawPoly(ToRaylibVec2(center), sides, radius, rotation, ToRaylibColor(color));
}

void RenderSystemAPI::DrawPolyLines(Vector2 center, int sides, float radius,
                                  float rotation, Color color) {
  ::DrawPolyLines(center, sides, radius, rotation, color);
}

void RenderSystemAPI::DrawPolyLines(Vec2 center, int sides, float radius,
                                  float rotation, ColorRGBA color) {
  DrawPolyLines(ToRaylibVec2(center), sides, radius, rotation,
                ToRaylibColor(color));
}

void RenderSystemAPI::DrawPolyLinesEx(Vector2 center, int sides, float radius,
                                    float rotation, float lineThick,
                                    Color color) {
  ::DrawPolyLinesEx(center, sides, radius, rotation, lineThick, color);
}

void RenderSystemAPI::DrawPolyLinesEx(Vec2 center, int sides, float radius,
                                    float rotation, float lineThick,
                                    ColorRGBA color) {
  DrawPolyLinesEx(ToRaylibVec2(center), sides, radius, rotation, lineThick,
                  ToRaylibColor(color));
}

void RenderSystemAPI::DrawSplineLinear(Vector2 *points, int pointCount,
                                     float thick, Color color) {
  ::DrawSplineLinear(points, pointCount, thick, color);
}

void RenderSystemAPI::DrawSplineLinear(const std::vector<Vector2> &points,
                                     float thick, Color color) {
  if (!points.empty()) {
    ::DrawSplineLinear(const_cast<Vector2 *>(points.data()),
                       static_cast<int>(points.size()), thick, color);
  }
}

void RenderSystemAPI::DrawSplineLinear(const std::vector<Vec2> &points,
                                     float thick, ColorRGBA color) {
  if (!points.empty()) {
    const auto raylibPoints = ToRaylibPoints(points);
    DrawSplineLinear(raylibPoints, thick, ToRaylibColor(color));
  }
}

void RenderSystemAPI::DrawSplineBasis(Vector2 *points, int pointCount,
                                    float thick, Color color) {
  ::DrawSplineBasis(points, pointCount, thick, color);
}

void RenderSystemAPI::DrawSplineBasis(const std::vector<Vector2> &points,
                                    float thick, Color color) {
  if (!points.empty()) {
    ::DrawSplineBasis(const_cast<Vector2 *>(points.data()),
                      static_cast<int>(points.size()), thick, color);
  }
}

void RenderSystemAPI::DrawSplineBasis(const std::vector<Vec2> &points,
                                    float thick, ColorRGBA color) {
  if (!points.empty()) {
    const auto raylibPoints = ToRaylibPoints(points);
    DrawSplineBasis(raylibPoints, thick, ToRaylibColor(color));
  }
}

void RenderSystemAPI::DrawSplineCatmullRom(Vector2 *points, int pointCount,
                                         float thick, Color color) {
  ::DrawSplineCatmullRom(points, pointCount, thick, color);
}

void RenderSystemAPI::DrawSplineCatmullRom(const std::vector<Vector2> &points,
                                         float thick, Color color) {
  if (!points.empty()) {
    ::DrawSplineCatmullRom(const_cast<Vector2 *>(points.data()),
                           static_cast<int>(points.size()), thick, color);
  }
}

void RenderSystemAPI::DrawSplineCatmullRom(const std::vector<Vec2> &points,
                                         float thick, ColorRGBA color) {
  if (!points.empty()) {
    const auto raylibPoints = ToRaylibPoints(points);
    DrawSplineCatmullRom(raylibPoints, thick, ToRaylibColor(color));
  }
}

void RenderSystemAPI::DrawSplineBezierQuadratic(Vector2 *points, int pointCount,
                                              float thick, Color color) {
  ::DrawSplineBezierQuadratic(points, pointCount, thick, color);
}

void RenderSystemAPI::DrawSplineBezierQuadratic(
    const std::vector<Vector2> &points, float thick, Color color) {
  if (!points.empty()) {
    ::DrawSplineBezierQuadratic(const_cast<Vector2 *>(points.data()),
                                static_cast<int>(points.size()), thick, color);
  }
}

void RenderSystemAPI::DrawSplineBezierQuadratic(
    const std::vector<Vec2> &points, float thick, ColorRGBA color) {
  if (!points.empty()) {
    const auto raylibPoints = ToRaylibPoints(points);
    DrawSplineBezierQuadratic(raylibPoints, thick, ToRaylibColor(color));
  }
}

void RenderSystemAPI::DrawSplineBezierCubic(Vector2 *points, int pointCount,
                                          float thick, Color color) {
  ::DrawSplineBezierCubic(points, pointCount, thick, color);
}

void RenderSystemAPI::DrawSplineBezierCubic(const std::vector<Vector2> &points,
                                          float thick, Color color) {
  if (!points.empty()) {
    ::DrawSplineBezierCubic(const_cast<Vector2 *>(points.data()),
                            static_cast<int>(points.size()), thick, color);
  }
}

void RenderSystemAPI::DrawSplineBezierCubic(const std::vector<Vec2> &points,
                                          float thick, ColorRGBA color) {
  if (!points.empty()) {
    const auto raylibPoints = ToRaylibPoints(points);
    DrawSplineBezierCubic(raylibPoints, thick, ToRaylibColor(color));
  }
}

void RenderSystemAPI::DrawTexture(Texture2D texture, int posX, int posY,
                                Color tint) {
  ::DrawTexture(texture, posX, posY, tint);
}

void RenderSystemAPI::DrawTexture(Texture2D texture, int posX, int posY,
                                ColorRGBA tint) {
  DrawTexture(texture, posX, posY, ToRaylibColor(tint));
}

void RenderSystemAPI::DrawTextureV(Texture2D texture, Vector2 position,
                                 Color tint) {
  ::DrawTextureV(texture, position, tint);
}

void RenderSystemAPI::DrawTextureV(Texture2D texture, Vec2 position,
                                 ColorRGBA tint) {
  DrawTextureV(texture, ToRaylibVec2(position), ToRaylibColor(tint));
}

void RenderSystemAPI::DrawTextureEx(Texture2D texture, Vector2 position,
                                  float rotation, float scale, Color tint) {
  ::DrawTextureEx(texture, position, rotation, scale, tint);
}

void RenderSystemAPI::DrawTextureEx(Texture2D texture, Vec2 position,
                                  float rotation, float scale,
                                  ColorRGBA tint) {
  DrawTextureEx(texture, ToRaylibVec2(position), rotation, scale,
                ToRaylibColor(tint));
}

void RenderSystemAPI::DrawTextureRec(Texture2D texture, Rectangle source,
                                   Vector2 position, Color tint) {
  ::DrawTextureRec(texture, source, position, tint);
}

void RenderSystemAPI::DrawTextureRec(Texture2D texture, Rect source,
                                   Vec2 position, ColorRGBA tint) {
  DrawTextureRec(texture, ToRaylibRect(source), ToRaylibVec2(position),
                 ToRaylibColor(tint));
}

void RenderSystemAPI::DrawTexturePro(Texture2D texture, Rectangle source,
                                   Rectangle dest, Vector2 origin,
                                   float rotation, Color tint) {
  ::DrawTexturePro(texture, source, dest, origin, rotation, tint);
}

void RenderSystemAPI::DrawTexturePro(Texture2D texture, Rect source, Rect dest,
                                   Vec2 origin, float rotation,
                                   ColorRGBA tint) {
  DrawTexturePro(texture, ToRaylibRect(source), ToRaylibRect(dest),
                 ToRaylibVec2(origin), rotation, ToRaylibColor(tint));
}

void RenderSystemAPI::DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo,
                                      Rectangle dest, Vector2 origin,
                                      float rotation, Color tint) {
  ::DrawTextureNPatch(texture, nPatchInfo, dest, origin, rotation, tint);
}

void RenderSystemAPI::DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo,
                                      Rect dest, Vec2 origin, float rotation,
                                      ColorRGBA tint) {
  DrawTextureNPatch(texture, nPatchInfo, ToRaylibRect(dest),
                    ToRaylibVec2(origin), rotation, ToRaylibColor(tint));
}

void RenderSystemAPI::DrawTextPro(Font font, const std::string &text,
                                Vector2 position, Vector2 origin,
                                float rotation, float fontSize, float spacing,
                                Color tint) {
  ::DrawTextPro(font, text.c_str(), position, origin, rotation, fontSize,
                spacing, tint);
}

void RenderSystemAPI::DrawTextPro(Font font, const std::string &text,
                                Vec2 position, Vec2 origin, float rotation,
                                float fontSize, float spacing,
                                ColorRGBA tint) {
  DrawTextPro(font, text, ToRaylibVec2(position), ToRaylibVec2(origin), rotation,
              fontSize, spacing, ToRaylibColor(tint));
}

void RenderSystemAPI::DrawTextCodepoint(Font font, int codepoint,
                                      Vector2 position, float fontSize,
                                      Color tint) {
  ::DrawTextCodepoint(font, codepoint, position, fontSize, tint);
}

void RenderSystemAPI::DrawTextCodepoint(Font font, int codepoint,
                                      Vec2 position, float fontSize,
                                      ColorRGBA tint) {
  DrawTextCodepoint(font, codepoint, ToRaylibVec2(position), fontSize,
                    ToRaylibColor(tint));
}

void RenderSystemAPI::DrawTextCodepoints(Font font, const int *codepoints,
                                       int codepointCount, Vector2 position,
                                       float fontSize, float spacing,
                                       Color tint) {
  ::DrawTextCodepoints(font, codepoints, codepointCount, position, fontSize,
                       spacing, tint);
}

void RenderSystemAPI::DrawTextCodepoints(Font font, const int *codepoints,
                                       int codepointCount, Vec2 position,
                                       float fontSize, float spacing,
                                       ColorRGBA tint) {
  DrawTextCodepoints(font, codepoints, codepointCount, ToRaylibVec2(position),
                     fontSize, spacing, ToRaylibColor(tint));
}

} // namespace core
} // namespace game
