#include "Core/GameContext.h"
#include "Core/TraceCompat.h"

namespace {
constexpr int kVirtualWidth = 1920;
constexpr int kVirtualHeight = 1080;
} // namespace

namespace New::Core {

GameContext::GameContext() : world_(registry_) {}
GameContext::~GameContext() { Shutdown(); }

bool GameContext::Initialize() {
  if (initialized_) {
    return true;
  }

  if (!resourceManager_) {
    resourceManager_ = std::make_unique<ResourceManager>();
  }
  if (!renderer_) {
    renderer_ = std::make_unique<GameRenderer>();
  }
  if (!inputManager_) {
    inputManager_ = std::make_unique<InputManager>();
  }
  if (!systemRunner_) {
    systemRunner_ = std::make_unique<SystemRunner>();
  }
  if (!definitionRegistry_) {
    definitionRegistry_ = std::make_unique<Data::DefinitionRegistry>();
  }

  if (!resourceManager_->Initialize()) {
    return false;
  }

  if (!renderer_->Initialize(kVirtualWidth, kVirtualHeight)) {
    resourceManager_->Shutdown();
    return false;
  }

  // 日本語フォントを事前ロード（失敗時はデフォルトフォントにフォールバック）
  const bool fontLoaded = resourceManager_->LoadDefaultFont(
      "assets/fonts/NotoSansJP-Medium.ttf", 22);
  if (!fontLoaded) {
    TRACELOG(
        LOG_WARNING,
        "GameContext: default font load failed, fallback to raylib default");
  }

  if (!inputManager_->Initialize(renderer_->GetVirtualWidth(),
                                 renderer_->GetVirtualHeight())) {
    renderer_->Shutdown();
    resourceManager_->Shutdown();
    return false;
  }

  initialized_ = true;
  return true;
}

void GameContext::Shutdown() {
  if (!initialized_) {
    return;
  }

  if (systemRunner_) {
    systemRunner_->Shutdown();
  }
  if (inputManager_) {
    inputManager_->Shutdown();
  }
  if (renderer_) {
    renderer_->Shutdown();
  }
  if (resourceManager_) {
    resourceManager_->Shutdown();
  }
  if (definitionRegistry_) {
    definitionRegistry_->Clear();
  }

  initialized_ = false;
}

void GameContext::SetResourceManager(
    std::unique_ptr<IResourceManager> manager) {
  if (initialized_) {
    return;
  }
  resourceManager_ = std::move(manager);
}

void GameContext::SetInputManager(std::unique_ptr<IInputManager> manager) {
  if (initialized_) {
    return;
  }
  inputManager_ = std::move(manager);
}

} // namespace New::Core
