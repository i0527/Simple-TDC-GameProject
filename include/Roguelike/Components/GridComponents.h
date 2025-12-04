/**
 * @file GridComponents.h
 * @brief グリッド/マップ関連のコンポーネント定義
 * 
 * NetHack風ローグライクのタイルベースマップシステムを実現する
 * コンポーネント群を定義。
 */
#pragma once

#include <cstdint>
#include <vector>
#include <entt/entt.hpp>

namespace Roguelike::Components {

/**
 * @brief グリッド座標（タイル単位）
 * 
 * ピクセル座標ではなく、マップ上のタイル位置を表す。
 * (0,0)がマップ左上。
 */
struct GridPosition {
    int x = 0;
    int y = 0;
    
    bool operator==(const GridPosition& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const GridPosition& other) const {
        return !(*this == other);
    }
    
    GridPosition operator+(const GridPosition& other) const {
        return {x + other.x, y + other.y};
    }
};

/**
 * @brief タイルの種類
 * 
 * NetHackの表示文字と対応付けられる。
 */
enum class TileType : uint8_t {
    Void = 0,       // 何もない（未生成）         ' '
    Floor,          // 床（部屋内）               '.'
    Wall,           // 壁                        '#'
    Corridor,       // 通路                      '#'
    DoorClosed,     // 閉じたドア                 '+'
    DoorOpen,       // 開いたドア                 '\''
    StairsUp,       // 上り階段                   '<'
    StairsDown,     // 下り階段                   '>'
    Water,          // 水                        '~'
    Lava,           // 溶岩                      '~'
};

/**
 * @brief 単一タイル情報
 * 
 * マップの1マスの状態を表す。
 */
struct Tile {
    TileType type = TileType::Void;
    bool explored = false;      // 一度でも見たか（マップに残る）
    bool visible = false;       // 現在視界内か（毎ターン更新）
    entt::entity occupant = entt::null;  // タイル上のエンティティ（プレイヤー/モンスター）
    entt::entity item = entt::null;      // 落ちているアイテム
};

/**
 * @brief マップ全体データ
 * 
 * ダンジョンの1フロア分のデータを保持。
 * GameContextにサービスとして登録、または専用エンティティに付与。
 */
struct MapData {
    int width = 0;
    int height = 0;
    int currentFloor = 1;       // 現在の階層（1から開始）
    std::vector<Tile> tiles;
    
    /**
     * @brief マップを初期化（全てVoidで埋める）
     */
    void Initialize(int w, int h) {
        width = w;
        height = h;
        tiles.resize(w * h);
        for (auto& tile : tiles) {
            tile = Tile{};
        }
    }
    
    /**
     * @brief 指定座標のタイルを取得（参照）
     */
    Tile& At(int x, int y) { 
        return tiles[y * width + x]; 
    }
    
    /**
     * @brief 指定座標のタイルを取得（const参照）
     */
    const Tile& At(int x, int y) const { 
        return tiles[y * width + x]; 
    }
    
    /**
     * @brief 座標がマップ範囲内か判定
     */
    bool InBounds(int x, int y) const { 
        return x >= 0 && x < width && y >= 0 && y < height; 
    }
    
    /**
     * @brief 指定座標が歩行可能か判定
     */
    bool IsWalkable(int x, int y) const {
        if (!InBounds(x, y)) return false;
        const auto& tile = At(x, y);
        switch (tile.type) {
            case TileType::Floor:
            case TileType::Corridor:
            case TileType::DoorOpen:
            case TileType::StairsUp:
            case TileType::StairsDown:
                return true;
            case TileType::DoorClosed:
                return false;  // 閉じたドアは歩行不可（開ける必要あり）
            default:
                return false;
        }
    }
    
    /**
     * @brief 指定座標が視界を遮るか判定
     */
    bool BlocksVision(int x, int y) const {
        if (!InBounds(x, y)) return true;
        const auto& tile = At(x, y);
        switch (tile.type) {
            case TileType::Wall:
            case TileType::DoorClosed:
            case TileType::Void:
                return true;
            default:
                return false;
        }
    }
    
    /**
     * @brief 指定座標にエンティティがいるか判定
     */
    bool IsOccupied(int x, int y) const {
        if (!InBounds(x, y)) return true;
        return At(x, y).occupant != entt::null;
    }
    
    /**
     * @brief 全タイルの視界フラグをリセット
     */
    void ClearVisible() {
        for (auto& tile : tiles) {
            tile.visible = false;
        }
    }
    
    /**
     * @brief タイルを可視状態にする
     */
    void SetVisible(int x, int y) {
        if (InBounds(x, y)) {
            auto& tile = At(x, y);
            tile.visible = true;
            tile.explored = true;
        }
    }
};

/**
 * @brief プレイヤーマーカー（タグコンポーネント）
 */
struct PlayerTag {};

/**
 * @brief モンスターマーカー（タグコンポーネント）
 */
struct MonsterTag {};

} // namespace Roguelike::Components

