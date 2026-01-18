#include "WaveLoader.hpp"

// 標準ライブラリ
#include <algorithm>
#include <fstream>
#include <set>

// プロジェクト内
#include "../../utils/Log.h"

namespace game {
namespace core {
namespace game {

namespace {
// waveの間に入れる固定ディレイ（最小実装）
constexpr float WAVE_GAP_SECONDS = 2.0f;

bool IsStringArray(const nlohmann::json& j) {
    return j.is_array() && std::all_of(j.begin(), j.end(), [](const auto& v) { return v.is_string(); });
}

bool IsObjectArray(const nlohmann::json& j) {
    return j.is_array() && std::all_of(j.begin(), j.end(), [](const auto& v) { return v.is_object(); });
}

} // namespace

std::vector<SpawnEvent> WaveLoader::LoadStageSpawnEvents(const nlohmann::json& stageData) {
    std::vector<SpawnEvent> result;

    try {
        // wave_ids 優先（存在する場合）
        if (stageData.contains("wave_ids") && IsStringArray(stageData["wave_ids"])) {
            EnsureWaveCacheLoaded();
            result = LoadWaveIdList(stageData["wave_ids"]);
            SortByTime(result);
            return result;
        }

        // waves: 文字列配列（waveID列）
        if (stageData.contains("waves") && IsStringArray(stageData["waves"])) {
            EnsureWaveCacheLoaded();
            result = LoadWaveIdList(stageData["waves"]);
            SortByTime(result);
            return result;
        }

        // waves: オブジェクト配列（インライン定義）
        if (stageData.contains("waves") && IsObjectArray(stageData["waves"])) {
            result = LoadInlineWaves(stageData["waves"]);
            SortByTime(result);
            return result;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("WaveLoader::LoadStageSpawnEvents error: {}", e.what());
    }

    return result;
}

void WaveLoader::EnsureWaveCacheLoaded() {
    if (!waveCache_.empty()) {
        return;
    }

    // 候補: 実行時の配置が不明なため、複数パスを試す
    // - data/stages.json からは wave_id が参照されるが、波データは assets/data/waves にあるケースがある
    // - ビルドスクリプトでどこにコピーされるかに依存するため、両方試す
    const std::vector<std::string> candidateFiles = {
        "assets/data/waves/debug.json",
        "assets/data/waves/debug2.json",
        "assets/data/waves/sample.json",
        "data/waves/debug.json",
        "data/waves/debug2.json",
        "data/waves/sample.json",
        // 定義フォルダ（開発用）
        "assets/data/definitions/waves/debug.json",
        "assets/data/definitions/waves/debug2.json",
        "assets/data/definitions/waves/sample.json",
    };

    std::set<std::string> loaded;
    for (const auto& path : candidateFiles) {
        if (loaded.find(path) != loaded.end()) {
            continue;
        }
        if (LoadWaveFile(path)) {
            loaded.insert(path);
        }
    }

    if (waveCache_.empty()) {
        LOG_WARN("WaveLoader: wave cache is empty (no wave files loaded)");
    } else {
        LOG_INFO("WaveLoader: loaded {} wave definitions", waveCache_.size());
    }
}

bool WaveLoader::LoadWaveFile(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            return false;
        }

        nlohmann::json data = nlohmann::json::parse(file, nullptr, true, true);
        if (!data.is_array()) {
            LOG_WARN("WaveLoader: wave file is not array: {}", path);
            return false;
        }

        for (const auto& wave : data) {
            if (!wave.is_object()) {
                continue;
            }
            if (!wave.contains("id") || !wave["id"].is_string()) {
                continue;
            }

            const std::string waveId = wave["id"].get<std::string>();
            std::vector<SpawnEvent> events;

            // entries形式
            if (wave.contains("entries") && wave["entries"].is_array()) {
                for (const auto& entry : wave["entries"]) {
                    if (!entry.is_object()) continue;
                    const std::string enemyId = entry.value("enemyId", "");
                    if (enemyId.empty()) continue;
                    const int lane = entry.value("lane", 0);
                    const float delay = entry.value("delay", 0.0f);
                    const int count = entry.value("count", 1);
                    const float interval = entry.value("interval", 0.0f);
                    for (int i = 0; i < count; ++i) {
                        events.push_back(SpawnEvent{delay + interval * static_cast<float>(i), enemyId, lane});
                    }
                }
            }

            // spawn_groups形式
            if (wave.contains("spawn_groups") && wave["spawn_groups"].is_array()) {
                for (const auto& group : wave["spawn_groups"]) {
                    if (!group.is_object()) continue;
                    const std::string enemyId = group.value("entity_id", "");
                    if (enemyId.empty()) continue;
                    const float delay = group.value("delay_from_wave_start", 0.0f);
                    const int count = group.value("count", 1);
                    const float interval = group.value("spawn_interval", 0.0f);
                    for (int i = 0; i < count; ++i) {
                        events.push_back(SpawnEvent{delay + interval * static_cast<float>(i), enemyId, 0});
                    }
                }
            }

            SortByTime(events);
            waveCache_[waveId] = events;
        }

        LOG_DEBUG("WaveLoader: loaded wave file: {}", path);
        return true;
    } catch (const nlohmann::json::parse_error& e) {
        LOG_ERROR("WaveLoader JSON parse error ({}): {}", path, e.what());
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("WaveLoader error ({}): {}", path, e.what());
        return false;
    }
}

std::vector<SpawnEvent> WaveLoader::LoadInlineWaves(const nlohmann::json& wavesArray) {
    std::vector<SpawnEvent> result;

    float waveStart = 0.0f;
    int waveIndex = 0;

    for (const auto& w : wavesArray) {
        if (!w.is_object()) {
            continue;
        }

        // 既存ステージの例: { "type": "goblin", "count": 5, "interval": 0.5 }
        const std::string type = w.value("type", "");
        const int count = w.value("count", 1);
        const float interval = w.value("interval", 0.0f);
        const float delay = w.value("delay", 0.0f);

        if (type.empty()) {
            continue;
        }

        // インラインの場合は enemyId が "goblin" のように短いので、まずそのまま使う
        //（後続TODOで敵ID→スプライト/定義に繋ぐ）
        float lastTimeInWave = 0.0f;
        for (int i = 0; i < count; ++i) {
            const float t = waveStart + delay + interval * static_cast<float>(i);
            lastTimeInWave = std::max(lastTimeInWave, delay + interval * static_cast<float>(i));
            result.push_back(SpawnEvent{t, type, 0});
        }

        ++waveIndex;
        waveStart += lastTimeInWave + WAVE_GAP_SECONDS;
    }

    (void)waveIndex;
    SortByTime(result);
    return result;
}

std::vector<SpawnEvent> WaveLoader::LoadWaveIdList(const nlohmann::json& waveIdArray) {
    std::vector<SpawnEvent> result;

    float waveStart = 0.0f;
    for (const auto& waveIdJ : waveIdArray) {
        if (!waveIdJ.is_string()) {
            continue;
        }
        const std::string waveId = waveIdJ.get<std::string>();
        auto it = waveCache_.find(waveId);
        if (it == waveCache_.end()) {
            LOG_WARN("WaveLoader: wave not found: {}", waveId);
            continue;
        }

        float lastTimeInWave = 0.0f;
        for (const auto& ev : it->second) {
            result.push_back(SpawnEvent{waveStart + ev.time, ev.enemyId, ev.lane});
            lastTimeInWave = std::max(lastTimeInWave, ev.time);
        }
        waveStart += lastTimeInWave + WAVE_GAP_SECONDS;
    }

    SortByTime(result);
    return result;
}

void WaveLoader::SortByTime(std::vector<SpawnEvent>& events) {
    std::sort(events.begin(), events.end(), [](const SpawnEvent& a, const SpawnEvent& b) {
        return a.time < b.time;
    });
}

} // namespace game
} // namespace core
} // namespace game

