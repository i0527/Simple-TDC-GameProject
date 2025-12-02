/**
 * @file DungeonGenerator.h
 * @brief BSP分割法によるダンジョン自動生成
 * 
 * NetHack風のランダムダンジョンを生成する。
 * BSP（Binary Space Partitioning）アルゴリズムを使用し、
 * 部屋と通路で構成されるダンジョンを作成。
 */
#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <memory>
#include "../Components/GridComponents.h"

namespace Roguelike::Generators {

/**
 * @brief 矩形領域
 */
struct Rect {
    int x, y, width, height;
    
    int Left() const { return x; }
    int Right() const { return x + width - 1; }
    int Top() const { return y; }
    int Bottom() const { return y + height - 1; }
    int CenterX() const { return x + width / 2; }
    int CenterY() const { return y + height / 2; }
};

/**
 * @brief BSPノード（空間分割用）
 */
struct BSPNode {
    Rect bounds;
    std::unique_ptr<BSPNode> left;
    std::unique_ptr<BSPNode> right;
    Rect room;  // 実際の部屋（葉ノードのみ）
    bool hasRoom = false;
    
    bool IsLeaf() const { return !left && !right; }
};

/**
 * @brief ダンジョン生成器
 */
class DungeonGenerator {
public:
    // 生成パラメータ
    struct Config {
        int width = 80;
        int height = 40;
        int minRoomSize = 5;
        int maxRoomSize = 12;
        int splitDepth = 5;
        float doorChance = 0.3f;
        unsigned int seed = 0;  // 0 = ランダム
    };
    
    /**
     * @brief ダンジョンを生成
     * @param map 出力先マップデータ
     * @param config 生成パラメータ
     */
    void Generate(Components::MapData& map, const Config& config = Config{}) {
        config_ = config;
        
        // シード設定
        if (config_.seed == 0) {
            std::random_device rd;
            config_.seed = rd();
        }
        rng_.seed(config_.seed);
        
        // マップ初期化（全て壁）
        map.Initialize(config_.width, config_.height);
        for (int y = 0; y < map.height; y++) {
            for (int x = 0; x < map.width; x++) {
                map.At(x, y).type = Components::TileType::Wall;
            }
        }
        
        // 部屋リストをクリア
        rooms_.clear();
        
        // BSPツリー構築
        Rect rootBounds = {1, 1, config_.width - 2, config_.height - 2};
        root_ = std::make_unique<BSPNode>();
        root_->bounds = rootBounds;
        
        SplitNode(root_.get(), 0);
        
        // 部屋を生成
        CreateRooms(root_.get(), map);
        
        // 通路で接続
        ConnectRooms(root_.get(), map);
        
        // 階段を配置
        PlaceStairs(map);
    }
    
    /**
     * @brief 生成された部屋リストを取得
     */
    const std::vector<Rect>& GetRooms() const { return rooms_; }
    
    /**
     * @brief ランダムな床タイルの座標を取得
     */
    bool GetRandomFloorPosition(const Components::MapData& map, int& outX, int& outY) {
        std::vector<std::pair<int, int>> floors;
        for (int y = 0; y < map.height; y++) {
            for (int x = 0; x < map.width; x++) {
                if (map.At(x, y).type == Components::TileType::Floor) {
                    floors.push_back({x, y});
                }
            }
        }
        
        if (floors.empty()) return false;
        
        std::uniform_int_distribution<> dist(0, static_cast<int>(floors.size()) - 1);
        auto& pos = floors[dist(rng_)];
        outX = pos.first;
        outY = pos.second;
        return true;
    }
    
    /**
     * @brief 上り階段の位置を取得
     */
    bool GetStairsUpPosition(int& outX, int& outY) const {
        outX = stairsUpX_;
        outY = stairsUpY_;
        return stairsUpX_ >= 0;
    }
    
    /**
     * @brief 下り階段の位置を取得
     */
    bool GetStairsDownPosition(int& outX, int& outY) const {
        outX = stairsDownX_;
        outY = stairsDownY_;
        return stairsDownX_ >= 0;
    }
    
    /**
     * @brief シードを設定
     */
    void SetSeed(unsigned int seed) {
        baseSeed_ = seed;
    }
    
    /**
     * @brief 現在のシードを取得
     */
    unsigned int GetSeed() const {
        return config_.seed;
    }
    
    /**
     * @brief 簡易インターフェースでダンジョン生成
     * @param width マップ幅
     * @param height マップ高さ
     * @param floor 階層（シードに影響）
     * @return 生成されたマップデータ
     */
    Components::MapData Generate(int width, int height, int floor = 1) {
        Components::MapData map;
        Config cfg;
        cfg.width = width;
        cfg.height = height;
        cfg.seed = baseSeed_ + floor * 12345;  // 階層ごとに異なるシード
        cfg.splitDepth = 4 + floor / 3;        // 深い階層ほど複雑に
        cfg.minRoomSize = 4;
        cfg.maxRoomSize = 10;
        
        Generate(map, cfg);
        return map;
    }
    
    /**
     * @brief 上り階段の位置を取得（pair版）
     */
    std::pair<int, int> GetStairsUpPosition() const {
        return {stairsUpX_, stairsUpY_};
    }
    
    /**
     * @brief 下り階段の位置を取得（pair版）
     */
    std::pair<int, int> GetStairsDownPosition() const {
        return {stairsDownX_, stairsDownY_};
    }

private:
    unsigned int baseSeed_ = 0;  ///< 基本シード
    /**
     * @brief ノードを分割
     */
    void SplitNode(BSPNode* node, int depth) {
        if (depth >= config_.splitDepth) return;
        
        const auto& b = node->bounds;
        
        // 最小サイズ以下なら分割しない
        int minSize = config_.minRoomSize * 2 + 3;
        if (b.width < minSize && b.height < minSize) return;
        
        // 分割方向を決定
        bool splitHorizontal;
        if (b.width < minSize) {
            splitHorizontal = true;
        } else if (b.height < minSize) {
            splitHorizontal = false;
        } else {
            std::uniform_int_distribution<> dist(0, 1);
            splitHorizontal = dist(rng_) == 0;
        }
        
        // 分割位置を決定
        int maxSplit, minSplit;
        if (splitHorizontal) {
            minSplit = config_.minRoomSize + 2;
            maxSplit = b.height - config_.minRoomSize - 2;
        } else {
            minSplit = config_.minRoomSize + 2;
            maxSplit = b.width - config_.minRoomSize - 2;
        }
        
        if (maxSplit <= minSplit) return;
        
        std::uniform_int_distribution<> dist(minSplit, maxSplit);
        int splitPos = dist(rng_);
        
        // 子ノード作成
        node->left = std::make_unique<BSPNode>();
        node->right = std::make_unique<BSPNode>();
        
        if (splitHorizontal) {
            node->left->bounds = {b.x, b.y, b.width, splitPos};
            node->right->bounds = {b.x, b.y + splitPos, b.width, b.height - splitPos};
        } else {
            node->left->bounds = {b.x, b.y, splitPos, b.height};
            node->right->bounds = {b.x + splitPos, b.y, b.width - splitPos, b.height};
        }
        
        // 再帰的に分割
        SplitNode(node->left.get(), depth + 1);
        SplitNode(node->right.get(), depth + 1);
    }
    
    /**
     * @brief 部屋を生成
     */
    void CreateRooms(BSPNode* node, Components::MapData& map) {
        if (node->IsLeaf()) {
            // 葉ノード → 部屋を作成
            const auto& b = node->bounds;
            
            // 部屋サイズをランダムに決定
            int roomW = RandomRange(config_.minRoomSize, 
                                    std::min(config_.maxRoomSize, b.width - 2));
            int roomH = RandomRange(config_.minRoomSize, 
                                    std::min(config_.maxRoomSize, b.height - 2));
            
            // 部屋位置をランダムに決定（境界内に収まるように）
            int roomX = RandomRange(b.x + 1, b.x + b.width - roomW - 1);
            int roomY = RandomRange(b.y + 1, b.y + b.height - roomH - 1);
            
            node->room = {roomX, roomY, roomW, roomH};
            node->hasRoom = true;
            rooms_.push_back(node->room);
            
            // マップに部屋を描画
            for (int y = roomY; y < roomY + roomH; y++) {
                for (int x = roomX; x < roomX + roomW; x++) {
                    if (map.InBounds(x, y)) {
                        map.At(x, y).type = Components::TileType::Floor;
                    }
                }
            }
        } else {
            if (node->left) CreateRooms(node->left.get(), map);
            if (node->right) CreateRooms(node->right.get(), map);
        }
    }
    
    /**
     * @brief 部屋を通路で接続
     */
    void ConnectRooms(BSPNode* node, Components::MapData& map) {
        if (node->IsLeaf()) return;
        
        // 子ノードを再帰的に接続
        if (node->left) ConnectRooms(node->left.get(), map);
        if (node->right) ConnectRooms(node->right.get(), map);
        
        // 左右の子を接続
        if (node->left && node->right) {
            Rect roomA = GetRoomFromNode(node->left.get());
            Rect roomB = GetRoomFromNode(node->right.get());
            
            CreateCorridor(map, roomA.CenterX(), roomA.CenterY(),
                          roomB.CenterX(), roomB.CenterY());
        }
    }
    
    /**
     * @brief ノードから部屋を取得（再帰的に探索）
     */
    Rect GetRoomFromNode(BSPNode* node) {
        if (node->hasRoom) {
            return node->room;
        }
        
        // ランダムに子を選択
        if (node->left && node->right) {
            std::uniform_int_distribution<> dist(0, 1);
            if (dist(rng_) == 0) {
                return GetRoomFromNode(node->left.get());
            } else {
                return GetRoomFromNode(node->right.get());
            }
        } else if (node->left) {
            return GetRoomFromNode(node->left.get());
        } else if (node->right) {
            return GetRoomFromNode(node->right.get());
        }
        
        // フォールバック
        return {0, 0, 1, 1};
    }
    
    /**
     * @brief 2点間に通路を作成（L字型）
     */
    void CreateCorridor(Components::MapData& map, int x1, int y1, int x2, int y2) {
        // 50%の確率で水平→垂直 or 垂直→水平
        std::uniform_int_distribution<> dist(0, 1);
        bool horizontalFirst = dist(rng_) == 0;
        
        if (horizontalFirst) {
            // 水平に移動してから垂直
            CreateHorizontalCorridor(map, x1, x2, y1);
            CreateVerticalCorridor(map, y1, y2, x2);
        } else {
            // 垂直に移動してから水平
            CreateVerticalCorridor(map, y1, y2, x1);
            CreateHorizontalCorridor(map, x1, x2, y2);
        }
    }
    
    void CreateHorizontalCorridor(Components::MapData& map, int x1, int x2, int y) {
        int minX = std::min(x1, x2);
        int maxX = std::max(x1, x2);
        
        for (int x = minX; x <= maxX; x++) {
            if (map.InBounds(x, y)) {
                auto& tile = map.At(x, y);
                if (tile.type == Components::TileType::Wall) {
                    tile.type = Components::TileType::Corridor;
                }
            }
        }
    }
    
    void CreateVerticalCorridor(Components::MapData& map, int y1, int y2, int x) {
        int minY = std::min(y1, y2);
        int maxY = std::max(y1, y2);
        
        for (int y = minY; y <= maxY; y++) {
            if (map.InBounds(x, y)) {
                auto& tile = map.At(x, y);
                if (tile.type == Components::TileType::Wall) {
                    tile.type = Components::TileType::Corridor;
                }
            }
        }
    }
    
    /**
     * @brief 階段を配置
     */
    void PlaceStairs(Components::MapData& map) {
        if (rooms_.size() < 2) return;
        
        // 最も離れた2つの部屋に階段を配置
        int maxDist = 0;
        size_t roomA = 0, roomB = 1;
        
        for (size_t i = 0; i < rooms_.size(); i++) {
            for (size_t j = i + 1; j < rooms_.size(); j++) {
                int dx = rooms_[i].CenterX() - rooms_[j].CenterX();
                int dy = rooms_[i].CenterY() - rooms_[j].CenterY();
                int dist = dx * dx + dy * dy;
                if (dist > maxDist) {
                    maxDist = dist;
                    roomA = i;
                    roomB = j;
                }
            }
        }
        
        // 上り階段を配置
        stairsUpX_ = rooms_[roomA].CenterX();
        stairsUpY_ = rooms_[roomA].CenterY();
        if (map.InBounds(stairsUpX_, stairsUpY_)) {
            map.At(stairsUpX_, stairsUpY_).type = Components::TileType::StairsUp;
        }
        
        // 下り階段を配置
        stairsDownX_ = rooms_[roomB].CenterX();
        stairsDownY_ = rooms_[roomB].CenterY();
        if (map.InBounds(stairsDownX_, stairsDownY_)) {
            map.At(stairsDownX_, stairsDownY_).type = Components::TileType::StairsDown;
        }
    }
    
    int RandomRange(int min, int max) {
        if (min >= max) return min;
        std::uniform_int_distribution<> dist(min, max);
        return dist(rng_);
    }
    
    Config config_;
    std::mt19937 rng_;
    std::unique_ptr<BSPNode> root_;
    std::vector<Rect> rooms_;
    int stairsUpX_ = -1, stairsUpY_ = -1;
    int stairsDownX_ = -1, stairsDownY_ = -1;
};

} // namespace Roguelike::Generators

