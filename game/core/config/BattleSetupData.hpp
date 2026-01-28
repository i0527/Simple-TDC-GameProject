#pragma once

// 標準ライブラリ
#include <string>
#include <utility>
#include <vector>

// プロジェクト内
#include "../game/WaveLoader.hpp"

namespace game {
namespace core {

/// @brief カスタムステージの敵キューエントリ
struct CustomEnemyEntry {
    std::string enemyId;    // キャラクターID
    int level;              // レベル（1～保有レベル）
    float spawnDelay;       // 前のエントリからの遅延（秒）
    
    CustomEnemyEntry() : enemyId(""), level(1), spawnDelay(1.0f) {}
};

/// @brief カスタムステージの敵キュー
struct CustomEnemyQueue {
    std::vector<CustomEnemyEntry> queue;
    bool isValid = false;
    
    CustomEnemyQueue() : isValid(false) {}
};

/// @brief 戦闘セットアップ用データ
struct BattleSetupData {
    struct LaneConfig {
        float y = 360.0f;
        float startX = 120.0f;
        float endX = 1800.0f;
        float minGap = 72.0f;
    };

    struct TowerState {
        int currentHp = 1000;
        int maxHp = 1000;
        float x = 0.0f;
        float y = 0.0f;
        float width = 140.0f;
        float height = 260.0f;
    };

    bool hasValidStage = false;
    std::string stageId;
    std::string stageName;
    std::string gameStateText = "Battle";

    LaneConfig lane;
    TowerState playerTower;
    TowerState enemyTower;

    int currentWave = 1;
    int totalWaves = 1;
    std::vector<::game::core::game::SpawnEvent> spawnSchedule;

    int gold = 500;
    int goldMaxCap = 2000;
    float goldMaxCurrent = 2000.0f;
    float goldMaxGrowthPerSecond = 60.0f;
    float goldRegenPerSecond = 10.0f;

    float gameSpeed = 1.0f;
    bool isPaused = false;

    std::vector<std::pair<int, std::string>> formationSlots;
    
    CustomEnemyQueue customQueue;
};

} // namespace core
} // namespace game
