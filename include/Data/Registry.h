/**
 * @file Registry.h
 * @brief 統一定義データレジストリ
 * 
 * 全てのゲーム定義（キャラクター、ステージ、UI、マップ等）を
 * 一元管理し、IDベースで高速にアクセスできるようにする。
 * データ駆動設計の中核となるレジストリ。
 */
#pragma once

#include "Data/Definitions/CharacterDef.h"
#include "Data/Definitions/StageDef.h"
#include "Data/Definitions/UILayoutDef.h"
#include "Data/Definitions/AnimationDef.h"
#include "Data/Definitions/MapDef.h"
#include "Data/SoundDef.h"
#include "Data/EffectDef.h"
#include <unordered_map>
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
 * @brief 統一定義データレジストリ
 * 
 * 全ての定義データを一元管理し、IDベースのアクセスを提供。
 * TD/Roguelike統合プラットフォームで使用する統合レジストリ。
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
    
    // コピー禁止
    DefinitionRegistry(const DefinitionRegistry&) = delete;
    DefinitionRegistry& operator=(const DefinitionRegistry&) = delete;
    
    // ムーブ許可
    DefinitionRegistry(DefinitionRegistry&&) = default;
    DefinitionRegistry& operator=(DefinitionRegistry&&) = default;
    
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
    
    size_t CharacterCount() const { return characters_.size(); }
    
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
    
    // ===== マップ定義（Roguelike用）=====
    
    void RegisterMap(const MapDef& def) {
        if (def.id.empty()) return;
        maps_[def.id] = def;
    }
    
    void RegisterMap(MapDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        maps_[std::move(id)] = std::move(def);
    }
    
    const MapDef& GetMap(const std::string& id) const {
        auto it = maps_.find(id);
        if (it == maps_.end()) {
            throw DefinitionNotFoundException("Map", id);
        }
        return it->second;
    }
    
    const MapDef* TryGetMap(const std::string& id) const {
        auto it = maps_.find(id);
        return it != maps_.end() ? &it->second : nullptr;
    }
    
    bool HasMap(const std::string& id) const {
        return maps_.count(id) > 0;
    }
    
    std::vector<std::string> GetAllMapIds() const {
        std::vector<std::string> ids;
        ids.reserve(maps_.size());
        for (const auto& [id, _] : maps_) {
            ids.push_back(id);
        }
        return ids;
    }
    
    size_t MapCount() const { return maps_.size(); }
    
    // ===== アニメーション定義 =====
    
    void RegisterAnimation(const AnimationDef& def) {
        if (def.id.empty()) return;
        animations_[def.id] = def;
    }
    
    void RegisterAnimation(AnimationDef&& def) {
        if (def.id.empty()) return;
        std::string id = def.id;
        animations_[std::move(id)] = std::move(def);
    }
    
    const AnimationDef& GetAnimation(const std::string& id) const {
        auto it = animations_.find(id);
        if (it == animations_.end()) {
            throw DefinitionNotFoundException("Animation", id);
        }
        return it->second;
    }
    
    const AnimationDef* TryGetAnimation(const std::string& id) const {
        auto it = animations_.find(id);
        return it != animations_.end() ? &it->second : nullptr;
    }
    
    bool HasAnimation(const std::string& id) const {
        return animations_.count(id) > 0;
    }
    
    size_t AnimationCount() const { return animations_.size(); }
    
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
    
    bool HasScreenEffect(const std::string& id) const {
        return screenEffects_.count(id) > 0;
    }
    
    size_t ScreenEffectCount() const { return screenEffects_.size(); }
    
    // ===== ユーティリティ =====
    
    /**
     * @brief 全定義をクリア
     */
    void Clear() {
        characters_.clear();
        stages_.clear();
        uiLayouts_.clear();
        maps_.clear();
        animations_.clear();
        sounds_.clear();
        music_.clear();
        particleEffects_.clear();
        screenEffects_.clear();
    }
    
    /**
     * @brief 統計情報を取得
     */
    std::string GetStats() const {
        return "DefinitionRegistry: Characters=" + std::to_string(characters_.size())
             + ", Stages=" + std::to_string(stages_.size())
             + ", UILayouts=" + std::to_string(uiLayouts_.size())
             + ", Maps=" + std::to_string(maps_.size())
             + ", Animations=" + std::to_string(animations_.size())
             + ", Sounds=" + std::to_string(sounds_.size())
             + ", Music=" + std::to_string(music_.size())
             + ", ParticleEffects=" + std::to_string(particleEffects_.size())
             + ", ScreenEffects=" + std::to_string(screenEffects_.size());
    }

private:
    std::unordered_map<std::string, CharacterDef> characters_;
    std::unordered_map<std::string, StageDef> stages_;
    std::unordered_map<std::string, UILayoutDef> uiLayouts_;
    std::unordered_map<std::string, MapDef> maps_;
    std::unordered_map<std::string, AnimationDef> animations_;
    std::unordered_map<std::string, SoundDef> sounds_;
    std::unordered_map<std::string, MusicDef> music_;
    std::unordered_map<std::string, ParticleEffectDef> particleEffects_;
    std::unordered_map<std::string, ScreenEffectDef> screenEffects_;
};

} // namespace Data

