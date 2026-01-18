#include "FieldManager.hpp"
#include "../../utils/Log.h"
#include <raylib.h>
#include <cmath>

namespace game {
namespace core {
namespace gamescene {

FieldManager::FieldManager(int width, int height, int cellSize,
                           float originX, float originY)
    : width_(width), height_(height), cellSize_(cellSize),
      originX_(originX), originY_(originY), registry_(nullptr) {
}

bool FieldManager::Initialize(entt::registry* registry) {
    registry_ = registry;
    
    // デフォルトマップを生成
    GenerateDefaultMap();
    
    LOG_INFO("FieldManager initialized: {}x{} grid, cell size: {}", 
             width_, height_, cellSize_);
    return true;
}

void FieldManager::Shutdown() {
    tiles_.clear();
    enemyPath_.clear();
    gridMap_.clear();
    LOG_INFO("FieldManager shutdown");
}

void FieldManager::Render(bool showGrid) {
    // 背景タイル描画
    DrawTiles();
    
    // 敵パス描画
    DrawEnemyPath();
    
    // グリッド線描画
    if (showGrid) {
        DrawGrid();
    }
}

Vector2 FieldManager::GridToPixel(int gx, int gy) const {
    return Vector2{
        originX_ + gx * cellSize_,
        originY_ + gy * cellSize_
    };
}

std::pair<int, int> FieldManager::PixelToGrid(float px, float py) const {
    int gx = static_cast<int>((px - originX_) / cellSize_);
    int gy = static_cast<int>((py - originY_) / cellSize_);
    return {gx, gy};
}

bool FieldManager::IsValidGridPosition(int gx, int gy) const {
    return gx >= 0 && gx < width_ && gy >= 0 && gy < height_;
}

bool FieldManager::PlaceUnit(entt::entity unitEntity, int gx, int gy) {
    if (!IsPlaceable(gx, gy)) {
        return false;
    }
    
    auto key = std::make_pair(gx, gy);
    gridMap_[key] = unitEntity;
    LOG_DEBUG("Unit placed at ({}, {})", gx, gy);
    return true;
}

bool FieldManager::RemoveUnit(int gx, int gy) {
    auto key = std::make_pair(gx, gy);
    auto it = gridMap_.find(key);
    if (it != gridMap_.end()) {
        gridMap_.erase(it);
        LOG_DEBUG("Unit removed from ({}, {})", gx, gy);
        return true;
    }
    return false;
}

entt::entity FieldManager::GetUnitAt(int gx, int gy) const {
    auto key = std::make_pair(gx, gy);
    auto it = gridMap_.find(key);
    if (it != gridMap_.end()) {
        return it->second;
    }
    return entt::null;
}

bool FieldManager::IsPlaceable(int gx, int gy) const {
    // 範囲外チェック
    if (!IsValidGridPosition(gx, gy)) {
        return false;
    }
    
    // 既にユニットが配置されているかチェック
    auto key = std::make_pair(gx, gy);
    if (gridMap_.find(key) != gridMap_.end()) {
        return false;
    }
    
    // タイルタイプチェック（パスやブロックには配置不可）
    for (const auto& tile : tiles_) {
        if (tile.gridX == gx && tile.gridY == gy) {
            return tile.type == CellType::Normal;
        }
    }
    
    // デフォルトで配置可能
    return true;
}

void FieldManager::GenerateDefaultMap() {
    // 全セルを通常タイルとして初期化
    tiles_.clear();
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            MapTile tile;
            tile.type = CellType::Normal;
            tile.gridX = x;
            tile.gridY = y;
            tiles_.push_back(tile);
        }
    }
    
    // 簡単な敵パスを生成（左から右へ）
    enemyPath_.clear();
    int pathY = height_ / 2;
    for (int x = 0; x < width_; ++x) {
        Vector2 pos = GridToPixel(x, pathY);
        enemyPath_.push_back(pos);
        
        // パスタイルとしてマーク
        for (auto& tile : tiles_) {
            if (tile.gridX == x && tile.gridY == pathY) {
                tile.type = CellType::Path;
                break;
            }
        }
    }
    
    // スポーン位置（左端）
    if (!tiles_.empty()) {
        tiles_[pathY * width_].type = CellType::SpawnPoint;
    }
    
    // ゴール位置（右端）
    if (!tiles_.empty()) {
        tiles_[pathY * width_ + (width_ - 1)].type = CellType::Goal;
    }
    
    LOG_INFO("Default map generated with path at y={}", pathY);
}

void FieldManager::DrawGrid() {
    Color gridColor = Color{100, 110, 120, 80};
    
    // 垂直線
    for (int x = 0; x <= width_; ++x) {
        float xPos = originX_ + x * cellSize_;
        DrawLineEx(
            Vector2{xPos, originY_},
            Vector2{xPos, originY_ + height_ * cellSize_},
            1.0f, gridColor
        );
    }
    
    // 水平線
    for (int y = 0; y <= height_; ++y) {
        float yPos = originY_ + y * cellSize_;
        DrawLineEx(
            Vector2{originX_, yPos},
            Vector2{originX_ + width_ * cellSize_, yPos},
            1.0f, gridColor
        );
    }
}

void FieldManager::DrawTiles() {
    for (const auto& tile : tiles_) {
        Vector2 pos = GridToPixel(tile.gridX, tile.gridY);
        Color color;
        
        switch (tile.type) {
        case CellType::Normal:
            color = Color{60, 80, 60, 255}; // 濃緑（配置可能）
            break;
        case CellType::Path:
            color = Color{100, 100, 80, 255}; // 薄黄（敵パス）
            break;
        case CellType::Blocked:
            color = Color{80, 80, 80, 255}; // グレー（配置不可）
            break;
        case CellType::SpawnPoint:
            color = Color{180, 60, 60, 255}; // 赤（スポーン）
            break;
        case CellType::Goal:
            color = Color{240, 170, 60, 255}; // ゴールド（ゴール）
            break;
        default:
            color = GRAY;
            break;
        }
        
        DrawRectangle(
            static_cast<int>(pos.x),
            static_cast<int>(pos.y),
            cellSize_,
            cellSize_,
            color
        );
    }
}

void FieldManager::DrawEnemyPath() {
    if (enemyPath_.size() < 2) {
        return;
    }
    
    Color pathLineColor = Color{240, 200, 100, 180};
    for (size_t i = 0; i < enemyPath_.size() - 1; ++i) {
        Vector2 start = enemyPath_[i];
        Vector2 end = enemyPath_[i + 1];
        
        // パスの中心に線を描画
        start.x += cellSize_ / 2.0f;
        start.y += cellSize_ / 2.0f;
        end.x += cellSize_ / 2.0f;
        end.y += cellSize_ / 2.0f;
        
        DrawLineEx(start, end, 3.0f, pathLineColor);
    }
}

} // namespace gamescene
} // namespace core
} // namespace game
