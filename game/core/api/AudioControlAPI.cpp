#include "AudioControlAPI.hpp"

// 標準ライブラリ
#include <algorithm>

// プロジェクト内
#include "BaseSystemAPI.hpp"
#include "../../utils/Log.h"

namespace game {
namespace core {

namespace {
constexpr float kDefaultCrossfadeSeconds = 0.5f;
}

AudioControlAPI::AudioControlAPI()
    : systemAPI_(nullptr),
      isInitialized_(false),
      masterVolume_(1.0f),
      seVolume_(1.0f),
      bgmVolume_(1.0f),
      crossfadeDuration_(kDefaultCrossfadeSeconds),
      crossfadeTimer_(0.0f),
      crossfadeActive_(false),
      currentMusic_(nullptr),
      currentMusicName_(""),
      nextMusic_(nullptr),
      nextMusicName_("") {}

bool AudioControlAPI::Initialize(BaseSystemAPI* systemAPI) {
    if (!systemAPI) {
        LOG_ERROR("AudioControlAPI: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    masterVolume_ = systemAPI_->Audio().GetMasterVolume();
    seVolume_ = systemAPI_->Audio().GetSEVolume();
    bgmVolume_ = systemAPI_->Audio().GetBGMVolume();
    crossfadeDuration_ = kDefaultCrossfadeSeconds;
    crossfadeTimer_ = 0.0f;
    crossfadeActive_ = false;
    currentMusic_ = nullptr;
    currentMusicName_.clear();
    nextMusic_ = nullptr;
    nextMusicName_.clear();

    isInitialized_ = true;
    LOG_INFO("AudioControlAPI initialized");
    return true;
}

void AudioControlAPI::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    StopBGM();
    systemAPI_ = nullptr;
    isInitialized_ = false;
}

void AudioControlAPI::Update(float deltaTime) {
    if (!isInitialized_ || !systemAPI_) {
        return;
    }

    systemAPI_->Audio().UpdateAudio(deltaTime);

    if (currentMusic_ && !systemAPI_->Audio().IsMusicStreamPlaying(currentMusic_)) {
        currentMusic_ = nullptr;
        currentMusicName_.clear();
    }

    if (crossfadeActive_) {
        crossfadeTimer_ += deltaTime;
        const float progress = GetCrossfadeProgress();
        ApplyBGMVolumes(1.0f - progress, progress);

        UpdateMusicStreamSafe(currentMusic_);
        UpdateMusicStreamSafe(nextMusic_);

        if (progress >= 1.0f) {
            StopMusicStreamSafe(currentMusic_);
            currentMusic_ = nextMusic_;
            currentMusicName_ = nextMusicName_;
            nextMusic_ = nullptr;
            nextMusicName_.clear();
            crossfadeActive_ = false;
            crossfadeTimer_ = 0.0f;
        }
        return;
    }

    if (currentMusic_) {
        ApplyBGMVolumes(1.0f, 0.0f);
        UpdateMusicStreamSafe(currentMusic_);
    }
}

bool AudioControlAPI::PlayBGM(const std::string& name) {
    if (!isInitialized_ || !systemAPI_) {
        LOG_ERROR("AudioControlAPI: Not initialized");
        return false;
    }

    if (name.empty()) {
        StopBGM();
        return true;
    }

    if (crossfadeActive_ && nextMusicName_ == name) {
        return true;
    }

    if (currentMusic_ && currentMusicName_ == name &&
        systemAPI_->Audio().IsMusicStreamPlaying(currentMusic_)) {
        return true;
    }

    void* musicPtr = systemAPI_->Resource().GetMusic(name);
    if (!musicPtr) {
        LOG_ERROR("AudioControlAPI: Failed to get music: {}", name);
        return false;
    }

    ::Music* music = static_cast<::Music*>(musicPtr);

    if (!currentMusic_ || !systemAPI_->Audio().IsMusicStreamPlaying(currentMusic_)) {
        StopMusicStreamSafe(currentMusic_);
        currentMusic_ = music;
        currentMusicName_ = name;
        nextMusic_ = nullptr;
        nextMusicName_.clear();
        crossfadeActive_ = false;
        crossfadeTimer_ = 0.0f;

        systemAPI_->Audio().PlayMusicStream(currentMusic_);
        ApplyBGMVolumes(1.0f, 0.0f);
        LOG_INFO("AudioControlAPI: Playing BGM: {}", name);
        return true;
    }

    StopMusicStreamSafe(nextMusic_);
    nextMusic_ = music;
    nextMusicName_ = name;
    crossfadeActive_ = true;
    crossfadeTimer_ = 0.0f;

    systemAPI_->Audio().PlayMusicStream(nextMusic_);
    systemAPI_->Audio().SetMusicVolume(nextMusic_, 0.0f);
    LOG_INFO("AudioControlAPI: Crossfade to BGM: {}", name);
    return true;
}

void AudioControlAPI::StopBGM() {
    if (!isInitialized_) {
        return;
    }

    StopMusicStreamSafe(currentMusic_);
    StopMusicStreamSafe(nextMusic_);

    currentMusic_ = nullptr;
    currentMusicName_.clear();
    nextMusic_ = nullptr;
    nextMusicName_.clear();
    crossfadeActive_ = false;
    crossfadeTimer_ = 0.0f;
}

bool AudioControlAPI::PlaySE(const std::string& name) {
    if (!isInitialized_ || !systemAPI_) {
        LOG_ERROR("AudioControlAPI: Not initialized");
        return false;
    }
    return systemAPI_->Audio().PlaySound(name);
}

void AudioControlAPI::SetMasterVolume(float volume) {
    masterVolume_ = ClampVolume(volume);
    if (systemAPI_) {
        systemAPI_->Audio().SetMasterVolume(masterVolume_);
    }

    if (crossfadeActive_) {
        const float progress = GetCrossfadeProgress();
        ApplyBGMVolumes(1.0f - progress, progress);
    } else {
        ApplyBGMVolumes(1.0f, 0.0f);
    }
}

void AudioControlAPI::SetSEVolume(float volume) {
    seVolume_ = ClampVolume(volume);
    if (systemAPI_) {
        systemAPI_->Audio().SetSEVolume(seVolume_);
    }
}

void AudioControlAPI::SetBGMVolume(float volume) {
    bgmVolume_ = ClampVolume(volume);
    if (systemAPI_) {
        systemAPI_->Audio().SetBGMVolume(bgmVolume_);
    }

    if (crossfadeActive_) {
        const float progress = GetCrossfadeProgress();
        ApplyBGMVolumes(1.0f - progress, progress);
    } else {
        ApplyBGMVolumes(1.0f, 0.0f);
    }
}

float AudioControlAPI::GetMasterVolume() const {
    return masterVolume_;
}

float AudioControlAPI::GetSEVolume() const {
    return seVolume_;
}

float AudioControlAPI::GetBGMVolume() const {
    return bgmVolume_;
}

std::string AudioControlAPI::GetCurrentBGMName() const {
    return currentMusicName_;
}

float AudioControlAPI::ClampVolume(float volume) const {
    return std::max(0.0f, std::min(1.0f, volume));
}

float AudioControlAPI::GetBaseBGMVolume() const {
    return masterVolume_ * bgmVolume_;
}

float AudioControlAPI::GetCrossfadeProgress() const {
    if (!crossfadeActive_) {
        return 0.0f;
    }
    if (crossfadeDuration_ <= 0.0f) {
        return 1.0f;
    }
    return std::min(1.0f, crossfadeTimer_ / crossfadeDuration_);
}

void AudioControlAPI::ApplyBGMVolumes(float currentWeight, float nextWeight) {
    const float baseVolume = GetBaseBGMVolume();
    if (currentMusic_) {
        systemAPI_->Audio().SetMusicVolume(currentMusic_,
                                           baseVolume * currentWeight);
    }
    if (nextMusic_) {
        systemAPI_->Audio().SetMusicVolume(nextMusic_,
                                           baseVolume * nextWeight);
    }
}

void AudioControlAPI::UpdateMusicStreamSafe(::Music* music) const {
    if (!music) {
        return;
    }
    if (systemAPI_->Audio().IsMusicStreamPlaying(music)) {
        systemAPI_->Audio().UpdateMusicStream(music);
    }
}

void AudioControlAPI::StopMusicStreamSafe(::Music* music) const {
    if (!music) {
        return;
    }
    if (systemAPI_->Audio().IsMusicStreamPlaying(music)) {
        systemAPI_->Audio().StopMusicStream(music);
    }
}

} // namespace core
} // namespace game
