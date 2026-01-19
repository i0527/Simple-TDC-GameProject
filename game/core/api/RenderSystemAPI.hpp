#pragma once

// 標準ライブラリ
#include <functional>
#include <string>
#include <vector>

// プロジェクト内
#include "../config/RenderTypes.hpp"
#include "../config/RenderPrimitives.hpp"
#include "../config/GameConfig.hpp"

namespace game {
namespace core {

class BaseSystemAPI;

/// @brief 描画API
class RenderSystemAPI {
public:
  using ImGuiRenderCallback = std::function<void()>;

  explicit RenderSystemAPI(BaseSystemAPI* owner);
  ~RenderSystemAPI() = default;

  bool SetResolution(Resolution newResolution);
  Resolution GetCurrentResolution() const;
  int GetScreenWidth() const;
  int GetScreenHeight() const;
  int GetInternalWidth() const;
  int GetInternalHeight() const;

  void BeginRender();
  void BeginRenderEx(bool clearBackground = true);
  void EndRender();
  void EndFrame(ImGuiRenderCallback imGuiCallback = nullptr);

  void BeginImGui();
  void EndImGui();
  bool IsImGuiInitialized() const;
  void* GetImGuiJapaneseFont() const;

  static Vector2 ToRaylibVec2(const Vec2& v);
  static Rectangle ToRaylibRect(const Rect& r);
  static Color ToRaylibColor(const ColorRGBA& c);
  static Vec2 ToCoreVec2(const Vector2& v);

  Vector2 ScalePosition(float internalX, float internalY) const;
  float ScaleSize(float internalSize) const;
  float GetScaleFactor() const;

  void DrawTextRaylib(const std::string& text, float x, float y, float fontSize,
                      Color color);
  void DrawTextRaylib(const std::string& text, float x, float y, float fontSize,
                      ColorRGBA color);
  void DrawTextRaylibEx(const std::string& text, Vector2 position,
                        float fontSize, float spacing, Color color);
  void DrawTextRaylibEx(const std::string& text, Vec2 position,
                        float fontSize, float spacing, ColorRGBA color);

  void DrawTextDefault(const std::string& text, float x, float y, float fontSize,
                       Color color);
  void DrawTextDefault(const std::string& text, float x, float y, float fontSize,
                       ColorRGBA color);
  void DrawTextDefaultEx(const std::string& text, Vector2 position,
                         float fontSize, float spacing, Color color);
  void DrawTextDefaultEx(const std::string& text, Vec2 position,
                         float fontSize, float spacing, ColorRGBA color);

  void DrawTextWithFont(Font* font, const std::string& text, float x, float y,
                        float fontSize, Color color);
  void DrawTextWithFont(Font* font, const std::string& text, float x, float y,
                        float fontSize, ColorRGBA color);
  void DrawTextWithFontEx(Font* font, const std::string& text, Vector2 position,
                          float fontSize, float spacing, Color color);
  void DrawTextWithFontEx(Font* font, const std::string& text, Vec2 position,
                          float fontSize, float spacing, ColorRGBA color);
  Vector2 MeasureTextDefault(const std::string& text, float fontSize,
                             float spacing = 1.0f) const;
  Vec2 MeasureTextDefaultCore(const std::string& text, float fontSize,
                              float spacing = 1.0f) const;
  Vector2 MeasureTextWithFont(Font* font, const std::string& text,
                              float fontSize, float spacing = 1.0f) const;
  Vec2 MeasureTextWithFontCore(Font* font, const std::string& text,
                               float fontSize, float spacing = 1.0f) const;

  void DrawRectangle(float x, float y, float width, float height, Color color);
  void DrawRectangle(float x, float y, float width, float height,
                     ColorRGBA color);
  void DrawRectangleLines(float x, float y, float width, float height,
                          float thickness, Color color);
  void DrawRectangleLines(float x, float y, float width, float height,
                          float thickness, ColorRGBA color);
  void DrawCircle(float centerX, float centerY, float radius, Color color);
  void DrawCircle(float centerX, float centerY, float radius, ColorRGBA color);
  void DrawCircleLines(float centerX, float centerY, float radius,
                       float thickness, Color color);
  void DrawCircleLines(float centerX, float centerY, float radius,
                       float thickness, ColorRGBA color);
  void DrawLine(float startX, float startY, float endX, float endY,
                float thickness, Color color);
  void DrawLine(float startX, float startY, float endX, float endY,
                float thickness, ColorRGBA color);
  void DrawProgressBar(float x, float y, float width, float height,
                       float progress, Color fillColor = SKYBLUE,
                       Color emptyColor = DARKGRAY,
                       Color outlineColor = WHITE);
  void DrawProgressBar(float x, float y, float width, float height,
                       float progress, ColorRGBA fillColor,
                       ColorRGBA emptyColor, ColorRGBA outlineColor);

  void DrawPixel(int x, int y, Color color);
  void DrawPixel(int x, int y, ColorRGBA color);
  void DrawPixelV(Vector2 position, Color color);
  void DrawPixelV(Vec2 position, ColorRGBA color);
  void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);
  void DrawLineV(Vec2 startPos, Vec2 endPos, ColorRGBA color);
  void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick,
                      Color color);
  void DrawLineBezier(Vec2 startPos, Vec2 endPos, float thick, ColorRGBA color);
  void DrawLineStrip(Vector2* points, int pointCount, Color color);
  void DrawLineStrip(const std::vector<Vector2>& points, Color color);
  void DrawLineStrip(const std::vector<Vec2>& points, ColorRGBA color);
  void DrawCircleV(Vector2 center, float radius, Color color);
  void DrawCircleV(Vec2 center, float radius, ColorRGBA color);
  void DrawCircleSector(Vector2 center, float radius, float startAngle,
                        float endAngle, int segments, Color color);
  void DrawCircleSector(Vec2 center, float radius, float startAngle,
                        float endAngle, int segments, ColorRGBA color);
  void DrawCircleSectorLines(Vector2 center, float radius, float startAngle,
                             float endAngle, int segments, Color color);
  void DrawCircleSectorLines(Vec2 center, float radius, float startAngle,
                             float endAngle, int segments, ColorRGBA color);
  void DrawCircleGradient(int centerX, int centerY, float radius, Color color1,
                          Color color2);
  void DrawCircleGradient(int centerX, int centerY, float radius,
                          ColorRGBA color1, ColorRGBA color2);
  void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV,
                   Color color);
  void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV,
                   ColorRGBA color);
  void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV,
                        Color color);
  void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV,
                        ColorRGBA color);
  void DrawRing(Vector2 center, float innerRadius, float outerRadius,
                float startAngle, float endAngle, int segments, Color color);
  void DrawRing(Vec2 center, float innerRadius, float outerRadius,
                float startAngle, float endAngle, int segments,
                ColorRGBA color);
  void DrawRingLines(Vector2 center, float innerRadius, float outerRadius,
                     float startAngle, float endAngle, int segments,
                     Color color);
  void DrawRingLines(Vec2 center, float innerRadius, float outerRadius,
                     float startAngle, float endAngle, int segments,
                     ColorRGBA color);
  void DrawRectangleV(Vector2 position, Vector2 size, Color color);
  void DrawRectangleV(Vec2 position, Vec2 size, ColorRGBA color);
  void DrawRectangleRec(Rectangle rec, Color color);
  void DrawRectangleRec(Rect rec, ColorRGBA color);
  void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation,
                        Color color);
  void DrawRectanglePro(Rect rec, Vec2 origin, float rotation,
                        ColorRGBA color);
  void DrawRectangleGradientV(int x, int y, int width, int height,
                              Color color1, Color color2);
  void DrawRectangleGradientV(int x, int y, int width, int height,
                              ColorRGBA color1, ColorRGBA color2);
  void DrawRectangleGradientH(int x, int y, int width, int height,
                              Color color1, Color color2);
  void DrawRectangleGradientH(int x, int y, int width, int height,
                              ColorRGBA color1, ColorRGBA color2);
  void DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2,
                               Color col3, Color col4);
  void DrawRectangleGradientEx(Rect rec, ColorRGBA col1, ColorRGBA col2,
                               ColorRGBA col3, ColorRGBA col4);
  void DrawRectangleRounded(Rectangle rec, float roundness, int segments,
                            Color color);
  void DrawRectangleRounded(Rect rec, float roundness, int segments,
                            ColorRGBA color);
  void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments,
                                 Color color);
  void DrawRectangleRoundedLines(Rect rec, float roundness, int segments,
                                 ColorRGBA color);
  void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
  void DrawTriangle(Vec2 v1, Vec2 v2, Vec2 v3, ColorRGBA color);
  void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
  void DrawTriangleLines(Vec2 v1, Vec2 v2, Vec2 v3, ColorRGBA color);
  void DrawTriangleFan(Vector2* points, int pointCount, Color color);
  void DrawTriangleFan(const std::vector<Vector2>& points, Color color);
  void DrawTriangleFan(const std::vector<Vec2>& points, ColorRGBA color);
  void DrawTriangleStrip(Vector2* points, int pointCount, Color color);
  void DrawTriangleStrip(const std::vector<Vector2>& points, Color color);
  void DrawTriangleStrip(const std::vector<Vec2>& points, ColorRGBA color);
  void DrawPoly(Vector2 center, int sides, float radius, float rotation,
                Color color);
  void DrawPoly(Vec2 center, int sides, float radius, float rotation,
                ColorRGBA color);
  void DrawPolyLines(Vector2 center, int sides, float radius, float rotation,
                     Color color);
  void DrawPolyLines(Vec2 center, int sides, float radius, float rotation,
                     ColorRGBA color);
  void DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation,
                       float lineThick, Color color);
  void DrawPolyLinesEx(Vec2 center, int sides, float radius, float rotation,
                       float lineThick, ColorRGBA color);

  void DrawSplineLinear(Vector2* points, int pointCount, float thick,
                        Color color);
  void DrawSplineLinear(const std::vector<Vector2>& points, float thick,
                        Color color);
  void DrawSplineLinear(const std::vector<Vec2>& points, float thick,
                        ColorRGBA color);
  void DrawSplineBasis(Vector2* points, int pointCount, float thick,
                       Color color);
  void DrawSplineBasis(const std::vector<Vector2>& points, float thick,
                       Color color);
  void DrawSplineBasis(const std::vector<Vec2>& points, float thick,
                       ColorRGBA color);
  void DrawSplineCatmullRom(Vector2* points, int pointCount, float thick,
                            Color color);
  void DrawSplineCatmullRom(const std::vector<Vector2>& points, float thick,
                            Color color);
  void DrawSplineCatmullRom(const std::vector<Vec2>& points, float thick,
                            ColorRGBA color);
  void DrawSplineBezierQuadratic(Vector2* points, int pointCount, float thick,
                                 Color color);
  void DrawSplineBezierQuadratic(const std::vector<Vector2>& points,
                                 float thick, Color color);
  void DrawSplineBezierQuadratic(const std::vector<Vec2>& points, float thick,
                                 ColorRGBA color);
  void DrawSplineBezierCubic(Vector2* points, int pointCount, float thick,
                             Color color);
  void DrawSplineBezierCubic(const std::vector<Vector2>& points, float thick,
                             Color color);
  void DrawSplineBezierCubic(const std::vector<Vec2>& points, float thick,
                             ColorRGBA color);

  void DrawTexture(Texture2D texture, int posX, int posY, Color tint);
  void DrawTexture(Texture2D texture, int posX, int posY, ColorRGBA tint);
  void DrawTextureV(Texture2D texture, Vector2 position, Color tint);
  void DrawTextureV(Texture2D texture, Vec2 position, ColorRGBA tint);
  void DrawTextureEx(Texture2D texture, Vector2 position, float rotation,
                     float scale, Color tint);
  void DrawTextureEx(Texture2D texture, Vec2 position, float rotation,
                     float scale, ColorRGBA tint);
  void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position,
                      Color tint);
  void DrawTextureRec(Texture2D texture, Rect source, Vec2 position,
                      ColorRGBA tint);
  void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest,
                      Vector2 origin, float rotation, Color tint);
  void DrawTexturePro(Texture2D texture, Rect source, Rect dest, Vec2 origin,
                      float rotation, ColorRGBA tint);
  void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo,
                         Rectangle dest, Vector2 origin, float rotation,
                         Color tint);
  void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo, Rect dest,
                         Vec2 origin, float rotation, ColorRGBA tint);

  void DrawTextPro(Font font, const std::string& text, Vector2 position,
                   Vector2 origin, float rotation, float fontSize,
                   float spacing, Color tint);
  void DrawTextPro(Font font, const std::string& text, Vec2 position,
                   Vec2 origin, float rotation, float fontSize,
                   float spacing, ColorRGBA tint);
  void DrawTextCodepoint(Font font, int codepoint, Vector2 position,
                         float fontSize, Color tint);
  void DrawTextCodepoint(Font font, int codepoint, Vec2 position,
                         float fontSize, ColorRGBA tint);
  void DrawTextCodepoints(Font font, const int* codepoints,
                          int codepointCount, Vector2 position, float fontSize,
                          float spacing, Color tint);
  void DrawTextCodepoints(Font font, const int* codepoints,
                          int codepointCount, Vec2 position, float fontSize,
                          float spacing, ColorRGBA tint);

  void DrawUiTexture(const std::string& textureKey, Rectangle dest,
                     Color tint = WHITE);
  void DrawUiTexture(const std::string& textureKey, Rect dest,
                     ColorRGBA tint);
  void DrawUiNineSlice(const std::string& textureKey, Rectangle dest, int left,
                       int top, int right, int bottom, Color tint = WHITE);
  void DrawUiNineSlice(const std::string& textureKey, Rect dest, int left,
                       int top, int right, int bottom, ColorRGBA tint);
  void DrawUiCursor(const std::string& textureKey, Vector2 position,
                    Vector2 hotspot, float scale = 1.0f,
                    Color tint = WHITE);
  void DrawUiCursor(const std::string& textureKey, Vec2 position,
                    Vec2 hotspot, float scale = 1.0f,
                    Color tint = WHITE);
  Color GetReadableTextColor(const std::string& textureKey,
                             float luminanceThreshold = 0.6f);

private:
  BaseSystemAPI* owner_;
};

} // namespace core
} // namespace game
