/**
 * @file StageLoader.h
 * @brief ステージ定義ローダー
 * 
 * JSONからステージ定義を読み込み、DefinitionRegistryに登録する
 */
#pragma once

#include "Data/Loaders/DataLoaderBase.h"
#include "Data/Definitions/StageDef.h"
#include <nlohmann/json.hpp>

namespace Data {

/**
 * @brief ステージ定義ローダー
 */
class StageLoader : public DataLoaderBase {
public:
    explicit StageLoader(DefinitionRegistry& registry)
        : DataLoaderBase(registry) {}
    
    /**
     * @brief 単一のステージ定義ファイルを読み込み
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
    
    /**
     * @brief ディレクトリ内の全ステージ定義を読み込み
     */
    int LoadAllStages(const std::string& directoryPath) {
        return LoadDirectory(directoryPath, ".stage.json",
            [this](const std::string& path) { return LoadStage(path); });
    }
    
    /**
     * @brief JSONからStageDefをパース
     */
    StageDef ParseStageDef(const json& j) {
        StageDef def;
        
        def.id = GetOr<std::string>(j, "id", "");
        def.name = GetOr<std::string>(j, "name", def.id);
        def.description = GetOr<std::string>(j, "description", "");
        def.backgroundPath = GetOr<std::string>(j, "backgroundPath", "");
        
        // Waves
        if (j.contains("waves") && j["waves"].is_array()) {
            for (const auto& waveJson : j["waves"]) {
                WaveDef wave;
                wave.waveNumber = GetOr(waveJson, "waveNumber", 0);
                wave.duration = GetOr(waveJson, "duration", 30.0f);
                wave.triggerCondition = GetOr<std::string>(waveJson, "triggerCondition", "time");
                
                if (waveJson.contains("enemies") && waveJson["enemies"].is_array()) {
                    for (const auto& enemyJson : waveJson["enemies"]) {
                        EnemySpawnEntry entry;
                        entry.characterId = GetOr<std::string>(enemyJson, "characterId", "");
                        entry.count = GetOr(enemyJson, "count", 1);
                        entry.delay = GetOr(enemyJson, "delay", 0.0f);
                        entry.interval = GetOr(enemyJson, "interval", 1.0f);
                        entry.lane = GetOr(enemyJson, "lane", 0);
                        wave.enemies.push_back(entry);
                    }
                }
                
                def.waves.push_back(wave);
            }
        }
        
        // 勝利条件
        def.baseHealth = GetOr(j, "baseHealth", 1000.0f);
        def.enemyBaseHealth = GetOr(j, "enemyBaseHealth", 1000.0f);
        def.timeLimit = GetOr(j, "timeLimit", 0.0f);
        
        // 報酬
        def.clearReward = GetOr(j, "clearReward", 100);
        def.firstClearBonus = GetOr(j, "firstClearBonus", 50);
        
        if (j.contains("dropCharacterIds") && j["dropCharacterIds"].is_array()) {
            for (const auto& charId : j["dropCharacterIds"]) {
                def.dropCharacterIds.push_back(charId.get<std::string>());
            }
        }
        
        // コスト
        def.startingCost = GetOr(j, "startingCost", 500.0f);
        def.costRegenRate = GetOr(j, "costRegenRate", 10.0f);
        def.maxCost = GetOr(j, "maxCost", 9999.0f);
        
        // レーン設定
        def.laneCount = GetOr(j, "laneCount", 1);
        def.laneHeight = GetOr(j, "laneHeight", 100.0f);
        
        // 難易度調整
        def.enemyHealthMultiplier = GetOr(j, "enemyHealthMultiplier", 1.0f);
        def.enemyAttackMultiplier = GetOr(j, "enemyAttackMultiplier", 1.0f);
        
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
};

} // namespace Data

