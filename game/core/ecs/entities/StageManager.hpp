#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

namespace game {
namespace core {
namespace entities {

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
    int waveCount;               // ウェーブ数
    int recommendedLevel;        // 推奨レベル
    std::string previewImageId;  // プレビュー画像ID
    std::vector<std::string> unlockOnClear; // クリア時に解放されるステージID
    nlohmann::json data;         // 元のJSONデータ（ゲームロジック用）
    
    // デフォルトコンストラクタ
    StageData()
        : id(""), stageNumber(0), chapter(0), chapterName(""), stageName(""),
          difficulty(1), starsEarned(0), isCleared(false), isLocked(true),
          isBoss(false), rewardGold(0), waveCount(0), recommendedLevel(1),
          previewImageId(""), unlockOnClear() {}
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
