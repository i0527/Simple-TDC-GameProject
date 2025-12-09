#pragma once

#include <filesystem>
#include <raylib.h>
#include <string>
#include <vector>

#include "Game/Scenes/IScene.h"

namespace Game::Scenes {

class TitleScene : public IScene {
public:
  enum class MenuAction { None, NewGame, ContinueGame, Settings, Exit };

  TitleScene(Font font, int screen_width, int screen_height);
  ~TitleScene() override;
  void Update(float delta_time) override;
  void Draw() override;

  bool ShouldStart() const { return start_requested_; }
  bool ShouldExit() const { return exit_requested_; }
  bool ConsumeStart() {
    bool v = start_requested_;
    start_requested_ = false;
    return v;
  }
  MenuAction ConsumeAction() {
    MenuAction action = pending_action_;
    pending_action_ = MenuAction::None;
    return action;
  }
  bool ConsumeExit() {
    bool v = exit_requested_;
    exit_requested_ = false;
    return v;
  }

private:
  struct MenuItem {
    std::string label;
    MenuAction action = MenuAction::None;
  };

  void TriggerAction(MenuAction action);
  void ToggleMute();

  Font font_;
  int screen_width_;
  int screen_height_;
  float blink_timer_ = 0.0f;
  bool show_prompt_ = true;
  bool start_requested_ = false;
  bool exit_requested_ = false;
  MenuAction pending_action_ = MenuAction::None;

  std::vector<MenuItem> menu_items_;
  int selected_index_ = 0;

  Music music_{};
  bool music_loaded_ = false;
  bool music_missing_ = false;
  bool music_muted_ = false;

  float info_message_timer_ = 0.0f;
  std::string info_message_;
  Rectangle bgm_toggle_rect_{};
};

} // namespace Game::Scenes
