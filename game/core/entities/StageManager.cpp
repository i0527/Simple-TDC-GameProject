#include "StageManager.hpp"
#include "../../utils/Log.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <memory>
#include <set>
#include <algorithm>

using json = nlohmann::json;

namespace game {
namespace core {
namespace entities {

StageManager::StageManager() {
}

StageManager::~StageManager() {
    Shutdown();
}

bool StageManager::Initialize(const std::string& json_path) {
    stages_.clear();
    stageNumberToId_.clear();
    
    if (!json_path.empty()) {
        // JSON からロード
        if (LoadFromJSON(json_path)) {
            LOG_INFO("StageManager initialized with {} stages from JSON", stages_.size());
            return true;
        }
        // JSONロード失敗時は警告のみ
        LOG_WARN("Stage JSON load failed, initializing default stages");
    }
    
    // JSON読み込み失敗時またはパスが空の場合はデフォルトステージを初期化
    InitializeDefaultStages();
    LOG_INFO("StageManager initialized with {} default stages", stages_.size());
    return true;
}

bool StageManager::LoadFromJSON(const std::string& json_path) {
    try {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open stage data file: {}", json_path);
            return false;
        }

        json data;
        file >> data;

        if (!data.contains("stages") || !data["stages"].is_array()) {
            LOG_ERROR("Invalid stage JSON format: missing 'stages' array");
            return false;
        }

        for (const auto& stage_json : data["stages"]) {
            StageData stage;
            
            // IDの取得（文字列または数値）
            if (stage_json.contains("id")) {
                if (stage_json["id"].is_string()) {
                    stage.id = stage_json["id"].get<std::string>();
                } else if (stage_json["id"].is_number()) {
                    int idNum = stage_json["id"].get<int>();
                    stage.id = std::to_string(idNum);
                } else {
                    LOG_WARN("Invalid stage ID type, skipping stage");
                    continue;
                }
            } else {
                // IDがない場合はスキップ（デバッグ用ステージなどはID必須）
                LOG_WARN("Stage missing 'id' field, skipping");
                continue;
            }

            // stageNumberを設定（JSONに含まれていれば使用、なければIDから推測）
            if (stage_json.contains("stageNumber") && stage_json["stageNumber"].is_number()) {
                stage.stageNumber = stage_json["stageNumber"].get<int>();
            } else {
                // IDが数値の場合はそれを使用、そうでなければスキップ
                try {
                    int idAsInt = std::stoi(stage.id);
                    stage.stageNumber = idAsInt;
                } catch (...) {
                    // IDが数値でない場合（例: "stage_debug"）はstageNumberを設定しない
                    // このようなステージはstageNumberToId_マッピングに含めない
                    LOG_DEBUG("Stage {} has non-numeric ID, skipping stageNumber mapping", stage.id);
                    stage.stageNumber = 0;  // 無効な値として設定
                }
            }
            
            // stageNumber -> id のマッピングを保存（stageNumberが有効な場合のみ）
            // 同じstageNumberが既に存在する場合は警告を出してスキップ
            if (stage.stageNumber > 0) {
                if (stageNumberToId_.find(stage.stageNumber) != stageNumberToId_.end()) {
                    LOG_WARN("Duplicate stageNumber {} found: existing ID '{}', new ID '{}'. Keeping existing.", 
                             stage.stageNumber, stageNumberToId_[stage.stageNumber], stage.id);
                    // 既存のマッピングを保持し、新しいものは無視
                } else {
                    stageNumberToId_[stage.stageNumber] = stage.id;
                }
            }

            // チャプター情報（JSONに含まれていれば使用）
            if (stage_json.contains("chapter") && stage_json["chapter"].is_number()) {
                stage.chapter = stage_json["chapter"].get<int>();
            } else {
                // stageNumberから自動計算（1-4: Chapter 1, 5-8: Chapter 2, 9-12: Chapter 3）
                stage.chapter = ((stage.stageNumber - 1) / 4) + 1;
            }
            
            // チャプター名
            if (stage_json.contains("chapterName") && stage_json["chapterName"].is_string()) {
                stage.chapterName = stage_json["chapterName"].get<std::string>();
            } else {
                // デフォルトのチャプター名
                switch (stage.chapter) {
                    case 1: stage.chapterName = "Chapter 1: 城壁の王国"; break;
                    case 2: stage.chapterName = "Chapter 2: 魔法学園"; break;
                    case 3: stage.chapterName = "Chapter 3: 竜の巣窟"; break;
                    default: stage.chapterName = "Chapter " + std::to_string(stage.chapter);
                }
            }

            // ステージ名
            if (stage_json.contains("name") && stage_json["name"].is_string()) {
                stage.stageName = stage_json["name"].get<std::string>();
            } else if (stage_json.contains("stageName") && stage_json["stageName"].is_string()) {
                stage.stageName = stage_json["stageName"].get<std::string>();
            } else {
                // デフォルトのステージ名
                int stageInChapter = ((stage.stageNumber - 1) % 4) + 1;
                switch (stage.chapter) {
                    case 1: stage.stageName = "砦への道 " + std::to_string(stageInChapter); break;
                    case 2: stage.stageName = "学園の試練 " + std::to_string(stageInChapter); break;
                    case 3: stage.stageName = "竜の領域 " + std::to_string(stageInChapter); break;
                    default: stage.stageName = "Stage " + std::to_string(stage.stageNumber);
                }
            }

            // 難易度
            if (stage_json.contains("difficulty") && stage_json["difficulty"].is_number()) {
                stage.difficulty = stage_json["difficulty"].get<int>();
            } else {
                // デフォルト計算: chapter + (stageInChapter - 1) / 2
                int stageInChapter = ((stage.stageNumber - 1) % 4) + 1;
                stage.difficulty = stage.chapter + (stageInChapter - 1) / 2;
            }

            // その他のフィールド
            stage.starsEarned = 0;  // セーブデータから読み込む
            stage.isCleared = false;  // セーブデータから読み込む
            
            // ロック状態（JSONに含まれていれば使用、なければ自動計算）
            if (stage_json.contains("isLocked") && stage_json["isLocked"].is_boolean()) {
                stage.isLocked = stage_json["isLocked"].get<bool>();
            } else {
                // デフォルト: 最初のステージ以外はロック
                stage.isLocked = (stage.stageNumber > 1);
            }
            
            // ボスステージ判定（JSONに含まれていれば使用、なければ自動計算）
            if (stage_json.contains("isBoss") && stage_json["isBoss"].is_boolean()) {
                stage.isBoss = stage_json["isBoss"].get<bool>();
            } else {
                // デフォルト: 各チャプターの4番目のステージ
                stage.isBoss = (stage.stageNumber % 4 == 0);
            }
            
            // 報酬ゴールド（rewardGoldを優先、なければrewards.gold、それもなければデフォルト）
            if (stage_json.contains("rewardGold") && stage_json["rewardGold"].is_number()) {
                stage.rewardGold = stage_json["rewardGold"].get<int>();
            } else if (stage_json.contains("rewards") && stage_json["rewards"].is_object()) {
                auto& rewards = stage_json["rewards"];
                if (rewards.contains("gold") && rewards["gold"].is_number()) {
                    stage.rewardGold = rewards["gold"].get<int>();
                } else {
                    stage.rewardGold = 100 * stage.stageNumber;
                }
            } else {
                stage.rewardGold = 100 * stage.stageNumber;
            }
            
            // ウェーブ数
            if (stage_json.contains("waves") && stage_json["waves"].is_array()) {
                stage.waveCount = static_cast<int>(stage_json["waves"].size());
            } else if (stage_json.contains("wave_ids") && stage_json["wave_ids"].is_array()) {
                stage.waveCount = static_cast<int>(stage_json["wave_ids"].size());
            } else {
                stage.waveCount = 5;  // デフォルト
            }
            
            // 推奨レベル
            if (stage_json.contains("recommendedLevel") && stage_json["recommendedLevel"].is_number()) {
                stage.recommendedLevel = stage_json["recommendedLevel"].get<int>();
            } else {
                stage.recommendedLevel = 10 + (stage.stageNumber - 1) * 5;
            }
            
            // プレビュー画像ID
            if (stage_json.contains("previewImageId") && stage_json["previewImageId"].is_string()) {
                stage.previewImageId = stage_json["previewImageId"].get<std::string>();
            } else {
                stage.previewImageId = "";
            }

            // 元のJSONデータを保持
            stage.data = stage_json;

            stages_[stage.id] = stage;
        }
        
        LOG_INFO("Loaded {} stages from JSON", stages_.size());
        return true;
    } catch (const json::parse_error& e) {
        LOG_ERROR("JSON parse error: {}", e.what());
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse stage data: {}", e.what());
        return false;
    }
}

std::shared_ptr<StageData> StageManager::GetStage(const std::string& stage_id) {
    return GetStageDataById(stage_id);
}

std::shared_ptr<StageData> StageManager::GetStageDataById(const std::string& stage_id) {
    auto it = stages_.find(stage_id);
    if (it == stages_.end()) {
        LOG_WARN("Stage not found: {}", stage_id);
        return nullptr;
    }
    
    // マスターデータをコピーして返す
    return std::make_shared<StageData>(it->second);
}

std::shared_ptr<StageData> StageManager::GetStageData(int stageNumber) {
    auto it = stageNumberToId_.find(stageNumber);
    if (it == stageNumberToId_.end()) {
        LOG_WARN("Stage not found for stageNumber: {}", stageNumber);
        return nullptr;
    }
    
    return GetStageDataById(it->second);
}

std::vector<StageData> StageManager::GetAllStageData() const {
    std::vector<StageData> result;
    result.reserve(stages_.size());
    
    // stageNumberでソートして返す（重複を避けるため、stageNumberToId_マッピングを使用）
    std::set<int> processedStageNumbers;
    
    // まず、stageNumberToId_マッピングに含まれるステージを追加（重複なし）
    for (const auto& pair : stageNumberToId_) {
        int stageNumber = pair.first;
        const std::string& id = pair.second;
        auto it = stages_.find(id);
        if (it != stages_.end()) {
            result.push_back(it->second);
            processedStageNumbers.insert(stageNumber);
        }
    }
    
    // 次に、stageNumberToId_に含まれていないステージを追加（デバッグ用ステージなど）
    bool debugStageAdded = false;  // デバッグ用ステージは1つだけ追加
    for (const auto& pair : stages_) {
        const std::string& id = pair.first;
        const StageData& stage = pair.second;
        if (stage.stageNumber > 0 && processedStageNumbers.find(stage.stageNumber) == processedStageNumbers.end()) {
            // stageNumberが有効で、まだ処理されていない場合
            result.push_back(stage);
            processedStageNumbers.insert(stage.stageNumber);
        } else if (stage.stageNumber == 0 && !debugStageAdded) {
            // stageNumberが無効な場合（デバッグ用ステージなど）は、最初の1つだけ追加
            result.push_back(stage);
            debugStageAdded = true;  // 1つ追加したらフラグを立てる
        }
    }
    
    // stageNumberでソート（stageNumberが0のものは最後に）
    std::sort(result.begin(), result.end(), 
              [](const StageData& a, const StageData& b) {
                  if (a.stageNumber == 0 && b.stageNumber == 0) {
                      return a.id < b.id;  // 両方ともstageNumberが0の場合はIDでソート
                  }
                  if (a.stageNumber == 0) return false;  // aが0の場合は後ろに
                  if (b.stageNumber == 0) return true;   // bが0の場合は後ろに
                  return a.stageNumber < b.stageNumber;
              });
    
    return result;
}

std::vector<std::string> StageManager::GetAllStageIds() const {
    std::vector<std::string> ids;
    for (const auto& pair : stages_) {
        ids.push_back(pair.first);
    }
    return ids;
}

bool StageManager::HasStage(const std::string& stage_id) const {
    return stages_.find(stage_id) != stages_.end();
}

void StageManager::InitializeDefaultStages() {
    // このメソッドはJSON読み込み失敗時のフォールバックとして使用されます。
    // 通常はJSONファイルからステージデータを読み込みます。
    stages_.clear();
    stageNumberToId_.clear();
    
    // Chapter 1: 城壁の王国 (Stages 1-4)
    for (int i = 1; i <= 4; ++i) {
        StageData stage;
        stage.id = std::to_string(i);
        stage.stageNumber = i;
        stage.chapter = 1;
        stage.chapterName = "Chapter 1: 城壁の王国";
        stage.stageName = "砦への道 " + std::to_string(i);
        stage.difficulty = 1 + (i - 1) / 2; // 1,1,2,2
        stage.starsEarned = 0;
        stage.isCleared = false;
        stage.isLocked = (i > 1); // Stage 1以外はロック
        stage.isBoss = (i == 4);
        stage.rewardGold = 100 * i;
        stage.waveCount = 5;
        stage.recommendedLevel = 10 + (i - 1) * 5;
        stage.previewImageId = "";
        
        // デフォルトJSONデータ
        stage.data = json::object();
        stage.data["id"] = i;
        stage.data["waves"] = json::array();
        
        stages_[stage.id] = stage;
        stageNumberToId_[stage.stageNumber] = stage.id;
    }

    // Chapter 2: 魔法学園 (Stages 5-8)
    for (int i = 5; i <= 8; ++i) {
        StageData stage;
        stage.id = std::to_string(i);
        stage.stageNumber = i;
        stage.chapter = 2;
        stage.chapterName = "Chapter 2: 魔法学園";
        stage.stageName = "学園の試練 " + std::to_string(i - 4);
        stage.difficulty = 2 + (i - 5) / 2; // 2,2,3,3
        stage.starsEarned = 0;
        stage.isCleared = false;
        stage.isLocked = true;
        stage.isBoss = (i == 8);
        stage.rewardGold = 150 * (i - 4);
        stage.waveCount = 5;
        stage.recommendedLevel = 25 + (i - 5) * 5;
        stage.previewImageId = "";
        
        // デフォルトJSONデータ
        stage.data = json::object();
        stage.data["id"] = i;
        stage.data["waves"] = json::array();
        
        stages_[stage.id] = stage;
        stageNumberToId_[stage.stageNumber] = stage.id;
    }

    // Chapter 3: 竜の巣窟 (Stages 9-12)
    for (int i = 9; i <= 12; ++i) {
        StageData stage;
        stage.id = std::to_string(i);
        stage.stageNumber = i;
        stage.chapter = 3;
        stage.chapterName = "Chapter 3: 竜の巣窟";
        stage.stageName = "竜の領域 " + std::to_string(i - 8);
        stage.difficulty = 3 + (i - 9) / 2; // 3,3,4,5
        stage.starsEarned = 0;
        stage.isCleared = false;
        stage.isLocked = true;
        stage.isBoss = (i == 12);
        stage.rewardGold = 200 * (i - 8);
        stage.waveCount = 5;
        stage.recommendedLevel = 40 + (i - 9) * 5;
        stage.previewImageId = "";
        
        // デフォルトJSONデータ
        stage.data = json::object();
        stage.data["id"] = i;
        stage.data["waves"] = json::array();
        
        stages_[stage.id] = stage;
        stageNumberToId_[stage.stageNumber] = stage.id;
    }
    
    LOG_INFO("Initialized {} default stages", stages_.size());
}

void StageManager::Shutdown() {
    stages_.clear();
    stageNumberToId_.clear();
}

} // namespace entities
} // namespace core
} // namespace game
