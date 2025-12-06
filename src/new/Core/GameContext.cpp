#include "Core/GameContext.h"

namespace New::Core {

GameContext::GameContext() : world_(registry_) {}
GameContext::~GameContext() { Shutdown(); }

bool GameContext::Initialize() {
  if (initialized_) {
    return true;
  }

  systemRunner_ = std::make_unique<SystemRunner>();
  renderer_ = std::make_unique<GameRenderer>();

  if (!renderer_->Initialize()) {
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
  if (renderer_) {
    renderer_->Shutdown();
  }

  initialized_ = false;
}

} // namespace New::Core
