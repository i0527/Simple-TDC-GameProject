#pragma once

#include "Data/DefinitionRegistry.h"
#include <string>

namespace Shared::Data {

/// @brief Stage定義のJSON読み込み・書き込み
class StageLoader {
public:
    static bool LoadFromJson(const std::string& json_path, DefinitionRegistry& registry);
    static bool SaveToJson(const std::string& json_path, const DefinitionRegistry& registry);
    static bool SaveSingleStage(const std::string& json_path, const StageDef& def);
};

} // namespace Shared::Data
