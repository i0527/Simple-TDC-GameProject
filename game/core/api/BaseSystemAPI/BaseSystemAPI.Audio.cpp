#include "../AudioSystemAPI.hpp"
#include "../BaseSystemAPI.hpp"

// Project
#include "../../../utils/Log.h"

namespace game {
namespace core {

AudioSystemAPI::AudioSystemAPI(BaseSystemAPI* owner) : owner_(owner) {}

void AudioSystemAPI::UpdateAudio(float deltaTime) {
  if (!owner_->isInitialized_) {
    return;
  }

  if (owner_->currentMusic_ != nullptr) {
    try {
      if (::IsMusicStreamPlaying(*owner_->currentMusic_)) {
        ::UpdateMusicStream(*owner_->currentMusic_);
      } else {
        owner_->currentMusic_ = nullptr;
        owner_->currentMusicName_.clear();
      }
    } catch (...) {
      LOG_WARN("AudioSystemAPI: Music became invalid during update");
      owner_->currentMusic_ = nullptr;
      owner_->currentMusicName_.clear();
    }
  }

  auto it = owner_->playingSounds_.begin();
  while (it != owner_->playingSounds_.end()) {
    Sound *sound = it->second;
    if (sound) {
      try {
        if (!::IsSoundPlaying(*sound)) {
          it = owner_->playingSounds_.erase(it);
          continue;
        }
      } catch (...) {
        LOG_WARN("AudioSystemAPI: Sound became invalid during update");
        it = owner_->playingSounds_.erase(it);
        continue;
      }
    }
    ++it;
  }
}

bool AudioSystemAPI::PlaySound(const std::string &name) {
  if (!owner_->isInitialized_) {
    LOG_ERROR("AudioSystemAPI: Not initialized");
    return false;
  }

  void *soundPtr = owner_->Resource().GetSound(name);
  if (!soundPtr) {
    LOG_ERROR("AudioSystemAPI: Failed to get sound: {}", name);
    return false;
  }

  Sound *sound = static_cast<Sound *>(soundPtr);

  ::PlaySound(*sound);

  UpdateSoundVolume(sound);

  owner_->playingSounds_[name] = sound;

  LOG_DEBUG("AudioSystemAPI: Playing sound: {}", name);
  return true;
}

bool AudioSystemAPI::PlayMusic(const std::string &name) {
  if (!owner_->isInitialized_) {
    LOG_ERROR("AudioSystemAPI: Not initialized");
    return false;
  }

  if (owner_->currentMusic_ != nullptr && owner_->currentMusicName_ == name &&
      ::IsMusicStreamPlaying(*owner_->currentMusic_)) {
    LOG_DEBUG("AudioSystemAPI: Music already playing: {}", name);
    return true;
  }

  if (owner_->currentMusic_ != nullptr) {
    ::StopMusicStream(*owner_->currentMusic_);
  }

  void *musicPtr = owner_->Resource().GetMusic(name);
  if (!musicPtr) {
    LOG_ERROR("AudioSystemAPI: Failed to get music: {}", name);
    return false;
  }

  Music *music = static_cast<Music *>(musicPtr);

  ::PlayMusicStream(*music);

  UpdateMusicVolume(music);

  owner_->currentMusic_ = music;
  owner_->currentMusicName_ = name;

  LOG_INFO("AudioSystemAPI: Playing music: {}", name);
  return true;
}

void AudioSystemAPI::StopSound() {
  if (!owner_->isInitialized_) {
    return;
  }

  for (auto &pair : owner_->playingSounds_) {
    Sound *sound = pair.second;
    if (sound) {
      ::StopSound(*sound);
    }
  }

  owner_->playingSounds_.clear();
  LOG_DEBUG("AudioSystemAPI: Stopped all sounds");
}

void AudioSystemAPI::StopSound(const std::string &name) {
  if (!owner_->isInitialized_) {
    return;
  }

  auto it = owner_->playingSounds_.find(name);
  if (it != owner_->playingSounds_.end()) {
    Sound *sound = it->second;
    if (sound) {
      ::StopSound(*sound);
    }
    owner_->playingSounds_.erase(it);
    LOG_DEBUG("AudioSystemAPI: Stopped sound: {}", name);
  }
}

void AudioSystemAPI::StopMusic() {
  if (!owner_->isInitialized_ || !owner_->currentMusic_) {
    return;
  }

  ::StopMusicStream(*owner_->currentMusic_);
  owner_->currentMusic_ = nullptr;
  owner_->currentMusicName_.clear();
  LOG_DEBUG("AudioSystemAPI: Stopped music");
}

bool AudioSystemAPI::IsSoundPlaying(const std::string &name) const {
  if (!owner_->isInitialized_) {
    return false;
  }

  auto it = owner_->playingSounds_.find(name);
  if (it != owner_->playingSounds_.end()) {
    Sound *sound = it->second;
    if (sound) {
      try {
        return ::IsSoundPlaying(*sound);
      } catch (...) {
        return false;
      }
    }
  }

  return false;
}

bool AudioSystemAPI::IsMusicPlaying() const {
  if (!owner_->isInitialized_ || !owner_->currentMusic_) {
    return false;
  }

  try {
    return ::IsMusicStreamPlaying(*owner_->currentMusic_);
  } catch (...) {
    return false;
  }
}

bool AudioSystemAPI::IsMusicStreamPlaying(Music *music) const {
  if (!owner_->isInitialized_ || !music) {
    return false;
  }

  try {
    return ::IsMusicStreamPlaying(*music);
  } catch (...) {
    return false;
  }
}

void AudioSystemAPI::PlayMusicStream(Music *music) {
  if (!owner_->isInitialized_ || !music) {
    return;
  }

  ::PlayMusicStream(*music);
}

void AudioSystemAPI::StopMusicStream(Music *music) {
  if (!owner_->isInitialized_ || !music) {
    return;
  }

  if (::IsMusicStreamPlaying(*music)) {
    ::StopMusicStream(*music);
  }
}

void AudioSystemAPI::UpdateMusicStream(Music *music) {
  if (!owner_->isInitialized_ || !music) {
    return;
  }

  if (::IsMusicStreamPlaying(*music)) {
    ::UpdateMusicStream(*music);
  }
}

void AudioSystemAPI::SetMusicVolume(Music *music, float volume) {
  if (!owner_->isInitialized_ || !music) {
    return;
  }

  ::SetMusicVolume(*music, ClampVolume(volume));
}

std::string AudioSystemAPI::GetCurrentMusicName() const {
  if (!owner_->isInitialized_ || !owner_->currentMusic_) {
    return "";
  }

  return owner_->currentMusicName_;
}

void AudioSystemAPI::SetMasterVolume(float volume) {
  owner_->masterVolume_ = ClampVolume(volume);

  ::SetMasterVolume(owner_->masterVolume_);

  for (auto &pair : owner_->playingSounds_) {
    if (pair.second) {
      UpdateSoundVolume(pair.second);
    }
  }
  if (owner_->currentMusic_) {
    UpdateMusicVolume(owner_->currentMusic_);
  }

  LOG_DEBUG("AudioSystemAPI: Master volume set to {:.2f}",
            owner_->masterVolume_);
}

void AudioSystemAPI::SetSEVolume(float volume) {
  owner_->seVolume_ = ClampVolume(volume);

  for (auto &pair : owner_->playingSounds_) {
    if (pair.second) {
      UpdateSoundVolume(pair.second);
    }
  }

  LOG_DEBUG("AudioSystemAPI: SE volume set to {:.2f}", owner_->seVolume_);
}

void AudioSystemAPI::SetBGMVolume(float volume) {
  owner_->bgmVolume_ = ClampVolume(volume);

  if (owner_->currentMusic_) {
    UpdateMusicVolume(owner_->currentMusic_);
  }

  LOG_DEBUG("AudioSystemAPI: BGM volume set to {:.2f}", owner_->bgmVolume_);
}

float AudioSystemAPI::GetMasterVolume() const { return owner_->masterVolume_; }

float AudioSystemAPI::GetSEVolume() const { return owner_->seVolume_; }

float AudioSystemAPI::GetBGMVolume() const { return owner_->bgmVolume_; }

float AudioSystemAPI::ClampVolume(float volume) const {
  return std::max(0.0f, std::min(1.0f, volume));
}

void AudioSystemAPI::UpdateSoundVolume(Sound *sound) const {
  if (!sound) {
    return;
  }

  float finalVolume = owner_->masterVolume_ * owner_->seVolume_;
  ::SetSoundVolume(*sound, finalVolume);
}

void AudioSystemAPI::UpdateMusicVolume(Music *music) const {
  if (!music) {
    return;
  }

  float finalVolume = owner_->masterVolume_ * owner_->bgmVolume_;
  ::SetMusicVolume(*music, finalVolume);
}

} // namespace core
} // namespace game

