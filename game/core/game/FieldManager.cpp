#include "FieldManager.hpp"
#include "../../utils/Log.h"
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
    
    // 繝・ヵ繧ｩ繝ｫ繝医・繝・・繧堤函謌・
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
    // 閭梧勹繧ｿ繧､繝ｫ謠冗判
    DrawTiles();
    
    // 謨ｵ繝代せ謠冗判
    DrawEnemyPath();
    
    // 繧ｰ繝ｪ繝・ラ邱壽緒逕ｻ
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
    // 遽・峇螟悶メ繧ｧ繝・け
    if (!IsValidGridPosition(gx, gy)) {
        return false;
    }
    
    // 譌｢縺ｫ繝ｦ繝九ャ繝医′驟咲ｽｮ縺輔ｌ縺ｦ縺・ｋ縺九メ繧ｧ繝・け
    auto key = std::make_pair(gx, gy);
    if (gridMap_.find(key) != gridMap_.end()) {
        return false;
    }
    
    // 繧ｿ繧､繝ｫ繧ｿ繧､繝励メ繧ｧ繝・け・医ヱ繧ｹ繧・ヶ繝ｭ繝・け縺ｫ縺ｯ驟咲ｽｮ荳榊庄・・
    for (const auto& tile : tiles_) {
        if (tile.gridX == gx && tile.gridY == gy) {
            return tile.type == CellType::Normal;
        }
    }
    
    // 繝・ヵ繧ｩ繝ｫ繝医〒驟咲ｽｮ蜿ｯ閭ｽ
    return true;
}

void FieldManager::GenerateDefaultMap() {
    // 蜈ｨ繧ｻ繝ｫ繧帝壼ｸｸ繧ｿ繧､繝ｫ縺ｨ縺励※蛻晄悄蛹・
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
    
    // 邁｡蜊倥↑謨ｵ繝代せ繧堤函謌撰ｼ亥ｷｦ縺九ｉ蜿ｳ縺ｸ・・
    enemyPath_.clear();
    int pathY = height_ / 2;
    for (int x = 0; x < width_; ++x) {
        Vector2 pos = GridToPixel(x, pathY);
        enemyPath_.push_back(pos);
        
        // 繝代せ繧ｿ繧､繝ｫ縺ｨ縺励※繝槭・繧ｯ
        for (auto& tile : tiles_) {
            if (tile.gridX == x && tile.gridY == pathY) {
                tile.type = CellType::Path;
                break;
            }
        }
    }
    
    // 繧ｹ繝昴・繝ｳ菴咲ｽｮ・亥ｷｦ遶ｯ・・
    if (!tiles_.empty()) {
        tiles_[pathY * width_].type = CellType::SpawnPoint;
    }
    
    // 繧ｴ繝ｼ繝ｫ菴咲ｽｮ・亥承遶ｯ・・
    if (!tiles_.empty()) {
        tiles_[pathY * width_ + (width_ - 1)].type = CellType::Goal;
    }
    
    LOG_INFO("Default map generated with path at y={}", pathY);
}

void FieldManager::DrawGrid() {
    Color gridColor = Color{100, 110, 120, 80};
    
    // 蝙ら峩邱・
    for (int x = 0; x <= width_; ++x) {
        float xPos = originX_ + x * cellSize_;
        DrawLineEx(
            Vector2{xPos, originY_},
            Vector2{xPos, originY_ + height_ * cellSize_},
            1.0f, gridColor
        );
    }
    
    // 豌ｴ蟷ｳ邱・
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
            color = Color{60, 80, 60, 255}; // 豼・ｷ托ｼ磯・鄂ｮ蜿ｯ閭ｽ・・
            break;
        case CellType::Path:
            color = Color{100, 100, 80, 255}; // 阮・ｻ・ｼ域雰繝代せ・・
            break;
        case CellType::Blocked:
            color = Color{80, 80, 80, 255}; // 繧ｰ繝ｬ繝ｼ・磯・鄂ｮ荳榊庄・・
            break;
        case CellType::SpawnPoint:
            color = Color{180, 60, 60, 255}; // 襍､・医せ繝昴・繝ｳ・・
            break;
        case CellType::Goal:
            color = Color{240, 170, 60, 255}; // 繧ｴ繝ｼ繝ｫ繝会ｼ医ざ繝ｼ繝ｫ・・
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
        
        // 繝代せ縺ｮ荳ｭ蠢・↓邱壹ｒ謠冗判
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
