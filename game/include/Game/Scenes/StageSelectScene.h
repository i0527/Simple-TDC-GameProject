#pragma once

#include <memory>
#include <string>
#include <vector>

#include <raylib.h>

#include "Data/DefinitionRegistry.h"
#include "Game/Scenes/IScene.h"

namespace Game::Scenes {

class StageSelectScene : public IScene {
public:
  StageSelectScene(Shared::Data::DefinitionRegistry &definitions,
                   const Font &font);

  void Update(float delta_time) override;
  void Draw() override;

  bool ShouldStartGame() const { return start_requested_; }
  bool ConsumeQuitRequest();
  std::string ConsumeSelectedStage();

private:
  struct StageEntry {
    std::string id;
    std::string domain;
    int subdomain = 0;
    std::string name;
  };

  void BuildStageList();
  void HandleInput();

  Shared::Data::DefinitionRegistry &definitions_;
  Font font_;

  std::vector<StageEntry> stages_;
  int selected_index_ = 0;
  bool start_requested_ = false;
  bool quit_requested_ = false;
  std::string selected_stage_id_;
};

} // namespace Game::Scenes
