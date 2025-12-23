#pragma once

#include "Data/DefinitionRegistry.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Shared::Data {

/// @brief データ定義のバリデーションを行う
class DataValidator {
public:
    /// @brief Registryの全定義データをバリデーション
    /// @param registry バリデーション対象のRegistry
    /// @return バリデーション成功時 true
    static bool Validate(const DefinitionRegistry& registry);

    /// @brief エンティティ定義をスキーマに対して検証
    /// @param entityJson エンティティJSON
    /// @param schemaPath スキーマファイルパス
    /// @return 検証成功時 true
    static bool ValidateEntityAgainstSchema(const nlohmann::json& entityJson, 
                                           const std::string& schemaPath = "assets/definitions/schemas/entity.schema.json");

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

    // JSON スキーマ検証ヘルパー
    static bool ValidateJsonSchema(const nlohmann::json& data, const nlohmann::json& schema, 
                                  const std::string& path = "");
    static void ValidateProperty(const nlohmann::json& data, const nlohmann::json& schema,
                                const std::string& propName, const std::string& currentPath);
};

} // namespace Shared::Data
