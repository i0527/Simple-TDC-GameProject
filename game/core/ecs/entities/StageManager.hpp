#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

namespace game {
namespace core {
namespace entities {

/// @brief ボーナス条件
struct BonusCondition {
    std::string description;     // "3分以内クリア"
    std::string conditionType;   // "tower_hp_percent", "unit_count", "gold_spent", "clear_time"
    int conditionValue;          // 条件値
    std::string conditionOperator; // "gte" (以上), "lte" (以下), "eq" (等しい)
    std::string rewardType;      // "gold", "item" など
    int rewardValue;             // 報酬値
    
    BonusCondition() 
        : description(""), 
          conditionType(""), 
          conditionValue(0), 
          conditionOperator(""),
          rewardType("gold"), 
          rewardValue(0) {}
};

/// @brief 獲得モンスター情報
struct RewardMonster {
    std::string monsterId;    // "hatslime"
    int level;                // 獲得時のレベル
    
    RewardMonster() : monsterId(""), level(1) {}
};

/// @brief 出現モンスター詳細情報
struct EnemySpawn {
    std::string monsterId;
    int minLevel;
    int maxLevel;
    int count;
    std::string spawnPattern;  // "slow_sequence", "simultaneous", "fast" など
    
    EnemySpawn() : monsterId(""), minLevel(1), maxLevel(1), count(1), spawnPattern("slow_sequence") {}
};

/// @brief ボス戦フェーズ情報
struct BossPhase {
    int hpPercentMin;         // HP 下限 (%) (例: 70)
    int hpPercentMax;         // HP 上限 (%) (例: 100)
    std::string description;   // フェーズの説明
    std::vector<std::string> actions;  // 行動パターン
    
    BossPhase() : hpPercentMin(0), hpPercentMax(100), description("") {}
};

/// @brief 統合されたステージデータ構造
/// 
/// StageSelectOverlayとGameSceneの両方で使用されるステージ情報を保持します。
struct StageData {
    std::string id;              // JSONのid（文字列）
    int stageNumber;             // 表示用の番号（1-12など）
    int chapter;                 // チャプター番号（1-3）
    std::string chapterName;     // チャプター名
    std::string stageName;       // ステージ名
    int difficulty;              // 難易度（1-5）
    int starsEarned;             // 獲得スター数（0-3）
    bool isCleared;              // クリア済みか
    bool isLocked;               // ロックされているか
    bool isBoss;                 // ボスステージか
    int rewardGold;              // 報酬ゴールド
    int rewardTickets;           // 報酬チケット（毎回クリアで付与）
    int waveCount;               // ウェーブ数
    int recommendedLevel;        // 推奨レベル
    std::string previewImageId;  // プレビュー画像ID
    std::vector<std::string> unlockOnClear; // クリア時に解放されるステージID
    nlohmann::json data;         // 元のJSONデータ（ゲームロジック用）
    
    // 拡張フィールド（monster_system.md対応）
    std::vector<BonusCondition> bonusConditions;  // ボーナス条件
    std::vector<RewardMonster> rewardMonsters;    // 獲得モンスター
    std::vector<EnemySpawn> enemySpawns;          // 出現モンスター詳細
    std::vector<BossPhase> bossPhases;            // ボス戦フェーズ情報
    
    // 新機能フィールド
    bool isInfinite = false;                      // 無限ステージかどうか
    bool isCustom = false;                        // カスタムステージかどうか
    bool isTutorial = false;                      // チュートリアルステージかどうか
    int difficultyLevel = 0;                      // 無限ステージの難易度レベル（0=易、1=難）
    bool allowGiveUp = false;                     // ギブアップ可能かどうか
    bool rewardCharacterOnEveryClear = false;    // 毎回クリア時にキャラクター報酬を付与するか
    
    // デフォルトコンストラクタ
    StageData()
        : id(""), stageNumber(0), chapter(0), chapterName(""), stageName(""),
          difficulty(1), starsEarned(0), isCleared(false), isLocked(true),
          isBoss(false), rewardGold(0), rewardTickets(0), waveCount(0), recommendedLevel(1),
          previewImageId(""), unlockOnClear(),
          bonusConditions(), rewardMonsters(), enemySpawns(), bossPhases(),
          isInfinite(false), isCustom(false), isTutorial(false), difficultyLevel(0),
          allowGiveUp(false), rewardCharacterOnEveryClear(false) {}
};

// ステージマスターデータ管理
class StageManager {
public:
    StageManager();
    ~StageManager();

    // 初期化（JSON / ハードコードからデータロード）
    bool Initialize(const std::string& json_path = "");

    // マスターデータからステージを取得（後方互換性のため残す）
    std::shared_ptr<StageData> GetStage(const std::string& stage_id);
    
    // デフォルトステージデータを初期化（JSON読み込み失敗時のフォールバック）
    void InitializeDefaultStages();
    
    // 統合されたStageDataを取得（推奨）
    std::shared_ptr<StageData> GetStageDataById(const std::string& stage_id);
    
    // stageNumberで検索
    std::shared_ptr<StageData> GetStageData(int stageNumber);
    
    // 全ステージデータを取得
    std::vector<StageData> GetAllStageData() const;

    // 全ステージIDを取得
    std::vector<std::string> GetAllStageIds() const;

    // ステージ存在確認
    bool HasStage(const std::string& stage_id) const;

    // ステージ数
    size_t GetStageCount() const { return stages_.size(); }

    // 全マスターデータ取得（デバッグ用）
    const std::unordered_map<std::string, StageData>& GetAllStages() const {
        return stages_;
    }
    void SetMasters(const std::unordered_map<std::string, StageData>& stages);

    // 終了処理
    void Shutdown();

private:
    // マスターデータ（ID -> StageData）
    std::unordered_map<std::string, StageData> stages_;
    
    // stageNumber -> id のマッピング
    std::unordered_map<int, std::string> stageNumberToId_;

    // ロードは StageLoader に委譲
};

} // namespace entities
} // namespace core
} // namespace game
