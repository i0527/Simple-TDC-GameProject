#pragma once

#include <filesystem>
#include <string>

#include <raylib.h>

#include "Core/SettingsManager.h"

namespace Game::Audio {

// シンプルなBGMサービス。ファイルパス解決、再生、音量反映を担当する。
class BgmService {
public:
  explicit BgmService(Shared::Core::SettingsManager *settings = nullptr);
  ~BgmService();

  BgmService(const BgmService &) = delete;
  BgmService &operator=(const BgmService &) = delete;

  bool Load(const std::string &relative_path);
  void Update(const Shared::Core::SettingsData *override_settings = nullptr);
  void Stop();
  // 設定マネージャの値、または一時的なドラフト（override_settings が非null）
  void SyncSettings(const Shared::Core::SettingsData *override_settings = nullptr);

  bool IsLoaded() const { return loaded_; }
  bool IsMissing() const { return missing_; }
  bool IsManuallyMuted() const { return manual_mute_; }
  void SetManualMute(bool muted,
                     const Shared::Core::SettingsData *override_settings = nullptr);
  std::string CurrentPathUtf8() const { return path_utf8_; }

private:
  std::filesystem::path ResolvePath(const std::string &relative) const;
  void ApplySettings(const Shared::Core::SettingsData *override_settings);

  Shared::Core::SettingsManager *settings_ = nullptr;
  Music music_{};
  bool loaded_ = false;
  bool missing_ = false;
  bool manual_mute_ = false;
  std::string path_utf8_;
};

} // namespace Game::Audio

