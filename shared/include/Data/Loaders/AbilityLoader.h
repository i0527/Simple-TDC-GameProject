#pragma once

#include "Data/DefinitionRegistry.h"
#include <string>

namespace Shared::Data {

/// @brief Ability定義のJSON読み込み・書き込み
class AbilityLoader {
public:
    static bool LoadFromJson(const std::string& json_path, DefinitionRegistry& registry);
    static bool SaveToJson(const std::string& json_path, const DefinitionRegistry& registry);
};

} // namespace Shared::Data
