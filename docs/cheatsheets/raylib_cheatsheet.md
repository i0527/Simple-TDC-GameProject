# Raylib 5.5 関数リファレンス チートシート

Raylibとraymathの主要な関数をまとめたクイックリファレンスです。

## 目次

- [Core Module (rcore)](#core-module-rcore)
  - [ウィンドウ管理](#ウィンドウ管理)
  - [カーソル管理](#カーソル管理)
  - [描画管理](#描画管理)
  - [シェーダー管理](#シェーダー管理)
  - [タイミング](#タイミング)
  - [入力処理](#入力処理)
  - [ファイルシステム](#ファイルシステム)
- [Shapes Module (rshapes)](#shapes-module-rshapes)
- [Textures Module (rtextures)](#textures-module-rtextures)
- [Text Module (rtext)](#text-module-rtext)
- [Models Module (rmodels)](#models-module-rmodels)
- [Audio Module (raudio)](#audio-module-raudio)
- [構造体 (Structs)](#構造体-structs)
- [カラー定義](#カラー定義)
- [数学関数 (raymath)](#数学関数-raymath)

---

## Core Module (rcore)

### ウィンドウ管理

```cpp
// ウィンドウ初期化・終了
void InitWindow(int width, int height, const char *title);
void CloseWindow(void);
bool WindowShouldClose(void);
bool IsWindowReady(void);

// ウィンドウ状態
bool IsWindowFullscreen(void);
bool IsWindowHidden(void);
bool IsWindowMinimized(void);
bool IsWindowMaximized(void);
bool IsWindowFocused(void);
bool IsWindowResized(void);
bool IsWindowState(unsigned int flag);

// ウィンドウ設定
void SetWindowState(unsigned int flags);
void ClearWindowState(unsigned int flags);
void ToggleFullscreen(void);
void ToggleBorderlessWindowed(void);
void MaximizeWindow(void);
void MinimizeWindow(void);
void RestoreWindow(void);
void SetWindowIcon(Image image);
void SetWindowIcons(Image *images, int count);
void SetWindowTitle(const char *title);
void SetWindowPosition(int x, int y);
void SetWindowMonitor(int monitor);
void SetWindowMinSize(int width, int height);
void SetWindowMaxSize(int width, int height);
void SetWindowSize(int width, int height);
void SetWindowOpacity(float opacity);
void SetWindowFocused(void);

// ウィンドウ情報取得
void *GetWindowHandle(void);
int GetScreenWidth(void);
int GetScreenHeight(void);
int GetRenderWidth(void);
int GetRenderHeight(void);
int GetMonitorCount(void);
int GetCurrentMonitor(void);
Vector2 GetMonitorPosition(int monitor);
int GetMonitorWidth(int monitor);
int GetMonitorHeight(int monitor);
int GetMonitorPhysicalWidth(int monitor);
int GetMonitorPhysicalHeight(int monitor);
int GetMonitorRefreshRate(int monitor);
Vector2 GetWindowPosition(void);
Vector2 GetWindowScaleDPI(void);
const char *GetMonitorName(int monitor);

// クリップボード
void SetClipboardText(const char *text);
const char *GetClipboardText(void);
Image GetClipboardImage(void);

// イベント待機
void EnableEventWaiting(void);
void DisableEventWaiting(void);
```

### カーソル管理

```cpp
void ShowCursor(void);
void HideCursor(void);
bool IsCursorHidden(void);
void EnableCursor(void);
void DisableCursor(void);
bool IsCursorOnScreen(void);
```

### 描画管理

```cpp
// 基本描画
void ClearBackground(Color color);
void BeginDrawing(void);
void EndDrawing(void);

// 2D/3Dモード
void BeginMode2D(Camera2D camera);
void EndMode2D(void);
void BeginMode3D(Camera3D camera);
void EndMode3D(void);

// レンダーテクスチャ
void BeginTextureMode(RenderTexture2D target);
void EndTextureMode(void);

// シェーダー
void BeginShaderMode(Shader shader);
void EndShaderMode(void);

// ブレンドモード
void BeginBlendMode(int mode);
void EndBlendMode(void);

// シザーモード
void BeginScissorMode(int x, int y, int width, int height);
void EndScissorMode(void);

// VRステレオモード
void BeginVrStereoMode(VrStereoConfig config);
void EndVrStereoMode(void);
VrStereoConfig LoadVrStereoConfig(VrDeviceInfo device);
void UnloadVrStereoConfig(VrStereoConfig config);
```

### シェーダー管理

```cpp
Shader LoadShader(const char *vsFileName, const char *fsFileName);
Shader LoadShaderFromMemory(const char *vsCode, const char *fsCode);
bool IsShaderValid(Shader shader);
int GetShaderLocation(Shader shader, const char *uniformName);
int GetShaderLocationAttrib(Shader shader, const char *attribName);
void SetShaderValue(Shader shader, int locIndex, const void *value, int uniformType);
void SetShaderValueV(Shader shader, int locIndex, const void *value, int uniformType, int count);
void SetShaderValueMatrix(Shader shader, int locIndex, Matrix mat);
void SetShaderValueTexture(Shader shader, int locIndex, Texture2D texture);
void UnloadShader(Shader shader);
```

### スクリーン空間変換

```cpp
Ray GetScreenToWorldRay(Vector2 position, Camera camera);
Ray GetScreenToWorldRayEx(Vector2 position, Camera camera, int width, int height);
Vector2 GetWorldToScreen(Vector3 position, Camera camera);
Vector2 GetWorldToScreenEx(Vector3 position, Camera camera, int width, int height);
Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera);
Vector2 GetScreenToWorld2D(Vector2 position, Camera2D camera);
Matrix GetCameraMatrix(Camera camera);
Matrix GetCameraMatrix2D(Camera2D camera);
```

### タイミング

```cpp
void SetTargetFPS(int fps);
float GetFrameTime(void);
double GetTime(void);
int GetFPS(void);

// カスタムフレーム制御
void SwapScreenBuffer(void);
void PollInputEvents(void);
void WaitTime(double seconds);
```

### ランダム値生成

```cpp
void SetRandomSeed(unsigned int seed);
int GetRandomValue(int min, int max);
int *LoadRandomSequence(unsigned int count, int min, int max);
void UnloadRandomSequence(int *sequence);
```

### その他

```cpp
void TakeScreenshot(const char *fileName);
void SetConfigFlags(unsigned int flags);
void OpenURL(const char *url);
void TraceLog(int logLevel, const char *text, ...);
void SetTraceLogLevel(int logLevel);
```

### 入力処理

#### キーボード

```cpp
bool IsKeyPressed(int key);
bool IsKeyPressedRepeat(int key);
bool IsKeyDown(int key);
bool IsKeyReleased(int key);
bool IsKeyUp(int key);
int GetKeyPressed(void);
int GetCharPressed(void);
void SetExitKey(int key);
```

#### ゲームパッド

```cpp
bool IsGamepadAvailable(int gamepad);
const char *GetGamepadName(int gamepad);
bool IsGamepadButtonPressed(int gamepad, int button);
bool IsGamepadButtonDown(int gamepad, int button);
bool IsGamepadButtonReleased(int gamepad, int button);
bool IsGamepadButtonUp(int gamepad, int button);
int GetGamepadButtonPressed(void);
int GetGamepadAxisCount(int gamepad);
float GetGamepadAxisMovement(int gamepad, int axis);
int SetGamepadMappings(const char *mappings);
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration);
```

#### マウス

```cpp
bool IsMouseButtonPressed(int button);
bool IsMouseButtonDown(int button);
bool IsMouseButtonReleased(int button);
bool IsMouseButtonUp(int button);
int GetMouseX(void);
int GetMouseY(void);
Vector2 GetMousePosition(void);
Vector2 GetMouseDelta(void);
void SetMousePosition(int x, int y);
void SetMouseOffset(int offsetX, int offsetY);
void SetMouseScale(float scaleX, float scaleY);
float GetMouseWheelMove(void);
Vector2 GetMouseWheelMoveV(void);
void SetMouseCursor(int cursor);
```

#### タッチ

```cpp
int GetTouchX(void);
int GetTouchY(void);
Vector2 GetTouchPosition(int index);
int GetTouchPointId(int index);
int GetTouchPointCount(void);
```

#### ジェスチャー

```cpp
void SetGesturesEnabled(unsigned int flags);
bool IsGestureDetected(unsigned int gesture);
int GetGestureDetected(void);
float GetGestureHoldDuration(void);
Vector2 GetGestureDragVector(void);
float GetGestureDragAngle(void);
Vector2 GetGesturePinchVector(void);
float GetGesturePinchAngle(void);
```

#### カメラ

```cpp
void UpdateCamera(Camera *camera, int mode);
void UpdateCameraPro(Camera *camera, Vector3 movement, Vector3 rotation, float zoom);
```

### ファイルシステム

```cpp
// ファイル存在確認
bool FileExists(const char *fileName);
bool DirectoryExists(const char *dirPath);
bool IsFileExtension(const char *fileName, const char *ext);
int GetFileLength(const char *fileName);
const char *GetFileExtension(const char *fileName);
const char *GetFileName(const char *filePath);
const char *GetFileNameWithoutExt(const char *filePath);
const char *GetDirectoryPath(const char *filePath);
const char *GetPrevDirectoryPath(const char *dirPath);
const char *GetWorkingDirectory(void);
const char *GetApplicationDirectory(void);

// ディレクトリ操作
int MakeDirectory(const char *dirPath);
bool ChangeDirectory(const char *dir);
bool IsPathFile(const char *path);
bool IsFileNameValid(const char *fileName);
FilePathList LoadDirectoryFiles(const char *dirPath);
FilePathList LoadDirectoryFilesEx(const char *basePath, const char *filter, bool scanSubdirs);
void UnloadDirectoryFiles(FilePathList files);
bool IsFileDropped(void);
FilePathList LoadDroppedFiles(void);
void UnloadDroppedFiles(FilePathList files);
long GetFileModTime(const char *fileName);

// ファイル読み込み・保存
unsigned char *LoadFileData(const char *fileName, int *dataSize);
void UnloadFileData(unsigned char *data);
bool SaveFileData(const char *fileName, void *data, int dataSize);
bool ExportDataAsCode(const unsigned char *data, int dataSize, const char *fileName);
char *LoadFileText(const char *fileName);
void UnloadFileText(char *text);
bool SaveFileText(const char *fileName, char *text);

// 圧縮・エンコード
unsigned char *CompressData(const unsigned char *data, int dataSize, int *compDataSize);
unsigned char *DecompressData(const unsigned char *compData, int compDataSize, int *dataSize);
char *EncodeDataBase64(const unsigned char *data, int dataSize, int *outputSize);
unsigned char *DecodeDataBase64(const unsigned char *data, int *outputSize);
unsigned int ComputeCRC32(unsigned char *data, int dataSize);
unsigned int *ComputeMD5(unsigned char *data, int dataSize);
unsigned int *ComputeSHA1(unsigned char *data, int dataSize);
```

---

## Shapes Module (rshapes)

### テクスチャ設定

```cpp
void SetShapesTexture(Texture2D texture, Rectangle source);
Texture2D GetShapesTexture(void);
Rectangle GetShapesTextureRectangle(void);
```

### 基本図形描画

```cpp
// ピクセル
void DrawPixel(int posX, int posY, Color color);
void DrawPixelV(Vector2 position, Color color);

// 線
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);
void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);
void DrawLineStrip(const Vector2 *points, int pointCount, Color color);
void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color);

// 円
void DrawCircle(int centerX, int centerY, float radius, Color color);
void DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color);
void DrawCircleSectorLines(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color);
void DrawCircleGradient(int centerX, int centerY, float radius, Color inner, Color outer);
void DrawCircleV(Vector2 center, float radius, Color color);
void DrawCircleLines(int centerX, int centerY, float radius, Color color);
void DrawCircleLinesV(Vector2 center, float radius, Color color);

// 楕円
void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color);
void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color);

// リング
void DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);
void DrawRingLines(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);

// 矩形
void DrawRectangle(int posX, int posY, int width, int height, Color color);
void DrawRectangleV(Vector2 position, Vector2 size, Color color);
void DrawRectangleRec(Rectangle rec, Color color);
void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color);
void DrawRectangleGradientV(int posX, int posY, int width, int height, Color top, Color bottom);
void DrawRectangleGradientH(int posX, int posY, int width, int height, Color left, Color right);
void DrawRectangleGradientEx(Rectangle rec, Color topLeft, Color bottomLeft, Color topRight, Color bottomRight);
void DrawRectangleLines(int posX, int posY, int width, int height, Color color);
void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color);
void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color);
void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, Color color);
void DrawRectangleRoundedLinesEx(Rectangle rec, float roundness, int segments, float lineThick, Color color);

// 三角形
void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void DrawTriangleFan(const Vector2 *points, int pointCount, Color color);
void DrawTriangleStrip(const Vector2 *points, int pointCount, Color color);

// 多角形
void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color);
void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color);
void DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation, float lineThick, Color color);
```

### スプライン描画

```cpp
void DrawSplineLinear(const Vector2 *points, int pointCount, float thick, Color color);
void DrawSplineBasis(const Vector2 *points, int pointCount, float thick, Color color);
void DrawSplineCatmullRom(const Vector2 *points, int pointCount, float thick, Color color);
void DrawSplineBezierQuadratic(const Vector2 *points, int pointCount, float thick, Color color);
void DrawSplineBezierCubic(const Vector2 *points, int pointCount, float thick, Color color);
void DrawSplineSegmentLinear(Vector2 p1, Vector2 p2, float thick, Color color);
void DrawSplineSegmentBasis(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float thick, Color color);
void DrawSplineSegmentCatmullRom(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float thick, Color color);
void DrawSplineSegmentBezierQuadratic(Vector2 p1, Vector2 c2, Vector2 p3, float thick, Color color);
void DrawSplineSegmentBezierCubic(Vector2 p1, Vector2 c2, Vector2 c3, Vector2 p4, float thick, Color color);

// スプライン点評価
Vector2 GetSplinePointLinear(Vector2 startPos, Vector2 endPos, float t);
Vector2 GetSplinePointBasis(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float t);
Vector2 GetSplinePointCatmullRom(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4, float t);
Vector2 GetSplinePointBezierQuad(Vector2 p1, Vector2 c2, Vector2 p3, float t);
Vector2 GetSplinePointBezierCubic(Vector2 p1, Vector2 c2, Vector2 c3, Vector2 p4, float t);
```

### 衝突判定

```cpp
bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);
bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2);
bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);
bool CheckCollisionCircleLine(Vector2 center, float radius, Vector2 p1, Vector2 p2);
bool CheckCollisionPointRec(Vector2 point, Rectangle rec);
bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius);
bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3);
bool CheckCollisionPointLine(Vector2 point, Vector2 p1, Vector2 p2, int threshold);
bool CheckCollisionPointPoly(Vector2 point, const Vector2 *points, int pointCount);
bool CheckCollisionLines(Vector2 startPos1, Vector2 endPos1, Vector2 startPos2, Vector2 endPos2, Vector2 *collisionPoint);
Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2);
```

---

## Textures Module (rtextures)

### 画像読み込み

```cpp
Image LoadImage(const char *fileName);
Image LoadImageRaw(const char *fileName, int width, int height, int format, int headerSize);
Image LoadImageAnim(const char *fileName, int *frames);
Image LoadImageAnimFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int *frames);
Image LoadImageFromMemory(const char *fileType, const unsigned char *fileData, int dataSize);
Image LoadImageFromTexture(Texture2D texture);
Image LoadImageFromScreen(void);
bool IsImageValid(Image image);
void UnloadImage(Image image);
bool ExportImage(Image image, const char *fileName);
unsigned char *ExportImageToMemory(Image image, const char *fileType, int *fileSize);
bool ExportImageAsCode(Image image, const char *fileName);
```

### 画像生成

```cpp
Image GenImageColor(int width, int height, Color color);
Image GenImageGradientLinear(int width, int height, int direction, Color start, Color end);
Image GenImageGradientRadial(int width, int height, float density, Color inner, Color outer);
Image GenImageGradientSquare(int width, int height, float density, Color inner, Color outer);
Image GenImageChecked(int width, int height, int checksX, int checksY, Color col1, Color col2);
Image GenImageWhiteNoise(int width, int height, float factor);
Image GenImagePerlinNoise(int width, int height, int offsetX, int offsetY, float scale);
Image GenImageCellular(int width, int height, int tileSize);
Image GenImageText(int width, int height, const char *text);
```

### 画像操作

```cpp
Image ImageCopy(Image image);
Image ImageFromImage(Image image, Rectangle rec);
Image ImageFromChannel(Image image, int selectedChannel);
Image ImageText(const char *text, int fontSize, Color color);
Image ImageTextEx(Font font, const char *text, float fontSize, float spacing, Color tint);
void ImageFormat(Image *image, int newFormat);
void ImageToPOT(Image *image, Color fill);
void ImageCrop(Image *image, Rectangle crop);
void ImageAlphaCrop(Image *image, float threshold);
void ImageAlphaClear(Image *image, Color color, float threshold);
void ImageAlphaMask(Image *image, Image alphaMask);
void ImageAlphaPremultiply(Image *image);
void ImageBlurGaussian(Image *image, int blurSize);
void ImageKernelConvolution(Image *image, const float *kernel, int kernelSize);
void ImageResize(Image *image, int newWidth, int newHeight);
void ImageResizeNN(Image *image, int newWidth, int newHeight);
void ImageResizeCanvas(Image *image, int newWidth, int newHeight, int offsetX, int offsetY, Color fill);
void ImageMipmaps(Image *image);
void ImageDither(Image *image, int rBpp, int gBpp, int bBpp, int aBpp);
void ImageFlipVertical(Image *image);
void ImageFlipHorizontal(Image *image);
void ImageRotate(Image *image, int degrees);
void ImageRotateCW(Image *image);
void ImageRotateCCW(Image *image);
void ImageColorTint(Image *image, Color color);
void ImageColorInvert(Image *image);
void ImageColorGrayscale(Image *image);
void ImageColorContrast(Image *image, float contrast);
void ImageColorBrightness(Image *image, int brightness);
void ImageColorReplace(Image *image, Color color, Color replace);
Color *LoadImageColors(Image image);
Color *LoadImagePalette(Image image, int maxPaletteSize, int *colorCount);
void UnloadImageColors(Color *colors);
void UnloadImagePalette(Color *colors);
Rectangle GetImageAlphaBorder(Image image, float threshold);
Color GetImageColor(Image image, int x, int y);
```

### 画像描画（CPU）

```cpp
void ImageClearBackground(Image *dst, Color color);
void ImageDrawPixel(Image *dst, int posX, int posY, Color color);
void ImageDrawPixelV(Image *dst, Vector2 position, Color color);
void ImageDrawLine(Image *dst, int startPosX, int startPosY, int endPosX, int endPosY, Color color);
void ImageDrawLineV(Image *dst, Vector2 start, Vector2 end, Color color);
void ImageDrawLineEx(Image *dst, Vector2 start, Vector2 end, int thick, Color color);
void ImageDrawCircle(Image *dst, int centerX, int centerY, int radius, Color color);
void ImageDrawCircleV(Image *dst, Vector2 center, int radius, Color color);
void ImageDrawCircleLines(Image *dst, int centerX, int centerY, int radius, Color color);
void ImageDrawCircleLinesV(Image *dst, Vector2 center, int radius, Color color);
void ImageDrawRectangle(Image *dst, int posX, int posY, int width, int height, Color color);
void ImageDrawRectangleV(Image *dst, Vector2 position, Vector2 size, Color color);
void ImageDrawRectangleRec(Image *dst, Rectangle rec, Color color);
void ImageDrawRectangleLines(Image *dst, Rectangle rec, int thick, Color color);
void ImageDrawTriangle(Image *dst, Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void ImageDrawTriangleEx(Image *dst, Vector2 v1, Vector2 v2, Vector2 v3, Color c1, Color c2, Color c3);
void ImageDrawTriangleLines(Image *dst, Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void ImageDrawTriangleFan(Image *dst, Vector2 *points, int pointCount, Color color);
void ImageDrawTriangleStrip(Image *dst, Vector2 *points, int pointCount, Color color);
void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec, Color tint);
void ImageDrawText(Image *dst, const char *text, int posX, int posY, int fontSize, Color color);
void ImageDrawTextEx(Image *dst, Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint);
```

### テクスチャ読み込み

```cpp
Texture2D LoadTexture(const char *fileName);
Texture2D LoadTextureFromImage(Image image);
TextureCubemap LoadTextureCubemap(Image image, int layout);
RenderTexture2D LoadRenderTexture(int width, int height);
bool IsTextureValid(Texture2D texture);
void UnloadTexture(Texture2D texture);
bool IsRenderTextureValid(RenderTexture2D target);
void UnloadRenderTexture(RenderTexture2D target);
void UpdateTexture(Texture2D texture, const void *pixels);
void UpdateTextureRec(Texture2D texture, Rectangle rec, const void *pixels);
```

### テクスチャ設定

```cpp
void GenTextureMipmaps(Texture2D *texture);
void SetTextureFilter(Texture2D texture, int filter);
void SetTextureWrap(Texture2D texture, int wrap);
```

### テクスチャ描画

```cpp
void DrawTexture(Texture2D texture, int posX, int posY, Color tint);
void DrawTextureV(Texture2D texture, Vector2 position, Color tint);
void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);
void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint);
void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);
void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo, Rectangle dest, Vector2 origin, float rotation, Color tint);
```

### カラー操作

```cpp
bool ColorIsEqual(Color col1, Color col2);
Color Fade(Color color, float alpha);
int ColorToInt(Color color);
Vector4 ColorNormalize(Color color);
Color ColorFromNormalized(Vector4 normalized);
Vector3 ColorToHSV(Color color);
Color ColorFromHSV(float hue, float saturation, float value);
Color ColorTint(Color color, Color tint);
Color ColorBrightness(Color color, float factor);
Color ColorContrast(Color color, float contrast);
Color ColorAlpha(Color color, float alpha);
Color ColorAlphaBlend(Color dst, Color src, Color tint);
Color ColorLerp(Color color1, Color color2, float factor);
Color GetColor(unsigned int hexValue);
Color GetPixelColor(void *srcPtr, int format);
void SetPixelColor(void *dstPtr, Color color, int format);
int GetPixelDataSize(int width, int height, int format);
```

---

## Text Module (rtext)

### フォント読み込み

```cpp
Font GetFontDefault(void);
Font LoadFont(const char *fileName);
Font LoadFontEx(const char *fileName, int fontSize, int *codepoints, int codepointCount);
Font LoadFontFromImage(Image image, Color key, int firstChar);
Font LoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount);
bool IsFontValid(Font font);
GlyphInfo *LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type);
Image GenImageFontAtlas(const GlyphInfo *glyphs, Rectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod);
void UnloadFontData(GlyphInfo *glyphs, int glyphCount);
void UnloadFont(Font font);
bool ExportFontAsCode(Font font, const char *fileName);
```

### テキスト描画

```cpp
void DrawFPS(int posX, int posY);
void DrawText(const char *text, int posX, int posY, int fontSize, Color color);
void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint);
void DrawTextPro(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint);
void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint);
void DrawTextCodepoints(Font font, const int *codepoints, int codepointCount, Vector2 position, float fontSize, float spacing, Color tint);
```

### テキスト情報

```cpp
void SetTextLineSpacing(int spacing);
int MeasureText(const char *text, int fontSize);
Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing);
int GetGlyphIndex(Font font, int codepoint);
GlyphInfo GetGlyphInfo(Font font, int codepoint);
Rectangle GetGlyphAtlasRec(Font font, int codepoint);
```

### UTF-8管理

```cpp
char *LoadUTF8(const int *codepoints, int length);
void UnloadUTF8(char *text);
int *LoadCodepoints(const char *text, int *count);
void UnloadCodepoints(int *codepoints);
int GetCodepointCount(const char *text);
int GetCodepoint(const char *text, int *codepointSize);
int GetCodepointNext(const char *text, int *codepointSize);
int GetCodepointPrevious(const char *text, int *codepointSize);
const char *CodepointToUTF8(int codepoint, int *utf8Size);
```

### 文字列操作

```cpp
int TextCopy(char *dst, const char *src);
bool TextIsEqual(const char *text1, const char *text2);
unsigned int TextLength(const char *text);
const char *TextFormat(const char *text, ...);
const char *TextSubtext(const char *text, int position, int length);
char *TextReplace(const char *text, const char *replace, const char *by);
char *TextInsert(const char *text, const char *insert, int position);
const char *TextJoin(const char **textList, int count, const char *delimiter);
const char **TextSplit(const char *text, char delimiter, int *count);
void TextAppend(char *text, const char *append, int *position);
int TextFindIndex(const char *text, const char *find);
const char *TextToUpper(const char *text);
const char *TextToLower(const char *text);
const char *TextToPascal(const char *text);
const char *TextToSnake(const char *text);
const char *TextToCamel(const char *text);
int TextToInteger(const char *text);
float TextToFloat(const char *text);
```

---

## Models Module (rmodels)

### 基本3D図形描画

```cpp
void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color);
void DrawPoint3D(Vector3 position, Color color);
void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color);
void DrawTriangle3D(Vector3 v1, Vector3 v2, Vector3 v3, Color color);
void DrawTriangleStrip3D(const Vector3 *points, int pointCount, Color color);
void DrawCube(Vector3 position, float width, float height, float length, Color color);
void DrawCubeV(Vector3 position, Vector3 size, Color color);
void DrawCubeWires(Vector3 position, float width, float height, float length, Color color);
void DrawCubeWiresV(Vector3 position, Vector3 size, Color color);
void DrawSphere(Vector3 centerPos, float radius, Color color);
void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color);
void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color);
void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color);
void DrawCylinderEx(Vector3 startPos, Vector3 endPos, float startRadius, float endRadius, int sides, Color color);
void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color);
void DrawCylinderWiresEx(Vector3 startPos, Vector3 endPos, float startRadius, float endRadius, int sides, Color color);
void DrawCapsule(Vector3 startPos, Vector3 endPos, float radius, int slices, int rings, Color color);
void DrawCapsuleWires(Vector3 startPos, Vector3 endPos, float radius, int slices, int rings, Color color);
void DrawPlane(Vector3 centerPos, Vector2 size, Color color);
void DrawRay(Ray ray, Color color);
void DrawGrid(int slices, float spacing);
```

### モデル管理

```cpp
Model LoadModel(const char *fileName);
Model LoadModelFromMesh(Mesh mesh);
bool IsModelValid(Model model);
void UnloadModel(Model model);
BoundingBox GetModelBoundingBox(Model model);
```

### モデル描画

```cpp
void DrawModel(Model model, Vector3 position, float scale, Color tint);
void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint);
void DrawModelWires(Model model, Vector3 position, float scale, Color tint);
void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint);
void DrawModelPoints(Model model, Vector3 position, float scale, Color tint);
void DrawModelPointsEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint);
void DrawBoundingBox(BoundingBox box, Color color);
void DrawBillboard(Camera camera, Texture2D texture, Vector3 position, float scale, Color tint);
void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector2 size, Color tint);
void DrawBillboardPro(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector3 up, Vector2 size, Vector2 origin, float rotation, Color tint);
```

### メッシュ管理

```cpp
void UploadMesh(Mesh *mesh, bool dynamic);
void UpdateMeshBuffer(Mesh mesh, int index, const void *data, int dataSize, int offset);
void UnloadMesh(Mesh mesh);
void DrawMesh(Mesh mesh, Material material, Matrix transform);
void DrawMeshInstanced(Mesh mesh, Material material, const Matrix *transforms, int instances);
BoundingBox GetMeshBoundingBox(Mesh mesh);
void GenMeshTangents(Mesh *mesh);
bool ExportMesh(Mesh mesh, const char *fileName);
bool ExportMeshAsCode(Mesh mesh, const char *fileName);
```

### メッシュ生成

```cpp
Mesh GenMeshPoly(int sides, float radius);
Mesh GenMeshPlane(float width, float length, int resX, int resZ);
Mesh GenMeshCube(float width, float height, float length);
Mesh GenMeshSphere(float radius, int rings, int slices);
Mesh GenMeshHemiSphere(float radius, int rings, int slices);
Mesh GenMeshCylinder(float radius, float height, int slices);
Mesh GenMeshCone(float radius, float height, int slices);
Mesh GenMeshTorus(float radius, float size, int radSeg, int sides);
Mesh GenMeshKnot(float radius, float size, int radSeg, int sides);
Mesh GenMeshHeightmap(Image heightmap, Vector3 size);
Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize);
```

### マテリアル管理

```cpp
Material *LoadMaterials(const char *fileName, int *materialCount);
Material LoadMaterialDefault(void);
bool IsMaterialValid(Material material);
void UnloadMaterial(Material material);
void SetMaterialTexture(Material *material, int mapType, Texture2D texture);
void SetModelMeshMaterial(Model *model, int meshId, int materialId);
```

### アニメーション

```cpp
ModelAnimation *LoadModelAnimations(const char *fileName, int *animCount);
void UpdateModelAnimation(Model model, ModelAnimation anim, int frame);
void UpdateModelAnimationBones(Model model, ModelAnimation anim, int frame);
void UnloadModelAnimation(ModelAnimation anim);
void UnloadModelAnimations(ModelAnimation *animations, int animCount);
bool IsModelAnimationValid(Model model, ModelAnimation anim);
```

### 3D衝突判定

```cpp
bool CheckCollisionSpheres(Vector3 center1, float radius1, Vector3 center2, float radius2);
bool CheckCollisionBoxes(BoundingBox box1, BoundingBox box2);
bool CheckCollisionBoxSphere(BoundingBox box, Vector3 center, float radius);
RayCollision GetRayCollisionSphere(Ray ray, Vector3 center, float radius);
RayCollision GetRayCollisionBox(Ray ray, BoundingBox box);
RayCollision GetRayCollisionMesh(Ray ray, Mesh mesh, Matrix transform);
RayCollision GetRayCollisionTriangle(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3);
RayCollision GetRayCollisionQuad(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4);
```

---

## Audio Module (raudio)

### オーディオデバイス管理

```cpp
void InitAudioDevice(void);
void CloseAudioDevice(void);
bool IsAudioDeviceReady(void);
void SetMasterVolume(float volume);
float GetMasterVolume(void);
```

### ウェーブ・サウンド読み込み

```cpp
Wave LoadWave(const char *fileName);
Wave LoadWaveFromMemory(const char *fileType, const unsigned char *fileData, int dataSize);
bool IsWaveValid(Wave wave);
Sound LoadSound(const char *fileName);
Sound LoadSoundFromWave(Wave wave);
Sound LoadSoundAlias(Sound source);
bool IsSoundValid(Sound sound);
void UpdateSound(Sound sound, const void *data, int sampleCount);
void UnloadWave(Wave wave);
void UnloadSound(Sound sound);
void UnloadSoundAlias(Sound alias);
bool ExportWave(Wave wave, const char *fileName);
bool ExportWaveAsCode(Wave wave, const char *fileName);
```

### サウンド管理

```cpp
void PlaySound(Sound sound);
void StopSound(Sound sound);
void PauseSound(Sound sound);
void ResumeSound(Sound sound);
bool IsSoundPlaying(Sound sound);
void SetSoundVolume(Sound sound, float volume);
void SetSoundPitch(Sound sound, float pitch);
void SetSoundPan(Sound sound, float pan);
Wave WaveCopy(Wave wave);
void WaveCrop(Wave *wave, int initFrame, int finalFrame);
void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels);
float *LoadWaveSamples(Wave wave);
void UnloadWaveSamples(float *samples);
```

### ミュージック管理

```cpp
Music LoadMusicStream(const char *fileName);
Music LoadMusicStreamFromMemory(const char *fileType, const unsigned char *data, int dataSize);
bool IsMusicValid(Music music);
void UnloadMusicStream(Music music);
void PlayMusicStream(Music music);
bool IsMusicStreamPlaying(Music music);
void UpdateMusicStream(Music music);
void StopMusicStream(Music music);
void PauseMusicStream(Music music);
void ResumeMusicStream(Music music);
void SeekMusicStream(Music music, float position);
void SetMusicVolume(Music music, float volume);
void SetMusicPitch(Music music, float pitch);
void SetMusicPan(Music music, float pan);
float GetMusicTimeLength(Music music);
float GetMusicTimePlayed(Music music);
```

### オーディオストリーム

```cpp
AudioStream LoadAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels);
bool IsAudioStreamValid(AudioStream stream);
void UnloadAudioStream(AudioStream stream);
void UpdateAudioStream(AudioStream stream, const void *data, int frameCount);
bool IsAudioStreamProcessed(AudioStream stream);
void PlayAudioStream(AudioStream stream);
void PauseAudioStream(AudioStream stream);
void ResumeAudioStream(AudioStream stream);
bool IsAudioStreamPlaying(AudioStream stream);
void StopAudioStream(AudioStream stream);
void SetAudioStreamVolume(AudioStream stream, float volume);
void SetAudioStreamPitch(AudioStream stream, float pitch);
void SetAudioStreamPan(AudioStream stream, float pan);
void SetAudioStreamBufferSizeDefault(int size);
void SetAudioStreamCallback(AudioStream stream, AudioCallback callback);
void AttachAudioStreamProcessor(AudioStream stream, AudioCallback processor);
void DetachAudioStreamProcessor(AudioStream stream, AudioCallback processor);
void AttachAudioMixedProcessor(AudioCallback processor);
void DetachAudioMixedProcessor(AudioCallback processor);
```

---

## 構造体 (Structs)

```cpp
struct Vector2;                // Vector2, 2 components
struct Vector3;                // Vector3, 3 components
struct Vector4;                // Vector4, 4 components
struct Matrix;                 // Matrix, 4x4 components, column major, OpenGL style, right handed
struct Color;                  // Color, 4 components, R8G8B8A8 (32bit)
struct Rectangle;              // Rectangle, 4 components

struct Image;                  // Image, pixel data stored in CPU memory (RAM)
struct Texture;                // Texture, tex data stored in GPU memory (VRAM)
struct RenderTexture;          // RenderTexture, fbo for texture rendering
struct NPatchInfo;             // NPatchInfo, n-patch layout info
struct GlyphInfo;              // GlyphInfo, font characters glyphs info
struct Font;                   // Font, font texture and GlyphInfo array data

struct Camera2D;               // Camera2D, defines position/orientation in 2d space
struct Camera3D;               // Camera, defines position/orientation in 3d space

struct Shader;                 // Shader
struct MaterialMap;            // MaterialMap
struct Material;               // Material, includes shader and maps
struct Mesh;                   // Mesh, vertex data and vao/vbo
struct Model;                  // Model, meshes, materials and animation data
struct ModelAnimation;         // ModelAnimation
struct Transform;              // Transform, vertex transformation data
struct BoneInfo;               // Bone, skeletal animation bone
struct Ray;                    // Ray, ray for raycasting
struct RayCollision;           // RayCollision, ray hit information
struct BoundingBox;            // BoundingBox

struct Wave;                   // Wave, audio wave data
struct AudioStream;            // AudioStream, custom audio stream
struct Sound;                  // Sound
struct Music;                  // Music, audio stream, anything longer than ~10 seconds should be streamed

struct VrDeviceInfo;           // VrDeviceInfo, Head-Mounted-Display device parameters
struct VrStereoConfig;         // VrStereoConfig, VR stereo rendering configuration for simulator

struct FilePathList;           // File path list

struct AutomationEvent;        // Automation event
struct AutomationEventList;    // Automation event list
```

---

## カラー定義

```cpp
// カスタムカラーパレット（白背景用）
#define LIGHTGRAY  (Color){ 200, 200, 200, 255 }   // Light Gray
#define GRAY       (Color){ 130, 130, 130, 255 }   // Gray
#define DARKGRAY   (Color){ 80, 80, 80, 255 }      // Dark Gray
#define YELLOW     (Color){ 253, 249, 0, 255 }     // Yellow
#define GOLD       (Color){ 255, 203, 0, 255 }     // Gold
#define ORANGE     (Color){ 255, 161, 0, 255 }     // Orange
#define PINK       (Color){ 255, 109, 194, 255 }   // Pink
#define RED        (Color){ 230, 41, 55, 255 }     // Red
#define MAROON     (Color){ 190, 33, 55, 255 }     // Maroon
#define GREEN      (Color){ 0, 228, 48, 255 }      // Green
#define LIME       (Color){ 0, 158, 47, 255 }      // Lime
#define DARKGREEN  (Color){ 0, 117, 44, 255 }      // Dark Green
#define SKYBLUE    (Color){ 102, 191, 255, 255 }   // Sky Blue
#define BLUE       (Color){ 0, 121, 241, 255 }     // Blue
#define DARKBLUE   (Color){ 0, 82, 172, 255 }      // Dark Blue
#define PURPLE     (Color){ 200, 122, 255, 255 }   // Purple
#define VIOLET     (Color){ 135, 60, 190, 255 }    // Violet
#define DARKPURPLE (Color){ 112, 31, 126, 255 }    // Dark Purple
#define BEIGE      (Color){ 211, 176, 131, 255 }   // Beige
#define BROWN      (Color){ 127, 106, 79, 255 }    // Brown
#define DARKBROWN  (Color){ 76, 63, 47, 255 }      // Dark Brown

// 基本色
#define WHITE      (Color){ 255, 255, 255, 255 }   // White
#define BLACK      (Color){ 0, 0, 0, 255 }         // Black
#define BLANK      (Color){ 0, 0, 0, 0 }           // Blank (Transparent)
#define MAGENTA    (Color){ 255, 0, 255, 255 }     // Magenta
#define RAYWHITE   (Color){ 245, 245, 245, 255 }   // My own White (raylib logo)
```

---

## 数学関数 (raymath)

### ユーティリティ数学

```cpp
float Clamp(float value, float min, float max);
float Lerp(float start, float end, float amount);
float Normalize(float value, float start, float end);
float Remap(float value, float inputStart, float inputEnd, float outputStart, float outputEnd);
float Wrap(float value, float min, float max);
int FloatEquals(float x, float y);
```

### Vector2 数学

```cpp
Vector2 Vector2Zero(void);
Vector2 Vector2One(void);
Vector2 Vector2Add(Vector2 v1, Vector2 v2);
Vector2 Vector2AddValue(Vector2 v, float add);
Vector2 Vector2Subtract(Vector2 v1, Vector2 v2);
Vector2 Vector2SubtractValue(Vector2 v, float sub);
float Vector2Length(Vector2 v);
float Vector2LengthSqr(Vector2 v);
float Vector2DotProduct(Vector2 v1, Vector2 v2);
float Vector2Distance(Vector2 v1, Vector2 v2);
float Vector2DistanceSqr(Vector2 v1, Vector2 v2);
float Vector2Angle(Vector2 v1, Vector2 v2);
Vector2 Vector2Scale(Vector2 v, float scale);
Vector2 Vector2Multiply(Vector2 v1, Vector2 v2);
Vector2 Vector2Negate(Vector2 v);
Vector2 Vector2Divide(Vector2 v1, Vector2 v2);
Vector2 Vector2Normalize(Vector2 v);
Vector2 Vector2Transform(Vector2 v, Matrix mat);
Vector2 Vector2Lerp(Vector2 v1, Vector2 v2, float amount);
Vector2 Vector2Reflect(Vector2 v, Vector2 normal);
Vector2 Vector2Rotate(Vector2 v, float angle);
Vector2 Vector2MoveTowards(Vector2 v, Vector2 target, float maxDistance);
Vector2 Vector2Invert(Vector2 v);
Vector2 Vector2Clamp(Vector2 v, Vector2 min, Vector2 max);
Vector2 Vector2ClampValue(Vector2 v, float min, float max);
int Vector2Equals(Vector2 p, Vector2 q);
```

### Vector3 数学

```cpp
Vector3 Vector3Zero(void);
Vector3 Vector3One(void);
Vector3 Vector3Add(Vector3 v1, Vector3 v2);
Vector3 Vector3AddValue(Vector3 v, float add);
Vector3 Vector3Subtract(Vector3 v1, Vector3 v2);
Vector3 Vector3SubtractValue(Vector3 v, float sub);
Vector3 Vector3Scale(Vector3 v, float scalar);
Vector3 Vector3Multiply(Vector3 v1, Vector3 v2);
Vector3 Vector3CrossProduct(Vector3 v1, Vector3 v2);
Vector3 Vector3Perpendicular(Vector3 v);
float Vector3Length(const Vector3 v);
float Vector3LengthSqr(const Vector3 v);
float Vector3DotProduct(Vector3 v1, Vector3 v2);
float Vector3Distance(Vector3 v1, Vector3 v2);
float Vector3DistanceSqr(Vector3 v1, Vector3 v2);
float Vector3Angle(Vector3 v1, Vector3 v2);
Vector3 Vector3Negate(Vector3 v);
Vector3 Vector3Divide(Vector3 v1, Vector3 v2);
Vector3 Vector3Normalize(Vector3 v);
void Vector3OrthoNormalize(Vector3 *v1, Vector3 *v2);
Vector3 Vector3Transform(Vector3 v, Matrix mat);
Vector3 Vector3RotateByQuaternion(Vector3 v, Quaternion q);
Vector3 Vector3RotateByAxisAngle(Vector3 v, Vector3 axis, float angle);
Vector3 Vector3Lerp(Vector3 v1, Vector3 v2, float amount);
Vector3 Vector3Reflect(Vector3 v, Vector3 normal);
Vector3 Vector3Min(Vector3 v1, Vector3 v2);
Vector3 Vector3Max(Vector3 v1, Vector3 v2);
Vector3 Vector3Barycenter(Vector3 p, Vector3 a, Vector3 b, Vector3 c);
Vector3 Vector3Unproject(Vector3 source, Matrix projection, Matrix view);
float3 Vector3ToFloatV(Vector3 v);
Vector3 Vector3Invert(Vector3 v);
Vector3 Vector3Clamp(Vector3 v, Vector3 min, Vector3 max);
Vector3 Vector3ClampValue(Vector3 v, float min, float max);
int Vector3Equals(Vector3 p, Vector3 q);
Vector3 Vector3Refract(Vector3 v, Vector3 n, float r);
```

### Matrix 数学

```cpp
float MatrixDeterminant(Matrix mat);
float MatrixTrace(Matrix mat);
Matrix MatrixTranspose(Matrix mat);
Matrix MatrixInvert(Matrix mat);
Matrix MatrixIdentity(void);
Matrix MatrixAdd(Matrix left, Matrix right);
Matrix MatrixSubtract(Matrix left, Matrix right);
Matrix MatrixMultiply(Matrix left, Matrix right);
Matrix MatrixTranslate(float x, float y, float z);
Matrix MatrixRotate(Vector3 axis, float angle);
Matrix MatrixRotateX(float angle);
Matrix MatrixRotateY(float angle);
Matrix MatrixRotateZ(float angle);
Matrix MatrixRotateXYZ(Vector3 angle);
Matrix MatrixRotateZYX(Vector3 angle);
Matrix MatrixScale(float x, float y, float z);
Matrix MatrixFrustum(double left, double right, double bottom, double top, double near, double far);
Matrix MatrixPerspective(double fovy, double aspect, double near, double far);
Matrix MatrixOrtho(double left, double right, double bottom, double top, double near, double far);
Matrix MatrixLookAt(Vector3 eye, Vector3 target, Vector3 up);
float16 MatrixToFloatV(Matrix mat);
```

### Quaternion 数学

```cpp
Quaternion QuaternionAdd(Quaternion q1, Quaternion q2);
Quaternion QuaternionAddValue(Quaternion q, float add);
Quaternion QuaternionSubtract(Quaternion q1, Quaternion q2);
Quaternion QuaternionSubtractValue(Quaternion q, float sub);
Quaternion QuaternionIdentity(void);
float QuaternionLength(Quaternion q);
Quaternion QuaternionNormalize(Quaternion q);
Quaternion QuaternionInvert(Quaternion q);
Quaternion QuaternionMultiply(Quaternion q1, Quaternion q2);
Quaternion QuaternionScale(Quaternion q, float mul);
Quaternion QuaternionDivide(Quaternion q1, Quaternion q2);
Quaternion QuaternionLerp(Quaternion q1, Quaternion q2, float amount);
Quaternion QuaternionNlerp(Quaternion q1, Quaternion q2, float amount);
Quaternion QuaternionSlerp(Quaternion q1, Quaternion q2, float amount);
Quaternion QuaternionFromVector3ToVector3(Vector3 from, Vector3 to);
Quaternion QuaternionFromMatrix(Matrix mat);
Matrix QuaternionToMatrix(Quaternion q);
Quaternion QuaternionFromAxisAngle(Vector3 axis, float angle);
void QuaternionToAxisAngle(Quaternion q, Vector3 *outAxis, float *outAngle);
Quaternion QuaternionFromEuler(float pitch, float yaw, float roll);
Vector3 QuaternionToEuler(Quaternion q);
Quaternion QuaternionTransform(Quaternion q, Matrix mat);
int QuaternionEquals(Quaternion p, Quaternion q);
```

---

## 検索インデックス

### よく使う関数

- **ウィンドウ初期化**: `InitWindow`, `CloseWindow`, `WindowShouldClose`
- **描画ループ**: `BeginDrawing`, `EndDrawing`, `ClearBackground`
- **2D描画**: `BeginMode2D`, `EndMode2D`
- **図形描画**: `DrawRectangle`, `DrawCircle`, `DrawLine`, `DrawTriangle`
- **テクスチャ**: `LoadTexture`, `DrawTexture`, `UnloadTexture`
- **テキスト**: `DrawText`, `DrawTextEx`, `MeasureText`
- **入力**: `IsKeyPressed`, `IsMouseButtonPressed`, `GetMousePosition`
- **タイミング**: `GetFrameTime`, `GetFPS`, `SetTargetFPS`
- **数学**: `Vector2Add`, `Vector2Distance`, `Vector2Lerp`, `Clamp`, `Lerp`

### カテゴリ別検索

- **描画**: `Draw*` で始まる関数
- **読み込み**: `Load*` で始まる関数
- **解放**: `Unload*` で始まる関数
- **チェック**: `Is*` で始まる関数
- **設定**: `Set*` で始まる関数
- **取得**: `Get*` で始まる関数
- **衝突判定**: `CheckCollision*` で始まる関数

---

**注意**: このチートシートは Raylib 5.5 をベースにしています。詳細なパラメータや使用例については、[Raylib公式ドキュメント](https://www.raylib.com/cheatsheet/cheatsheet.html)を参照してください。

