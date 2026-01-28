#include "StageLoader.hpp"

// 標準ライブラリ
#include <algorithm>
#include <fstream>
#include <set>
#include <vector>

// 外部ライブラリ
#include <nlohmann/json.hpp>

// プロジェクト�E
#include "../../../utils/Log.h"

using json = nlohmann::json;

namespace game {
namespace core {
namespace entities {

bool StageLoader::LoadFromJSON(
    const std::string& json_path,
    std::unordered_map<std::string, StageData>& outStages,
    std::unordered_map<int, std::string>& outStageNumberToId) {
    outStages.clear();
    outStageNumberToId.clear();

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

            // IDの取得（文字�Eまた�E数値�E�E
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
                // IDがなぁE��合�EスキチE�E�E�デバッグ用スチE�EジなどはID忁E��！E
                LOG_WARN("Stage missing 'id' field, skipping");
                continue;
            }

            // stageNumberを設定！ESONに含まれてぁE��ば使用、なければIDから推測�E�E
            if (stage_json.contains("stageNumber") &&
                stage_json["stageNumber"].is_number()) {
                stage.stageNumber = stage_json["stageNumber"].get<int>();
            } else {
                // IDが数値の場合�Eそれを使用、そぁE��なければスキチE�E
                try {
                    int idAsInt = std::stoi(stage.id);
                    stage.stageNumber = idAsInt;
                } catch (...) {
                    // IDが数値でなぁE��合（侁E "stage_debug"�E��EstageNumberを設定しなぁE
                    // こ�EようなスチE�EジはstageNumberToId_マッピングに含めなぁE
                    LOG_DEBUG("Stage {} has non-numeric ID, skipping stageNumber mapping",
                              stage.id);
                    stage.stageNumber = 0; // 無効な値として設宁E
                }
            }

            // stageNumber -> id のマッピングを保存！EtageNumberが有効な場合�Eみ�E�E
            // 同じstageNumberが既に存在する場合�E警告を出してスキチE�E
            if (stage.stageNumber > 0) {
                if (outStageNumberToId.find(stage.stageNumber) !=
                    outStageNumberToId.end()) {
                    LOG_WARN(
                        "Duplicate stageNumber {} found: existing ID '{}', new ID "
                        "'{}'. Keeping existing.",
                        stage.stageNumber, outStageNumberToId[stage.stageNumber],
                        stage.id);
                    // 既存�Eマッピングを保持し、新しいも�Eは無要E
                } else {
                    outStageNumberToId[stage.stageNumber] = stage.id;
                }
            }

            // チャプター惁E���E�ESONに含まれてぁE��ば使用�E�E
            if (stage_json.contains("chapter") &&
                stage_json["chapter"].is_number()) {
                stage.chapter = stage_json["chapter"].get<int>();
            } else {
                // stageNumberから自動計算！E-4: Chapter 1, 5-8: Chapter 2, 9-12: Chapter 3�E�E
                stage.chapter = ((stage.stageNumber - 1) / 4) + 1;
            }

            // チャプター吁E
            if (stage_json.contains("chapterName") &&
                stage_json["chapterName"].is_string()) {
                stage.chapterName = stage_json["chapterName"].get<std::string>();
            } else {
                // チE��ォルト�Eチャプター吁E
                switch (stage.chapter) {
                case 1:
                    stage.chapterName = "Chapter 1: 城壁�E王国";
                    break;
                case 2:
                    stage.chapterName = "Chapter 2: 魔法学院";
                    break;
                case 3:
                    stage.chapterName = "Chapter 3: 竜の巣穴";
                    break;
                default:
                    stage.chapterName =
                        "Chapter " + std::to_string(stage.chapter);
                }
            }

            // スチE�Eジ吁E
            if (stage_json.contains("name") && stage_json["name"].is_string()) {
                stage.stageName = stage_json["name"].get<std::string>();
            } else if (stage_json.contains("stageName") &&
                       stage_json["stageName"].is_string()) {
                stage.stageName = stage_json["stageName"].get<std::string>();
            } else {
                // チE��ォルト�EスチE�Eジ吁E
                int stageInChapter = ((stage.stageNumber - 1) % 4) + 1;
                switch (stage.chapter) {
                case 1:
                    stage.stageName =
                        "砦への遁E" + std::to_string(stageInChapter);
                    break;
                case 2:
                    stage.stageName =
                        "学園�E試練 " + std::to_string(stageInChapter);
                    break;
                case 3:
                    stage.stageName =
                        "竜�E領域 " + std::to_string(stageInChapter);
                    break;
                default:
                    stage.stageName =
                        "Stage " + std::to_string(stage.stageNumber);
                }
            }

            // 難易度
            if (stage_json.contains("difficulty") &&
                stage_json["difficulty"].is_number()) {
                stage.difficulty = stage_json["difficulty"].get<int>();
            } else {
                // チE��ォルト計箁E chapter + (stageInChapter - 1) / 2
                int stageInChapter = ((stage.stageNumber - 1) % 4) + 1;
                stage.difficulty = stage.chapter + (stageInChapter - 1) / 2;
            }

            // そ�E他�EフィールチE
            stage.starsEarned = 0; // セーブデータから読み込む
            stage.isCleared = false; // セーブデータから読み込む

            // ロチE��状態！ESONに含まれてぁE��ば使用、なければ自動計算！E
            if (stage_json.contains("isLocked") &&
                stage_json["isLocked"].is_boolean()) {
                stage.isLocked = stage_json["isLocked"].get<bool>();
            } else {
                // チE��ォルチE 最初�EスチE�Eジ以外�EロチE��
                stage.isLocked = (stage.stageNumber > 1);
            }

            stage.isBoss = stage_json.value("isBoss", false);
            stage.rewardGold = stage_json.value("rewardGold", 100);
            stage.rewardTickets = stage_json.value("rewardTickets", 0);
            stage.waveCount = stage_json.value("waveCount", 5);
            stage.recommendedLevel = stage_json.value("recommendedLevel", 1);
            stage.previewImageId = stage_json.value("previewImageId", "");
            stage.unlockOnClear.clear();
            if (stage_json.contains("unlockOnClear") &&
                stage_json["unlockOnClear"].is_array()) {
                for (const auto& unlockId : stage_json["unlockOnClear"]) {
                    if (unlockId.is_string()) {
                        stage.unlockOnClear.push_back(unlockId.get<std::string>());
                    }
                }
            }
            
            // 拡張フィールドのパース（オプショナル）
            try {
                // ボーナス条件
                stage.bonusConditions.clear();
                if (stage_json.contains("bonusConditions") &&
                    stage_json["bonusConditions"].is_array()) {
                    for (const auto& bc : stage_json["bonusConditions"]) {
                        BonusCondition bonus;
                        if (bc.contains("description") && bc["description"].is_string()) {
                            bonus.description = bc["description"].get<std::string>();
                        }
                        if (bc.contains("conditionType") && bc["conditionType"].is_string()) {
                            bonus.conditionType = bc["conditionType"].get<std::string>();
                        }
                        if (bc.contains("conditionValue") && bc["conditionValue"].is_number()) {
                            bonus.conditionValue = bc["conditionValue"].get<int>();
                        }
                        if (bc.contains("conditionOperator") && bc["conditionOperator"].is_string()) {
                            bonus.conditionOperator = bc["conditionOperator"].get<std::string>();
                        }
                        if (bc.contains("rewardType") && bc["rewardType"].is_string()) {
                            bonus.rewardType = bc["rewardType"].get<std::string>();
                        }
                        if (bc.contains("rewardValue") && bc["rewardValue"].is_number()) {
                            bonus.rewardValue = bc["rewardValue"].get<int>();
                        }
                        stage.bonusConditions.push_back(bonus);
                    }
                }
                
                // 獲得モンスター
                stage.rewardMonsters.clear();
                if (stage_json.contains("rewardMonsters") &&
                    stage_json["rewardMonsters"].is_array()) {
                    for (const auto& rm : stage_json["rewardMonsters"]) {
                        RewardMonster reward;
                        if (rm.contains("monsterId") && rm["monsterId"].is_string()) {
                            reward.monsterId = rm["monsterId"].get<std::string>();
                        }
                        if (rm.contains("level") && rm["level"].is_number()) {
                            reward.level = rm["level"].get<int>();
                        }
                        stage.rewardMonsters.push_back(reward);
                    }
                }
                
                // 出現モンスター詳細
                stage.enemySpawns.clear();
                if (stage_json.contains("enemySpawns") &&
                    stage_json["enemySpawns"].is_array()) {
                    for (const auto& es : stage_json["enemySpawns"]) {
                        EnemySpawn spawn;
                        if (es.contains("monsterId") && es["monsterId"].is_string()) {
                            spawn.monsterId = es["monsterId"].get<std::string>();
                        }
                        if (es.contains("minLevel") && es["minLevel"].is_number()) {
                            spawn.minLevel = es["minLevel"].get<int>();
                        }
                        if (es.contains("maxLevel") && es["maxLevel"].is_number()) {
                            spawn.maxLevel = es["maxLevel"].get<int>();
                        }
                        if (es.contains("count") && es["count"].is_number()) {
                            spawn.count = es["count"].get<int>();
                        }
                        if (es.contains("spawnPattern") && es["spawnPattern"].is_string()) {
                            spawn.spawnPattern = es["spawnPattern"].get<std::string>();
                        }
                        stage.enemySpawns.push_back(spawn);
                    }
                }
                
                // ボス戦フェーズ情報
                stage.bossPhases.clear();
                if (stage_json.contains("bossPhases") &&
                    stage_json["bossPhases"].is_array()) {
                    for (const auto& bp : stage_json["bossPhases"]) {
                        BossPhase phase;
                        if (bp.contains("hpPercentMin") && bp["hpPercentMin"].is_number()) {
                            phase.hpPercentMin = bp["hpPercentMin"].get<int>();
                        }
                        if (bp.contains("hpPercentMax") && bp["hpPercentMax"].is_number()) {
                            phase.hpPercentMax = bp["hpPercentMax"].get<int>();
                        }
                        if (bp.contains("description") && bp["description"].is_string()) {
                            phase.description = bp["description"].get<std::string>();
                        }
                        if (bp.contains("actions") && bp["actions"].is_array()) {
                            for (const auto& action : bp["actions"]) {
                                if (action.is_string()) {
                                    phase.actions.push_back(action.get<std::string>());
                                }
                            }
                        }
                        stage.bossPhases.push_back(phase);
                    }
                }
                
                // 新機能フィールド
                stage.isInfinite = stage_json.value("isInfinite", false);
                stage.isCustom = stage_json.value("isCustom", false);
                stage.isTutorial = stage_json.value("isTutorial", false);
                stage.difficultyLevel = stage_json.value("difficultyLevel", 0);
                stage.allowGiveUp = stage_json.value("allowGiveUp", false);
                stage.rewardCharacterOnEveryClear = stage_json.value("rewardCharacterOnEveryClear", false);
            } catch (const std::exception& e) {
                LOG_WARN("StageLoader: Failed to parse extended fields for stage {}: {}", 
                         stage.id, e.what());
                // デフォルト値を使用（既に初期化済み）
            }
            
            stage.data = stage_json;

            outStages[stage.id] = stage;
        }

        LOG_INFO("StageLoader: Loaded {} stages from JSON", outStages.size());
        return true;
    } catch (const json::parse_error& e) {
        LOG_ERROR("StageLoader: JSON parse error: {}", e.what());
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("StageLoader: Exception: {}", e.what());
        return false;
    }
}

bool StageLoader::SaveToJSON(
    const std::string& json_path,
    const std::unordered_map<std::string, StageData>& stages) {
    try {
        std::vector<StageData> ordered;
        ordered.reserve(stages.size());
        for (const auto& pair : stages) {
            ordered.push_back(pair.second);
        }
        std::sort(ordered.begin(), ordered.end(),
                  [](const StageData& a, const StageData& b) {
                      const bool aHasNumber = a.stageNumber > 0;
                      const bool bHasNumber = b.stageNumber > 0;
                      if (aHasNumber != bHasNumber) {
                          return aHasNumber;
                      }
                      if (aHasNumber && bHasNumber && a.stageNumber != b.stageNumber) {
                          return a.stageNumber < b.stageNumber;
                      }
                      return a.id < b.id;
                  });

        json root = json::object();
        root["stages"] = json::array();
        for (const auto& stage : ordered) {
            json stageJson = stage.data.is_object() ? stage.data : json::object();
            stageJson["id"] = stage.id;
            stageJson["stageNumber"] = stage.stageNumber;
            stageJson["chapter"] = stage.chapter;
            stageJson["chapterName"] = stage.chapterName;
            stageJson["difficulty"] = stage.difficulty;
            stageJson["isBoss"] = stage.isBoss;
            stageJson["isLocked"] = stage.isLocked;
            stageJson["rewardGold"] = stage.rewardGold;
            stageJson["rewardTickets"] = stage.rewardTickets;
            stageJson["waveCount"] = stage.waveCount;
            stageJson["recommendedLevel"] = stage.recommendedLevel;
            stageJson["previewImageId"] = stage.previewImageId;
            stageJson["unlockOnClear"] = stage.unlockOnClear;

            if (stageJson.contains("stageName") && stageJson["stageName"].is_string()) {
                stageJson["stageName"] = stage.stageName;
            } else {
                stageJson["name"] = stage.stageName;
            }

            root["stages"].push_back(stageJson);
        }

        std::ofstream file(json_path);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open stage data file for write: {}", json_path);
            return false;
        }
        file << root.dump(2);
        return true;
    } catch (const json::parse_error& e) {
        LOG_ERROR("StageLoader: JSON parse error: {}", e.what());
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("StageLoader: Exception: {}", e.what());
        return false;
    }
}

void StageLoader::LoadDefault(
    std::unordered_map<std::string, StageData>& outStages,
    std::unordered_map<int, std::string>& outStageNumberToId) {
    outStages.clear();
    outStageNumberToId.clear();

    // Chapter 1: 城壁�E王国 (Stages 1-4)
    for (int i = 1; i <= 4; ++i) {
        StageData stage;
        stage.id = std::to_string(i);
        stage.stageNumber = i;
        stage.chapter = 1;
        stage.chapterName = "Chapter 1: 城壁�E王国";
        stage.stageName = "砦への遁E" + std::to_string(i);
        stage.difficulty = 1 + (i - 1) / 2; // 1,1,2,2
        stage.starsEarned = 0;
        stage.isCleared = false;
        stage.isLocked = (i > 1); // Stage 1以外�EロチE��
        stage.isBoss = (i == 4);
        stage.rewardGold = 100 * i;
        stage.rewardTickets = 0;
        stage.waveCount = 5;
        stage.recommendedLevel = 10 + (i - 1) * 5;
        stage.previewImageId = "";
        stage.unlockOnClear.clear();
        if (i < 4) {
            stage.unlockOnClear.push_back(std::to_string(i + 1));
        }

        // チE��ォルチESONチE�Eタ
        stage.data = json::object();
        stage.data["id"] = i;
        stage.data["waves"] = json::array();

        outStages[stage.id] = stage;
        outStageNumberToId[stage.stageNumber] = stage.id;
    }

    // Chapter 2: 魔法学院 (Stages 5-8)
    for (int i = 5; i <= 8; ++i) {
        StageData stage;
        stage.id = std::to_string(i);
        stage.stageNumber = i;
        stage.chapter = 2;
        stage.chapterName = "Chapter 2: 魔法学院";
        stage.stageName = "学園の試練 " + std::to_string(i - 4);
        stage.difficulty = 2 + (i - 5) / 2; // 2,2,3,3
        stage.starsEarned = 0;
        stage.isCleared = false;
        stage.isLocked = true;
        stage.isBoss = (i == 8);
        stage.rewardGold = 150 * (i - 4);
        stage.rewardTickets = 0;
        stage.waveCount = 5;
        stage.recommendedLevel = 25 + (i - 5) * 5;
        stage.previewImageId = "";
        stage.unlockOnClear.clear();
        if (i < 8) {
            stage.unlockOnClear.push_back(std::to_string(i + 1));
        }

        // チE��ォルチESONチE�Eタ
        stage.data = json::object();
        stage.data["id"] = i;
        stage.data["waves"] = json::array();

        outStages[stage.id] = stage;
        outStageNumberToId[stage.stageNumber] = stage.id;
    }

    // Chapter 3: 竜の巣穴 (Stages 9-12)
    for (int i = 9; i <= 12; ++i) {
        StageData stage;
        stage.id = std::to_string(i);
        stage.stageNumber = i;
        stage.chapter = 3;
        stage.chapterName = "Chapter 3: 竜の巣穴";
        stage.stageName = "竜の領域 " + std::to_string(i - 8);
        stage.difficulty = 3 + (i - 9) / 2; // 3,3,4,5
        stage.starsEarned = 0;
        stage.isCleared = false;
        stage.isLocked = true;
        stage.isBoss = (i == 12);
        stage.rewardGold = 200 * (i - 8);
        stage.rewardTickets = 0;
        stage.waveCount = 5;
        stage.recommendedLevel = 40 + (i - 9) * 5;
        stage.previewImageId = "";
        stage.unlockOnClear.clear();
        if (i < 12) {
            stage.unlockOnClear.push_back(std::to_string(i + 1));
        }

        // チE��ォルチESONチE�Eタ
        stage.data = json::object();
        stage.data["id"] = i;
        stage.data["waves"] = json::array();

        outStages[stage.id] = stage;
        outStageNumberToId[stage.stageNumber] = stage.id;
    }

    LOG_INFO("Initialized {} default stages", outStages.size());
}

} // namespace entities
} // namespace core
} // namespace game
