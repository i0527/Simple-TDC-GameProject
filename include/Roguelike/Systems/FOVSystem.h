/**
 * @file FOVSystem.h
 * @brief 視界（Field of View）計算システム
 * 
 * Recursive Shadowcastingアルゴリズムによる視界計算。
 * プレイヤーから見える範囲を計算し、マップの可視状態を更新する。
 */
#pragma once

#include <cmath>
#include <algorithm>
#include "../Components/GridComponents.h"

namespace Roguelike::Systems {

/**
 * @brief 視界計算システム
 * 
 * Shadowcasting法を使用して、障害物による影を考慮した
 * 視界範囲を計算する。
 */
class FOVSystem {
public:
    /**
     * @brief 視界を計算
     * @param map マップデータ（visible/exploredが更新される）
     * @param viewerX 視点のX座標
     * @param viewerY 視点のY座標
     * @param radius 視界半径
     */
    static void Calculate(Components::MapData& map, int viewerX, int viewerY, int radius) {
        // 全タイルの可視状態をリセット
        map.ClearVisible();
        
        // 視点自体は常に見える
        if (map.InBounds(viewerX, viewerY)) {
            map.SetVisible(viewerX, viewerY);
        }
        
        // 8方向（八分円）に対してShadowcastingを実行
        for (int octant = 0; octant < 8; octant++) {
            CastLight(map, viewerX, viewerY, radius, 1, 1.0f, 0.0f, 
                      multipliers[0][octant], multipliers[1][octant],
                      multipliers[2][octant], multipliers[3][octant]);
        }
    }

private:
    // 8方向の変換行列
    static constexpr int multipliers[4][8] = {
        { 1,  0,  0, -1, -1,  0,  0,  1 },
        { 0,  1, -1,  0,  0, -1,  1,  0 },
        { 0,  1,  1,  0,  0, -1, -1,  0 },
        { 1,  0,  0,  1, -1,  0,  0, -1 }
    };
    
    /**
     * @brief 再帰的な影計算
     */
    static void CastLight(Components::MapData& map, 
                          int cx, int cy, int radius,
                          int row, float startSlope, float endSlope,
                          int xx, int xy, int yx, int yy) {
        if (startSlope < endSlope) return;
        
        float nextStartSlope = startSlope;
        
        for (int i = row; i <= radius; i++) {
            bool blocked = false;
            
            for (int dx = -i, dy = -i; dx <= 0; dx++) {
                // 実際の座標を計算
                int mapX = cx + dx * xx + dy * xy;
                int mapY = cy + dx * yx + dy * yy;
                
                // スロープ計算
                float leftSlope = (dx - 0.5f) / (dy + 0.5f);
                float rightSlope = (dx + 0.5f) / (dy - 0.5f);
                
                if (startSlope < rightSlope) {
                    continue;
                } else if (endSlope > leftSlope) {
                    break;
                }
                
                // 距離チェック（円形視界）
                float distance = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                if (distance <= radius) {
                    // タイルを可視に
                    if (map.InBounds(mapX, mapY)) {
                        map.SetVisible(mapX, mapY);
                    }
                }
                
                // 障害物チェック
                if (blocked) {
                    if (map.BlocksVision(mapX, mapY)) {
                        nextStartSlope = rightSlope;
                        continue;
                    } else {
                        blocked = false;
                        startSlope = nextStartSlope;
                    }
                } else {
                    if (map.BlocksVision(mapX, mapY) && i < radius) {
                        blocked = true;
                        CastLight(map, cx, cy, radius, i + 1, startSlope, leftSlope,
                                  xx, xy, yx, yy);
                        nextStartSlope = rightSlope;
                    }
                }
            }
            
            if (blocked) break;
        }
    }
};

} // namespace Roguelike::Systems

