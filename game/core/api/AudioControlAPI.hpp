#pragma once

// 標準ライブラリ
#include <string>

// プロジェクト内
#include "../config/RenderTypes.hpp"

namespace game {
namespace core {

class BaseSystemAPI;

/// @brief サウンド制御API（BGM/SE/フェード統一）
class AudioControlAPI {
public:
    AudioControlAPI();
    ~AudioControlAPI() = default;

    bool Initialize(BaseSystemAPI* systemAPI);
    void Shutdown();
    void Update(float deltaTime);

    // ===== BGM =====
    bool PlayBGM(const std::string& name);
    void StopBGM();

    // ===== SE =====
    bool PlaySE(const std::string& name);

    // ===== Volume =====
    void SetMasterVolume(float volume);
    void SetSEVolume(float volume);
    void SetBGMVolume(float volume);
    float GetMasterVolume() const;
    float GetSEVolume() const;
    float GetBGMVolume() const;

    std::string GetCurrentBGMName() const;

private:
    float ClampVolume(float volume) const;
    float GetBaseBGMVolume() const;
    float GetCrossfadeProgress() const;
    void ApplyBGMVolumes(float currentWeight, float nextWeight);
    void UpdateMusicStreamSafe(Music* music) const;
    void StopMusicStreamSafe(Music* music) const;

    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
    float masterVolume_;
    float seVolume_;
    float bgmVolume_;

    float crossfadeDuration_;
    float crossfadeTimer_;
    bool crossfadeActive_;

    ::Music* currentMusic_;
    std::string currentMusicName_;
    ::Music* nextMusic_;
    std::string nextMusicName_;
};

} // namespace core
} // namespace game
