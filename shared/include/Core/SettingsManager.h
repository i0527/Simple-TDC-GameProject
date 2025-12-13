#pragma once

#include <string>

namespace Shared::Core {

struct SettingsData {
  float masterVolume = 1.0f;   // 0.0 - 1.0
  float bgmVolume = 1.0f;      // 0.0 - 1.0
  float sfxVolume = 1.0f;      // 0.0 - 1.0
  bool masterMuted = false;
  bool bgmMuted = false;
  bool sfxMuted = false;
  bool showInputGuide = true;
  float speedMultiplier = 1.0f;          // 1.0 / 2.0 / 4.0
  std::string windowMode = "window";     // "window" / "fullscreen"
  std::string language = "ja";           // 互換用（UI未使用）
  std::string quality = "high";          // 互換用（UI未使用）
};

class SettingsManager {
public:
  SettingsManager() = default;

  bool Load(const std::string &path);
  bool Save(const std::string &path) const;

  SettingsData &Data() { return data_; }
  const SettingsData &Data() const { return data_; }

  // デフォルト値にリセット
  void ResetToDefaults();

private:
  bool Validate(SettingsData &candidate) const;
  SettingsData data_;
};

} // namespace Shared::Core
