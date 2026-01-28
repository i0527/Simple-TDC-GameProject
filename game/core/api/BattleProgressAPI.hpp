#pragma once

// 標準ライブラリ
#include <string>
#include <unordered_map>
#include <vector>

// プロジェクト内
#include "../config/BattleSetupData.hpp"
#include "../game/WaveLoader.hpp"

namespace game {
namespace core {

class SharedContext;
class ECSystemAPI;
class GameplayDataAPI;
class SetupAPI;
class SceneOverlayControlAPI;

namespace ui {
    struct BattleHUDAction;
}

/// @brief 戦闘進行管理API（Wave/Spawn/経済/勝敗/戦闘ロジック）
class BattleProgressAPI {
public:
    struct LaneConfig {
        float y = 360.0f;
        float startX = 120.0f;
        float endX = 1800.0f;
        float minGap = 72.0f;
    };

    struct TowerState {
        int currentHp = 1000;
        int maxHp = 1000;
        float x = 0.0f;       // タワー中心X
        float y = 0.0f;       // タワー底辺Y
        float width = 140.0f;
        float height = 260.0f;
    };

    enum class BattleResult {
        InProgress,
        Victory,
        Defeat
    };

    struct AttackLogEntry {
        float time = 0.0f;
        std::string attackerId;
        std::string targetId;
        int damage = 0;
        bool hit = false;
    };

    BattleProgressAPI();
    ~BattleProgressAPI() = default;

    bool Initialize(SharedContext* sharedContext);
    void InitializeFromStage();
    void InitializeFromSetupData(const BattleSetupData& data);

    void Update(float deltaTime);
    void HandleHUDAction(const ui::BattleHUDAction& action);

    // ========== 状態取得 ==========
    const LaneConfig& GetLane() const { return lane_; }
    const TowerState& GetPlayerTower() const { return playerTower_; }
    const TowerState& GetEnemyTower() const { return enemyTower_; }
    BattleResult GetBattleResult() const { return battleResult_; }
    float GetBattleTime() const { return battleTime_; }
    int GetGold() const { return gold_; }
    int GetGoldMaxCurrent() const;
    float GetGameSpeed() const { return gameSpeed_; }
    bool IsPaused() const { return isPaused_; }
    const std::string& GetGameStateText() const { return gameStateText_; }
    const std::unordered_map<std::string, float>& GetUnitCooldownUntil() const { return unitCooldownUntil_; }
    const std::vector<AttackLogEntry>& GetAttackLog() const { return attackLog_; }
    void ClearAttackLog() { attackLog_.clear(); }
    void SetAttackLogEnabled(bool enabled) { attackLogEnabled_ = enabled; }
    bool IsAttackLogEnabled() const { return attackLogEnabled_; }

    // ========== 戦闘統計情報 ==========
    struct BattleStats {
        int playerTowerHp = 0;
        int playerTowerMaxHp = 1000;
        int spawnedUnitCount = 0;
        int totalGoldSpent = 0;
        float clearTime = 0.0f;
    };
    BattleStats GetBattleStats() const;

    // ========== 状態操作 ==========
    void SetGameSpeed(float speed);
    void SetPaused(bool paused);
    
    // ========== 無限ステージ関連 ==========
    bool IsInfiniteStage() const { return isInfinite_; }
    void RequestGiveUp() { giveUpRequested_ = true; }
    bool HasGiveUpRequest() const { return giveUpRequested_; }
    float GetSurvivalTime() const { return survivalTime_; }
    int CalculateInfiniteReward(float survivalTime, int difficultyLevel) const;
    int GetCurrentWaveNumber() const { return currentWaveNumber_; }
    float GetEnemyStatMultiplier() const { return enemyStatMultiplier_; }

private:
    void UpdateBattle(float deltaTime);
    void CheckBattleEnd();

    SharedContext* sharedContext_;
    ECSystemAPI* ecsAPI_;
    GameplayDataAPI* gameplayDataAPI_;
    SetupAPI* setupAPI_;
    SceneOverlayControlAPI* sceneOverlayAPI_;

    LaneConfig lane_;
    TowerState playerTower_;
    TowerState enemyTower_;
    BattleResult battleResult_;

    float battleTime_;

    int currentWave_;
    int totalWaves_;
    std::vector<::game::core::game::SpawnEvent> spawnSchedule_;
    size_t spawnCursor_;

    int gold_;
    int goldMaxCap_;
    float goldMaxCurrent_;
    float goldMaxGrowthPerSecond_;
    float goldRegenPerSecond_;
    float goldRegenAccumulator_;
    float gameSpeed_;
    bool isPaused_;
    std::string gameStateText_;

    std::unordered_map<std::string, float> unitCooldownUntil_;
    std::unordered_map<std::string, std::string> enemyToCharacterId_;

    bool isInitialized_;
    bool attackLogEnabled_ = true;
    std::vector<AttackLogEntry> attackLog_;

    // 戦闘統計情報
    int spawnedUnitCount_ = 0;
    int totalGoldSpent_ = 0;
    
    // 無限ステージ関連
    bool isInfinite_ = false;
    bool giveUpRequested_ = false;
    float survivalTime_ = 0.0f;
    int difficultyLevel_ = 0;
    float waveTimer_ = 0.0f;
    int currentWaveNumber_ = 1;
    float enemyStatMultiplier_ = 1.0f;
    float enemySpawnRateMultiplier_ = 1.0f;
    float lastDifficultyUpdateTime_ = 0.0f;
    
    void UpdateInfiniteDifficulty(float deltaTime);
};

} // namespace core
} // namespace game
