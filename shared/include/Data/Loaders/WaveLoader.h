#pragma once

#include "Data/DefinitionRegistry.h"
#include <string>

namespace Shared::Data {

/// @brief Wave定義のJSON読み込み・書き込み
class WaveLoader {
public:
    static bool LoadFromJson(const std::string& json_path, DefinitionRegistry& registry);
    static bool SaveToJson(const std::string& json_path, const DefinitionRegistry& registry);
    static bool SaveSingleWave(const std::string& json_path, const WaveDef& def);
};

} // namespace Shared::Data
