#pragma once

#include "Data/DefinitionRegistry.h"
#include <string>

namespace Shared::Data {

/// @brief Entity定義のJSON読み込み・書き込み
class EntityLoader {
public:
    /// @brief JSONファイルから Entity定義を読み込み、RegistryへRegister
    /// @param json_path JSONファイルパス
    /// @param registry 登録先のRegistry
    /// @return 成功時 true
    static bool LoadFromJson(const std::string& json_path, DefinitionRegistry& registry);

    /// @brief RegistryのEntity定義をJSONファイルへ書き込み
    /// @param json_path 書き込み先JSONファイルパス
    /// @param registry 対象のRegistry
    /// @return 成功時 true
    static bool SaveToJson(const std::string& json_path, const DefinitionRegistry& registry);
};

} // namespace Shared::Data
