#pragma once

#include <string>
#include <vector>

#include <raylib.h>

#include "Game/Scenes/IScene.h"

namespace Game::Scenes {

class HomeScene : public IScene {
public:
  enum class Action {
    None,
    StageSelect,
    Formation,
    SaveAndTitle,
    SaveAndExit,
    QuitWithoutSave
  };

  HomeScene(Font font, int screen_width, int screen_height);

  void Update(float delta_time) override;
  void Draw() override;

  Action ConsumeAction();

private:
  struct MenuItem {
    std::string label;
    Action action = Action::None;
  };

  void HandleInput();
  void Trigger(Action action);

  Font font_;
  int screen_width_;
  int screen_height_;
  std::vector<MenuItem> menu_items_;
  int selected_index_ = 0;
  Action pending_action_ = Action::None;
};

} // namespace Game::Scenes
