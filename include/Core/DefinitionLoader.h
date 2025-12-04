/**
 * @file DefinitionLoader.h
 * @brief JSON定義ローダー
 * 
 * JSONファイルからキャラクター、ステージ、スキル等の
 * 定義データを読み込み、DefinitionRegistryに登録する。
 * 
 * Phase 4B〜4D対応: UI, Animation, Sound, Effect定義も統合
 */
#pragma once

#include "Definitions.h"
#include "DefinitionRegistry.h"
#include "Core/UILoader.h"
#include "Data/AnimationLoader.h"
#include "Data/AsepriteLoader.h"
#include "Data/SoundLoader.h"
#include "Data/EffectLoader.h"
#include "Core/FileUtils.h"
#include <nlohmann/json.hpp>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <functional>

// C++17互換のends_with
namespace {
    inline bool StringEndsWith(const std::string& str, const std::string& suffix) {
        if (suffix.size() > str.size()) return false;
        return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
}

namespace Data {

using json = nlohmann::json;

/**
 * @brief 定義データローダー
 * 
 * JSONファイルからゲーム定義を読み込む。
 * ディレクトリ単位での一括読み込みをサポート。
 * 
 * 使用例:
 * @code
 * DefinitionLoader loader(registry);
 * 
 * // 単一ファイル読み込み
 * loader.LoadCharacter("assets/definitions/characters/cupslime.json");
 * 
 * // ディレクトリ一括読み込み
 * loader.LoadAllCharacters("assets/definitions/characters/");
 * loader.LoadAllStages("assets/definitions/stages/");
 * 
 * // 全部読み込み
 * loader.LoadAll("assets/definitions/");
 * @endcode
 */
class DefinitionLoader {
public:
    using ErrorHandler = std::function<void(const std::string& path, const std::string& error)>;
    
    explicit DefinitionLoader(DefinitionRegistry& registry)
        : registry_(registry) {
        // デフォルトエラーハンドラ
        errorHandler_ = [](const std::string& path, const std::string& error) {
            std::cerr << "DefinitionLoader Error [" << path << "]: " << error << "\n";
        };
    }
    
    /**
     * @brief エラーハンドラを設定
     */
    void SetErrorHandler(ErrorHandler handler) {
        errorHandler_ = std::move(handler);
    }
    
    // ===== 単一ファイル読み込み =====
    
    /**
     * @brief キャラクター定義を読み込み
     */
    bool LoadCharacter(const std::string& filePath) {
        try {
            auto jsonData = LoadJsonFile(filePath);
            if (jsonData.is_null()) return false;
            
            auto def = ParseCharacterDef(jsonData);
            if (def.id.empty()) {
                def.id = GetFileNameWithoutExtension(filePath);
            }
            registry_.RegisterCharacter(std::move(def));
            return true;
        } catch (const std::exception& e) {
            errorHandler_(filePath, e.what());
            return false;
        }
    }
    
    /**
     * @brief スキル定義を読み込み
     */
    bool LoadSkill(const std::string& filePath) {
        try {
            auto jsonData = LoadJsonFile(filePath);
            if (jsonData.is_null()) return false;
            
            auto def = ParseSkillDef(jsonData);
            if (def.id.empty()) {
                def.id = GetFileNameWithoutExtension(filePath);
            }
            registry_.RegisterSkill(std::move(def));
            return true;
        } catch (const std::exception& e) {
            errorHandler_(filePath, e.what());
            return false;
        }
    }
    
    /**
     * @brief ステージ定義を読み込み
     */
    bool LoadStage(const std::string& filePath) {
        try {
            auto jsonData = LoadJsonFile(filePath);
            if (jsonData.is_null()) return false;
            
            auto def = ParseStageDef(jsonData);
            if (def.id.empty()) {
                def.id = GetFileNameWithoutExtension(filePath);
            }
            registry_.RegisterStage(std::move(def));
            return true;
        } catch (const std::exception& e) {
            errorHandler_(filePath, e.what());
            return false;
        }
    }
    
    // ===== ディレクトリ一括読み込み =====
    
    /**
     * @brief ディレクトリ内の全キャラクター定義を読み込み
     */
    int LoadAllCharacters(const std::string& directoryPath) {
        return LoadDirectory(directoryPath, ".character.json", 
            [this](const std::string& path) { return LoadCharacter(path); });
    }
    
    /**
     * @brief ディレクトリ内の全スキル定義を読み込み
     */
    int LoadAllSkills(const std::string& directoryPath) {
        return LoadDirectory(directoryPath, ".skill.json",
            [this](const std::string& path) { return LoadSkill(path); });
    }
    
    /**
     * @brief ディレクトリ内の全ステージ定義を読み込み
     */
    int LoadAllStages(const std::string& directoryPath) {
        return LoadDirectory(directoryPath, ".stage.json",
            [this](const std::string& path) { return LoadStage(path); });
    }
    
    /**
     * @brief ディレクトリ内の全UIレイアウト定義を読み込み
     */
    int LoadAllUILayouts(const std::string& directoryPath) {
        return LoadDirectory(directoryPath, ".ui.json",
            [this](const std::string& path) {
                auto def = Data::UILoader::LoadFromFile(path);
                if (def) {
                    registry_.RegisterUILayout(std::move(*def));
                    return true;
                }
                return false;
            });
    }
    
    /**
     * @brief ディレクトリ内の全アニメーション定義を読み込み
     */
    int LoadAllAnimations(const std::string& directoryPath) {
        namespace fs = std::filesystem;
        int count = 0;
        
        if (!fs::exists(directoryPath)) return 0;
        
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (!entry.is_regular_file()) continue;
            
            std::string path = entry.path().string();
            std::string filename = entry.path().filename().string();
            
            // Aseprite形式
            if (StringEndsWith(filename, ".aseprite.json")) {
                auto def = AsepriteLoader::LoadFromFile(path);
                if (def) {
                    registry_.RegisterSpriteAnimation(std::move(*def));
                    count++;
                }
            }
            // 標準形式
            else if (StringEndsWith(filename, ".anim.json")) {
                auto def = AnimationLoader::LoadFromFile(path);
                if (def) {
                    registry_.RegisterSpriteAnimation(std::move(*def));
                    count++;
                }
            }
        }
        
        return count;
    }
    
    /**
     * @brief ディレクトリ内の全サウンド定義を読み込み
     */
    int LoadAllSounds(const std::string& directoryPath) {
        namespace fs = std::filesystem;
        int count = 0;
        
        if (!fs::exists(directoryPath)) return 0;
        
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (!entry.is_regular_file()) continue;
            
            std::string path = entry.path().string();
            std::string filename = entry.path().filename().string();
            
            // サウンドエフェクト
            if (StringEndsWith(filename, ".sound.json")) {
                auto def = SoundLoader::LoadSound(path);
                if (def) {
                    registry_.RegisterSound(std::move(*def));
                    count++;
                }
            }
            // BGM
            else if (StringEndsWith(filename, ".music.json")) {
                auto def = SoundLoader::LoadMusic(path);
                if (def) {
                    registry_.RegisterMusic(std::move(*def));
                    count++;
                }
            }
            // サウンドバンク
            else if (StringEndsWith(filename, ".soundbank.json")) {
                auto def = SoundLoader::LoadSoundBank(path);
                if (def) {
                    registry_.RegisterSoundBank(std::move(*def));
                    count++;
                }
            }
        }
        
        return count;
    }
    
    /**
     * @brief ディレクトリ内の全エフェクト定義を読み込み
     */
    int LoadAllEffects(const std::string& directoryPath) {
        namespace fs = std::filesystem;
        int count = 0;
        
        if (!fs::exists(directoryPath)) return 0;
        
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (!entry.is_regular_file()) continue;
            
            std::string path = entry.path().string();
            std::string filename = entry.path().filename().string();
            
            // パーティクル
            if (StringEndsWith(filename, ".particle.json")) {
                auto def = EffectLoader::LoadParticleEffect(path);
                if (def) {
                    registry_.RegisterParticleEffect(std::move(*def));
                    count++;
                }
            }
            // 画面エフェクト
            else if (StringEndsWith(filename, ".screen.json")) {
                auto def = EffectLoader::LoadScreenEffect(path);
                if (def) {
                    registry_.RegisterScreenEffect(std::move(*def));
                    count++;
                }
            }
            // 複合エフェクト
            else if (StringEndsWith(filename, ".composite.json")) {
                auto def = EffectLoader::LoadCompositeEffect(path);
                if (def) {
                    registry_.RegisterCompositeEffect(std::move(*def));
                    count++;
                }
            }
        }
        
        return count;
    }
    
    /**
     * @brief 全定義を一括読み込み
     * @param basePath 基準ディレクトリ（characters/, stages/, skills/, ui/ 等を含む）
     */
    void LoadAll(const std::string& basePath) {
        namespace fs = std::filesystem;
        
        std::string charactersPath = basePath + "/characters";
        std::string skillsPath = basePath + "/skills";
        std::string stagesPath = basePath + "/stages";
        std::string uiPath = basePath + "/ui";
        std::string animationsPath = basePath + "/animations";
        std::string soundsPath = basePath + "/sounds";
        std::string effectsPath = basePath + "/effects";
        
        // キャラクター
        if (fs::exists(charactersPath)) {
            int count = LoadAllCharacters(charactersPath);
            std::cout << "Loaded " << count << " character definitions\n";
        }
        
        // スキル
        if (fs::exists(skillsPath)) {
            int count = LoadAllSkills(skillsPath);
            std::cout << "Loaded " << count << " skill definitions\n";
        }
        
        // ステージ
        if (fs::exists(stagesPath)) {
            int count = LoadAllStages(stagesPath);
            std::cout << "Loaded " << count << " stage definitions\n";
        }
        
        // UIレイアウト
        if (fs::exists(uiPath)) {
            int count = LoadAllUILayouts(uiPath);
            std::cout << "Loaded " << count << " UI layout definitions\n";
        }
        
        // アニメーション
        if (fs::exists(animationsPath)) {
            int count = LoadAllAnimations(animationsPath);
            std::cout << "Loaded " << count << " animation definitions\n";
        }
        
        // sprites フォルダ（Aseprite出力用）も確認
        std::string spritesPath = basePath + "/../sprites";
        if (fs::exists(spritesPath)) {
            int count = LoadAllAnimations(spritesPath);
            std::cout << "Loaded " << count << " sprite animation definitions\n";
        }
        
        // サウンド
        if (fs::exists(soundsPath)) {
            int count = LoadAllSounds(soundsPath);
            std::cout << "Loaded " << count << " sound definitions\n";
        }
        
        // エフェクト
        if (fs::exists(effectsPath)) {
            int count = LoadAllEffects(effectsPath);
            std::cout << "Loaded " << count << " effect definitions\n";
        }
        
        std::cout << registry_.GetStats() << "\n";
    }

private:
    DefinitionRegistry& registry_;
    ErrorHandler errorHandler_;
    
    // ===== ユーティリティ =====
    
    json LoadJsonFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            errorHandler_(filePath, "Could not open file");
            return json();
        }
        
        try {
            return json::parse(file);
        } catch (const json::parse_error& e) {
            errorHandler_(filePath, std::string("JSON parse error: ") + e.what());
            return json();
        }
    }
    
    std::string GetFileNameWithoutExtension(const std::string& path) {
        namespace fs = std::filesystem;
        fs::path p(path);
        std::string filename = p.stem().string();
        // .character.json のような二重拡張子を処理
        if (StringEndsWith(filename, ".character") || StringEndsWith(filename, ".skill") || 
            StringEndsWith(filename, ".stage")) {
            filename = fs::path(filename).stem().string();
        }
        return filename;
    }
    
    int LoadDirectory(const std::string& path, const std::string& suffix,
                      std::function<bool(const std::string&)> loader) {
        namespace fs = std::filesystem;
        int count = 0;
        
        if (!fs::exists(path)) {
            return 0;
        }
        
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().string();
                // サフィックスチェック（.character.json等）またはプレーンな.json
                if (StringEndsWith(filename, suffix) || 
                    (suffix.find(".json") != std::string::npos && StringEndsWith(filename, ".json"))) {
                    if (loader(filename)) {
                        count++;
                    }
                }
            }
        }
        
        return count;
    }
    
    // ===== パーサー =====
    
    template<typename T>
    T GetOr(const json& j, const std::string& key, T defaultValue) {
        if (j.contains(key) && !j[key].is_null()) {
            return j[key].get<T>();
        }
        return defaultValue;
    }
    
    Rect ParseRect(const json& j) {
        Rect r;
        r.x = GetOr(j, "x", 0.0f);
        r.y = GetOr(j, "y", 0.0f);
        r.width = GetOr(j, "width", 0.0f);
        r.height = GetOr(j, "height", 0.0f);
        return r;
    }
    
    Rarity ParseRarity(const std::string& s) {
        if (s == "normal") return Rarity::Normal;
        if (s == "rare") return Rarity::Rare;
        if (s == "super_rare") return Rarity::SuperRare;
        if (s == "uber_rare") return Rarity::UberRare;
        if (s == "legend") return Rarity::Legend;
        return Rarity::Normal;
    }
    
    AttackType ParseAttackType(const std::string& s) {
        if (s == "single") return AttackType::Single;
        if (s == "area") return AttackType::Area;
        if (s == "wave") return AttackType::Wave;
        return AttackType::Single;
    }
    
    StatusEffectType ParseStatusEffectType(const std::string& s) {
        if (s == "slow") return StatusEffectType::Slow;
        if (s == "stun") return StatusEffectType::Stun;
        if (s == "poison") return StatusEffectType::Poison;
        if (s == "burn") return StatusEffectType::Burn;
        if (s == "freeze") return StatusEffectType::Freeze;
        if (s == "attack_up") return StatusEffectType::AttackUp;
        if (s == "attack_down") return StatusEffectType::AttackDown;
        if (s == "defense_up") return StatusEffectType::DefenseUp;
        if (s == "defense_down") return StatusEffectType::DefenseDown;
        if (s == "speed_up") return StatusEffectType::SpeedUp;
        if (s == "regeneration") return StatusEffectType::Regeneration;
        if (s == "shield") return StatusEffectType::Shield;
        if (s == "invincible") return StatusEffectType::Invincible;
        return StatusEffectType::None;
    }
    
    SkillTargetType ParseSkillTargetType(const std::string& s) {
        if (s == "self") return SkillTargetType::Self;
        if (s == "single_enemy") return SkillTargetType::SingleEnemy;
        if (s == "single_ally") return SkillTargetType::SingleAlly;
        if (s == "all_enemies") return SkillTargetType::AllEnemies;
        if (s == "all_allies") return SkillTargetType::AllAllies;
        if (s == "area") return SkillTargetType::Area;
        return SkillTargetType::SingleEnemy;
    }
    
    SkillEffectType ParseSkillEffectType(const std::string& s) {
        if (s == "damage") return SkillEffectType::Damage;
        if (s == "heal") return SkillEffectType::Heal;
        if (s == "status_apply") return SkillEffectType::StatusApply;
        if (s == "summon") return SkillEffectType::Summon;
        if (s == "knockback") return SkillEffectType::Knockback;
        if (s == "pull") return SkillEffectType::Pull;
        return SkillEffectType::Damage;
    }
    
    AnimationDef ParseAnimationDef(const json& j) {
        AnimationDef def;
        def.name = GetOr<std::string>(j, "name", "");
        def.loop = GetOr(j, "loop", true);
        def.nextAnimation = GetOr<std::string>(j, "next_animation", "");
        
        if (j.contains("frames") && j["frames"].is_array()) {
            for (const auto& frameJson : j["frames"]) {
                FrameDef frame;
                frame.index = GetOr(frameJson, "index", 0);
                frame.duration = GetOr(frameJson, "duration", 0.1f);
                frame.tag = GetOr<std::string>(frameJson, "tag", "");
                def.frames.push_back(frame);
            }
        }
        
        return def;
    }
    
    CharacterDef ParseCharacterDef(const json& j) {
        CharacterDef def;
        
        // 識別
        def.id = GetOr<std::string>(j, "id", "");
        def.name = GetOr<std::string>(j, "name", def.id);
        def.description = GetOr<std::string>(j, "description", "");
        def.rarity = ParseRarity(GetOr<std::string>(j, "rarity", "normal"));
        
        if (j.contains("traits") && j["traits"].is_array()) {
            for (const auto& trait : j["traits"]) {
                def.traits.push_back(trait.get<std::string>());
            }
        }
        
        // スプライト
        if (j.contains("sprite")) {
            const auto& sprite = j["sprite"];
            def.spritePath = GetOr<std::string>(sprite, "path", "");
            def.frameWidth = GetOr(sprite, "frame_width", 64);
            def.frameHeight = GetOr(sprite, "frame_height", 64);
            def.framesPerRow = GetOr(sprite, "frames_per_row", 8);
            def.scale = GetOr(sprite, "scale", 1.0f);
        }
        
        // アニメーション
        if (j.contains("animations")) {
            def.defaultAnimation = GetOr<std::string>(j["animations"], "default", "idle");
            
            if (j["animations"].contains("list") && j["animations"]["list"].is_object()) {
                for (const auto& [name, animJson] : j["animations"]["list"].items()) {
                    auto animDef = ParseAnimationDef(animJson);
                    animDef.name = name;
                    def.animations[name] = std::move(animDef);
                }
            }
        }
        
        // ステータス
        if (j.contains("stats")) {
            const auto& stats = j["stats"];
            def.maxHealth = GetOr(stats, "max_health", 100.0f);
            def.attack = GetOr(stats, "attack", 10.0f);
            def.defense = GetOr(stats, "defense", 0.0f);
            def.attackInterval = GetOr(stats, "attack_interval", 1.0f);
            def.moveSpeed = GetOr(stats, "move_speed", 50.0f);
            def.knockbackResist = GetOr(stats, "knockback_resist", 0.0f);
        }
        
        // 戦闘
        if (j.contains("combat")) {
            const auto& combat = j["combat"];
            def.attackType = ParseAttackType(GetOr<std::string>(combat, "attack_type", "single"));
            
            if (combat.contains("attack_range")) {
                def.attackRange = ParseRect(combat["attack_range"]);
            }
            if (combat.contains("hitbox")) {
                def.hitbox = ParseRect(combat["hitbox"]);
            }
            
            def.attackCount = GetOr(combat, "attack_count", 1);
            def.criticalChance = GetOr(combat, "critical_chance", 0.0f);
            def.criticalMultiplier = GetOr(combat, "critical_multiplier", 1.5f);
        }
        
        // スキル
        if (j.contains("skills") && j["skills"].is_array()) {
            for (const auto& skillId : j["skills"]) {
                def.skillIds.push_back(skillId.get<std::string>());
            }
        }
        
        // コスト
        if (j.contains("cost")) {
            const auto& cost = j["cost"];
            def.cost = GetOr(cost, "value", 100.0f);
            def.cooldownTime = GetOr(cost, "cooldown", 5.0f);
        }
        
        // 成長率
        if (j.contains("growth")) {
            const auto& growth = j["growth"];
            def.healthGrowth = GetOr(growth, "health", 1.1f);
            def.attackGrowth = GetOr(growth, "attack", 1.1f);
        }
        
        // オプション
        def.isEnemy = GetOr(j, "is_enemy", false);
        def.maxSpawnCount = GetOr(j, "max_spawn_count", 0);
        
        return def;
    }
    
    SkillEffectDef ParseSkillEffectDef(const json& j) {
        SkillEffectDef def;
        def.type = ParseSkillEffectType(GetOr<std::string>(j, "type", "damage"));
        def.value = GetOr(j, "value", 0.0f);
        def.isPercentage = GetOr(j, "is_percentage", false);
        def.statusEffectId = GetOr<std::string>(j, "status_effect_id", "");
        def.summonCharacterId = GetOr<std::string>(j, "summon_character_id", "");
        def.summonCount = GetOr(j, "summon_count", 1);
        return def;
    }
    
    SkillDef ParseSkillDef(const json& j) {
        SkillDef def;
        
        def.id = GetOr<std::string>(j, "id", "");
        def.name = GetOr<std::string>(j, "name", def.id);
        def.description = GetOr<std::string>(j, "description", "");
        
        // 発動条件
        if (j.contains("activation")) {
            const auto& act = j["activation"];
            def.cooldown = GetOr(act, "cooldown", 10.0f);
            def.activationChance = GetOr(act, "chance", 1.0f);
            def.activateOnAttack = GetOr(act, "on_attack", false);
            def.activateOnDamaged = GetOr(act, "on_damaged", false);
            def.activateOnDeath = GetOr(act, "on_death", false);
            def.healthThreshold = GetOr(act, "health_threshold", 0.0f);
        }
        
        // ターゲティング
        if (j.contains("targeting")) {
            const auto& target = j["targeting"];
            def.targetType = ParseSkillTargetType(GetOr<std::string>(target, "type", "single_enemy"));
            if (target.contains("effect_area")) {
                def.effectArea = ParseRect(target["effect_area"]);
            }
            def.maxTargets = GetOr(target, "max_targets", 1);
        }
        
        // 効果
        if (j.contains("effects") && j["effects"].is_array()) {
            for (const auto& effectJson : j["effects"]) {
                def.effects.push_back(ParseSkillEffectDef(effectJson));
            }
        }
        
        // アニメーション
        def.animationName = GetOr<std::string>(j, "animation_name", "");
        def.effectSpritePath = GetOr<std::string>(j, "effect_sprite_path", "");
        
        return def;
    }
    
    EnemySpawnEntry ParseEnemySpawnEntry(const json& j) {
        EnemySpawnEntry entry;
        entry.characterId = GetOr<std::string>(j, "character_id", "");
        entry.count = GetOr(j, "count", 1);
        entry.delay = GetOr(j, "delay", 0.0f);
        entry.interval = GetOr(j, "interval", 1.0f);
        entry.lane = GetOr(j, "lane", 0);
        return entry;
    }
    
    WaveDef ParseWaveDef(const json& j) {
        WaveDef def;
        def.waveNumber = GetOr(j, "wave_number", 0);
        def.duration = GetOr(j, "duration", 30.0f);
        def.triggerCondition = GetOr<std::string>(j, "trigger_condition", "all_dead");
        
        if (j.contains("enemies") && j["enemies"].is_array()) {
            for (const auto& enemyJson : j["enemies"]) {
                def.enemies.push_back(ParseEnemySpawnEntry(enemyJson));
            }
        }
        
        return def;
    }
    
    StageDef ParseStageDef(const json& j) {
        StageDef def;
        
        def.id = GetOr<std::string>(j, "id", "");
        def.name = GetOr<std::string>(j, "name", def.id);
        def.description = GetOr<std::string>(j, "description", "");
        def.backgroundPath = GetOr<std::string>(j, "background_path", "");
        
        // Wave
        if (j.contains("waves") && j["waves"].is_array()) {
            int waveNum = 1;
            for (const auto& waveJson : j["waves"]) {
                auto waveDef = ParseWaveDef(waveJson);
                waveDef.waveNumber = waveNum++;
                def.waves.push_back(std::move(waveDef));
            }
        }
        
        // 勝利条件
        if (j.contains("victory_conditions")) {
            const auto& vc = j["victory_conditions"];
            def.baseHealth = GetOr(vc, "base_health", 1000.0f);
            def.enemyBaseHealth = GetOr(vc, "enemy_base_health", 1000.0f);
            def.timeLimit = GetOr(vc, "time_limit", 0.0f);
        }
        
        // 報酬
        if (j.contains("rewards")) {
            const auto& rewards = j["rewards"];
            def.clearReward = GetOr(rewards, "clear", 100);
            def.firstClearBonus = GetOr(rewards, "first_clear_bonus", 50);
            
            if (rewards.contains("drop_characters") && rewards["drop_characters"].is_array()) {
                for (const auto& charId : rewards["drop_characters"]) {
                    def.dropCharacterIds.push_back(charId.get<std::string>());
                }
            }
        }
        
        // コスト設定
        if (j.contains("cost")) {
            const auto& cost = j["cost"];
            def.startingCost = GetOr(cost, "starting", 500.0f);
            def.costRegenRate = GetOr(cost, "regen_rate", 10.0f);
            def.maxCost = GetOr(cost, "max", 9999.0f);
        }
        
        // レーン設定
        if (j.contains("lanes")) {
            const auto& lanes = j["lanes"];
            def.laneCount = GetOr(lanes, "count", 1);
            def.laneHeight = GetOr(lanes, "height", 100.0f);
        }
        
        // 難易度調整
        if (j.contains("difficulty")) {
            const auto& diff = j["difficulty"];
            def.enemyHealthMultiplier = GetOr(diff, "enemy_health", 1.0f);
            def.enemyAttackMultiplier = GetOr(diff, "enemy_attack", 1.0f);
        }
        
        return def;
    }
};

} // namespace Data
