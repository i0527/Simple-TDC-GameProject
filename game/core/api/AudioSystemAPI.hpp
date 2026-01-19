#pragma once

// 標準ライブラリ
#include <string>

// プロジェクト内
#include "../config/RenderTypes.hpp"

namespace game {
namespace core {

class BaseSystemAPI;

/// @brief オーディオAPI
class AudioSystemAPI {
public:
  explicit AudioSystemAPI(BaseSystemAPI* owner);
  ~AudioSystemAPI() = default;

  void UpdateAudio(float deltaTime);
  bool PlaySound(const std::string& name);
  bool PlayMusic(const std::string& name);
  void StopSound();
  void StopSound(const std::string& name);
  void StopMusic();

  bool IsSoundPlaying(const std::string& name) const;
  bool IsMusicPlaying() const;
  std::string GetCurrentMusicName() const;

  bool IsMusicStreamPlaying(Music* music) const;
  void PlayMusicStream(Music* music);
  void StopMusicStream(Music* music);
  void UpdateMusicStream(Music* music);
  void SetMusicVolume(Music* music, float volume);

  void SetMasterVolume(float volume);
  void SetSEVolume(float volume);
  void SetBGMVolume(float volume);
  float GetMasterVolume() const;
  float GetSEVolume() const;
  float GetBGMVolume() const;

private:
  BaseSystemAPI* owner_;

  float ClampVolume(float volume) const;
  void UpdateSoundVolume(Sound* sound) const;
  void UpdateMusicVolume(Music* music) const;
};

} // namespace core
} // namespace game
