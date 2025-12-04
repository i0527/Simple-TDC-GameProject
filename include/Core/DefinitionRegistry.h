/**
 * @file DefinitionRegistry.h
 * @brief 定義データのレジストリ（キャッシュ）
 * 
 * JSONからロードされた定義データを保持し、
 * IDベースで高速にアクセスできるようにする。
 * ゲーム中に頻繁に参照される「設計図」のカタログ。
 */
#pragma once

#include "Definitions.h"
#include "Core/UIDefinitions.h"
#include "Data/AnimationDef.h"
#include "Data/SoundDef.h"
#include "Data/EffectDef.h"
#include <unordered_map>
#include <optional>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

namespace Data {

/**
 * @brief 定義が見つからない場合の例外
 */
class DefinitionNotFoundException : public std::runtime_error {
public:
    DefinitionNotFoundException(const std::string& type, const std::string& id)
        : std::runtime_error("Definition not found: " + type + "/" + id) {}
};

/**
 * @brief 定義データレジストリ
 * 
 * 全ての定義データを一元管理し、IDベースのアクセスを提供。
 * 
 * 使用例:
 * @code
 * DefinitionRegistry registry;
 * 
 * // 定義の登録
 * CharacterDef slime;
 * slime.id = "cupslime";
 * slime.name = "カップスライム";
 * registry.RegisterCharacter(slime);
 * 
 * // 定義の取得
 * const auto& def = registry.GetCharacter("cupslime");
 * 
 * // 安全な取得
 * if (auto* ptr = registry.TryGetCharacter("unknown")) {
 *     // 見つかった
 * }
 * @endcode
 */
class DefinitionRegistry {
public:
    DefinitionRegistry() = default;
    ~DefinitionRegistry() = default;
    
    // ===== キャラクター定義 =====
    
    void RegisterCharacter(const CharacterDef& def) {
        if (def.id.empty()) {
            std::cerr << "Warning: Attempted to register character with empty ID\n";
            return;
        }
        characters_[def.id] = def;
    }
    
    void RegisterCharacter(CharacterDef&& def) {
        if (def.id.empty()) {
            std::cerr << "Warning: Attempted to register character with empty ID\n";
            return;
        }
        std::string id = def.id;
        characters_[std::move(id)] = std::move(def);
    }
    
    const CharacterDef& GetCharacter(const std::string& id) const {
        auto it = characters_.find(id);
        if (it == characters_.end()) {
            throw DefinitionNotFoundException("Character", id);
        }
        return it->second;
    }
    
    const CharacterDef* TryGetCharacter(const std::string& id) const {
        auto it = characters_.find(id);
        return it != characters_.end() ? &it->second : nullptr;
    }
    
    bool HasCharacter(const std::string& id) const {
        return characters_.count(id) > 0;
    }
    
    std::vector<std::string> GetAllCharacterIds() const {
        std::vector<std::string> ids;
        ids.reserve(characters_.size());
        for (const auto& [id, _] : characters_) {
            ids.push_back(id);
        }
        return ids;
    }
    
    std::vector<std::string> GetCharacterIdsByTrait(const std::string& trait) const {
        std::vector<std::string> result;
        for (const auto& [id, def] : characters_) {
            for (const auto& t : def.traits) {
                if (t == trait) {
                    result.push_back(id);
                    break;
                }
            }
        }
        return result;
    }
    
    std::vector<std::string> GetCharacterIdsByRarity(Rarity rarity) const {
        std::vector<std::string> result;
        for (const auto& [id, def] : characters_) {
            if (def.rarity == rarity) {
                result.push_back(id);
            }
        }
        return result;
    }
    
    size_t CharacterCount() const { return characters_.size(); }
    
    // ===== スキル定義 =====
    
    void RegisterSkill(const SkillDef& def) {
        if (def.id.empty()) return;
        skills_[def.id] = def;
    }
    
    void RegisterSkill(SkillDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        skills_[std::move(id)] = std::move(def);
    }
    
    const SkillDef& GetSkill(const std::string& id) const {
        auto it = skills_.find(id);
        if (it == skills_.end()) {
            throw DefinitionNotFoundException("Skill", id);
        }
        return it->second;
    }
    
    const SkillDef* TryGetSkill(const std::string& id) const {
        auto it = skills_.find(id);
        return it != skills_.end() ? &it->second : nullptr;
    }
    
    bool HasSkill(const std::string& id) const {
        return skills_.count(id) > 0;
    }
    
    size_t SkillCount() const { return skills_.size(); }
    
    // ===== ステータス効果定義 =====
    
    void RegisterStatusEffect(const StatusEffectDef& def) {
        if (def.id.empty()) return;
        statusEffects_[def.id] = def;
    }
    
    void RegisterStatusEffect(StatusEffectDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        statusEffects_[std::move(id)] = std::move(def);
    }
    
    const StatusEffectDef& GetStatusEffect(const std::string& id) const {
        auto it = statusEffects_.find(id);
        if (it == statusEffects_.end()) {
            throw DefinitionNotFoundException("StatusEffect", id);
        }
        return it->second;
    }
    
    const StatusEffectDef* TryGetStatusEffect(const std::string& id) const {
        auto it = statusEffects_.find(id);
        return it != statusEffects_.end() ? &it->second : nullptr;
    }
    
    size_t StatusEffectCount() const { return statusEffects_.size(); }
    
    // ===== ステージ定義 =====
    
    void RegisterStage(const StageDef& def) {
        if (def.id.empty()) return;
        stages_[def.id] = def;
    }
    
    void RegisterStage(StageDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        stages_[std::move(id)] = std::move(def);
    }
    
    const StageDef& GetStage(const std::string& id) const {
        auto it = stages_.find(id);
        if (it == stages_.end()) {
            throw DefinitionNotFoundException("Stage", id);
        }
        return it->second;
    }
    
    const StageDef* TryGetStage(const std::string& id) const {
        auto it = stages_.find(id);
        return it != stages_.end() ? &it->second : nullptr;
    }
    
    bool HasStage(const std::string& id) const {
        return stages_.count(id) > 0;
    }
    
    std::vector<std::string> GetAllStageIds() const {
        std::vector<std::string> ids;
        ids.reserve(stages_.size());
        for (const auto& [id, _] : stages_) {
            ids.push_back(id);
        }
        return ids;
    }
    
    size_t StageCount() const { return stages_.size(); }
    
    // ===== UIレイアウト定義 =====
    
    void RegisterUILayout(const UILayoutDef& def) {
        if (def.id.empty()) return;
        uiLayouts_[def.id] = def;
    }
    
    void RegisterUILayout(UILayoutDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        uiLayouts_[std::move(id)] = std::move(def);
    }
    
    const UILayoutDef& GetUILayout(const std::string& id) const {
        auto it = uiLayouts_.find(id);
        if (it == uiLayouts_.end()) {
            throw DefinitionNotFoundException("UILayout", id);
        }
        return it->second;
    }
    
    const UILayoutDef* TryGetUILayout(const std::string& id) const {
        auto it = uiLayouts_.find(id);
        return it != uiLayouts_.end() ? &it->second : nullptr;
    }
    
    bool HasUILayout(const std::string& id) const {
        return uiLayouts_.count(id) > 0;
    }
    
    std::vector<std::string> GetAllUILayoutIds() const {
        std::vector<std::string> ids;
        ids.reserve(uiLayouts_.size());
        for (const auto& [id, _] : uiLayouts_) {
            ids.push_back(id);
        }
        return ids;
    }
    
    size_t UILayoutCount() const { return uiLayouts_.size(); }
    
    // ===== アニメーション定義 =====
    
    void RegisterSpriteAnimation(const SpriteAnimationDef& def) {
        if (def.id.empty()) return;
        spriteAnimations_[def.id] = def;
    }
    
    void RegisterSpriteAnimation(SpriteAnimationDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        spriteAnimations_[std::move(id)] = std::move(def);
    }
    
    const SpriteAnimationDef& GetSpriteAnimation(const std::string& id) const {
        auto it = spriteAnimations_.find(id);
        if (it == spriteAnimations_.end()) {
            throw DefinitionNotFoundException("SpriteAnimation", id);
        }
        return it->second;
    }
    
    const SpriteAnimationDef* TryGetSpriteAnimation(const std::string& id) const {
        auto it = spriteAnimations_.find(id);
        return it != spriteAnimations_.end() ? &it->second : nullptr;
    }
    
    bool HasSpriteAnimation(const std::string& id) const {
        return spriteAnimations_.count(id) > 0;
    }
    
    std::vector<std::string> GetAllSpriteAnimationIds() const {
        std::vector<std::string> ids;
        ids.reserve(spriteAnimations_.size());
        for (const auto& [id, _] : spriteAnimations_) {
            ids.push_back(id);
        }
        return ids;
    }
    
    size_t SpriteAnimationCount() const { return spriteAnimations_.size(); }
    
    // ===== サウンド定義 =====
    
    void RegisterSound(const SoundDef& def) {
        if (def.id.empty()) return;
        sounds_[def.id] = def;
    }
    
    void RegisterSound(SoundDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        sounds_[std::move(id)] = std::move(def);
    }
    
    const SoundDef& GetSound(const std::string& id) const {
        auto it = sounds_.find(id);
        if (it == sounds_.end()) {
            throw DefinitionNotFoundException("Sound", id);
        }
        return it->second;
    }
    
    const SoundDef* TryGetSound(const std::string& id) const {
        auto it = sounds_.find(id);
        return it != sounds_.end() ? &it->second : nullptr;
    }
    
    bool HasSound(const std::string& id) const {
        return sounds_.count(id) > 0;
    }
    
    std::vector<std::string> GetAllSoundIds() const {
        std::vector<std::string> ids;
        ids.reserve(sounds_.size());
        for (const auto& [id, _] : sounds_) {
            ids.push_back(id);
        }
        return ids;
    }
    
    size_t SoundCount() const { return sounds_.size(); }
    
    // ===== BGM定義 =====
    
    void RegisterMusic(const MusicDef& def) {
        if (def.id.empty()) return;
        music_[def.id] = def;
    }
    
    void RegisterMusic(MusicDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        music_[std::move(id)] = std::move(def);
    }
    
    const MusicDef& GetMusic(const std::string& id) const {
        auto it = music_.find(id);
        if (it == music_.end()) {
            throw DefinitionNotFoundException("Music", id);
        }
        return it->second;
    }
    
    const MusicDef* TryGetMusic(const std::string& id) const {
        auto it = music_.find(id);
        return it != music_.end() ? &it->second : nullptr;
    }
    
    bool HasMusic(const std::string& id) const {
        return music_.count(id) > 0;
    }
    
    size_t MusicCount() const { return music_.size(); }
    
    // ===== サウンドバンク定義 =====
    
    void RegisterSoundBank(const SoundBankDef& def) {
        if (def.id.empty()) return;
        soundBanks_[def.id] = def;
    }
    
    void RegisterSoundBank(SoundBankDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        soundBanks_[std::move(id)] = std::move(def);
    }
    
    const SoundBankDef& GetSoundBank(const std::string& id) const {
        auto it = soundBanks_.find(id);
        if (it == soundBanks_.end()) {
            throw DefinitionNotFoundException("SoundBank", id);
        }
        return it->second;
    }
    
    const SoundBankDef* TryGetSoundBank(const std::string& id) const {
        auto it = soundBanks_.find(id);
        return it != soundBanks_.end() ? &it->second : nullptr;
    }
    
    size_t SoundBankCount() const { return soundBanks_.size(); }
    
    // ===== パーティクルエフェクト定義 =====
    
    void RegisterParticleEffect(const ParticleEffectDef& def) {
        if (def.id.empty()) return;
        particleEffects_[def.id] = def;
    }
    
    void RegisterParticleEffect(ParticleEffectDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        particleEffects_[std::move(id)] = std::move(def);
    }
    
    const ParticleEffectDef& GetParticleEffect(const std::string& id) const {
        auto it = particleEffects_.find(id);
        if (it == particleEffects_.end()) {
            throw DefinitionNotFoundException("ParticleEffect", id);
        }
        return it->second;
    }
    
    const ParticleEffectDef* TryGetParticleEffect(const std::string& id) const {
        auto it = particleEffects_.find(id);
        return it != particleEffects_.end() ? &it->second : nullptr;
    }
    
    bool HasParticleEffect(const std::string& id) const {
        return particleEffects_.count(id) > 0;
    }
    
    size_t ParticleEffectCount() const { return particleEffects_.size(); }
    
    // ===== 画面エフェクト定義 =====
    
    void RegisterScreenEffect(const ScreenEffectDef& def) {
        if (def.id.empty()) return;
        screenEffects_[def.id] = def;
    }
    
    void RegisterScreenEffect(ScreenEffectDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        screenEffects_[std::move(id)] = std::move(def);
    }
    
    const ScreenEffectDef& GetScreenEffect(const std::string& id) const {
        auto it = screenEffects_.find(id);
        if (it == screenEffects_.end()) {
            throw DefinitionNotFoundException("ScreenEffect", id);
        }
        return it->second;
    }
    
    const ScreenEffectDef* TryGetScreenEffect(const std::string& id) const {
        auto it = screenEffects_.find(id);
        return it != screenEffects_.end() ? &it->second : nullptr;
    }
    
    size_t ScreenEffectCount() const { return screenEffects_.size(); }
    
    // ===== 複合エフェクト定義 =====
    
    void RegisterCompositeEffect(const CompositeEffectDef& def) {
        if (def.id.empty()) return;
        compositeEffects_[def.id] = def;
    }
    
    void RegisterCompositeEffect(CompositeEffectDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        compositeEffects_[std::move(id)] = std::move(def);
    }
    
    const CompositeEffectDef& GetCompositeEffect(const std::string& id) const {
        auto it = compositeEffects_.find(id);
        if (it == compositeEffects_.end()) {
            throw DefinitionNotFoundException("CompositeEffect", id);
        }
        return it->second;
    }
    
    const CompositeEffectDef* TryGetCompositeEffect(const std::string& id) const {
        auto it = compositeEffects_.find(id);
        return it != compositeEffects_.end() ? &it->second : nullptr;
    }
    
    size_t CompositeEffectCount() const { return compositeEffects_.size(); }
    
    // ===== ユーティリティ =====
    
    /**
     * @brief 全定義をクリア
     */
    void Clear() {
        characters_.clear();
        skills_.clear();
        statusEffects_.clear();
        stages_.clear();
        uiLayouts_.clear();
        spriteAnimations_.clear();
        sounds_.clear();
        music_.clear();
        soundBanks_.clear();
        particleEffects_.clear();
        screenEffects_.clear();
        compositeEffects_.clear();
    }
    
    /**
     * @brief 統計情報を取得
     */
    std::string GetStats() const {
        return "DefinitionRegistry: Characters=" + std::to_string(characters_.size())
             + ", Skills=" + std::to_string(skills_.size())
             + ", StatusEffects=" + std::to_string(statusEffects_.size())
             + ", Stages=" + std::to_string(stages_.size())
             + ", UILayouts=" + std::to_string(uiLayouts_.size())
             + ", SpriteAnimations=" + std::to_string(spriteAnimations_.size())
             + ", Sounds=" + std::to_string(sounds_.size())
             + ", Music=" + std::to_string(music_.size())
             + ", ParticleEffects=" + std::to_string(particleEffects_.size())
             + ", ScreenEffects=" + std::to_string(screenEffects_.size());
    }

private:
    std::unordered_map<std::string, CharacterDef> characters_;
    std::unordered_map<std::string, SkillDef> skills_;
    std::unordered_map<std::string, StatusEffectDef> statusEffects_;
    std::unordered_map<std::string, StageDef> stages_;
    std::unordered_map<std::string, UILayoutDef> uiLayouts_;
    std::unordered_map<std::string, SpriteAnimationDef> spriteAnimations_;
    std::unordered_map<std::string, SoundDef> sounds_;
    std::unordered_map<std::string, MusicDef> music_;
    std::unordered_map<std::string, SoundBankDef> soundBanks_;
    std::unordered_map<std::string, ParticleEffectDef> particleEffects_;
    std::unordered_map<std::string, ScreenEffectDef> screenEffects_;
    std::unordered_map<std::string, CompositeEffectDef> compositeEffects_;
};

} // namespace Data
