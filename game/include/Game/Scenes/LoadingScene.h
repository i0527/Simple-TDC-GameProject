#pragma once

#include <functional>
#include <string>

#include <raylib.h>

#include "Game/Scenes/IScene.h"

namespace Game::Scenes {

/// @brief 起動時にリソースをロードし、完了後に遷移させる簡易ローディングシーン
class LoadingScene : public IScene {
public:
  using Task = std::function<bool(std::string &message)>;

  LoadingScene(Font font, int screen_width, int screen_height, Task task);

  void Update(float delta_time) override;
  void Draw() override;

  bool IsDone() const { return done_; }
  bool HasError() const { return error_; }
  const std::string &GetStatusMessage() const { return status_message_; }

private:
  Font font_;
  int screen_width_;
  int screen_height_;
  Task task_;

  bool started_ = false;
  bool done_ = false;
  bool error_ = false;
  std::string status_message_ = "Loading...";

  float spinner_timer_ = 0.0f;
  float spinner_angle_ = 0.0f;
};

} // namespace Game::Scenes
