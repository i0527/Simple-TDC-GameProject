/**
 * @file MapLoader.h
 * @brief マップ定義ローダー
 * 
 * JSONからマップ定義を読み込み、DefinitionRegistryに登録する
 */
#pragma once

#include "Data/Loaders/DataLoaderBase.h"
#include "Data/Definitions/MapDef.h"
#include <nlohmann/json.hpp>

namespace Data {

/**
 * @brief マップ定義ローダー
 */
class MapLoader : public DataLoaderBase {
public:
    explicit MapLoader(DefinitionRegistry& registry)
        : DataLoaderBase(registry) {}
    
    /**
     * @brief 単一のマップ定義ファイルを読み込み
     */
    bool LoadMap(const std::string& filePath) {
        try {
            auto jsonData = LoadJsonFile(filePath);
            if (jsonData.is_null()) return false;
            
            auto def = ParseMapDef(jsonData);
            if (def.id.empty()) {
                def.id = GetFileNameWithoutExtension(filePath);
            }
            registry_.RegisterMap(std::move(def));
            return true;
        } catch (const std::exception& e) {
            errorHandler_(filePath, e.what());
            return false;
        }
    }
    
    /**
     * @brief ディレクトリ内の全マップ定義を読み込み
     */
    int LoadAllMaps(const std::string& directoryPath) {
        return LoadDirectory(directoryPath, ".map.json",
            [this](const std::string& path) { return LoadMap(path); });
    }
    
    /**
     * @brief JSONからMapDefをパース
     */
    MapDef ParseMapDef(const json& j) {
        MapDef def;
        
        def.id = GetOr<std::string>(j, "id", "");
        def.name = GetOr<std::string>(j, "name", def.id);
        def.description = GetOr<std::string>(j, "description", "");
        
        // タイプ
        std::string typeStr = GetOr<std::string>(j, "type", "generated");
        def.type = (typeStr == "fixed") ? MapType::Fixed : MapType::Generated;
        
        // 生成設定
        if (j.contains("generator")) {
            const auto& gen = j["generator"];
            std::string genTypeStr = GetOr<std::string>(gen, "type", "bsp");
            if (genTypeStr == "bsp") {
                def.generator.type = DungeonGeneratorType::BSP;
            } else if (genTypeStr == "cellular") {
                def.generator.type = DungeonGeneratorType::Cellular;
            } else if (genTypeStr == "roomFirst") {
                def.generator.type = DungeonGeneratorType::RoomFirst;
            }
            
            def.generator.roomCount = GetOr(gen, "roomCount", 10);
            
            if (gen.contains("minRoomSize")) {
                const auto& size = gen["minRoomSize"];
                def.generator.minRoomSize.width = GetOr(size, "width", 5.0f);
                def.generator.minRoomSize.height = GetOr(size, "height", 5.0f);
            }
            
            if (gen.contains("maxRoomSize")) {
                const auto& size = gen["maxRoomSize"];
                def.generator.maxRoomSize.width = GetOr(size, "width", 15.0f);
                def.generator.maxRoomSize.height = GetOr(size, "height", 15.0f);
            }
            
            def.generator.corridorWidth = GetOr(gen, "corridorWidth", 1);
        }
        
        // 固定マップ
        if (j.contains("fixed")) {
            const auto& fixed = j["fixed"];
            def.fixed.tiles = GetOr<std::string>(fixed, "tiles", "");
            def.fixed.width = GetOr(fixed, "width", 0);
            def.fixed.height = GetOr(fixed, "height", 0);
        }
        
        // 共通設定
        def.width = GetOr(j, "width", 100);
        def.height = GetOr(j, "height", 100);
        
        return def;
    }

private:
    template<typename T>
    T GetOr(const json& j, const std::string& key, T defaultValue) {
        if (j.contains(key) && !j[key].is_null()) {
            try {
                return j[key].get<T>();
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
};

} // namespace Data

