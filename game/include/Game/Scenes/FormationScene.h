#pragma once

#include <raylib.h>

#include "Game/Scenes/IScene.h"

namespace Game::Scenes {

class FormationScene : public IScene {
public:
  FormationScene(Font font, int screen_width, int screen_height);

  void Update(float delta_time) override;
  void Draw() override;

  bool ConsumeReturnHome();

private:
  Font font_;
  int screen_width_;
  int screen_height_;
  bool return_home_requested_ = false;
};

} // namespace Game::Scenes
