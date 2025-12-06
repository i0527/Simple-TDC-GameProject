# Raylib 5.0 リソース管理 完全ガイド
>
> Raylib 特有のリソース管理に絞った詳細ガイドです。ライブラリ全般の概要は `gamedev_libs_guide.md`、詳細は `libs_guide.md` を参照してください。

## 概要

Raylib はシンプルで使いやすいゲーム開発フレームワークですが、**リソースは明示的に管理する必要があります**。本ドキュメントは、メモリリークを防ぎ、効率的なリソース管理を実現するための注意点を詳細に説明します。

---

## 1. リソース管理の基本原則

### 1.1 所有権の明確化

Raylib では、ロードされたリソースは以下の2つの場所に存在します：

| メモリ領域 | 機能 | ロード関数 | アンロード関数 |
|-----------|------|----------|--------------|
| **RAM (CPU側)** | ピクセルデータ、頂点データなどの処理用 | `LoadImage()`, `LoadImageAnim()` | `UnloadImage()` |
| **VRAM (GPU側)** | テクスチャ、シェーダー、レンダーテクスチャなどの描画用 | `LoadTexture()`, `LoadShader()` | `UnloadTexture()`, `UnloadShader()` |

**重要**: RAM のリソースと VRAM のリソースは独立しており、**両方を明示的に管理する必要があります**。

### 1.2 リソースライフサイクル

```
ロード (メモリ確保)
  ↓
初期化 (GPU へのアップロード等)
  ↓
使用 (ゲームループ内で参照)
  ↓
アンロード (メモリ解放)
```

### 1.3 CloseWindow() の動作

**重要な注意**: `CloseWindow()` は全リソースを自動的には解放しません。

```cpp
// CloseWindow() が処理すること
- ウィンドウ閉鎖
- OpenGL コンテキスト破棄
- Audio デバイス関連の初期化リソース削除

// CloseWindow() が処理しないこと ← 明示的に実行が必要
- ロードした全テクスチャ
- ロードした全サウンド/音楽
- ロードした全フォント
- ロードした全モデル/メッシュ
- ロードしたシェーダー
```

---

## 2. テクスチャ管理

### 2.1 テクスチャのライフサイクル

#### RAM → VRAM 変換フロー

```cpp
// 1. ファイルからメモリに読み込み (RAM)
Image image = LoadImage("texture.png");  // RAM に画像データ
// この時点で VRAM には何もない

// 2. VRAM にアップロード
Texture2D texture = LoadTextureFromImage(image);  // RAM → VRAM
// VRAM にテクスチャがコピーされた

// 3. RAM のメモリ解放
UnloadImage(image);  // RAM 解放（描画に影響しない）

// 4. VRAM 解放（使用終了時）
UnloadTexture(texture);  // VRAM 解放
```

### 2.2 効率的なロード方法

```cpp
// 推奨: 直接ファイルから VRAM にロード
Texture2D texture = LoadTexture("texture.png");
// 内部的には Image → Texture に変換され、中間の Image は自動削除される

// 非推奨: 不要な中間ステップ
Image image = LoadImage("texture.png");
Texture2D texture = LoadTextureFromImage(image);
UnloadImage(image);  // 余分な処理
```

### 2.3 テクスチャの共有管理

**重要**: 複数のオブジェクトが同じテクスチャを使用する場合、**一度だけロードし、一度だけアンロードする**。

```cpp
// ❌ 間違った方法: 複数ボタンが同じテクスチャを重複ロード
class Button {
    Texture2D texture;
    Button() { texture = LoadTexture("button.png"); }
    ~Button() { UnloadTexture(texture); }
};

// ✓ 正しい方法: リソースマネージャーで集中管理
class ResourceManager {
    std::unordered_map<std::string, Texture2D> textures;
    
    Texture2D GetTexture(const std::string& path) {
        if (textures.find(path) == textures.end()) {
            textures[path] = LoadTexture(path.c_str());
        }
        return textures[path];
    }
    
    ~ResourceManager() {
        for (auto& pair : textures) {
            UnloadTexture(pair.second);
        }
        textures.clear();
    }
};
```

### 2.4 テクスチャの更新

```cpp
// 一度ロード後、データ更新が必要な場合
Texture2D texture = LoadTexture("base.png");

// ...

// テクスチャ内容を更新 (VRAM 内で更新、リロード不要)
Image updated = LoadImage("updated.png");
UpdateTexture(texture, updated.data);
UnloadImage(updated);  // Image は不要になったら削除
```

### 2.5 RenderTexture (オフスクリーンレンダリング)

```cpp
// オフスクリーンレンダリング用テクスチャ
RenderTexture2D target = LoadRenderTexture(800, 600);

// ゲームループ内
BeginTextureMode(target);
{
    ClearBackground(BLACK);
    DrawCircle(400, 300, 100, RED);
}
EndTextureMode();

// target.texture で通常のテクスチャとして使用可能
DrawTexture(target.texture, 0, 0, WHITE);

// 終了時にアンロード
UnloadRenderTexture(target);
```

### 2.6 リソース確認

```cpp
// テクスチャがロードされているか確認
if (IsTextureReady(texture)) {
    DrawTexture(texture, 0, 0, WHITE);
} else {
    TRACELOG(LOG_WARNING, "Texture not ready");
}
```

---

## 3. フォント管理

### 3.1 フォントのロード/アンロード

```cpp
// デフォルトフォント (自動管理)
DrawText("Hello", 10, 10, 20, BLACK);  // 内部でデフォルトフォント使用

// カスタムフォント (明示的管理)
Font font = LoadFont("myfont.ttf");

// 使用
DrawTextEx(font, "Hello", {10, 10}, 20, 2, BLACK);

// アンロード (必須)
UnloadFont(font);
```

### 3.2 フォントの詳細管理

```cpp
// グリフ情報の取得 (v5.0 での変更)
Font font = LoadFont("myfont.ttf");

// v5.0: CharInfo → GlyphInfo へ変更
GlyphInfo glyph = font.glyphs[0];

// 個別グリフ情報取得 (推奨)
int codepoint = 'A';
GlyphInfo info = GetGlyphInfo(font, codepoint);

// アンロード
UnloadFont(font);
```

### 3.3 フォントのサイズ指定

```cpp
// サイズ付きフォント生成
Font font = LoadFontEx("myfont.ttf", 32, NULL, 0);

// またはビルトイン関数で直接指定
Font fontLarge = LoadFontEx("myfont.ttf", 48, NULL, 0);
Font fontSmall = LoadFontEx("myfont.ttf", 16, NULL, 0);

// 複数フォントサイズを管理する場合はマップで管理
std::map<int, Font> fonts;
fonts[16] = LoadFontEx("myfont.ttf", 16, NULL, 0);
fonts[32] = LoadFontEx("myfont.ttf", 32, NULL, 0);
fonts[48] = LoadFontEx("myfont.ttf", 48, NULL, 0);

// アンロード時
for (auto& [size, font] : fonts) {
    UnloadFont(font);
}
fonts.clear();
```

### 3.4 フォント関連のメモリ管理

```cpp
// テキストのコードポイント取得 (v5.0)
int count = 0;
int* codepoints = LoadCodepoints("Hello", &count);  // メモリ確保

// 使用
for (int i = 0; i < count; i++) {
    GlyphInfo glyph = GetGlyphInfo(font, codepoints[i]);
}

// 明示的にアンロード (重要)
UnloadCodepoints(codepoints);
```

---

## 4. サウンド・音楽管理

### 4.1 オーディオデバイスの初期化

**重要**: すべてのサウンド/音楽をロードする**前に**オーディオデバイスを初期化する。

```cpp
// ❌ 間違った順序
Sound sound = LoadSound("sound.wav");  // クラッシュまたは失敗
InitAudioDevice();

// ✓ 正しい順序
InitAudioDevice();  // 最初に初期化

Sound sound = LoadSound("sound.wav");
Music music = LoadMusicStream("background.mp3");

// 使用
PlaySound(sound);
PlayMusicStream(music);

// アンロード
UnloadSound(sound);
UnloadMusicStream(music);

// シャットダウン (必須)
CloseAudioDevice();
```

### 4.2 サウンド vs 音楽

| 項目 | Sound | Music |
|------|-------|-------|
| **メモリ** | 全データを RAM に読み込み | ストリーミング（少量のバッファ） |
| **適用** | 短い効果音 | 背景音楽、長い音声 |
| **管理** | RAM に常駐 | 動的に読み込み |
| **アンロード** | `UnloadSound()` | `UnloadMusicStream()` |

```cpp
// 効果音: Sound を使用
Sound effect = LoadSound("jump.wav");      // < 1MB 推奨
PlaySound(effect);
UnloadSound(effect);

// 背景音楽: Music を使用
Music bgm = LoadMusicStream("theme.mp3");  // サイズ制限なし
PlayMusicStream(bgm);
// メインループ内で update 必須
UpdateMusicStream(bgm);
UnloadMusicStream(bgm);
```

### 4.3 音楽のストリーミング

```cpp
// メインループ
while (!WindowShouldClose()) {
    // 音楽ストリーミング更新 (毎フレーム必須)
    if (IsMusicStreamPlaying(bgm)) {
        UpdateMusicStream(bgm);  // バッファ管理
    }
    
    BeginDrawing();
    {
        // 描画処理
    }
    EndDrawing();
}
```

### 4.4 マルチプルサウンド管理

```cpp
class AudioManager {
    std::map<std::string, Sound> sounds;
    Music currentMusic;
    
public:
    void LoadSound(const std::string& name, const std::string& path) {
        sounds[name] = ::LoadSound(path.c_str());
    }
    
    void PlaySound(const std::string& name) {
        if (sounds.find(name) != sounds.end()) {
            ::PlaySound(sounds[name]);
        }
    }
    
    void LoadMusic(const std::string& path) {
        if (IsMusicStreamPlaying(currentMusic)) {
            UnloadMusicStream(currentMusic);
        }
        currentMusic = ::LoadMusicStream(path.c_str());
    }
    
    void UpdateMusic() {
        if (IsMusicStreamPlaying(currentMusic)) {
            UpdateMusicStream(currentMusic);
        }
    }
    
    ~AudioManager() {
        for (auto& [name, sound] : sounds) {
            UnloadSound(sound);
        }
        sounds.clear();
        
        if (IsMusicStreamPlaying(currentMusic)) {
            UnloadMusicStream(currentMusic);
        }
    }
};
```

---

## 5. モデル・メッシュ管理

### 5.1 モデルのロード/アンロード

```cpp
// モデルのロード
Model model = LoadModel("player.obj");

// 描画
DrawModel(model, {0, 0, 0}, 1.0f, WHITE);

// アンロード (メッシュ含む)
UnloadModel(model);
```

### 5.2 モデルが所有するメッシュ

**重要**: `UnloadModel()` はモデル内のメッシュも一緒にアンロードします。

```cpp
// ❌ ダブルアンロード (クラッシュ)
Model model = LoadModel("mesh.obj");
Mesh mesh = model.meshes[0];
UnloadModel(model);      // メッシュも一緒に削除
UnloadMesh(mesh);        // 無効なメッシュをアンロード（エラー）

// ✓ 正しい方法: モデルのみアンロード
Model model = LoadModel("mesh.obj");
// メッシュを直接参照しない、または必要なら複製
UnloadModel(model);      // メッシュも含めて削除
```

### 5.3 独立したメッシュ

```cpp
// メッシュを独立して作成
Mesh mesh = GenMeshCube(2.0f, 2.0f, 2.0f);

// 使用
Model model = LoadModelFromMesh(mesh);  // モデル内にメッシュをコピー
DrawModel(model, {0, 0, 0}, 1.0f, WHITE);

// アンロード順序
UnloadModel(model);     // モデルと内部メッシュを削除
// UnloadMesh(mesh);    // オリジナルメッシュは既に削除されている可能性
```

### 5.4 動的メッシュの更新

```cpp
// 動的メッシュ更新
Mesh mesh = GenMeshPlane(10, 10, 10, 10);

// 頂点データ更新
Vector3* vertices = (Vector3*)malloc(mesh.vertexCount * sizeof(Vector3));
memcpy(vertices, mesh.vertices, mesh.vertexCount * sizeof(Vector3));

// 頂点座標を変更
for (int i = 0; i < mesh.vertexCount; i++) {
    vertices[i].y += sin(vertices[i].x) * 0.5f;
}

// メッシュに反映
UpdateMeshBuffer(mesh, 0, vertices, mesh.vertexCount * sizeof(Vector3), 0);

// 使用
Model model = LoadModelFromMesh(mesh);
DrawModel(model, {0, 0, 0}, 1.0f, WHITE);

// クリーンアップ
free(vertices);
UnloadModel(model);
UnloadMesh(mesh);
```

### 5.5 メッシュの VBO 管理

```cpp
// カスタムメッシュの場合、VBO ID を手動で管理
Mesh mesh = {0};
mesh.vertexCount = vertices.size();
mesh.vertices = (float*)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
mesh.indices = (unsigned short*)RL_MALLOC(indices.size() * sizeof(unsigned short));

// VBO ID 配列の確保
mesh.vboId = (unsigned int*)RL_CALLOC(7, sizeof(unsigned int));

// メッシュバッファ更新でアップロード
UpdateMeshBuffer(mesh, 0, mesh.vertices, mesh.vertexCount * 3 * sizeof(float), 0);
UpdateMeshBuffer(mesh, 6, mesh.indices, indices.size() * sizeof(unsigned short), 0);

// クリーンアップ
UnloadMesh(mesh);  // VBO も含めて削除
```

---

## 6. シェーダー管理

### 6.1 シェーダーのロード/アンロード

```cpp
// シェーダーのロード
Shader shader = LoadShader("vertex.vs", "fragment.fs");

// シェーディング開始
BeginShaderMode(shader);
{
    DrawCube({0, 0, 0}, 1, 1, 1, RED);
}
EndShaderMode();

// アンロード (必須)
UnloadShader(shader);
```

### 6.2 シェーダーユニフォーム値の設定

```cpp
Shader shader = LoadShader(NULL, "custom.fs");

// シェーダーユニフォーム位置の取得
int timeLoc = GetShaderLocation(shader, "time");
int colorLoc = GetShaderLocation(shader, "customColor");

// 描画ループ内
float time = GetTime();
BeginShaderMode(shader);
{
    // フロート値の設定
    SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);
    
    // ベクトル値の設定
    Vector3 color = {1.0f, 0.5f, 0.2f};
    SetShaderValue(shader, colorLoc, &color, SHADER_UNIFORM_VEC3);
    
    DrawCube({0, 0, 0}, 1, 1, 1, WHITE);
}
EndShaderMode();

UnloadShader(shader);
```

### 6.3 シェーダーテクスチャの設定

```cpp
Shader shader = LoadShader(NULL, "texture.fs");
Texture2D texture = LoadTexture("pattern.png");

int texLoc = GetShaderLocation(shader, "mainTexture");

BeginShaderMode(shader);
{
    SetShaderValueTexture(shader, texLoc, texture);
    DrawRectangle(0, 0, 800, 600, WHITE);
}
EndShaderMode();

UnloadTexture(texture);
UnloadShader(shader);
```

---

## 7. メモリリーク検出と回避

### 7.1 メモリリークの一般的なパターン

```cpp
// ❌ パターン 1: アンロード忘れ
void GameInit() {
    textureDuck = LoadTexture("duck.png");
}

void GameUpdate() {
    // ...
}

void GameClose() {
    // UnloadTexture(textureDuck);  // ← アンロード忘れ!
}

// ❌ パターン 2: ダブルロード
void LoadResources() {
    if (!IsTextureReady(texture)) {
        texture = LoadTexture("sprite.png");
    }
    texture = LoadTexture("sprite.png");  // ← 前のテクスチャがメモリに残る
}

// ❌ パターン 3: 条件付きアンロード
void Cleanup() {
    if (someCondition) {
        UnloadTexture(texture);
    }
    // someCondition が false の場合、リークが発生
}
```

### 7.2 リソース管理クラスの実装

```cpp
template<typename T>
class ResourceManager {
private:
    std::unordered_map<std::string, std::shared_ptr<T>> resources;
    
    // リソース削除関数
    template<typename U>
    static void Deleter(U* ptr);
    
public:
    std::shared_ptr<T> Load(const std::string& name, const std::string& path) {
        // キャッシュ確認
        auto it = resources.find(path);
        if (it != resources.end()) {
            return it->second;
        }
        
        // ロード
        T resource = LoadResource(path);
        auto ptr = std::make_shared<T>(resource);
        resources[path] = ptr;
        return ptr;
    }
    
    void Unload(const std::string& path) {
        auto it = resources.find(path);
        if (it != resources.end()) {
            resources.erase(it);
            // std::shared_ptr が削除時に自動的に Unload 関数を呼び出し
        }
    }
    
    void UnloadAll() {
        resources.clear();
    }
};

// テクスチャ用特殊化
template<>
void ResourceManager<Texture2D>::Deleter<Texture2D>(Texture2D* ptr) {
    if (ptr) {
        UnloadTexture(*ptr);
        delete ptr;
    }
}
```

### 7.3 Valgrind でのメモリリーク検査

```bash
# Raylib アプリケーションのメモリリーク検査
valgrind --leak-check=full --show-leak-kinds=all \
    --track-origins=yes --verbose ./myapp

# 結果の見方
# - LEAK SUMMARY: 総メモリリーク
# - definitely lost: 確実にリークしているメモリ
# - indirectly lost: 直接アクセス不可能なメモリ
```

---

## 8. リソース管理チェックリスト

### 初期化フェーズ

- [ ] `InitWindow()` 呼び出し
- [ ] オーディオ必要な場合: `InitAudioDevice()` 呼び出し
- [ ] リソースマネージャー初期化
- [ ] 全リソース（テクスチャ、フォント、モデル、シェーダー）をロード

### 実行フェーズ

- [ ] 毎フレーム音楽ストリーミング更新 (`UpdateMusicStream()`)
- [ ] リソースが必要な時のみアクティブ化
- [ ] 動的ロードの場合、キャッシュ確認

### 終了フェーズ

```cpp
// リソース削除順序の推奨順序
void Cleanup() {
    // 1. ゲーム固有のリソース削除
    resourceManager.UnloadAll();
    
    // 2. テクスチャ/フォント削除
    for (auto& [name, texture] : textures) {
        UnloadTexture(texture);
    }
    textures.clear();
    
    for (auto& [name, font] : fonts) {
        UnloadFont(font);
    }
    fonts.clear();
    
    // 3. モデル/メッシュ削除
    for (auto& [name, model] : models) {
        UnloadModel(model);
    }
    models.clear();
    
    // 4. シェーダー削除
    for (auto& [name, shader] : shaders) {
        UnloadShader(shader);
    }
    shaders.clear();
    
    // 5. オーディオリソース削除
    for (auto& [name, sound] : sounds) {
        UnloadSound(sound);
    }
    sounds.clear();
    
    if (IsMusicStreamPlaying(currentMusic)) {
        UnloadMusicStream(currentMusic);
    }
    
    // 6. オーディオデバイスクローズ
    CloseAudioDevice();
    
    // 7. ウィンドウクローズ
    CloseWindow();
}
```

---

## 9. パフォーマンス最適化

### 9.1 テクスチャのメモリ最適化

```cpp
// 圧縮フォーマット使用
Texture2D texture = LoadTexture("sprite.basis");  // ASTC, ETC2 等

// MIP マップ生成
Texture2D texture = LoadTexture("texture.png");
GenTextureMipmaps(&texture);
SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
```

### 9.2 リソースの遅延ロード

```cpp
class LazyResourceManager {
private:
    std::map<std::string, Texture2D> loadedTextures;
    std::set<std::string> textureNames;
    
public:
    void RegisterTexture(const std::string& name, const std::string& path) {
        textureNames.insert(name);
        // パスを記憶するが、まだロードしない
    }
    
    Texture2D GetTexture(const std::string& name) {
        // 初回アクセス時にロード
        if (loadedTextures.find(name) == loadedTextures.end()) {
            // ファイルパスから対応するファイルを検索してロード
            loadedTextures[name] = LoadTexture(GetTexturePath(name).c_str());
        }
        return loadedTextures[name];
    }
    
    void Cleanup() {
        for (auto& [name, texture] : loadedTextures) {
            UnloadTexture(texture);
        }
        loadedTextures.clear();
    }
};
```

### 9.3 アトラステクスチャの活用

```cpp
// テクスチャアトラス: 複数の小さい画像を1つのテクスチャに統合
Texture2D atlas = LoadTexture("sprites_atlas.png");

// アトラス内のスプライト位置を定義
Rectangle spriteRects[] = {
    {0, 0, 32, 32},     // スプライト 1
    {32, 0, 32, 32},    // スプライト 2
    {64, 0, 32, 32},    // スプライト 3
};

// 描画時にアトラスから該当部分を描画
DrawTextureRec(atlas, spriteRects[0], {100, 100}, WHITE);

// 一度のアンロードで全スプライト削除
UnloadTexture(atlas);
```

---

## 10. デバッグ支援

### 10.1 トレースログ

```cpp
// トレースログレベル設定
SetTraceLogLevel(LOG_DEBUG);

// リソースロード時の確認
Image img = LoadImage("missing.png");
if (img.data != NULL) {
    TRACELOG(LOG_INFO, "Image loaded: %dx%d", img.width, img.height);
} else {
    TRACELOG(LOG_ERROR, "Failed to load image");
}
```

### 10.2 リソースの確認関数

```cpp
// リソース準備状態の確認
bool IsTextureReady(Texture2D texture);
bool IsFontReady(Font font);
bool IsShaderReady(Shader shader);
bool IsModelReady(Model model);

// 使用例
Texture2D tex = LoadTexture("sprite.png");
if (IsTextureReady(tex)) {
    DrawTexture(tex, 0, 0, WHITE);
} else {
    TRACELOG(LOG_WARNING, "Texture not ready!");
}
```

### 10.3 メモリ用量の確認

```cpp
// メモリ確保関数を追跡
void* MemAlloc(unsigned int size);
void* MemRealloc(void* ptr, unsigned int size);
void MemFree(void* ptr);

// カスタム実装で追跡可能
static size_t totalAllocated = 0;
void* CustomMemAlloc(unsigned int size) {
    totalAllocated += size;
    TRACELOG(LOG_INFO, "Total allocated: %zu bytes", totalAllocated);
    return malloc(size);
}
```

---

## 11. よくある問題と解決策

### 問題 1: テクスチャが点滅する

**原因**: ゲームループの途中で `UnloadTexture()` を呼び出しているが、別のフレームで同じテクスチャを描画しようとしている。

```cpp
// ❌ 間違い: ループ内で複数回アンロード
for (int i = 0; i < sprites.size(); i++) {
    sprites[i].texture = LoadTexture("sprite.png");
    DrawTexture(sprites[i].texture, 0, 0, WHITE);
    UnloadTexture(sprites[i].texture);  // 毎フレーム削除!
}

// ✓ 正しい: 一度だけロードして共有
Texture2D spriteTexture = LoadTexture("sprite.png");
for (int i = 0; i < sprites.size(); i++) {
    sprites[i].texture = spriteTexture;  // 参照を共有
    DrawTexture(sprites[i].texture, 0, 0, WHITE);
}
// ゲーム終了時に一度だけ削除
UnloadTexture(spriteTexture);
```

### 問題 2: サウンドが再生されない

**原因**: `InitAudioDevice()` を呼び出す前にサウンドをロードしている。

```cpp
// ❌ 間違い
Sound sound = LoadSound("jump.wav");      // 失敗
InitAudioDevice();
PlaySound(sound);                         // 再生されない

// ✓ 正しい
InitAudioDevice();
Sound sound = LoadSound("jump.wav");      // 成功
PlaySound(sound);
```

### 問題 3: メモリ使用量が減らない

**原因**: `CloseWindow()` はメモリを OS に返さない場合がある。これは通常のメモリ管理で、リークではない。

```cpp
// メモリが OS に返されないのは仕様
CloseWindow();
// プログラム終了時にプロセスメモリ全体が解放される

// アプリケーションの設計が正しいか確認する方法
valgrind --leak-check=full ./myapp
// "definitely lost" が 0 なら OK
```

### 問題 4: UnloadModel() でクラッシュ

**原因**: モデルのメッシュが複数回アンロード要求されている。

```cpp
// ❌ 間違い
Model model = LoadModel("mesh.obj");
Mesh mesh = model.meshes[0];
UnloadModel(model);      // メッシュも一緒に削除
UnloadMesh(mesh);        // クラッシュ!

// ✓ 正しい
Model model = LoadModel("mesh.obj");
UnloadModel(model);      // メッシュ含めて削除完了
// UnloadMesh() は呼ばない
```

---

## 12. リソース管理ベストプラクティス

### 12.1 リソース管理の3原則

1. **一度ロードしたら明示的にアンロード**
2. **複数のオブジェクトが同じリソースを使用する場合は共有**
3. **リソースマネージャーで一元管理**

### 12.2 推奨するプロジェクト構造

```
src/
├── resources/
│   ├── ResourceManager.h
│   ├── ResourceManager.cpp
│   ├── TextureManager.h
│   └── AudioManager.h
├── game/
│   ├── Game.h
│   └── Game.cpp
└── main.cpp
```

### 12.3 テンプレートコード

```cpp
// ResourceManager.h
#pragma once
#include "raylib.h"
#include <unordered_map>
#include <memory>

class ResourceManager {
private:
    std::unordered_map<std::string, Texture2D> textures;
    std::unordered_map<std::string, Font> fonts;
    std::unordered_map<std::string, Sound> sounds;
    std::unordered_map<std::string, Model> models;
    
public:
    ResourceManager() = default;
    ~ResourceManager() { Cleanup(); }
    
    // テクスチャロード
    Texture2D LoadTexture(const std::string& name, const std::string& path) {
        if (textures.count(name)) return textures[name];
        textures[name] = ::LoadTexture(path.c_str());
        return textures[name];
    }
    
    // フォントロード
    Font LoadFont(const std::string& name, const std::string& path, int size) {
        if (fonts.count(name)) return fonts[name];
        fonts[name] = ::LoadFontEx(path.c_str(), size, NULL, 0);
        return fonts[name];
    }
    
    // モデルロード
    Model LoadModel(const std::string& name, const std::string& path) {
        if (models.count(name)) return models[name];
        models[name] = ::LoadModel(path.c_str());
        return models[name];
    }
    
    // クリーンアップ
    void Cleanup() {
        for (auto& [name, texture] : textures) {
            UnloadTexture(texture);
        }
        textures.clear();
        
        for (auto& [name, font] : fonts) {
            UnloadFont(font);
        }
        fonts.clear();
        
        for (auto& [name, sound] : sounds) {
            UnloadSound(sound);
        }
        sounds.clear();
        
        for (auto& [name, model] : models) {
            UnloadModel(model);
        }
        models.clear();
    }
};
```

---

## 13. 参考資料

- **Raylib 公式 API**: <https://www.raylib.com/api/>
- **Raylib GitHub**: <https://github.com/raysan5/raylib>
- **Raylib Wiki**: <https://github.com/raysan5/raylib/wiki>
- **メモリ管理チュートリアル**: <https://www.raylib.com/examples.html>
- **Valgrind ドキュメント**: <https://valgrind.org/>

---

**作成日**: 2024年12月
**バージョン**: 1.0
**対象**: Raylib 5.0 開発者向け
