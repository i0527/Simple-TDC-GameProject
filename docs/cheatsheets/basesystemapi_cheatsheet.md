# BaseSystemAPI 関数リファレンス チートシート

BaseSystemAPIの全関数をまとめたクイックリファレンスです。

## 目次

- [初期化・終了](#初期化終了)
- [リソース管理](#リソース管理)
- [描画管理](#描画管理)
- [入力処理](#入力処理)
- [タイミング](#タイミング)
- [ウィンドウ管理](#ウィンドウ管理)
- [衝突判定](#衝突判定)
- [オーディオ管理](#オーディオ管理)

---

## 初期化・終了

```cpp
bool Initialize(Resolution initialResolution);
void Shutdown();
bool IsInitialized() const;
```

## リソース管理

### 初期化

```cpp
void InitializeResources();
void InitializeResources(ProgressCallback callback);
bool IsResourcesInitialized() const;
```

### テクスチャ

```cpp
void* GetTexture(const std::string& name);
bool HasTexture(const std::string& name) const;
```

### サウンド・ミュージック

```cpp
void* GetSound(const std::string& name);
void* GetMusic(const std::string& name);
```

### フォント

```cpp
void* GetFont(const std::string& name);
void SetDefaultFont(const std::string& name, int fontSize);
void* GetDefaultFont() const;
```

### 進捗管理

```cpp
int ScanResourceFiles();
bool LoadNextResource(ProgressCallback callback = nullptr);
bool HasMoreResources() const;
LoadProgress GetCurrentProgress() const;
void ResetLoadingState();
```

## 描画管理

### 解像度

```cpp
bool SetResolution(Resolution newResolution);
Resolution GetCurrentResolution() const;
int GetScreenWidth() const;
int GetScreenHeight() const;
int GetInternalWidth() const;
int GetInternalHeight() const;
```

### フレーム制御

```cpp
void BeginRender();
void BeginRenderEx(bool clearBackground = true);
void EndRender();
void EndFrame();
```

### スケーリング

```cpp
Vector2 ScalePosition(float internalX, float internalY) const;
float ScaleSize(float internalSize) const;
float GetScaleFactor() const;
```

### テキスト描画

```cpp
void DrawTextRaylib(const std::string& text, float x, float y, float fontSize, Color color);
void DrawTextRaylibEx(const std::string& text, Vector2 position, float fontSize, float spacing, Color color);
void DrawTextDefault(const std::string& text, float x, float y, float fontSize, Color color);
void DrawTextDefaultEx(const std::string& text, Vector2 position, float fontSize, float spacing, Color color);
void DrawTextWithFont(Font* font, const std::string& text, float x, float y, float fontSize, Color color);
void DrawTextWithFontEx(Font* font, const std::string& text, Vector2 position, float fontSize, float spacing, Color color);
Vector2 MeasureTextDefault(const std::string& text, float fontSize, float spacing = 1.0f) const;
Vector2 MeasureTextWithFont(Font* font, const std::string& text, float fontSize, float spacing = 1.0f) const;
```

### 基本図形

```cpp
void DrawRectangle(float x, float y, float width, float height, Color color);
void DrawRectangleLines(float x, float y, float width, float height, float thickness, Color color);
void DrawCircle(float centerX, float centerY, float radius, Color color);
void DrawCircleLines(float centerX, float centerY, float radius, float thickness, Color color);
void DrawLine(float startX, float startY, float endX, float endY, float thickness, Color color);
void DrawProgressBar(float x, float y, float width, float height, float progress, Color fillColor, Color emptyColor, Color outlineColor);
```

### 拡張図形

```cpp
void DrawPixel(int x, int y, Color color);
void DrawPixelV(Vector2 position, Color color);
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);
void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color);
void DrawLineStrip(Vector2* points, int pointCount, Color color);
void DrawLineStrip(const std::vector<Vector2>& points, Color color);
void DrawCircleV(Vector2 center, float radius, Color color);
void DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color);
void DrawCircleSectorLines(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color);
void DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2);
void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color);
void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color);
void DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);
void DrawRingLines(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);
void DrawRectangleV(Vector2 position, Vector2 size, Color color);
void DrawRectangleRec(Rectangle rec, Color color);
void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color);
void DrawRectangleGradientV(int x, int y, int width, int height, Color color1, Color color2);
void DrawRectangleGradientH(int x, int y, int width, int height, Color color1, Color color2);
void DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2, Color col3, Color col4);
void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color);
void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, Color color);
void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void DrawTriangleFan(Vector2* points, int pointCount, Color color);
void DrawTriangleFan(const std::vector<Vector2>& points, Color color);
void DrawTriangleStrip(Vector2* points, int pointCount, Color color);
void DrawTriangleStrip(const std::vector<Vector2>& points, Color color);
void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color);
void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color);
void DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation, float lineThick, Color color);
```

### スプライン

```cpp
void DrawSplineLinear(Vector2* points, int pointCount, float thick, Color color);
void DrawSplineLinear(const std::vector<Vector2>& points, float thick, Color color);
void DrawSplineBasis(Vector2* points, int pointCount, float thick, Color color);
void DrawSplineBasis(const std::vector<Vector2>& points, float thick, Color color);
void DrawSplineCatmullRom(Vector2* points, int pointCount, float thick, Color color);
void DrawSplineCatmullRom(const std::vector<Vector2>& points, float thick, Color color);
void DrawSplineBezierQuadratic(Vector2* points, int pointCount, float thick, Color color);
void DrawSplineBezierQuadratic(const std::vector<Vector2>& points, float thick, Color color);
void DrawSplineBezierCubic(Vector2* points, int pointCount, float thick, Color color);
void DrawSplineBezierCubic(const std::vector<Vector2>& points, float thick, Color color);
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

### テキスト拡張

```cpp
void DrawTextPro(Font font, const std::string& text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint);
void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint);
void DrawTextCodepoints(Font font, const int* codepoints, int codepointCount, Vector2 position, float fontSize, float spacing, Color tint);
```

## 入力処理

### 状態更新

```cpp
void UpdateInput();  // 毎フレーム呼び出し必須
```

### キーボード

```cpp
bool IsKeyPressed(int key);
bool IsKeyPressedRepeat(int key);
bool IsKeyDown(int key);
bool IsKeyReleased(int key);
bool IsKeyUp(int key);
int GetKeyPressed();
int GetCharPressed();
void SetExitKey(int key);
```

### マウス

```cpp
bool IsMouseButtonPressed(int button);  // consumed機能付き
bool IsMouseButtonDown(int button);
bool IsMouseButtonReleased(int button);
bool IsMouseButtonUp(int button);
int GetMouseX();
int GetMouseY();
Vector2 GetMousePosition();
Vector2 GetMouseDelta();
float GetMouseWheelMove();
Vector2 GetMouseWheelMoveV();
void SetMousePosition(int x, int y);
void SetMouseOffset(int offsetX, int offsetY);
void SetMouseScale(float scaleX, float scaleY);
void SetMouseCursor(int cursor);
void ConsumeMouseButton(int button);  // 重複検出を防ぐ
```

### ゲームパッド

```cpp
bool IsGamepadAvailable(int gamepad);
const char* GetGamepadName(int gamepad);
bool IsGamepadButtonPressed(int gamepad, int button);
bool IsGamepadButtonDown(int gamepad, int button);
bool IsGamepadButtonReleased(int gamepad, int button);
bool IsGamepadButtonUp(int gamepad, int button);
float GetGamepadAxisMovement(int gamepad, int axis);
```

### タッチ・ジェスチャー

```cpp
int GetTouchPointCount();
Vector2 GetTouchPosition(int index);
bool IsGestureDetected(unsigned int gesture);
int GetGestureDetected();
```

## タイミング

```cpp
float GetFrameTime();  // フレーム時間（秒）
int GetFPS();  // FPS
void SetTargetFPS(int fps);
double GetTime();  // 経過時間（秒）
```

## ウィンドウ管理

```cpp
bool WindowShouldClose();
bool IsWindowReady();
```

## 衝突判定

```cpp
bool CheckCollisionPointRec(Vector2 point, Rectangle rec);
bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);
bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2);
bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);
Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2);
```

## オーディオ管理

### 更新

```cpp
void UpdateAudio(float deltaTime);  // 毎フレーム呼び出し必須
```

### 再生制御

```cpp
bool PlaySound(const std::string& name);
bool PlayMusic(const std::string& name);
void StopSound();
void StopSound(const std::string& name);
void StopMusic();
```

### 状態管理

```cpp
bool IsSoundPlaying(const std::string& name) const;
bool IsMusicPlaying() const;
std::string GetCurrentMusicName() const;
```

### ボリューム管理

```cpp
void SetMasterVolume(float volume);  // 0.0-1.0
void SetSEVolume(float volume);  // 0.0-1.0
void SetBGMVolume(float volume);  // 0.0-1.0
float GetMasterVolume() const;
float GetSEVolume() const;
float GetBGMVolume() const;
```

---

## 使用例

### 基本的な使用パターン

```cpp
// 初期化
BaseSystemAPI api;
api.Initialize(Resolution::FHD);
api.InitializeResources();

// メインループ
while (!api.WindowShouldClose()) {
    float deltaTime = api.GetFrameTime();
    
    // 入力更新
    api.UpdateInput();
    
    // 入力処理
    if (api.IsKeyPressed(KEY_SPACE)) {
        // スペースキーが押された
    }
    
    Vector2 mousePos = api.GetMousePosition();
    if (api.IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // マウス左クリック
        api.ConsumeMouseButton(MOUSE_LEFT_BUTTON);
    }
    
    // 描画
    api.BeginRender();
    api.DrawRectangle(100, 100, 200, 200, RED);
    api.DrawTextDefault("Hello", 150, 150, 32, WHITE);
    api.EndRender();
    api.EndFrame();
    
    // オーディオ更新
    api.UpdateAudio(deltaTime);
}

// 終了
api.Shutdown();
```

---

**注意**: このチートシートは BaseSystemAPI の機能をまとめたものです。詳細なパラメータや使用例については、各関数のドキュメントコメントを参照してください。
