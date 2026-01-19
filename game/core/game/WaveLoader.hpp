#pragma once

// 標準ライブラリ
#include <string>
#include <unordered_map>
#include <vector>

// 外部ライブラリ
#include <nlohmann/json.hpp>

namespace game {
namespace core {
namespace game {

/// @brief 1回のスポーンイベント（時刻はバトル開始からの秒）
struct SpawnEvent {
    float time = 0.0f;
    std::string enemyId;
    int lane = 0;
    int level = 1;
};

/// @brief ステージ定義（data/stages.json）の waves/wave_ids を解釈し、スポーンイベントに正規化する
class WaveLoader {
public:
    WaveLoader() = default;
    ~WaveLoader() = default;

    /// @brief ステージJSONからスポーンイベントを生成
    /// @param stageData StageManager::StageData::data など（nlohmann::json）
    /// @return スポーンイベント列（time昇順）
    std::vector<SpawnEvent> LoadStageSpawnEvents(const nlohmann::json& stageData);

private:
    // wave_id -> (spawn events relative to wave start)
    std::unordered_map<std::string, std::vector<SpawnEvent>> waveCache_;

    // waveファイルを検索してキャッシュ化
    void EnsureWaveCacheLoaded();
    bool LoadWaveFile(const std::string& path);

    // stageData["waves"] がオブジェクト配列の場合の解釈
    std::vector<SpawnEvent> LoadInlineWaves(const nlohmann::json& wavesArray, int defaultLevel);

    // stageData["waves"] / stageData["wave_ids"] が文字列配列の場合の解釈
    std::vector<SpawnEvent> LoadWaveIdList(const nlohmann::json& waveIdArray, int defaultLevel);

    static void SortByTime(std::vector<SpawnEvent>& events);
};

} // namespace game
} // namespace core
} // namespace game

