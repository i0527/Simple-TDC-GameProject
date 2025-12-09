#pragma once

#include <string>
#include <vector>

namespace Shared::Data {

/// @brief Wave（ウェーブ）定義
struct WaveDef {
    std::string id;
    std::string stage_id;
    int wave_number = 1;

    struct SpawnGroup {
        std::string entity_id;
        int count = 1;
        float spawn_interval = 1.0f;
        float delay_from_wave_start = 0.0f;
    };
    std::vector<SpawnGroup> spawn_groups;

    float duration = 0.0f;  // Wave全体の長さ（秒）
    std::vector<std::string> tags;
};

} // namespace Shared::Data
