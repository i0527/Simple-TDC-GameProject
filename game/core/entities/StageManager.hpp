#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

namespace game {
namespace core {
namespace entities {

// ステージ情報
struct Stage {
    std::string id;
    std::string name;
    std::string description;
    nlohmann::json data;  // 元のJSONデータを保持
};

// ステージマスターデータ管理
class StageManager {
public:
    StageManager();
    ~StageManager();

    // 初期化（JSON / ハードコードからデータロード）
    bool Initialize(const std::string& json_path = "");

    // マスターデータからステージを取得
    std::shared_ptr<Stage> GetStage(const std::string& stage_id);

    // 全ステージIDを取得
    std::vector<std::string> GetAllStageIds() const;

    // ステージ存在確認
    bool HasStage(const std::string& stage_id) const;

    // ステージ数
    size_t GetStageCount() const { return stages_.size(); }

    // 全マスターデータ取得（デバッグ用）
    const std::unordered_map<std::string, Stage>& GetAllStages() const {
        return stages_;
    }

    // 終了処理
    void Shutdown();

private:
    // マスターデータ（ID -> Stage）
    std::unordered_map<std::string, Stage> stages_;

    // JSONからデータロード
    bool LoadFromJSON(const std::string& json_path);
};

} // namespace entities
} // namespace core
} // namespace game
