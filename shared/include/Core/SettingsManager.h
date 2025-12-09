#pragma once

#include <string>

namespace Shared::Core {

struct SettingsData {
  float masterVolume = 1.0f; // 0.0 - 1.0
  bool bgmMuted = false;
  bool sfxMuted = false;
  std::string language = "ja";
  std::string quality = "high";
  std::string windowMode = "window"; // "window" / "fullscreen"
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
