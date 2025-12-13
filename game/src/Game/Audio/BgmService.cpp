#include "Game/Audio/BgmService.h"

#include <algorithm>
#include <iostream>

namespace Game::Audio {

namespace {
std::filesystem::path ResolveBase(const std::filesystem::path &base,
                                  const std::filesystem::path &rel) {
  if (base.empty()) {
    return rel;
  }
  return (base / rel).lexically_normal();
}
} // namespace

BgmService::BgmService(Shared::Core::SettingsManager *settings)
    : settings_(settings) {}

BgmService::~BgmService() { Stop(); }

bool BgmService::Load(const std::string &relative_path) {
  Stop();
  missing_ = false;
  path_utf8_.clear();

  const auto resolved = ResolvePath(relative_path);
  path_utf8_ = resolved.u8string();

  std::error_code ec;
  if (!std::filesystem::exists(resolved, ec) || ec) {
    missing_ = true;
    std::cout << "[BgmService] File not found (skip): " << path_utf8_
              << std::endl;
    return false;
  }

  music_ = LoadMusicStream(path_utf8_.c_str());
  if (!music_.ctxData) {
    missing_ = true;
    std::cout << "[BgmService] Load failed (skip): " << path_utf8_ << std::endl;
    return false;
  }

  loaded_ = true;
  PlayMusicStream(music_);
  SyncSettings();
  std::cout << "[BgmService] BGM loaded: " << path_utf8_ << std::endl;
  return true;
}

void BgmService::Update(const Shared::Core::SettingsData *override_settings) {
  if (!loaded_) {
    return;
  }
  UpdateMusicStream(music_);
  ApplySettings(override_settings);
}

void BgmService::Stop() {
  if (!loaded_) {
    return;
  }
  StopMusicStream(music_);
  UnloadMusicStream(music_);
  loaded_ = false;
  manual_mute_ = false;
  path_utf8_.clear();
}

void BgmService::SyncSettings(const Shared::Core::SettingsData *override_settings) {
  ApplySettings(override_settings);
}

void BgmService::SetManualMute(bool muted,
                               const Shared::Core::SettingsData *override_settings) {
  manual_mute_ = muted;
  ApplySettings(override_settings);
}

std::filesystem::path BgmService::ResolvePath(const std::string &relative) const {
  const std::filesystem::path rel(relative);
  const std::filesystem::path bases[] = {std::filesystem::current_path(),
                                         std::filesystem::current_path().parent_path(),
                                         std::filesystem::current_path().parent_path().parent_path()};

  for (const auto &base : bases) {
    if (base.empty()) {
      continue;
    }
    const auto candidate = ResolveBase(base, rel);
    std::error_code ec;
    if (std::filesystem::exists(candidate, ec) && !ec) {
      return candidate;
    }
  }
  return rel;
}

void BgmService::ApplySettings(const Shared::Core::SettingsData *override_settings) {
  if (!loaded_) {
    return;
  }

  float master = 1.0f;
  float bgm = 1.0f;
  bool master_muted = false;
  bool bgm_muted = false;

  if (override_settings) {
    const auto &s = *override_settings;
    master = std::clamp(s.masterVolume, 0.0f, 1.0f);
    bgm = std::clamp(s.bgmVolume, 0.0f, 1.0f);
    master_muted = s.masterMuted;
    bgm_muted = s.bgmMuted;
  } else if (settings_) {
    const auto &s = settings_->Data();
    master = std::clamp(s.masterVolume, 0.0f, 1.0f);
    bgm = std::clamp(s.bgmVolume, 0.0f, 1.0f);
    master_muted = s.masterMuted;
    bgm_muted = s.bgmMuted;
  }

  float master_volume = master_muted ? 0.0f : master;
  float music_volume = bgm_muted ? 0.0f : bgm;

  if (settings_) {
    SetMasterVolume(master_volume);
  }

  float final_volume = manual_mute_ ? 0.0f : (master_volume * music_volume);
  SetMusicVolume(music_, final_volume);
}

} // namespace Game::Audio

