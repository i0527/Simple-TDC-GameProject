/**
 * @file StageSerializer.h
 * @brief ステージ定義のJSONシリアライズ/デシリアライズ
 * 
 * Phase 4: REST API用のシリアライズ機能
 */
#pragma once

#include "Data/Definitions/StageDef.h"
#include <nlohmann/json.hpp>

namespace Data {

/**
 * @brief ステージ定義のシリアライザー
 */
class StageSerializer {
public:
    /**
     * @brief StageDefをJSONにシリアライズ
     */
    static nlohmann::json Serialize(const StageDef& def);
    
    /**
     * @brief JSONからStageDefをデシリアライズ
     */
    static StageDef Deserialize(const nlohmann::json& j);

private:
    static nlohmann::json SerializeEnemySpawnEntry(const EnemySpawnEntry& entry);
    static EnemySpawnEntry DeserializeEnemySpawnEntry(const nlohmann::json& j);
    static nlohmann::json SerializeWaveDef(const WaveDef& wave);
    static WaveDef DeserializeWaveDef(const nlohmann::json& j);
};

} // namespace Data

