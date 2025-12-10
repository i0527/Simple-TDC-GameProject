#pragma once

#include <raylib.h>
#include <string>

#include "Core/SettingsManager.h"

namespace Game::UI {

class SettingsPanel {
public:
  enum class Action { None, Apply, Cancel };

  void Open(const Shared::Core::SettingsData &initial);
  void OpenDefault();
  void Close();
  bool IsOpen() const { return open_; }

  // 入力処理（クリック等）。戻り値で適用/キャンセルを通知。
  Action Update(const Rectangle &panel_bounds);
  // 描画のみ（状態は Update 済みの draft_ を使用）
  void Draw(const Rectangle &panel_bounds, const Font &font) const;

  const Shared::Core::SettingsData &Draft() const { return draft_; }
  Shared::Core::SettingsData &Draft() { return draft_; }

private:
  bool open_ = false;
  Shared::Core::SettingsData draft_{};
};

} // namespace Game::UI
