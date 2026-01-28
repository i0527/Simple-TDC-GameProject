#include "../BaseSystemAPI.hpp"
#include "../../../utils/Log.h"
#include <iostream>

// 外部ライブラリ
#include <rlImGui.h>

namespace game {
namespace core {
BaseSystemAPI::BaseSystemAPI()
    : currentResolution_(Resolution::FHD),
      screenWidth_(GetResolutionWidth(Resolution::FHD)),
      screenHeight_(GetResolutionHeight(Resolution::FHD)),
      mainRenderTexture_({0}), isInitialized_(false),
      resourcesInitialized_(false), imGuiInitialized_(false),
      imGuiJapaneseFont_(nullptr), currentResourceIndex_(0),
      scanningCompleted_(false), masterVolume_(1.0f), seVolume_(1.0f),
      bgmVolume_(1.0f), currentMusic_(nullptr), currentMusicName_(""),
      fpsDisplayEnabled_(false), cursorDisplayEnabled_(false), logInitialized_(false), logDirectory_("logs"),
      logFileName_("game.log"), renderAPI_(this), resourceAPI_(this),
      audioAPI_(this), windowAPI_(this), timingAPI_(this), collisionAPI_(this) {
  GenerateFontCodepoints();
}

BaseSystemAPI::~BaseSystemAPI() {
  if (isInitialized_) {
    Shutdown();
  }
}

// ========== 初期化・終了 ==========

bool BaseSystemAPI::Initialize(Resolution initialResolution) {
  // ログシステムを初期化（Emscriptenでは無効化される）
  InitializeLogSystem();

  screenWidth_ = GetResolutionWidth(initialResolution);
  screenHeight_ = GetResolutionHeight(initialResolution);
  currentResolution_ = initialResolution;

  InitWindow(screenWidth_, screenHeight_, "tower of defense (´・ω・｀)");
  if (!IsWindowReady()) {
#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)
    LOG_ERROR("BaseSystemAPI: Failed to initialize Raylib window");
#else
    std::cerr << "BaseSystemAPI: Failed to initialize Raylib window" << std::endl;
#endif
    return false;
  }

  ClearWindowState(FLAG_WINDOW_RESIZABLE);

  SetTargetFPS(TARGET_FPS);

  InitAudioDevice();

  RecreateRenderTexture();
  if (mainRenderTexture_.id == 0) {
#if !defined(EMSCRIPTEN) && !defined(__EMSCRIPTEN__)
    LOG_ERROR("BaseSystemAPI: Failed to create RenderTexture");
#else
    std::cerr << "BaseSystemAPI: Failed to create RenderTexture" << std::endl;
#endif
    CloseAudioDevice();
    CloseWindow();
    return false;
  }

  isInitialized_ = true;

  LOG_INFO(
      "BaseSystemAPI: Initialized with resolution {}x{} (internal {}x{})",
      screenWidth_, screenHeight_, INTERNAL_WIDTH, INTERNAL_HEIGHT);
  return true;
}

void BaseSystemAPI::Shutdown() {
  if (!isInitialized_) {
    return;
  }

  LOG_INFO(
      "BaseSystemAPI shutdown: {} textures, {} sounds, {} musics, {} fonts",
      textures_.size(), sounds_.size(), musics_.size(), fonts_.size());

  defaultFont_.reset();

  musics_.clear();

  sounds_.clear();

  fonts_.clear();

  textures_.clear();

  if (imGuiInitialized_) {
    rlImGuiShutdown();
    imGuiInitialized_ = false;
  }

  if (mainRenderTexture_.id != 0) {
    UnloadRenderTexture(mainRenderTexture_);
    mainRenderTexture_ = {0};
  }

  for (auto &pair : playingSounds_) {
    Sound *sound = pair.second;
    if (sound) {
      ::StopSound(*sound);
    }
  }
  playingSounds_.clear();

  if (currentMusic_ != nullptr) {
    ::StopMusicStream(*currentMusic_);
  }
  currentMusic_ = nullptr;
  currentMusicName_.clear();

  CloseAudioDevice();

  if (IsWindowReady()) {
    CloseWindow();
  }

  isInitialized_ = false;
  resourcesInitialized_ = false;

  ShutdownLogSystem();
}

bool BaseSystemAPI::IsInitialized() const { return isInitialized_; }

RenderSystemAPI& BaseSystemAPI::Render() { return renderAPI_; }

const RenderSystemAPI& BaseSystemAPI::Render() const { return renderAPI_; }

ResourceSystemAPI& BaseSystemAPI::Resource() { return resourceAPI_; }

const ResourceSystemAPI& BaseSystemAPI::Resource() const {
  return resourceAPI_;
}

AudioSystemAPI& BaseSystemAPI::Audio() { return audioAPI_; }

const AudioSystemAPI& BaseSystemAPI::Audio() const { return audioAPI_; }

WindowSystemAPI& BaseSystemAPI::Window() { return windowAPI_; }

const WindowSystemAPI& BaseSystemAPI::Window() const { return windowAPI_; }

TimingSystemAPI& BaseSystemAPI::Timing() { return timingAPI_; }

const TimingSystemAPI& BaseSystemAPI::Timing() const { return timingAPI_; }

CollisionSystemAPI& BaseSystemAPI::Collision() { return collisionAPI_; }

const CollisionSystemAPI& BaseSystemAPI::Collision() const {
  return collisionAPI_;
}

void BaseSystemAPI::RecreateRenderTexture() {
  if (mainRenderTexture_.id != 0) {
    UnloadRenderTexture(mainRenderTexture_);
  }
  mainRenderTexture_ = LoadRenderTexture(INTERNAL_WIDTH, INTERNAL_HEIGHT);
}

Font *BaseSystemAPI::GetDefaultFontInternal() const {
  return defaultFont_.get();
}
} // namespace core
} // namespace game
