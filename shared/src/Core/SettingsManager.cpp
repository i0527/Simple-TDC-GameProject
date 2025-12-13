#include "Core/SettingsManager.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

namespace Shared::Core {

namespace {

float Clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

bool IsValidWindowMode(const std::string &m) {
  return m == "window" || m == "fullscreen";
}

bool IsValidQuality(const std::string &q) {
  return q == "low" || q == "medium" || q == "high";
}

} // namespace

bool SettingsManager::Validate(SettingsData &candidate) const {
  candidate.masterVolume = Clamp01(candidate.masterVolume);
  candidate.bgmVolume = Clamp01(candidate.bgmVolume);
  candidate.sfxVolume = Clamp01(candidate.sfxVolume);
  if (!IsValidWindowMode(candidate.windowMode)) {
    candidate.windowMode = "window";
  }
  if (candidate.language.empty()) {
    candidate.language = "ja";
  }
  if (!IsValidQuality(candidate.quality)) {
    candidate.quality = "high";
  }

  // 制限された速度倍率にスナップ
  if (candidate.speedMultiplier <= 1.1f && candidate.speedMultiplier >= 0.9f) {
    candidate.speedMultiplier = 1.0f;
  } else if (candidate.speedMultiplier <= 2.1f &&
             candidate.speedMultiplier >= 1.9f) {
    candidate.speedMultiplier = 2.0f;
  } else if (candidate.speedMultiplier <= 4.1f &&
             candidate.speedMultiplier >= 3.9f) {
    candidate.speedMultiplier = 4.0f;
  } else {
    candidate.speedMultiplier = 1.0f;
  }
  return true;
}

void SettingsManager::ResetToDefaults() { data_ = SettingsData{}; }

bool SettingsManager::Load(const std::string &path) {
  try {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
      // ファイル無し: デフォルトを保持
      return false;
    }
    nlohmann::json j = nlohmann::json::parse(ifs);

    SettingsData tmp;
    tmp.masterVolume = j.value("masterVolume", data_.masterVolume);
    tmp.bgmVolume = j.value("bgmVolume", data_.bgmVolume);
    tmp.sfxVolume = j.value("sfxVolume", data_.sfxVolume);
    tmp.masterMuted = j.value("masterMuted", data_.masterMuted);
    tmp.bgmMuted = j.value("bgmMuted", data_.bgmMuted);
    tmp.sfxMuted = j.value("sfxMuted", data_.sfxMuted);
    tmp.showInputGuide = j.value("showInputGuide", data_.showInputGuide);
    tmp.speedMultiplier = j.value("speedMultiplier", data_.speedMultiplier);
    tmp.language = j.value("language", data_.language);
    tmp.quality = j.value("quality", data_.quality);
    tmp.windowMode = j.value("windowMode", data_.windowMode);

    Validate(tmp);
    data_ = tmp;
    return true;
  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "[SettingsManager] JSON parse error: " << e.what()
              << std::endl;
    return false;
  } catch (const std::exception &e) {
    std::cerr << "[SettingsManager] Load error: " << e.what() << std::endl;
    return false;
  }
}

bool SettingsManager::Save(const std::string &path) const {
  try {
    nlohmann::json j;
    j["masterVolume"] = data_.masterVolume;
    j["bgmVolume"] = data_.bgmVolume;
    j["sfxVolume"] = data_.sfxVolume;
    j["masterMuted"] = data_.masterMuted;
    j["bgmMuted"] = data_.bgmMuted;
    j["sfxMuted"] = data_.sfxMuted;
    j["showInputGuide"] = data_.showInputGuide;
    j["speedMultiplier"] = data_.speedMultiplier;
    j["language"] = data_.language;
    j["quality"] = data_.quality;
    j["windowMode"] = data_.windowMode;

    std::ofstream ofs(path);
    if (!ofs.is_open()) {
      std::cerr << "[SettingsManager] Failed to open file for save: " << path
                << std::endl;
      return false;
    }
    ofs << j.dump(2);
    return true;
  } catch (const std::exception &e) {
    std::cerr << "[SettingsManager] Save error: " << e.what() << std::endl;
    return false;
  }
}

} // namespace Shared::Core
