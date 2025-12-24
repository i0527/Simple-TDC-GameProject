#pragma once

#include <entt/entt.hpp>
#include <unordered_map>
#include <string>
#include <filesystem>

#include "Data/DefinitionRegistry.h"
#include "Game/Components/NewCoreComponents.h"
#include "Game/Graphics/SpriteRenderer.h"
#include <raylib.h>

namespace Game::Systems {

/// @brief 新しい描画システム（PNGファイル直接描画）
class NewRenderingSystem {
public:
    /// @brief コンストラクタ
    /// @param definitions エンティティ定義レジストリ
    NewRenderingSystem(Shared::Data::DefinitionRegistry& definitions);
    
    /// @brief デストラクタ（テクスチャキャッシュのクリーンアップ）
    ~NewRenderingSystem();
    
    /// @brief エンティティ描画処理
    /// @param registry ECSレジストリ
    void DrawEntities(entt::registry& registry) const;

private:
    Shared::Data::DefinitionRegistry& definitions_;
    mutable std::unordered_map<std::string, Texture2D> texture_cache_;
    
    /// @brief エンティティ定義からキャラクターフォルダパスを解決
    /// @param def エンティティ定義
    /// @return キャラクターフォルダのパス（例: "assets/characters/sub/HatSlime"）
    std::string ResolveCharacterFolder(const Shared::Data::EntityDef* def) const;
    
    /// @brief アクション名から画像ファイルパスを構築
    /// @param folder_path キャラクターフォルダのパス
    /// @param action アクション名（"idle", "walk", "attack", "death"）
    /// @return 画像ファイルのパス
    std::string GetActionImagePath(const std::string& folder_path, const std::string& action) const;
    
    /// @brief テクスチャを読み込む（キャッシュから取得、または新規読み込み）
    /// @param path 画像ファイルのパス
    /// @return テクスチャ（読み込み失敗時はid=0のテクスチャ）
    Texture2D LoadOrGetTexture(const std::string& path) const;
    
    /// @brief JSONファイルからアクション名に対応するフレーム情報を取得
    /// @param jsonPath JSONファイルのパス
    /// @param actionName アクション名（"idle", "walk", "attack", "death"）
    /// @param frameIndex フレームインデックス
    /// @param outFrameRect 出力: フレームの矩形（テクスチャ内の位置・サイズ）
    /// @return 成功時true
    bool GetFrameRectFromJson(const std::string& jsonPath, const std::string& actionName, int frameIndex, Rectangle& outFrameRect) const;
    
    /// @brief アクション名からJSONファイルパスを構築
    /// @param folder_path キャラクターフォルダのパス
    /// @param action アクション名（"idle", "walk", "attack", "death"）
    /// @return JSONファイルのパス
    std::string GetActionJsonPath(const std::string& folder_path, const std::string& action) const;
};

} // namespace Game::Systems
