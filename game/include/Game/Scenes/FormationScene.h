#pragma once

#include <string>
#include <vector>

#include <raylib.h>

#include "Data/DefinitionRegistry.h"
#include "Game/Managers/FormationManager.h"
#include "Game/Scenes/IScene.h"

namespace Game::Scenes {

class FormationScene : public IScene {
public:
  FormationScene(Font font, int screen_width, int screen_height,
                 Shared::Data::DefinitionRegistry &definitions,
                 Game::Managers::FormationManager &formation_manager);

  void Update(float delta_time) override;
  void Draw() override;

  bool ConsumeReturnHome();

private:
  Font font_;
  int screen_width_;
  int screen_height_;
  Shared::Data::DefinitionRegistry &definitions_;
  Game::Managers::FormationManager &formation_manager_;

  std::vector<std::string> candidates_;
  std::vector<std::string> slots_;
  int selected_candidate_ = 0;
  int selected_slot_ = 0;
  bool apply_requested_ = false;
  bool return_home_requested_ = false;

  void RefreshData();
  void HandleInput();
  void DrawCandidates(const Rectangle &panel) const;
  void DrawSlots(const Rectangle &panel) const;
  void AssignSelectedToSlot(int slot_index);
};

} // namespace Game::Scenes
