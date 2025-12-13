#pragma once

#include <filesystem>
#include <raylib.h>
#include <string>
#include <vector>

#include "Core/SettingsManager.h"
#include "Data/UserDataManager.h"
#include "Game/Audio/BgmService.h"
#include "Game/Scenes/IScene.h"
#include "Game/UI/SettingsPanel.h"

namespace Game::Scenes {

class TitleScene : public IScene {
public:
  enum class MenuAction { None, NewGame, ContinueGame, Settings, Exit };

  TitleScene(Font font, int screen_width, int screen_height,
             Shared::Core::SettingsManager *settings = nullptr,
             Shared::Data::UserDataManager *user_data = nullptr,
             Game::Audio::BgmService *bgm = nullptr);
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
  int ConsumeRequestedSaveSlot();
  int ConsumeRequestedLoadSlot();
  void SetInfoMessage(const std::string &msg, float duration);

private:
  struct MenuItem {
    std::string label;
    MenuAction action = MenuAction::None;
    bool enabled = true;
  };

  void TriggerAction(MenuAction action);
  void ToggleMute();
  void DrawLoadPanel();
  void RefreshSlots();
  void EnsureSelectable();
  bool HasAnySave() const;

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

  Game::Audio::BgmService *bgm_service_ = nullptr;
  bool music_muted_ = false;

  float info_message_timer_ = 0.0f;
  std::string info_message_;
  Rectangle bgm_toggle_rect_{};

  Game::UI::SettingsPanel settings_panel_;
  Shared::Core::SettingsManager *settings_manager_ = nullptr;
  std::string settings_path_ = "saves/settings.json";
  Shared::Data::UserDataManager *user_data_manager_ = nullptr;

  struct SlotMeta {
    int slot = -1;
    bool exists = false;
    std::string saved_at;
    std::string stage;
    int gold = 0;
  };
  std::vector<SlotMeta> slot_meta_;
  bool show_load_menu_ = false;
  int requested_load_slot_ = -1;
  bool continue_available_ = false;
};

} // namespace Game::Scenes
