/**
 * @file DefinitionLoader.h
 * @brief 統一定義ローダー
 * 
 * 全ての定義タイプを統合的に読み込むローダー
 */
#pragma once

#include "Data/Registry.h"
#include "Data/Loaders/CharacterLoader.h"
#include "Data/Loaders/StageLoader.h"
#include "Data/Loaders/UILoader.h"
#include "Data/Loaders/MapLoader.h"
#include <string>

namespace Data {

/**
 * @brief 統一定義ローダー
 * 
 * 全ての定義タイプを統合的に読み込む
 */
class DefinitionLoader {
public:
    explicit DefinitionLoader(DefinitionRegistry& registry)
        : characterLoader_(registry)
        , stageLoader_(registry)
        , uiLoader_(registry)
        , mapLoader_(registry)
        , registry_(registry) {}
    
    /**
     * @brief 全ての定義を読み込む
     */
    void LoadAll(const std::string& basePath) {
        LoadAllCharacters(basePath + "/characters");
        LoadAllStages(basePath + "/stages");
        LoadAllUILayouts(basePath + "/ui");
        LoadAllMaps(basePath + "/maps");
    }
    
    /**
     * @brief キャラクター定義を読み込む
     */
    int LoadAllCharacters(const std::string& directoryPath) {
        return characterLoader_.LoadAllCharacters(directoryPath);
    }
    
    /**
     * @brief ステージ定義を読み込む
     */
    int LoadAllStages(const std::string& directoryPath) {
        return stageLoader_.LoadAllStages(directoryPath);
    }
    
    /**
     * @brief UIレイアウト定義を読み込む
     */
    int LoadAllUILayouts(const std::string& directoryPath) {
        return uiLoader_.LoadAllUILayouts(directoryPath);
    }
    
    /**
     * @brief マップ定義を読み込む
     */
    int LoadAllMaps(const std::string& directoryPath) {
        return mapLoader_.LoadAllMaps(directoryPath);
    }
    
    /**
     * @brief エラーハンドラを設定
     */
    void SetErrorHandler(DataLoaderBase::ErrorHandler handler) {
        characterLoader_.SetErrorHandler(handler);
        stageLoader_.SetErrorHandler(handler);
        uiLoader_.SetErrorHandler(handler);
        mapLoader_.SetErrorHandler(handler);
    }

private:
    CharacterLoader characterLoader_;
    StageLoader stageLoader_;
    UILoader uiLoader_;
    MapLoader mapLoader_;
    DefinitionRegistry& registry_;
};

} // namespace Data

