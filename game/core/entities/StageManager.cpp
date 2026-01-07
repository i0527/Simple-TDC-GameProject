#include "StageManager.hpp"
#include "../../utils/Log.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <memory>

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
    if (!json_path.empty()) {
        // JSON からロード
        if (LoadFromJSON(json_path)) {
            return true;
        }
        // JSONロード失敗時は警告のみ
        LOG_WARN("Stage JSON load failed");
    }
    
    return false;
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
            Stage stage;
            
            // IDの取得（文字列または数値）
            if (stage_json.contains("id")) {
                if (stage_json["id"].is_string()) {
                    stage.id = stage_json["id"].get<std::string>();
                } else if (stage_json["id"].is_number()) {
                    stage.id = std::to_string(stage_json["id"].get<int>());
                } else {
                    LOG_WARN("Invalid stage ID type, skipping stage");
                    continue;
                }
            } else {
                LOG_WARN("Stage missing 'id' field, skipping");
                continue;
            }

            // 名前と説明（オプション）
            stage.name = stage_json.value("name", "");
            stage.description = stage_json.value("description", "");

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

std::shared_ptr<Stage> StageManager::GetStage(const std::string& stage_id) {
    auto it = stages_.find(stage_id);
    if (it == stages_.end()) {
        LOG_WARN("Stage not found: {}", stage_id);
        return nullptr;
    }
    
    // マスターデータをコピーして返す
    return std::make_shared<Stage>(it->second);
}

std::vector<std::string> StageManager::GetAllStageIds() const {
    std::vector<std::string> ids;
    for (const auto& [id, _] : stages_) {
        ids.push_back(id);
    }
    return ids;
}

bool StageManager::HasStage(const std::string& stage_id) const {
    return stages_.find(stage_id) != stages_.end();
}

void StageManager::Shutdown() {
    stages_.clear();
}

} // namespace entities
} // namespace core
} // namespace game
