#pragma once

#include <string>
#include <vector>

#include <raylib.h>

#include "Game/Audio/BgmService.h"
#include "Data/UserDataManager.h"
#include "Game/Scenes/IScene.h"
#include "Game/UI/SettingsPanel.h"
#include "Core/SettingsManager.h"

namespace Game::Scenes {

class HomeScene : public IScene {
public:
  enum class Action {
    None,
    StageSelect,
    Formation,
    Settings,
    SaveMenu,
    LoadMenu,
    SaveAndTitle,
    SaveAndExit,
    QuitWithoutSave
  };

  HomeScene(Font font, int screen_width, int screen_height,
            Shared::Data::UserDataManager *user_data = nullptr,
            Shared::Core::SettingsManager *settings = nullptr,
            Game::Audio::BgmService *bgm = nullptr);
  ~HomeScene() override;

  void Update(float delta_time) override;
  void Draw() override;

  Action ConsumeAction();
  int ConsumeRequestedSaveSlot();
  int ConsumeRequestedLoadSlot();
  void SetInfoMessage(const std::string &msg);

private:
  struct MenuItem {
    std::string label;
    Action action = Action::None;
  };

  void HandleInput();
  void Trigger(Action action);
  void RefreshSlots();
  void DrawSaveLoadPanel(bool saving);
  // 設定パネルを更新し、入力を消費した場合は true を返す
  bool UpdateSettingsPanel(float delta_time);
  void DrawSettingsPanel() const;
  void UpdateMusic();

  Font font_;
  int screen_width_;
  int screen_height_;
  std::vector<MenuItem> menu_items_;
  int selected_index_ = 0;
  Action pending_action_ = Action::None;
  Shared::Data::UserDataManager *user_data_manager_ = nullptr;
  Shared::Core::SettingsManager *settings_manager_ = nullptr;
  Game::UI::SettingsPanel settings_panel_;
  std::string settings_path_ = "saves/settings.json";
  float settings_anim_timer_ = 0.0f;
  std::string info_message_;

  struct SlotMeta {
    int slot = -1;
    bool exists = false;
    std::string saved_at;
    std::string stage;
    int gold = 0;
  };
  std::vector<SlotMeta> slot_meta_;
  bool show_save_menu_ = false;
  bool show_load_menu_ = false;
  int requested_save_slot_ = -1;
  int requested_load_slot_ = -1;
  Game::Audio::BgmService *bgm_service_ = nullptr;
};

} // namespace Game::Scenes
