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

    /// @brief ディレクトリ配下の character.json（または *.character.json）を読み込む
    /// @param dir_path キャラフォルダの親ディレクトリ
    /// @param registry 登録先のRegistry
    /// @param allow_glob "*.character.json" を許容する場合 true
    /// @return 1件でもロードできたら true
    static bool LoadFromDirectory(const std::string& dir_path, DefinitionRegistry& registry, bool allow_glob = false);

    /// @brief 単一キャラクターJSONを読み込む（Aseprite相対パスを解決）
    static bool LoadSingleCharacter(const std::string& json_path, DefinitionRegistry& registry);

    /// @brief RegistryのEntity定義をJSONファイルへ書き込み
    /// @param json_path 書き込み先JSONファイルパス
    /// @param registry 対象のRegistry
    /// @return 成功時 true
    static bool SaveToJson(const std::string& json_path, const DefinitionRegistry& registry);

    /// @brief 単一のEntity定義をJSONファイルへ書き込み
    /// @param json_path 書き込み先JSONファイルパス
    /// @param def Entity定義
    /// @return 成功時 true
    static bool SaveSingleEntity(const std::string& json_path, const EntityDef& def);
};

} // namespace Shared::Data
