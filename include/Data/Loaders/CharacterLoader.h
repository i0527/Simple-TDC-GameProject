/**
 * @file CharacterLoader.h
 * @brief キャラクター定義ローダー
 * 
 * JSONからキャラクター定義を読み込み、DefinitionRegistryに登録する
 */
#pragma once

#include "Data/Loaders/DataLoaderBase.h"
#include "Data/Definitions/CharacterDef.h"
#include <nlohmann/json.hpp>

namespace Data {

/**
 * @brief キャラクター定義ローダー
 */
class CharacterLoader : public DataLoaderBase {
public:
    explicit CharacterLoader(DefinitionRegistry& registry)
        : DataLoaderBase(registry) {}
    
    /**
     * @brief 単一のキャラクター定義ファイルを読み込み
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
     * @brief ディレクトリ内の全キャラクター定義を読み込み
     */
    int LoadAllCharacters(const std::string& directoryPath) {
        return LoadDirectory(directoryPath, ".character.json",
            [this](const std::string& path) { return LoadCharacter(path); });
    }
    
    /**
     * @brief JSONからCharacterDefをパース
     */
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
        
        // ゲームモード
        std::string gameModeStr = GetOr<std::string>(j, "gameMode", "both");
        def.gameMode = ParseGameModeType(gameModeStr);
        
        // ビジュアル
        if (j.contains("visual")) {
            const auto& visual = j["visual"];
            
            // スプライト
            if (visual.contains("sprite")) {
                const auto& sprite = visual["sprite"];
                def.visual.sprite.atlasPath = GetOr<std::string>(sprite, "atlasPath", "");
                def.visual.sprite.jsonPath = GetOr<std::string>(sprite, "jsonPath", "");
            }
            
            // アニメーション
            if (visual.contains("animations")) {
                const auto& animations = visual["animations"];
                for (const auto& [name, animJson] : animations.items()) {
                    AnimationDef animDef;
                    animDef.name = name;
                    
                    if (animJson.contains("frames") && animJson["frames"].is_array()) {
                        for (const auto& frameJson : animJson["frames"]) {
                            FrameDef frame;
                            if (frameJson.is_string()) {
                                frame.index = 0; // TODO: フレーム名からインデックス解決
                                frame.tag = frameJson.get<std::string>();
                            } else {
                                frame.index = GetOr(frameJson, "index", 0);
                                frame.duration = GetOr(frameJson, "duration", 0.1f);
                                frame.tag = GetOr<std::string>(frameJson, "tag", "");
                            }
                            animDef.frames.push_back(frame);
                        }
                    }
                    
                    animDef.loop = GetOr(animJson, "loop", true);
                    animDef.nextAnimation = GetOr<std::string>(animJson, "nextAnimation", "");
                    def.visual.animations[name] = std::move(animDef);
                }
            }
            
            def.visual.defaultAnimation = GetOr<std::string>(visual, "defaultAnimation", "idle");
        }
        
        // ステータス
        if (j.contains("stats")) {
            const auto& stats = j["stats"];
            def.stats.hp = GetOr(stats, "hp", 100.0f);
            def.stats.attack = GetOr(stats, "attack", 15.0f);
            def.stats.defense = GetOr(stats, "defense", 5.0f);
            def.stats.moveSpeed = GetOr(stats, "moveSpeed", 120.0f);
        }
        
        // 戦闘
        if (j.contains("combat")) {
            const auto& combat = j["combat"];
            def.combat.attackType = ParseAttackType(GetOr<std::string>(combat, "attackType", "single"));
            def.combat.attackRange = GetOr(combat, "attackRange", 50.0f);
            def.combat.attackCooldown = GetOr(combat, "attackCooldown", 1.5f);
            
            if (combat.contains("hitbox")) {
                def.combat.hitbox = ParseRect(combat["hitbox"]);
            }
            
            if (combat.contains("attackRangeArea")) {
                def.combat.attackRangeArea = ParseRect(combat["attackRangeArea"]);
            }
            
            def.combat.criticalChance = GetOr(combat, "criticalChance", 0.0f);
            def.combat.criticalMultiplier = GetOr(combat, "criticalMultiplier", 1.5f);
        }
        
        // TD固有設定
        if (j.contains("td")) {
            const auto& td = j["td"];
            def.td.cost = GetOr(td, "cost", 100.0f);
            def.td.rechargeTime = GetOr(td, "rechargeTime", 3.0f);
            def.td.laneMovement = GetOr(td, "laneMovement", true);
            def.td.laneSpeed = GetOr(td, "laneSpeed", 50.0f);
        }
        
        // Roguelike固有設定
        if (j.contains("roguelike")) {
            const auto& roguelike = j["roguelike"];
            def.roguelike.gridMovement = GetOr(roguelike, "gridMovement", true);
            def.roguelike.turnBased = GetOr(roguelike, "turnBased", true);
            def.roguelike.movementCost = GetOr(roguelike, "movementCost", 1);
        }
        
        // スキル
        if (j.contains("skillIds") && j["skillIds"].is_array()) {
            for (const auto& skillId : j["skillIds"]) {
                def.skillIds.push_back(skillId.get<std::string>());
            }
        }
        
        // 成長率
        def.healthGrowth = GetOr(j, "healthGrowth", 1.1f);
        def.attackGrowth = GetOr(j, "attackGrowth", 1.1f);
        
        return def;
    }

private:
    template<typename T>
    T GetOr(const json& j, const std::string& key, T defaultValue) {
        if (j.contains(key) && !j[key].is_null()) {
            try {
                return j[key].get<T>();
            } catch (...) {
                return defaultValue;
            }
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
        if (s == "super_rare" || s == "superRare") return Rarity::SuperRare;
        if (s == "uber_rare" || s == "uberRare") return Rarity::UberRare;
        if (s == "legend") return Rarity::Legend;
        return Rarity::Normal;
    }
    
    AttackType ParseAttackType(const std::string& s) {
        if (s == "single") return AttackType::Single;
        if (s == "area") return AttackType::Area;
        if (s == "wave") return AttackType::Wave;
        return AttackType::Single;
    }
    
    GameModeType ParseGameModeType(const std::string& s) {
        if (s == "td") return GameModeType::TD;
        if (s == "roguelike") return GameModeType::Roguelike;
        if (s == "both") return GameModeType::Both;
        return GameModeType::Both;
    }
};

} // namespace Data

