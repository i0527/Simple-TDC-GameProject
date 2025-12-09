#pragma once

#include "Data/DefinitionRegistry.h"
#include <string>
#include <vector>

namespace Shared::Data {

/// @brief データ定義のバリデーションを行う
class DataValidator {
public:
    /// @brief Registryの全定義データをバリデーション
    /// @param registry バリデーション対象のRegistry
    /// @return バリデーション成功時 true
    static bool Validate(const DefinitionRegistry& registry);

    /// @brief バリデーションエラーメッセージを取得
    /// @return エラーメッセージリスト
    static const std::vector<std::string>& GetErrors();

    /// @brief エラーメッセージをクリア
    static void ClearErrors();

private:
    static std::vector<std::string> errors_;

    static bool ValidateEntities(const DefinitionRegistry& registry);
    static bool ValidateSkills(const DefinitionRegistry& registry);
    static bool ValidateStages(const DefinitionRegistry& registry);
    static bool ValidateWaves(const DefinitionRegistry& registry);
    static bool ValidateAbilities(const DefinitionRegistry& registry);
};

} // namespace Shared::Data
