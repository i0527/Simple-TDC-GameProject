/**
 * @file MapDef.h
 * @brief マップ定義構造体
 * 
 * Roguelikeゲーム用のマップ定義
 */
#pragma once

#include <string>
#include "Data/Definitions/CommonTypes.h"

namespace Data {

/**
 * @brief マップ生成タイプ
 */
enum class MapType {
    Generated,  // 自動生成
    Fixed       // 固定マップ
};

/**
 * @brief ダンジョン生成タイプ
 */
enum class DungeonGeneratorType {
    BSP,        // BSP分割
    Cellular,   // セルオートマトン
    RoomFirst   // 部屋優先
};

/**
 * @brief ダンジョン生成設定
 */
struct DungeonGeneratorConfig {
    DungeonGeneratorType type = DungeonGeneratorType::BSP;
    int roomCount = 10;
    Size minRoomSize{5, 5};
    Size maxRoomSize{15, 15};
    int corridorWidth = 1;
};

/**
 * @brief 固定マップデータ
 */
struct FixedMapData {
    std::string tiles;  // タイル文字列（例: "...#...\n.@.....\n...#..."）
    int width = 0;
    int height = 0;
};

/**
 * @brief マップ定義
 */
struct MapDef {
    std::string id;
    std::string name;
    std::string description;
    
    MapType type = MapType::Generated;
    
    // 生成マップの場合
    DungeonGeneratorConfig generator;
    
    // 固定マップの場合
    FixedMapData fixed;
    
    // 共通設定
    int width = 100;   // マップ幅（タイル数）
    int height = 100;  // マップ高さ（タイル数）
};

} // namespace Data

