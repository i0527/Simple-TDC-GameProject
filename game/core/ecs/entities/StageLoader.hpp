#pragma once

// 標準ライブラリ
#include <string>
#include <unordered_map>

// プロジェクト内
#include "StageManager.hpp"

namespace game {
namespace core {
namespace entities {

class StageLoader {
public:
    static bool LoadFromJSON(
        const std::string& json_path,
        std::unordered_map<std::string, StageData>& outStages,
        std::unordered_map<int, std::string>& outStageNumberToId);
    static bool SaveToJSON(
        const std::string& json_path,
        const std::unordered_map<std::string, StageData>& stages);

    static void LoadDefault(
        std::unordered_map<std::string, StageData>& outStages,
        std::unordered_map<int, std::string>& outStageNumberToId);
};

} // namespace entities
} // namespace core
} // namespace game
