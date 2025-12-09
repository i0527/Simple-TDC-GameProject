# Phase 6: Raylib 統合設計 - グラフィックス層（統合最適版）

**プロジェクト**: SimpleTDCGame_NewArch  
**バージョン**: 1.0.0（Raylib Integration Layer）  
**作成日**: 2025-12-08 / 08:10 JST  
**目的**: Raylib を統合したグラフィックス・入力・オーディオの実装仕様確定

---

## 📑 目次

1. [Raylib統合層 全体概要](#raylib統合層-全体概要)
2. [Graphics System（描画システム）](#graphics-system描画システム)
3. [Input System（入力システム）](#input-system入力システム)
4. [Audio System（オーディオシステム）](#audio-systemオーディオシステム)
5. [Resource Management（リソース管理）](#resource-managementリソース管理)
6. [Camera & Viewport](#camera--viewport)
7. [Particle & Effects（パーティクル）](#particle--effectsパーティクル)
8. [統合実装例](#統合実装例)

---

## Raylib統合層 全体概要

### アーキテクチャ図

```
SimpleTDCGame Application
┌──────────────────────────────────────────────────┐
│                                                   │
│  ┌─ Graphics System ─────────────────────────────┐│
│  │  - Renderer（描画エンジン）                   ││
│  │  - Sprite Management（スプライト管理）        ││
│  │  - TextureCache（テクスチャキャッシュ）       ││
│  │  - FontManager（フォント管理）                ││
│  │  - ShaderSystem（シェーダー）                 ││
│  └──────────────────────────────────────────────┘│
│                                                   │
│  ┌─ Input System ────────────────────────────────┐│
│  │  - InputManager（入力統一管理）               ││
│  │  - KeyboardInput（キー入力）                  ││
│  │  - MouseInput（マウス入力）                   ││
│  │  - GamepadInput（ゲームパッド - 将来対応）    ││
│  └──────────────────────────────────────────────┘│
│                                                   │
│  ┌─ Audio System ────────────────────────────────┐│
│  │  - AudioManager（オーディオ管理）             ││
│  │  - SoundCache（サウンドキャッシュ）           ││
│  │  - MusicPlayer（BGM再生）                     ││
│  │  - SoundEffects（SE再生）                     ││
│  │  - VolumeControl（ボリューム制御）            ││
│  └──────────────────────────────────────────────┘│
│                                                   │
│  ┌─ Resource Management ─────────────────────────┐│
│  │  - ResourceLoader（リソース読み込み）         ││
│  │  - AssetPipeline（アセットパイプライン）      ││
│  │  - MemoryPool（メモリプール）                 ││
│  │  - AsyncLoader（非同期ロード - オプション）   ││
│  └──────────────────────────────────────────────┘│
│                                                   │
│  ┌─ Camera & Viewport ───────────────────────────┐│
│  │  - CameraController（カメラ制御）             ││
│  │  - ViewportManager（ビューポート管理）        ││
│  │  - ScreenShake（画面振動）                    ││
│  └──────────────────────────────────────────────┘│
│                                                   │
│  ┌─ Particle & Effects ──────────────────────────┐│
│  │  - ParticleSystem（パーティクルシステム）     ││
│  │  - EffectPool（エフェクトプール）             ││
│  │  - PostProcessing（ポストプロセッシング）     ││
│  └──────────────────────────────────────────────┘│
│                                                   │
└──────────────────────────────────────────────────┘
         ↓
    Raylib API
         ↓
    GPU / Audio Device
```

### 設計方針

```yaml
Graphics特徴:
  ✅ レイヤー別描画（BackgroundLayer/GameLayer/UILayer）
  ✅ スプライト管理（自動キャッシング）
  ✅ シェーダー統合（カスタムエフェクト対応）
  ✅ 2D最適化（パフォーマンス重視）
  ✅ ゲーム解像度 1280x720 + UI描画

Input特徴:
  ✅ キー入力統一化（DirectInput風API）
  ✅ マウス入力統合
  ✅ ゲームパッド対応予定
  ✅ 入力イベントシステム
  ✅ キーバインディング設定可能

Audio特徴:
  ✅ BGM/SE分離管理
  ✅ ボリュームコントロール
  ✅ ループ再生対応
  ✅ メモリ効率的なキャッシング
  ✅ 同時再生制限

Resource特徴:
  ✅ 遅延ロード（Lazy Loading）
  ✅ メモリプール管理
  ✅ リソースの自動アンロード
  ✅ ホットリロード対応
  ✅ エラーハンドリング
```

---

## Graphics System（描画システム）

### Renderer インターフェース

```cpp
// raylib_integration/include/Graphics/Renderer.h
namespace Graphics {

class Renderer {
public:
  // ===== 初期化 =====
  static bool Initialize(int width, int height, const std::string& title);
  static void Shutdown();
  
  // ===== フレーム制御 =====
  static void BeginFrame();
  static void EndFrame();
  static bool ShouldClose();
  
  // ===== 描画API =====
  static void DrawRectangle(float x, float y, float width, float height,
                           const Color& color);
  static void DrawCircle(float x, float y, float radius, const Color& color);
  static void DrawSprite(const std::string& sprite_id, float x, float y,
                        float scale = 1.0f, float rotation = 0.0f,
                        const Color& tint = WHITE);
  
  // ===== テキスト描画 =====
  static void DrawText(const std::string& text, float x, float y, int font_size,
                      const Color& color);
  static void DrawTextEx(const std::string& font_id, const std::string& text,
                        float x, float y, float font_size, float spacing,
                        const Color& color);
  
  // ===== クリア =====
  static void ClearScreen(const Color& color = BLACK);
  
  // ===== レイヤー管理 =====
  static void SetDrawLayer(int layer);
  static int GetCurrentLayer();
  
  // ===== ビューポート =====
  static void SetViewport(int x, int y, int width, int height);
  static void ResetViewport();
  
  // ===== スクリーン情報 =====
  static int GetScreenWidth();
  static int GetScreenHeight();
  static float GetDeltaTime();
  static float GetFPS();
};

} // namespace Graphics
```

### Sprite Manager

```cpp
// raylib_integration/include/Graphics/SpriteManager.h
namespace Graphics {

struct SpriteData {
  std::string id;
  Texture2D texture;
  int source_x = 0, source_y = 0;
  int source_width, source_height;
  int origin_x = 0, origin_y = 0;  // ピボット
  bool is_loaded = false;
};

class SpriteManager {
private:
  static std::unordered_map<std::string, SpriteData> sprites_;
  static std::string sprite_directory_;

public:
  // ===== 初期化 =====
  static void Initialize(const std::string& sprite_dir);
  static void Shutdown();
  
  // ===== スプライト管理 =====
  static bool LoadSprite(const std::string& sprite_id,
                        const std::string& filepath);
  static bool LoadSpriteSheet(const std::string& base_id,
                             const std::string& filepath,
                             const nlohmann::json& sprite_def);
  static void UnloadSprite(const std::string& sprite_id);
  static void UnloadAll();
  
  // ===== スプライト取得 =====
  static const SpriteData* GetSprite(const std::string& sprite_id);
  static Texture2D GetTexture(const std::string& sprite_id);
  
  // ===== スプライト描画 =====
  static void DrawSprite(const std::string& sprite_id, float x, float y,
                        float scale = 1.0f, float rotation = 0.0f,
                        const Color& tint = WHITE);
  
  // ===== キャッシュ管理 =====
  static size_t GetLoadedSpriteCount();
  static void PrintStats();

private:
  static bool IsSpriteLoaded(const std::string& sprite_id);
  static void OnTextureLoaded(const std::string& sprite_id);
};

} // namespace Graphics
```

### Sprite Manager 実装

```cpp
// raylib_integration/src/Graphics/SpriteManager.cpp
namespace Graphics {

std::unordered_map<std::string, SpriteData> SpriteManager::sprites_;
std::string SpriteManager::sprite_directory_ = "assets/sprites";

void SpriteManager::Initialize(const std::string& sprite_dir) {
  sprite_directory_ = sprite_dir;
}

bool SpriteManager::LoadSprite(const std::string& sprite_id,
                              const std::string& filepath) {
  // スプライト既ロード確認
  if (sprites_.count(sprite_id) && sprites_[sprite_id].is_loaded) {
    return true;
  }
  
  // ファイル存在確認
  std::string full_path = sprite_directory_ + "/" + filepath;
  if (!std::filesystem::exists(full_path)) {
    std::cerr << "Sprite file not found: " << full_path << std::endl;
    return false;
  }
  
  // テクスチャロード
  Texture2D texture = LoadTexture(full_path.c_str());
  if (texture.id == 0) {
    std::cerr << "Failed to load texture: " << full_path << std::endl;
    return false;
  }
  
  // スプライト登録
  SpriteData& sprite = sprites_[sprite_id];
  sprite.id = sprite_id;
  sprite.texture = texture;
  sprite.source_width = texture.width;
  sprite.source_height = texture.height;
  sprite.is_loaded = true;
  
  return true;
}

bool SpriteManager::LoadSpriteSheet(const std::string& base_id,
                                   const std::string& filepath,
                                   const nlohmann::json& sprite_def) {
  // スプライトシート（複数フレーム）ロード
  Texture2D texture = LoadTexture(filepath.c_str());
  if (texture.id == 0) {
    return false;
  }
  
  // JSON定義からスプライト情報抽出
  // 例: {"frames": [{"id": "char_001_idle_0", "x": 0, "y": 0, "w": 64, "h": 64}, ...]}
  for (const auto& frame : sprite_def["frames"]) {
    std::string frame_id = base_id + "_" + std::string(frame["id"]);
    
    SpriteData sprite;
    sprite.id = frame_id;
    sprite.texture = texture;
    sprite.source_x = frame["x"];
    sprite.source_y = frame["y"];
    sprite.source_width = frame["w"];
    sprite.source_height = frame["h"];
    sprite.origin_x = frame.value("origin_x", 0);
    sprite.origin_y = frame.value("origin_y", 0);
    sprite.is_loaded = true;
    
    sprites_[frame_id] = sprite;
  }
  
  return true;
}

void SpriteManager::DrawSprite(const std::string& sprite_id, float x, float y,
                              float scale, float rotation, const Color& tint) {
  if (!sprites_.count(sprite_id)) {
    std::cerr << "Sprite not found: " << sprite_id << std::endl;
    return;
  }
  
  const SpriteData& sprite = sprites_[sprite_id];
  
  Rectangle source = {
    static_cast<float>(sprite.source_x),
    static_cast<float>(sprite.source_y),
    static_cast<float>(sprite.source_width),
    static_cast<float>(sprite.source_height)
  };
  
  Rectangle dest = {
    x - sprite.origin_x * scale,
    y - sprite.origin_y * scale,
    sprite.source_width * scale,
    sprite.source_height * scale
  };
  
  Vector2 origin = {
    static_cast<float>(sprite.origin_x * scale),
    static_cast<float>(sprite.origin_y * scale)
  };
  
  DrawTexturePro(sprite.texture, source, dest, origin, rotation, tint);
}

} // namespace Graphics
```

### Font Manager

```cpp
// raylib_integration/include/Graphics/FontManager.h
namespace Graphics {

class FontManager {
private:
  static std::unordered_map<std::string, Font> fonts_;
  static std::string font_directory_;

public:
  static void Initialize(const std::string& font_dir);
  static void Shutdown();
  
  // ===== フォント管理 =====
  static bool LoadFont(const std::string& font_id, const std::string& filepath,
                      int font_size = 32);
  static void UnloadFont(const std::string& font_id);
  
  // ===== フォント取得 =====
  static Font GetFont(const std::string& font_id);
  static Font GetDefaultFont();
  
  // ===== テキスト描画 =====
  static void DrawTextEx(const std::string& font_id, const std::string& text,
                        Vector2 position, float font_size, float spacing,
                        Color color);
  
  // ===== テキスト計測 =====
  static Vector2 MeasureText(const std::string& font_id, const std::string& text,
                            float font_size, float spacing);

private:
  static bool IsFontLoaded(const std::string& font_id);
};

} // namespace Graphics
```

### Texture Cache

```cpp
// raylib_integration/include/Graphics/TextureCache.h
namespace Graphics {

class TextureCache {
private:
  struct TextureEntry {
    Texture2D texture;
    std::string filepath;
    int reference_count = 0;
    float last_access_time = 0.0f;
  };
  
  static std::unordered_map<std::string, TextureEntry> cache_;
  static int max_cache_size_;
  static float cache_timeout_;

public:
  static void Initialize(int max_size = 256, float timeout = 300.0f);
  
  // ===== キャッシュ管理 =====
  static Texture2D Get(const std::string& filepath);
  static void Release(const std::string& filepath);
  static void Clear();
  static void Update(float delta_time);
  
  // ===== 統計 =====
  static int GetCacheSize();
  static void PrintStats();

private:
  static void EvictOldest();
  static void EvictByLRU();
};

} // namespace Graphics
```

---

## Input System（入力システム）

### Input Manager

```cpp
// raylib_integration/include/Input/InputManager.h
namespace Input {

enum class KeyCode : int {
  // ===== 文字キー =====
  A = KEY_A, B = KEY_B, C = KEY_C, D = KEY_D, E = KEY_E,
  F = KEY_F, G = KEY_G, H = KEY_H, I = KEY_I, J = KEY_J,
  K = KEY_K, L = KEY_L, M = KEY_M, N = KEY_N, O = KEY_O,
  P = KEY_P, Q = KEY_Q, R = KEY_R, S = KEY_S, T = KEY_T,
  U = KEY_U, V = KEY_V, W = KEY_W, X = KEY_X, Y = KEY_Y, Z = KEY_Z,
  
  // ===== 数字キー =====
  NUM_0 = KEY_ZERO, NUM_1 = KEY_ONE, NUM_2 = KEY_TWO,
  NUM_3 = KEY_THREE, NUM_4 = KEY_FOUR, NUM_5 = KEY_FIVE,
  NUM_6 = KEY_SIX, NUM_7 = KEY_SEVEN, NUM_8 = KEY_EIGHT, NUM_9 = KEY_NINE,
  
  // ===== ファンクションキー =====
  F1 = KEY_F1, F2 = KEY_F2, F3 = KEY_F3, F4 = KEY_F4,
  F5 = KEY_F5, F6 = KEY_F6, F7 = KEY_F7, F8 = KEY_F8,
  F9 = KEY_F9, F10 = KEY_F10, F11 = KEY_F11, F12 = KEY_F12,
  
  // ===== 特殊キー =====
  ESCAPE = KEY_ESCAPE,
  ENTER = KEY_ENTER,
  TAB = KEY_TAB,
  BACKSPACE = KEY_BACKSPACE,
  DELETE = KEY_DELETE,
  INSERT = KEY_INSERT,
  
  // ===== 矢印キー =====
  UP = KEY_UP,
  DOWN = KEY_DOWN,
  LEFT = KEY_LEFT,
  RIGHT = KEY_RIGHT,
  
  // ===== 修飾キー =====
  CTRL = KEY_LEFT_CONTROL,
  SHIFT = KEY_LEFT_SHIFT,
  ALT = KEY_LEFT_ALT,
  
  // ===== その他 =====
  SPACE = KEY_SPACE,
  NONE = KEY_NULL,
};

enum class MouseButton {
  LEFT = 0,
  RIGHT = 1,
  MIDDLE = 2,
};

class InputManager {
private:
  // ===== キー状態 =====
  std::unordered_map<int, bool> key_pressed_;
  std::unordered_map<int, bool> key_released_;
  std::unordered_map<int, bool> key_held_;
  
  // ===== マウス状態 =====
  Vector2 mouse_position_ = {0, 0};
  Vector2 mouse_delta_ = {0, 0};
  bool mouse_left_pressed_ = false;
  bool mouse_left_released_ = false;
  bool mouse_right_pressed_ = false;
  float scroll_delta_ = 0.0f;
  
  // ===== キーバインディング =====
  std::unordered_map<std::string, int> key_bindings_;

public:
  // ===== 初期化 =====
  static void Initialize();
  static void Update();
  
  // ===== キー入力 =====
  static bool IsKeyPressed(KeyCode key);
  static bool IsKeyReleased(KeyCode key);
  static bool IsKeyHeld(KeyCode key);
  static bool IsKeyDown(KeyCode key);  // IsKeyPressed || IsKeyHeld
  
  // ===== マウス入力 =====
  static Vector2 GetMousePosition();
  static Vector2 GetMouseDelta();
  static bool IsMouseButtonPressed(MouseButton button);
  static bool IsMouseButtonReleased(MouseButton button);
  static bool IsMouseButtonHeld(MouseButton button);
  static float GetScrollDelta();
  
  // ===== キーバインディング =====
  static void BindKey(const std::string& action, KeyCode key);
  static void UnbindKey(const std::string& action);
  static bool IsActionPressed(const std::string& action);
  static bool IsActionHeld(const std::string& action);
  
  // ===== テキスト入力 =====
  static int GetCharPressed();
  static std::string GetClipboardText();
  static void SetClipboardText(const std::string& text);

private:
  static InputManager& GetInstance();
  void UpdateInternal();
};

} // namespace Input
```

### Input Manager 実装

```cpp
// raylib_integration/src/Input/InputManager.cpp
namespace Input {

// シングルトン実装
static InputManager* g_input_manager = nullptr;

void InputManager::Initialize() {
  if (!g_input_manager) {
    g_input_manager = new InputManager();
  }
}

void InputManager::Update() {
  if (g_input_manager) {
    g_input_manager->UpdateInternal();
  }
}

void InputManager::UpdateInternal() {
  // 前フレーム状態をリセット
  key_pressed_.clear();
  key_released_.clear();
  mouse_left_pressed_ = false;
  mouse_left_released_ = false;
  mouse_right_pressed_ = false;
  scroll_delta_ = 0.0f;
  
  // 前フレーム位置を保存
  Vector2 prev_mouse_pos = mouse_position_;
  
  // 現在のマウス位置更新
  mouse_position_ = GetMousePosition();
  mouse_delta_ = {
    mouse_position_.x - prev_mouse_pos.x,
    mouse_position_.y - prev_mouse_pos.y
  };
  
  // スクロール量取得
  scroll_delta_ = GetMouseWheelMoveV().y;
  
  // マウスボタン更新
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    mouse_left_pressed_ = true;
  }
  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    mouse_left_released_ = true;
  }
  
  // キー状態更新（Raylib API利用）
  for (int key = KEY_APOSTROPHE; key < KEY_LAST; ++key) {
    if (IsKeyPressed(key)) {
      key_pressed_[key] = true;
      key_held_[key] = true;
    }
    if (IsKeyReleased(key)) {
      key_released_[key] = true;
      key_held_[key] = false;
    }
  }
}

bool InputManager::IsKeyPressed(KeyCode key) {
  return g_input_manager && g_input_manager->key_pressed_[static_cast<int>(key)];
}

bool InputManager::IsKeyReleased(KeyCode key) {
  return g_input_manager && g_input_manager->key_released_[static_cast<int>(key)];
}

bool InputManager::IsKeyHeld(KeyCode key) {
  return g_input_manager && g_input_manager->key_held_[static_cast<int>(key)];
}

Vector2 InputManager::GetMousePosition() {
  return g_input_manager ? g_input_manager->mouse_position_ : Vector2{0, 0};
}

void InputManager::BindKey(const std::string& action, KeyCode key) {
  if (g_input_manager) {
    g_input_manager->key_bindings_[action] = static_cast<int>(key);
  }
}

bool InputManager::IsActionPressed(const std::string& action) {
  if (!g_input_manager) return false;
  
  auto it = g_input_manager->key_bindings_.find(action);
  if (it == g_input_manager->key_bindings_.end()) return false;
  
  return IsKeyPressed(static_cast<KeyCode>(it->second));
}

} // namespace Input
```

---

## Audio System（オーディオシステム）

### Audio Manager

```cpp
// raylib_integration/include/Audio/AudioManager.h
namespace Audio {

enum class AudioType {
  BGM,        // Background Music
  SE,         // Sound Effect
  VOICE,      // Voice/Dialogue
};

class AudioManager {
private:
  struct SoundEntry {
    Sound sound;
    std::string filepath;
    bool is_loaded = false;
  };
  
  struct MusicEntry {
    Music music;
    std::string filepath;
    bool is_playing = false;
    bool loop = true;
  };
  
  static std::unordered_map<std::string, SoundEntry> sound_cache_;
  static std::unordered_map<std::string, MusicEntry> music_cache_;
  
  static float master_volume_;
  static float bgm_volume_;
  static float se_volume_;
  static float voice_volume_;
  
  static std::string current_bgm_;
  static int max_simultaneous_sounds_;

public:
  // ===== 初期化 =====
  static void Initialize();
  static void Shutdown();
  static void Update();
  
  // ===== サウンドロード =====
  static bool LoadSound(const std::string& sound_id, const std::string& filepath);
  static bool LoadMusic(const std::string& music_id, const std::string& filepath);
  static void UnloadSound(const std::string& sound_id);
  static void UnloadMusic(const std::string& music_id);
  
  // ===== 再生制御 =====
  static void PlaySound(const std::string& sound_id, float volume = 1.0f,
                       AudioType type = AudioType::SE);
  static void PlayMusic(const std::string& music_id, bool loop = true);
  static void StopMusic();
  static void PauseMusic();
  static void ResumeMusic();
  
  // ===== ボリューム制御 =====
  static void SetMasterVolume(float volume);  // 0.0 - 1.0
  static void SetBGMVolume(float volume);
  static void SetSEVolume(float volume);
  static void SetVoiceVolume(float volume);
  
  static float GetMasterVolume();
  static float GetBGMVolume();
  static float GetSEVolume();
  
  // ===== 状態確認 =====
  static bool IsMusicPlaying();
  static bool IsMusicPaused();
  static float GetMusicPlayTime();
  static float GetMusicDuration();

private:
  static void UpdateMusicVolume();
  static void UpdateSoundVolume();
};

} // namespace Audio
```

### Audio Manager 実装

```cpp
// raylib_integration/src/Audio/AudioManager.cpp
namespace Audio {

std::unordered_map<std::string, AudioManager::SoundEntry> 
  AudioManager::sound_cache_;
std::unordered_map<std::string, AudioManager::MusicEntry> 
  AudioManager::music_cache_;

float AudioManager::master_volume_ = 1.0f;
float AudioManager::bgm_volume_ = 0.7f;
float AudioManager::se_volume_ = 0.9f;
float AudioManager::voice_volume_ = 0.8f;
std::string AudioManager::current_bgm_;
int AudioManager::max_simultaneous_sounds_ = 16;

void AudioManager::Initialize() {
  InitAudioDevice();
}

void AudioManager::Shutdown() {
  // 全サウンドアンロード
  for (auto& [id, entry] : sound_cache_) {
    if (entry.is_loaded) {
      UnloadSound(entry.sound);
    }
  }
  sound_cache_.clear();
  
  // 全ミュージックアンロード
  for (auto& [id, entry] : music_cache_) {
    if (entry.music.ctxData != nullptr) {
      UnloadMusicStream(entry.music);
    }
  }
  music_cache_.clear();
  
  CloseAudioDevice();
}

void AudioManager::Update() {
  // ストリーミング再生のみ：毎フレーム更新が必要
  if (!current_bgm_.empty() && music_cache_.count(current_bgm_)) {
    auto& music = music_cache_[current_bgm_].music;
    UpdateMusicStream(music);
  }
}

bool AudioManager::LoadSound(const std::string& sound_id, 
                            const std::string& filepath) {
  if (sound_cache_.count(sound_id) && sound_cache_[sound_id].is_loaded) {
    return true;
  }
  
  Sound sound = LoadSound(filepath.c_str());
  if (sound.frameCount == 0) {
    std::cerr << "Failed to load sound: " << filepath << std::endl;
    return false;
  }
  
  SoundEntry& entry = sound_cache_[sound_id];
  entry.sound = sound;
  entry.filepath = filepath;
  entry.is_loaded = true;
  
  return true;
}

void AudioManager::PlaySound(const std::string& sound_id, float volume,
                            AudioType type) {
  if (!sound_cache_.count(sound_id)) {
    std::cerr << "Sound not found: " << sound_id << std::endl;
    return;
  }
  
  Sound& sound = sound_cache_[sound_id].sound;
  
  // ボリューム計算
  float final_volume = master_volume_;
  switch (type) {
    case AudioType::SE:
      final_volume *= se_volume_ * volume;
      break;
    case AudioType::VOICE:
      final_volume *= voice_volume_ * volume;
      break;
    default:
      final_volume *= volume;
  }
  
  SetSoundVolume(sound, final_volume);
  PlaySound(sound);
}

void AudioManager::PlayMusic(const std::string& music_id, bool loop) {
  // 前のBGM停止
  if (!current_bgm_.empty() && music_cache_.count(current_bgm_)) {
    StopMusicStream(music_cache_[current_bgm_].music);
  }
  
  if (!music_cache_.count(music_id)) {
    std::cerr << "Music not found: " << music_id << std::endl;
    return;
  }
  
  auto& music_entry = music_cache_[music_id];
  music_entry.loop = loop;
  
  PlayMusicStream(music_entry.music);
  current_bgm_ = music_id;
  
  UpdateMusicVolume();
}

void AudioManager::SetMasterVolume(float volume) {
  master_volume_ = glm::clamp(volume, 0.0f, 1.0f);
  UpdateMusicVolume();
  UpdateSoundVolume();
}

void AudioManager::UpdateMusicVolume() {
  if (!current_bgm_.empty() && music_cache_.count(current_bgm_)) {
    float final_volume = master_volume_ * bgm_volume_;
    SetMusicVolume(music_cache_[current_bgm_].music, final_volume);
  }
}

} // namespace Audio
```

---

## Resource Management（リソース管理）

### Resource Loader

```cpp
// raylib_integration/include/Resources/ResourceLoader.h
namespace Resources {

enum class ResourceType {
  SPRITE,
  TEXTURE,
  FONT,
  SOUND,
  MUSIC,
};

struct LoadRequest {
  std::string id;
  ResourceType type;
  std::string filepath;
  std::function<void(bool)> callback;  // オプション
};

class ResourceLoader {
private:
  static std::vector<LoadRequest> pending_loads_;
  static std::unordered_set<std::string> loaded_resources_;

public:
  // ===== 同期ロード =====
  static bool Load(const std::string& resource_id, ResourceType type,
                  const std::string& filepath);
  
  // ===== 非同期ロード（オプション） =====
  static void LoadAsync(const std::string& resource_id, ResourceType type,
                       const std::string& filepath,
                       std::function<void(bool)> callback = nullptr);
  
  // ===== アンロード =====
  static void Unload(const std::string& resource_id, ResourceType type);
  static void UnloadAll(ResourceType type);
  
  // ===== 状態確認 =====
  static bool IsLoaded(const std::string& resource_id);
  static int GetPendingLoadCount();
  
  // ===== 非同期ロード処理 =====
  static void UpdateAsyncLoads();

private:
  static bool LoadInternal(const LoadRequest& request);
};

} // namespace Resources
```

### Memory Pool

```cpp
// raylib_integration/include/Resources/MemoryPool.h
namespace Resources {

template<typename T>
class MemoryPool {
private:
  std::vector<std::unique_ptr<T>> pool_;
  std::queue<T*> available_;
  size_t initial_size_;
  size_t max_size_;

public:
  MemoryPool(size_t initial_size = 100, size_t max_size = 1000)
    : initial_size_(initial_size), max_size_(max_size) {
    for (size_t i = 0; i < initial_size; ++i) {
      auto obj = std::make_unique<T>();
      available_.push(obj.get());
      pool_.push_back(std::move(obj));
    }
  }
  
  T* Acquire() {
    if (available_.empty()) {
      if (pool_.size() < max_size_) {
        auto obj = std::make_unique<T>();
        T* ptr = obj.get();
        pool_.push_back(std::move(obj));
        return ptr;
      }
      return nullptr;
    }
    
    T* obj = available_.front();
    available_.pop();
    return obj;
  }
  
  void Release(T* obj) {
    if (obj) {
      obj->Reset();  // オブジェクトをリセット
      available_.push(obj);
    }
  }
  
  void Clear() {
    while (!available_.empty()) {
      available_.pop();
    }
    pool_.clear();
  }
  
  size_t GetPoolSize() const { return pool_.size(); }
  size_t GetAvailableCount() const { return available_.size(); }
};

} // namespace Resources
```

---

## Camera & Viewport

### Camera Controller

```cpp
// raylib_integration/include/Graphics/CameraController.h
namespace Graphics {

class CameraController {
private:
  Camera2D camera_;
  Vector2 target_position_;
  float target_zoom_;
  float zoom_speed_ = 0.1f;
  float pan_speed_ = 5.0f;
  
  // ===== カメラ振動 =====
  float shake_intensity_ = 0.0f;
  float shake_duration_ = 0.0f;
  float shake_elapsed_ = 0.0f;

public:
  CameraController();
  
  // ===== カメラ制御 =====
  void Update(float delta_time);
  void SetPosition(Vector2 position);
  void SetZoom(float zoom);
  void SetTarget(Vector2 target, float zoom);
  
  Vector2 GetPosition() const { return camera_.target; }
  float GetZoom() const { return camera_.zoom; }
  Camera2D GetCamera2D() const { return camera_; }
  
  // ===== スムーズ移動 =====
  void Pan(Vector2 direction, float speed = 0.0f);
  void Zoom(float delta_zoom);
  void SmoothPanTo(Vector2 target_pos, float duration);
  void SmoothZoomTo(float target_zoom, float duration);
  
  // ===== 画面振動 =====
  void Shake(float intensity, float duration);
  void UpdateShake(float delta_time);
  
  // ===== 座標変換 =====
  Vector2 ScreenToWorld(Vector2 screen_pos);
  Vector2 WorldToScreen(Vector2 world_pos);
  
private:
  void ApplyShake();
};

} // namespace Graphics
```

---

## Particle & Effects（パーティクル）

### Particle System

```cpp
// raylib_integration/include/Graphics/ParticleSystem.h
namespace Graphics {

struct Particle {
  Vector2 position;
  Vector2 velocity;
  float lifetime;
  float max_lifetime;
  Color color;
  float size;
  float rotation;
  float rotation_speed;
  
  void Update(float delta_time) {
    position.x += velocity.x * delta_time;
    position.y += velocity.y * delta_time;
    lifetime -= delta_time;
    
    // フェードアウト
    float alpha_factor = lifetime / max_lifetime;
    color.a = static_cast<unsigned char>(255 * alpha_factor);
    
    rotation += rotation_speed * delta_time;
  }
  
  bool IsAlive() const { return lifetime > 0.0f; }
};

class ParticleSystem {
private:
  std::vector<Particle> particles_;
  std::string sprite_id_;
  int max_particles_;

public:
  ParticleSystem(const std::string& sprite_id = "", int max_particles = 1000);
  
  // ===== パーティクル発行 =====
  void Emit(Vector2 position, Vector2 velocity, float lifetime,
           Color color = WHITE, float size = 1.0f);
  
  void EmitBurst(Vector2 position, int count, float speed_min,
                float speed_max, float lifetime, Color color = WHITE);
  
  // ===== 更新・描画 =====
  void Update(float delta_time);
  void Draw();
  
  // ===== 管理 =====
  void Clear();
  int GetActiveParticleCount() const;
  
private:
  void RemoveDeadParticles();
};

} // namespace Graphics
```

---

## 統合実装例

### Game Window 初期化

```cpp
// game/src/main_game.cpp
#include "Graphics/Renderer.h"
#include "Input/InputManager.h"
#include "Audio/AudioManager.h"
#include "Game/Application/Game.h"

int main() {
  const int SCREEN_WIDTH = 1280;
  const int SCREEN_HEIGHT = 720;
  const std::string WINDOW_TITLE = "SimpleTDCGame";
  
  // Graphics初期化
  if (!Graphics::Renderer::Initialize(SCREEN_WIDTH, SCREEN_HEIGHT, 
      WINDOW_TITLE)) {
    std::cerr << "Failed to initialize graphics" << std::endl;
    return 1;
  }
  
  // Input初期化
  Input::InputManager::Initialize();
  
  // Audio初期化
  Audio::AudioManager::Initialize();
  
  // リソース初期化
  Graphics::SpriteManager::Initialize("assets/sprites");
  Graphics::FontManager::Initialize("assets/fonts");
  
  // Game初期化
  auto game = std::make_unique<Game::Application::Game>();
  if (!game->Initialize()) {
    std::cerr << "Failed to initialize game" << std::endl;
    return 1;
  }
  
  // メインループ
  while (!Graphics::Renderer::ShouldClose()) {
    float delta_time = Graphics::Renderer::GetDeltaTime();
    
    // 入力更新
    Input::InputManager::Update();
    
    // ゲーム更新
    game->Update(delta_time);
    
    // 音声更新
    Audio::AudioManager::Update();
    
    // 描画開始
    Graphics::Renderer::BeginFrame();
    {
      // ゲーム描画
      game->Draw();
    }
    Graphics::Renderer::EndFrame();
  }
  
  // クリーンアップ
  game->Shutdown();
  
  Graphics::FontManager::Shutdown();
  Graphics::SpriteManager::SpriteManager::Shutdown();
  Audio::AudioManager::Shutdown();
  Graphics::Renderer::Shutdown();
  
  return 0;
}
```

### Game描画例

```cpp
// game/src/Game/Application/Game.cpp
#include "Graphics/Renderer.h"
#include "Graphics/CameraController.h"
#include "Input/InputManager.h"

namespace Game::Application {

void Game::Draw() {
  // 背景描画
  Graphics::Renderer::ClearScreen(DARK_BLUE);
  
  // ゲームフィールド描画
  Graphics::Renderer::SetDrawLayer(0);  // Background layer
  DrawGameField();
  
  // キャラクター・敵描画
  Graphics::Renderer::SetDrawLayer(1);  // Game layer
  DrawEntities();
  DrawEffects();
  
  // UI描画
  Graphics::Renderer::SetDrawLayer(2);  // UI layer
  DrawUI();
}

void Game::DrawGameField() {
  // ゲームフィールド背景
  Graphics::Renderer::DrawRectangle(0, 0, 1280, 720, {50, 50, 100, 255});
  
  // グリッド表示（オプション）
  for (int x = 0; x < 1280; x += 64) {
    Graphics::Renderer::DrawRectangle(x, 0, 1, 720, {100, 100, 100, 128});
  }
}

void Game::DrawEntities() {
  for (auto& entity : entities_) {
    if (!entity->IsVisible()) continue;
    
    // スプライト描画
    Graphics::Renderer::DrawSprite(
      entity->GetSpriteId(),
      entity->GetPosition().x,
      entity->GetPosition().y,
      entity->GetScale(),
      entity->GetRotation()
    );
    
    // デバッグ表示（開発時）
    if (show_debug_) {
      // HP表示
      float hp_ratio = static_cast<float>(entity->GetHP()) / 
                      entity->GetMaxHP();
      Graphics::Renderer::DrawRectangle(
        entity->GetPosition().x - 25,
        entity->GetPosition().y - 40,
        50, 8, RED
      );
      Graphics::Renderer::DrawRectangle(
        entity->GetPosition().x - 25,
        entity->GetPosition().y - 40,
        50 * hp_ratio, 8, GREEN
      );
    }
  }
}

void Game::DrawEffects() {
  for (auto& particle_system : particle_systems_) {
    particle_system->Draw();
  }
}

void Game::DrawUI() {
  // HUD描画
  std::string hp_text = "HP: " + std::to_string(player_hp_);
  Graphics::Renderer::DrawText(hp_text, 20, 20, 20, WHITE);
  
  std::string cost_text = "CP: " + std::to_string(current_cost_) + 
                         "/" + std::to_string(max_cost_);
  Graphics::Renderer::DrawText(cost_text, 20, 50, 20, WHITE);
}

} // namespace Game::Application
```

---

## 実装優先度

### Phase 6.1: Core Graphics（3日）

```
Day 1:
  ✅ Renderer メインAPI実装
  ✅ SpriteManager 実装
  ✅ 基本描画テスト

Day 2:
  ✅ FontManager実装
  ✅ TextureCache実装
  ✅ UI描画統合

Day 3:
  ✅ CameraController実装
  ✅ ParticleSystem実装
  ✅ パフォーマンス最適化
```

### Phase 6.2: Input & Audio（2日）

```
Day 1:
  ✅ InputManager実装
  ✅ キーバインディング
  ✅ マウス入力統合

Day 2:
  ✅ AudioManager実装
  ✅ SoundCache実装
  ✅ ボリュームコントロール
```

### Phase 6.3: Resource Management（2日）

```
Day 1:
  ✅ ResourceLoader実装
  ✅ MemoryPool実装
  ✅ ホットリロード統合

Day 2:
  ✅ 非同期ロード（オプション）
  ✅ メモリ最適化
  ✅ 統合テスト
```

---

## チェックリスト

```
Graphics System:
  ☐ Renderer 基本API
  ☐ SpriteManager + キャッシング
  ☐ FontManager
  ☐ TextureCache（LRU）
  ☐ ShaderSystem（オプション）

Input System:
  ☐ InputManager 統一API
  ☐ KeyCode列挙
  ☐ キーバインディング
  ☐ マウス入力
  ☐ ゲームパッド対応予定

Audio System:
  ☐ AudioManager
  ☐ SoundCache
  ☐ MusicPlayer
  ☐ ボリューム制御
  ☐ 同時再生制限

Resource Management:
  ☐ ResourceLoader（同期）
  ☐ ResourceLoader（非同期 - オプション）
  ☐ MemoryPool<T>
  ☐ ホットリロード連携

Camera & Viewport:
  ☐ CameraController
  ☐ スムーズ移動
  ☐ 画面振動効果
  ☐ 座標変換

Particle & Effects:
  ☐ ParticleSystem
  ☐ Particle構造体
  ☐ バースト発行
  ☐ パフォーマンス最適化
```

---

## 次のドキュメント

- [ ] **実装スケジュール詳細** (全6層 x 実装期間)
- [ ] **テスト戦略** (Unit/Integration/E2E/Performance)
- [ ] **デプロイメント & ビルド手順**
- [ ] **開発環境セットアップ**

---

## サマリー

Raylib統合設計（グラフィックス層）が完成しました：

```
✅ Graphics System - 描画・スプライト・フォント管理
✅ Input System - 統一キー入力・キーバインディング
✅ Audio System - BGM/SE管理・ボリュームコントロール
✅ Resource Management - リソースロード・メモリプール
✅ Camera & Viewport - カメラ制御・画面振動
✅ Particle & Effects - パーティクルシステム

🎉 完全な 6層アーキテクチャ確立！

全層構成：
  ✅ Layer 1: Core (基盤)
  ✅ Layer 2: Game (管理・制御)
  ✅ Layer 3: TD (ECS - ゲームロジック)
  ✅ Layer 4: Application (UI・シーン)
  ✅ Layer 5: Editor (開発ツール)
  ✅ Layer 6: Raylib Integration (グラフィックス)

次は実装スケジュール詳細化へ！
```

