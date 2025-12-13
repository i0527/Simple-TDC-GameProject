#pragma once

#include <raylib.h>
#include <string>
#include <optional>

#include "Core/SettingsManager.h"
#include "Game/UI/UiComponents.h"

namespace Game::UI {

class SettingsPanel {
public:
  enum class Action { None, Apply, Cancel };

  void Open(const Shared::Core::SettingsData &initial);
  void OpenDefault();
  void Close();
  bool IsOpen() const { return open_; }
  bool HasUnsavedChanges() const;

  // 入力処理（クリック等）。戻り値で適用/キャンセルを通知。
  Action Update(const Rectangle &panel_bounds,
                std::optional<Vector2> mouse_override = std::nullopt);
  // 描画のみ（状態は Update 済みの draft_ を使用）
  void Draw(const Rectangle &panel_bounds, const Font &font) const;

  const Shared::Core::SettingsData &Draft() const { return draft_; }
  Shared::Core::SettingsData &Draft() { return draft_; }

private:
  struct NumericFieldState {
    std::string text;
    bool active = false;
    bool just_activated = false;
    int caret = 0;
  };

  enum class ActiveSlider { None, Master, Bgm, Sfx };

  void SyncNumericFields();

  bool open_ = false;
  bool confirming_apply_ = false;
  Shared::Core::SettingsData draft_{};
  Shared::Core::SettingsData original_{};
  NumericFieldState master_field_;
  NumericFieldState bgm_field_;
  NumericFieldState sfx_field_;
  ActiveSlider active_slider_ = ActiveSlider::None;

  SliderComponent master_slider_;
  SliderComponent bgm_slider_;
  SliderComponent sfx_slider_;
  ButtonComponent apply_btn_;
  ButtonComponent cancel_btn_;
  ButtonComponent default_btn_;
  ButtonComponent speed_btns_[3];
  ToggleComponent master_mute_toggle_;
  ToggleComponent bgm_mute_toggle_;
  ToggleComponent sfx_mute_toggle_;
  ToggleComponent window_toggle_;
  ToggleComponent input_guide_toggle_;
};

} // namespace Game::UI
