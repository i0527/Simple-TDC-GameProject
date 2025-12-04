/**
 * @file StageDef.h
 * @brief ステージ定義構造体
 * 
 * TDゲーム用のステージ定義
 */
#pragma once

#include <string>
#include <vector>
#include "Data/Definitions/CommonTypes.h"

namespace Data {

/**
 * @brief Wave内の敵出現エントリ
 */
struct EnemySpawnEntry {
    std::string characterId;
    int count = 1;
    float delay = 0.0f;          // Wave開始からの遅延
    float interval = 1.0f;       // 複数体の出現間隔
    int lane = 0;                // 出現レーン
};

/**
 * @brief Wave定義
 */
struct WaveDef {
    int waveNumber = 0;
    std::vector<EnemySpawnEntry> enemies;
    float duration = 30.0f;       // Wave最大継続時間
    std::string triggerCondition; // 次Wave発動条件（"time", "all_dead"）
};

/**
 * @brief ステージ定義
 */
struct StageDef {
    std::string id;
    std::string name;
    std::string description;
    std::string backgroundPath;
    
    // Wave構成
    std::vector<WaveDef> waves;
    
    // 勝利条件
    float baseHealth = 1000.0f;      // 味方ベースHP
    float enemyBaseHealth = 1000.0f; // 敵ベースHP
    float timeLimit = 0.0f;          // 制限時間（0=無制限）
    
    // 報酬
    int clearReward = 100;           // クリア報酬
    int firstClearBonus = 50;        // 初回クリアボーナス
    std::vector<std::string> dropCharacterIds;  // ドロップするキャラID
    
    // コスト
    float startingCost = 500.0f;     // 開始時コスト
    float costRegenRate = 10.0f;     // コスト回復速度（/秒）
    float maxCost = 9999.0f;         // 最大コスト
    
    // レーン設定
    int laneCount = 1;               // レーン数
    float laneHeight = 100.0f;       // 各レーンの高さ
    
    // 難易度調整
    float enemyHealthMultiplier = 1.0f;
    float enemyAttackMultiplier = 1.0f;
};

} // namespace Data

