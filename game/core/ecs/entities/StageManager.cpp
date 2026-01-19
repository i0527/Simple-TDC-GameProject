#include "StageManager.hpp"
#include "StageLoader.hpp"
#include "../../../utils/Log.h"
#include <algorithm>
#include <memory>
#include <set>

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
        // JSON からローチE
        if (StageLoader::LoadFromJSON(json_path, stages_, stageNumberToId_)) {
            LOG_INFO("StageManager initialized with {} stages from JSON", stages_.size());
            return true;
        }
        // JSONロード失敗時は警告�Eみ
        LOG_WARN("Stage JSON load failed, initializing default stages");
    }
    
    // JSON読み込み失敗時また�Eパスが空の場合�EチE��ォルトスチE�Eジを�E期化
    InitializeDefaultStages();
    LOG_INFO("StageManager initialized with {} default stages", stages_.size());
    return true;
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
    
    // マスターチE�Eタをコピ�Eして返す
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
    
    // stageNumberでソートして返す�E�重褁E��避けるため、stageNumberToId_マッピングを使用�E�E
    std::set<int> processedStageNumbers;
    
    // まず、stageNumberToId_マッピングに含まれるスチE�Eジを追加�E�重褁E��し！E
    for (const auto& pair : stageNumberToId_) {
        auto stageNumber = pair.first;
        const std::string& id = pair.second;
        auto it = stages_.find(id);
        if (it != stages_.end()) {
            result.push_back(it->second);
            processedStageNumbers.insert(stageNumber);
        }
    }
    
    // 次に、stageNumberToId_に含まれてぁE��ぁE��チE�Eジを追加�E�デバッグ用スチE�Eジなど�E�E
    bool debugStageAdded = false;  // チE��チE��用スチE�Eジは1つだけ追加
    for (const auto& pair : stages_) {
        const std::string& id = pair.first;
        const StageData& stage = pair.second;
        if (stage.stageNumber > 0 && processedStageNumbers.find(stage.stageNumber) == processedStageNumbers.end()) {
            // stageNumberが有効で、まだ処琁E��れてぁE��ぁE��吁E
            result.push_back(stage);
            processedStageNumbers.insert(stage.stageNumber);
        } else if (stage.stageNumber == 0 && !debugStageAdded) {
            // stageNumberが無効な場合（デバッグ用スチE�Eジなど�E��E、最初�E1つだけ追加
            result.push_back(stage);
            debugStageAdded = true;  // 1つ追加したらフラグを立てめE
        }
    }
    
    // stageNumberでソート！EtageNumberぁEのも�Eは最後に�E�E
    std::sort(result.begin(), result.end(), 
              [](const StageData& a, const StageData& b) {
                  if (a.stageNumber == 0 && b.stageNumber == 0) {
                      return a.id < b.id;  // 両方ともstageNumberぁEの場合�EIDでソーチE
                  }
                  if (a.stageNumber == 0) return false;  // aぁEの場合�E後ろに
                  if (b.stageNumber == 0) return true;   // bぁEの場合�E後ろに
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

void StageManager::SetMasters(const std::unordered_map<std::string, StageData>& stages) {
    stages_ = stages;
    stageNumberToId_.clear();
    for (const auto& pair : stages_) {
        const auto& stage = pair.second;
        if (stage.stageNumber <= 0) {
            continue;
        }
        if (stageNumberToId_.find(stage.stageNumber) != stageNumberToId_.end()) {
            LOG_WARN("Duplicate stageNumber {} found for ID '{}'",
                     stage.stageNumber, stage.id);
            continue;
        }
        stageNumberToId_[stage.stageNumber] = stage.id;
    }
}

void StageManager::InitializeDefaultStages() {
    stages_.clear();
    stageNumberToId_.clear();
    StageLoader::LoadDefault(stages_, stageNumberToId_);
}

void StageManager::Shutdown() {
    stages_.clear();
    stageNumberToId_.clear();
}

} // namespace entities
} // namespace core
} // namespace game
