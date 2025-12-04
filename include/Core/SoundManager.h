/**
 * @file SoundManager.h
 * @brief サウンドマネージャー
 * 
 * Phase 5A: サウンドシステム実装
 * - SoundDef/MusicDefに基づくサウンド再生
 * - グループ管理・音量制御
 * - 3Dサウンド対応
 */
#pragma once

#include "Core/Platform.h"
#include "Data/SoundDef.h"
#include "Data/Registry.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <random>
#include <functional>

namespace Core {

/**
 * @brief サウンドインスタンス情報
 */
struct SoundInstance {
    Sound sound;                    // Raylibサウンド
    std::string soundId;            // 定義ID
    float volume = 1.0f;            // 現在音量
    float pitch = 1.0f;             // 現在ピッチ
    float startTime = 0.0f;         // 再生開始時刻
    bool playing = false;           // 再生中フラグ
    bool looping = false;           // ループ中フラグ
    
    // 3D音源
    bool is3D = false;
    float x = 0.0f, y = 0.0f;       // 位置
    float minDistance = 1.0f;
    float maxDistance = 100.0f;
    
    // フェード
    float fadeVolume = 1.0f;        // フェード音量
    float fadeTarget = 1.0f;        // フェード目標
    float fadeSpeed = 0.0f;         // フェード速度
};

/**
 * @brief ミュージックインスタンス情報
 */
struct MusicInstance {
    Music music;                    // Raylibミュージック
    std::string musicId;            // 定義ID
    float volume = 1.0f;
    bool playing = false;
    bool looping = true;
    
    // クロスフェード
    float fadeVolume = 1.0f;
    float fadeTarget = 1.0f;
    float fadeSpeed = 0.0f;
};

/**
 * @brief サウンドグループ
 */
struct SoundGroup {
    std::string id;
    float volume = 1.0f;
    bool muted = false;
    int maxInstances = 16;
    std::vector<size_t> activeInstances;  // アクティブなインスタンスインデックス
};

/**
 * @brief サウンドマネージャー
 */
class SoundManager {
public:
    SoundManager() : rng_(std::random_device{}()) {}
    
    ~SoundManager() {
        Shutdown();
    }
    
    /**
     * @brief 初期化
     */
    bool Initialize() {
        InitAudioDevice();
        
        if (!IsAudioDeviceReady()) {
            std::cerr << "SoundManager: Failed to initialize audio device" << std::endl;
            return false;
        }
        
        // デフォルトグループ作成
        CreateGroup("master", 1.0f);
        CreateGroup("sfx", 1.0f);
        CreateGroup("music", 0.8f);
        CreateGroup("voice", 1.0f);
        CreateGroup("ui", 1.0f);
        CreateGroup("ambient", 0.7f);
        
        initialized_ = true;
        std::cout << "SoundManager: Initialized" << std::endl;
        return true;
    }
    
    /**
     * @brief シャットダウン
     */
    void Shutdown() {
        if (!initialized_) return;
        
        // 全サウンド停止・解放
        for (auto& inst : soundInstances_) {
            if (IsSoundReady(inst.sound)) {
                StopSound(inst.sound);
                UnloadSound(inst.sound);
            }
        }
        soundInstances_.clear();
        
        // 全ミュージック停止・解放
        for (auto& [id, inst] : musicInstances_) {
            if (IsMusicReady(inst.music)) {
                StopMusicStream(inst.music);
                UnloadMusicStream(inst.music);
            }
        }
        musicInstances_.clear();
        
        // サウンドキャッシュ解放
        for (auto& [path, sound] : soundCache_) {
            if (IsSoundReady(sound)) {
                UnloadSound(sound);
            }
        }
        soundCache_.clear();
        
        CloseAudioDevice();
        initialized_ = false;
    }
    
    /**
     * @brief 全サウンド定義をプリロード
     */
    void PreloadAll() {
        if (!registry_) return;
        
        // サウンド定義からプリロード
        // 注: DefinitionRegistryにGetAllSoundsがない場合はスキップ
        std::cout << "SoundManager: Preloading sounds..." << std::endl;
    }
    
    /**
     * @brief 全リソースを解放
     */
    void UnloadAll() {
        // 全サウンド停止・解放
        for (auto& inst : soundInstances_) {
            if (IsSoundReady(inst.sound)) {
                StopSound(inst.sound);
            }
        }
        soundInstances_.clear();
        
        // 全ミュージック停止・解放
        for (auto& [id, inst] : musicInstances_) {
            if (IsMusicReady(inst.music)) {
                StopMusicStream(inst.music);
                UnloadMusicStream(inst.music);
            }
        }
        musicInstances_.clear();
        
        // サウンドキャッシュ解放
        for (auto& [path, sound] : soundCache_) {
            if (IsSoundReady(sound)) {
                UnloadSound(sound);
            }
        }
        soundCache_.clear();
        
        std::cout << "SoundManager: All resources unloaded" << std::endl;
    }
    
    /**
     * @brief 更新（毎フレーム呼び出し）
     */
    void Update(float dt) {
        if (!initialized_) return;
        
        // ミュージックストリーム更新
        for (auto& [id, inst] : musicInstances_) {
            if (inst.playing && IsMusicReady(inst.music)) {
                UpdateMusicStream(inst.music);
                
                // フェード処理
                if (inst.fadeSpeed != 0.0f) {
                    inst.fadeVolume += inst.fadeSpeed * dt;
                    if ((inst.fadeSpeed > 0 && inst.fadeVolume >= inst.fadeTarget) ||
                        (inst.fadeSpeed < 0 && inst.fadeVolume <= inst.fadeTarget)) {
                        inst.fadeVolume = inst.fadeTarget;
                        inst.fadeSpeed = 0.0f;
                        
                        // フェードアウト完了時は停止
                        if (inst.fadeTarget <= 0.0f) {
                            StopMusicStream(inst.music);
                            inst.playing = false;
                        }
                    }
                    UpdateMusicVolume(inst);
                }
            }
        }
        
        // サウンドインスタンス更新
        for (auto& inst : soundInstances_) {
            if (inst.playing) {
                // 再生終了チェック
                if (!IsSoundPlaying(inst.sound)) {
                    inst.playing = false;
                    continue;
                }
                
                // フェード処理
                if (inst.fadeSpeed != 0.0f) {
                    inst.fadeVolume += inst.fadeSpeed * dt;
                    if ((inst.fadeSpeed > 0 && inst.fadeVolume >= inst.fadeTarget) ||
                        (inst.fadeSpeed < 0 && inst.fadeVolume <= inst.fadeTarget)) {
                        inst.fadeVolume = inst.fadeTarget;
                        inst.fadeSpeed = 0.0f;
                        
                        if (inst.fadeTarget <= 0.0f) {
                            StopSound(inst.sound);
                            inst.playing = false;
                        }
                    }
                    SetSoundVolume(inst.sound, inst.volume * inst.fadeVolume * GetGroupVolume(inst.soundId));
                }
                
                // 3Dサウンド更新
                if (inst.is3D) {
                    Update3DSound(inst);
                }
            }
        }
        
        // 非アクティブインスタンスのクリーンアップ（定期的に）
        cleanupTimer_ += dt;
        if (cleanupTimer_ > 5.0f) {
            CleanupInactiveInstances();
            cleanupTimer_ = 0.0f;
        }
    }
    
    // ===== サウンド再生 =====
    
    /**
     * @brief サウンドを再生
     * @param soundId サウンド定義ID
     * @return 成功時true
     */
    bool PlaySound(const std::string& soundId) {
        if (!initialized_ || !registry_) return false;
        
        const Data::SoundDef* def = registry_->TryGetSound(soundId);
        if (!def) {
            std::cerr << "SoundManager: Sound not found: " << soundId << std::endl;
            return false;
        }
        
        return PlaySoundFromDef(*def);
    }
    
    /**
     * @brief 3Dサウンドを再生
     */
    bool PlaySound3D(const std::string& soundId, float x, float y) {
        if (!initialized_ || !registry_) return false;
        
        const Data::SoundDef* def = registry_->TryGetSound(soundId);
        if (!def) return false;
        
        return PlaySoundFromDef(*def, x, y);
    }
    
    /**
     * @brief サウンドイベントを発火
     */
    bool TriggerEvent(const std::string& bankId, const std::string& eventId) {
        if (!initialized_ || !registry_) return false;
        
        // TODO: SoundBankから該当イベントを検索して再生
        return false;
    }
    
    // ===== BGM再生 =====
    
    /**
     * @brief BGMを再生
     */
    bool PlayMusic(const std::string& musicId, bool fadeIn = true) {
        if (!initialized_ || !registry_) return false;
        
        const Data::MusicDef* def = registry_->TryGetMusic(musicId);
        if (!def) {
            std::cerr << "SoundManager: Music not found: " << musicId << std::endl;
            return false;
        }
        
        return PlayMusicFromDef(*def, fadeIn);
    }
    
    /**
     * @brief BGMを停止
     */
    void StopMusic(const std::string& musicId, bool fadeOut = true) {
        auto it = musicInstances_.find(musicId);
        if (it == musicInstances_.end()) return;
        
        if (fadeOut && it->second.playing) {
            it->second.fadeTarget = 0.0f;
            it->second.fadeSpeed = -1.0f / 2.0f;  // 2秒でフェードアウト
        } else {
            if (IsMusicReady(it->second.music)) {
                StopMusicStream(it->second.music);
            }
            it->second.playing = false;
        }
    }
    
    /**
     * @brief 全BGM停止
     */
    void StopAllMusic(bool fadeOut = true) {
        for (auto& [id, inst] : musicInstances_) {
            if (inst.playing) {
                if (fadeOut) {
                    inst.fadeTarget = 0.0f;
                    inst.fadeSpeed = -1.0f / 1.0f;
                } else {
                    if (IsMusicReady(inst.music)) {
                        StopMusicStream(inst.music);
                    }
                    inst.playing = false;
                }
            }
        }
    }
    
    /**
     * @brief BGMをクロスフェードで切り替え
     */
    bool CrossfadeTo(const std::string& newMusicId, float duration = 2.0f) {
        // 現在再生中のBGMをフェードアウト
        for (auto& [id, inst] : musicInstances_) {
            if (inst.playing && id != newMusicId) {
                inst.fadeTarget = 0.0f;
                inst.fadeSpeed = -1.0f / duration;
            }
        }
        
        // 新BGMをフェードイン
        return PlayMusic(newMusicId, true);
    }
    
    // ===== グループ制御 =====
    
    /**
     * @brief グループを作成
     */
    void CreateGroup(const std::string& groupId, float volume = 1.0f) {
        SoundGroup group;
        group.id = groupId;
        group.volume = volume;
        groups_[groupId] = group;
    }
    
    /**
     * @brief グループ音量を設定
     */
    void SetGroupVolume(const std::string& groupId, float volume) {
        if (groups_.count(groupId)) {
            groups_[groupId].volume = std::clamp(volume, 0.0f, 1.0f);
        }
    }
    
    /**
     * @brief グループをミュート/アンミュート
     */
    void SetGroupMuted(const std::string& groupId, bool muted) {
        if (groups_.count(groupId)) {
            groups_[groupId].muted = muted;
        }
    }
    
    /**
     * @brief マスター音量を設定
     */
    void SetMasterVolume(float volume) {
        SetGroupVolume("master", volume);
    }
    
    /**
     * @brief 定義レジストリを設定
     */
    void SetRegistry(Data::DefinitionRegistry* registry) {
        registry_ = registry;
    }
    
    /**
     * @brief リスナー位置を設定（3Dサウンド用）
     */
    void SetListenerPosition(float x, float y) {
        listenerX_ = x;
        listenerY_ = y;
    }
    
    // ===== 状態取得 =====
    
    bool IsInitialized() const { return initialized_; }
    bool IsMusicPlaying(const std::string& musicId) const {
        auto it = musicInstances_.find(musicId);
        return it != musicInstances_.end() && it->second.playing;
    }
    
    float GetGroupVolume(const std::string& groupId) const {
        auto it = groups_.find(groupId);
        if (it != groups_.end() && !it->second.muted) {
            float master = 1.0f;
            auto masterIt = groups_.find("master");
            if (masterIt != groups_.end() && !masterIt->second.muted) {
                master = masterIt->second.volume;
            }
            return it->second.volume * master;
        }
        return 0.0f;
    }

private:
    bool initialized_ = false;
    Data::DefinitionRegistry* registry_ = nullptr;
    
    // インスタンス管理
    std::vector<SoundInstance> soundInstances_;
    std::unordered_map<std::string, MusicInstance> musicInstances_;
    
    // キャッシュ
    std::unordered_map<std::string, Sound> soundCache_;
    
    // グループ
    std::unordered_map<std::string, SoundGroup> groups_;
    
    // 3Dサウンド用リスナー位置
    float listenerX_ = 0.0f;
    float listenerY_ = 0.0f;
    
    // クリーンアップタイマー
    float cleanupTimer_ = 0.0f;
    
    // 乱数
    std::mt19937 rng_;
    
    // クールダウン管理
    std::unordered_map<std::string, float> soundCooldowns_;
    
    /**
     * @brief サウンド定義から再生
     */
    bool PlaySoundFromDef(const Data::SoundDef& def, float x = 0.0f, float y = 0.0f) {
        // クールダウンチェック
        if (def.cooldown > 0.0f) {
            float now = static_cast<float>(GetTime());
            auto it = soundCooldowns_.find(def.id);
            if (it != soundCooldowns_.end() && (now - it->second) < def.cooldown) {
                return false;
            }
            soundCooldowns_[def.id] = now;
        }
        
        // バリエーション選択
        if (def.variations.empty()) return false;
        
        const Data::SoundVariation* variation = SelectVariation(def.variations);
        if (!variation) return false;
        
        // サウンドロード（キャッシュ利用）
        Sound sound = LoadSoundCached(variation->filePath);
        if (!IsSoundReady(sound)) {
            std::cerr << "SoundManager: Failed to load sound: " << variation->filePath << std::endl;
            return false;
        }
        
        // ピッチ・ボリューム計算
        float pitch = def.pitch + variation->pitchOffset;
        if (def.pitchVariation > 0.0f) {
            std::uniform_real_distribution<float> dist(-def.pitchVariation, def.pitchVariation);
            pitch += dist(rng_);
        }
        pitch = std::clamp(pitch, 0.5f, 2.0f);
        
        float volume = def.volume + variation->volumeOffset;
        if (def.volumeVariation > 0.0f) {
            std::uniform_real_distribution<float> dist(-def.volumeVariation, def.volumeVariation);
            volume += dist(rng_);
        }
        volume = std::clamp(volume, 0.0f, 1.0f);
        
        // グループ音量適用
        std::string groupId = def.group.empty() ? "sfx" : def.group;
        float groupVol = GetGroupVolume(groupId);
        
        // インスタンス作成
        SoundInstance inst;
        inst.sound = sound;
        inst.soundId = def.id;
        inst.volume = volume;
        inst.pitch = pitch;
        inst.startTime = static_cast<float>(GetTime());
        inst.playing = true;
        inst.looping = def.loop;
        inst.is3D = def.is3D;
        inst.x = x;
        inst.y = y;
        inst.minDistance = def.minDistance;
        inst.maxDistance = def.maxDistance;
        
        // フェードイン
        if (def.fadeInTime > 0.0f) {
            inst.fadeVolume = 0.0f;
            inst.fadeTarget = 1.0f;
            inst.fadeSpeed = 1.0f / def.fadeInTime;
        }
        
        // 再生
        SetSoundVolume(sound, volume * groupVol * inst.fadeVolume);
        SetSoundPitch(sound, pitch);
        ::PlaySound(sound);
        
        soundInstances_.push_back(inst);
        return true;
    }
    
    /**
     * @brief BGM定義から再生
     */
    bool PlayMusicFromDef(const Data::MusicDef& def, bool fadeIn) {
        // 既存インスタンスチェック
        auto it = musicInstances_.find(def.id);
        if (it != musicInstances_.end() && it->second.playing) {
            return true;  // 既に再生中
        }
        
        // ミュージックロード
        Music music = LoadMusicStream(def.filePath.c_str());
        if (!IsMusicReady(music)) {
            std::cerr << "SoundManager: Failed to load music: " << def.filePath << std::endl;
            return false;
        }
        
        // インスタンス作成
        MusicInstance inst;
        inst.music = music;
        inst.musicId = def.id;
        inst.volume = def.volume;
        inst.playing = true;
        inst.looping = def.loop.enabled;
        
        // フェードイン
        if (fadeIn) {
            inst.fadeVolume = 0.0f;
            inst.fadeTarget = 1.0f;
            inst.fadeSpeed = 1.0f / def.crossfadeDuration;
        }
        
        // グループ音量
        std::string groupId = def.group.empty() ? "music" : def.group;
        float groupVol = GetGroupVolume(groupId);
        
        SetMusicVolume(music, inst.volume * groupVol * inst.fadeVolume);
        music.looping = inst.looping;
        PlayMusicStream(music);
        
        musicInstances_[def.id] = inst;
        return true;
    }
    
    /**
     * @brief バリエーション選択
     */
    const Data::SoundVariation* SelectVariation(const std::vector<Data::SoundVariation>& variations) {
        if (variations.empty()) return nullptr;
        if (variations.size() == 1) return &variations[0];
        
        // 重み付きランダム選択
        float totalWeight = 0.0f;
        for (const auto& v : variations) {
            totalWeight += v.weight;
        }
        
        std::uniform_real_distribution<float> dist(0.0f, totalWeight);
        float r = dist(rng_);
        
        float cumulative = 0.0f;
        for (const auto& v : variations) {
            cumulative += v.weight;
            if (r <= cumulative) {
                return &v;
            }
        }
        
        return &variations.back();
    }
    
    /**
     * @brief サウンドをキャッシュから読み込み
     */
    Sound LoadSoundCached(const std::string& filePath) {
        auto it = soundCache_.find(filePath);
        if (it != soundCache_.end()) {
            return it->second;
        }
        
        Sound sound = LoadSound(filePath.c_str());
        if (IsSoundReady(sound)) {
            soundCache_[filePath] = sound;
        }
        return sound;
    }
    
    /**
     * @brief ミュージック音量更新
     */
    void UpdateMusicVolume(MusicInstance& inst) {
        if (!registry_) return;
        
        const Data::MusicDef* def = registry_->TryGetMusic(inst.musicId);
        std::string groupId = (def && !def->group.empty()) ? def->group : "music";
        float groupVol = GetGroupVolume(groupId);
        
        SetMusicVolume(inst.music, inst.volume * groupVol * inst.fadeVolume);
    }
    
    /**
     * @brief 3Dサウンド更新
     */
    void Update3DSound(SoundInstance& inst) {
        float dx = inst.x - listenerX_;
        float dy = inst.y - listenerY_;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // 距離減衰計算
        float attenuation = 1.0f;
        if (distance > inst.minDistance) {
            if (distance >= inst.maxDistance) {
                attenuation = 0.0f;
            } else {
                float range = inst.maxDistance - inst.minDistance;
                attenuation = 1.0f - (distance - inst.minDistance) / range;
            }
        }
        
        // パン計算（簡易版）
        float pan = 0.0f;
        if (distance > 0.1f) {
            pan = std::clamp(dx / distance, -1.0f, 1.0f);
        }
        
        // 音量適用
        SetSoundVolume(inst.sound, inst.volume * attenuation * inst.fadeVolume * GetGroupVolume("sfx"));
        SetSoundPan(inst.sound, pan);
    }
    
    /**
     * @brief 非アクティブインスタンスのクリーンアップ
     */
    void CleanupInactiveInstances() {
        soundInstances_.erase(
            std::remove_if(soundInstances_.begin(), soundInstances_.end(),
                [](const SoundInstance& inst) { return !inst.playing; }),
            soundInstances_.end()
        );
    }
};

} // namespace Core
