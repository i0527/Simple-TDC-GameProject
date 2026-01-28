#pragma once

// 標準ライブラリ
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// 外部ライブラリ
#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)
#include <spdlog/spdlog.h>
#endif

// プロジェクト内
#include "../config/GameConfig.hpp"
#include "../config/RenderTypes.hpp"
#include "AudioSystemAPI.hpp"
#include "CollisionSystemAPI.hpp"
#include "RenderSystemAPI.hpp"
#include "ResourceSystemAPI.hpp"
#include "TimingSystemAPI.hpp"
#include "WindowSystemAPI.hpp"


namespace game {
namespace core {

#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)
using LogLevel = spdlog::level::level_enum;
#else
using LogLevel = int;
#endif

/// @brief 統合システムAPIクラス（コア + API群の所有）
class BaseSystemAPI {
public:
  BaseSystemAPI();
  ~BaseSystemAPI();

  // ========== 初期化・終了 ==========
  bool Initialize(Resolution initialResolution);
  void Shutdown();
  bool IsInitialized() const;

  // ========== ログ管理 ==========
  void SetLogPath(const std::string &directory, const std::string &filename);
  void SetLogLevel(LogLevel level);

  // ========== APIアクセス ==========
  RenderSystemAPI &Render();
  const RenderSystemAPI &Render() const;
  ResourceSystemAPI &Resource();
  const ResourceSystemAPI &Resource() const;
  AudioSystemAPI &Audio();
  const AudioSystemAPI &Audio() const;
  WindowSystemAPI &Window();
  const WindowSystemAPI &Window() const;
  TimingSystemAPI &Timing();
  const TimingSystemAPI &Timing() const;
  CollisionSystemAPI &Collision();
  const CollisionSystemAPI &Collision() const;

private:
  friend class RenderSystemAPI;
  friend class ResourceSystemAPI;
  friend class AudioSystemAPI;
  friend class WindowSystemAPI;
  friend class TimingSystemAPI;
  friend class CollisionSystemAPI;

  // ========== プライベートメソッド ==========
  void RecreateRenderTexture();
  Font *GetDefaultFontInternal() const;
  void GenerateFontCodepoints();
  void InitializeLogSystem();
  void ShutdownLogSystem();
  float CalculateTextureLuminance(const std::string &textureKey);
  std::string ResolveTexturePath(const std::string &textureKey) const;

  // ========== メンバ変数 ==========
  Resolution currentResolution_;
  int screenWidth_;
  int screenHeight_;

  static constexpr int INTERNAL_WIDTH = 1920;
  static constexpr int INTERNAL_HEIGHT = 1080;

  RenderTexture2D mainRenderTexture_;
  bool isInitialized_;
  bool resourcesInitialized_;

#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)
  std::shared_ptr<spdlog::logger> logger_;
#endif
  bool logInitialized_;
  std::string logDirectory_;
  std::string logFileName_;

  std::unordered_map<std::string, std::shared_ptr<Texture2D>> textures_;
  std::unordered_map<std::string, std::shared_ptr<Sound>> sounds_;
  std::unordered_map<std::string, std::shared_ptr<Music>> musics_;
  std::unordered_map<std::string, std::shared_ptr<Font>> fonts_;
  std::shared_ptr<Font> defaultFont_;

  std::vector<int> fontCodepoints_;
  bool imGuiInitialized_;
  void *imGuiJapaneseFont_;

  std::vector<ResourceFileInfo> resourceFileList_;
  size_t currentResourceIndex_;
  bool scanningCompleted_;

  std::unordered_set<std::string> registeredTextureKeys_;
  std::vector<AssetLicenseEntry> assetLicenses_;

  float masterVolume_;
  float seVolume_;
  float bgmVolume_;
  Music *currentMusic_;
  std::string currentMusicName_;
  std::unordered_map<std::string, Sound *> playingSounds_;

  bool fpsDisplayEnabled_;
  bool cursorDisplayEnabled_;

  std::unordered_map<std::string, float> textureLuminanceCache_;
  std::unordered_map<std::string, Color> textureTextColorCache_;

  RenderSystemAPI renderAPI_;
  ResourceSystemAPI resourceAPI_;
  AudioSystemAPI audioAPI_;
  WindowSystemAPI windowAPI_;
  TimingSystemAPI timingAPI_;
  CollisionSystemAPI collisionAPI_;
};

} // namespace core
} // namespace game
