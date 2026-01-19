#pragma once

#include <vector>
#include <map>
#include <string>
#include "../config/RenderTypes.hpp"
#include <entt/entt.hpp>

namespace game {
namespace core {
namespace gamescene {

/// @brief 繧ｰ繝ｪ繝・ラ繧ｻ繝ｫ縺ｮ繧ｿ繧､繝・
enum class CellType {
    Normal,      // 騾壼ｸｸ・磯・鄂ｮ蜿ｯ閭ｽ・・
    Path,        // 謨ｵ縺ｮ騾ｲ霍ｯ
    Blocked,     // 驟咲ｽｮ荳榊庄
    SpawnPoint,  // 謨ｵ繧ｹ繝昴・繝ｳ菴咲ｽｮ
    Goal         // 繧ｴ繝ｼ繝ｫ菴咲ｽｮ
};

/// @brief 繝槭ャ繝励ち繧､繝ｫ諠・ｱ
struct MapTile {
    CellType type;
    int gridX;
    int gridY;
};

/// @brief 繝輔ぅ繝ｼ繝ｫ繝臥ｮ｡逅・け繝ｩ繧ｹ
///
/// 雋ｬ蜍・
/// - 繧ｰ繝ｪ繝・ラ繝吶・繧ｹ縺ｮ繝槭ャ繝礼ｮ｡逅・
/// - 繝斐け繧ｻ繝ｫ蠎ｧ讓吶→繧ｰ繝ｪ繝・ラ蠎ｧ讓吶・螟画鋤
/// - 繝槭ャ繝励ち繧､繝ｫ縺ｮ謠冗判
/// - 繝ｦ繝九ャ繝磯・鄂ｮ縺ｮ邂｡逅・
class FieldManager {
public:
    /// @brief 繧ｳ繝ｳ繧ｹ繝医Λ繧ｯ繧ｿ
    /// @param width 繧ｰ繝ｪ繝・ラ蟷・ｼ医そ繝ｫ謨ｰ・・
    /// @param height 繧ｰ繝ｪ繝・ラ鬮倥＆・医そ繝ｫ謨ｰ・・
    /// @param cellSize 繧ｻ繝ｫ繧ｵ繧､繧ｺ・医ヴ繧ｯ繧ｻ繝ｫ・・
    /// @param originX 繝輔ぅ繝ｼ繝ｫ繝牙次轤ｹX蠎ｧ讓呻ｼ医ヴ繧ｯ繧ｻ繝ｫ・・
    /// @param originY 繝輔ぅ繝ｼ繝ｫ繝牙次轤ｹY蠎ｧ讓呻ｼ医ヴ繧ｯ繧ｻ繝ｫ・・
    explicit FieldManager(int width = 30, int height = 16, int cellSize = 32,
                         float originX = 640.0f, float originY = 50.0f);
    
    ~FieldManager() = default;

    /// @brief 蛻晄悄蛹・
    /// @param registry ECS繝ｬ繧ｸ繧ｹ繝医Μ・亥ｰ・擂縺ｮ繝ｦ繝九ャ繝磯・鄂ｮ逕ｨ・・
    bool Initialize(entt::registry* registry);

    /// @brief 繧ｷ繝｣繝・ヨ繝繧ｦ繝ｳ
    void Shutdown();

    /// @brief 繝輔ぅ繝ｼ繝ｫ繝画緒逕ｻ
    /// @param showGrid 繧ｰ繝ｪ繝・ラ邱壹ｒ陦ｨ遉ｺ縺吶ｋ縺・
    void Render(bool showGrid = true);

    // ========== 蠎ｧ讓吝､画鋤 ==========

    /// @brief 繧ｰ繝ｪ繝・ラ蠎ｧ讓吶ｒ繝斐け繧ｻ繝ｫ蠎ｧ讓吶↓螟画鋤
    /// @param gx 繧ｰ繝ｪ繝・ラX蠎ｧ讓・
    /// @param gy 繧ｰ繝ｪ繝・ラY蠎ｧ讓・
    /// @return 繝斐け繧ｻ繝ｫ蠎ｧ讓・
    Vector2 GridToPixel(int gx, int gy) const;

    /// @brief 繝斐け繧ｻ繝ｫ蠎ｧ讓吶ｒ繧ｰ繝ｪ繝・ラ蠎ｧ讓吶↓螟画鋤
    /// @param px 繝斐け繧ｻ繝ｫX蠎ｧ讓・
    /// @param py 繝斐け繧ｻ繝ｫY蠎ｧ讓・
    /// @return 繧ｰ繝ｪ繝・ラ蠎ｧ讓呻ｼ・irst: gx, second: gy・・
    std::pair<int, int> PixelToGrid(float px, float py) const;

    /// @brief 繧ｰ繝ｪ繝・ラ蠎ｧ讓吶′譛牙柑遽・峇蜀・°遒ｺ隱・
    /// @param gx 繧ｰ繝ｪ繝・ラX蠎ｧ讓・
    /// @param gy 繧ｰ繝ｪ繝・ラY蠎ｧ讓・
    /// @return 譛牙柑遽・峇蜀・・蝣ｴ蜷・rue
    bool IsValidGridPosition(int gx, int gy) const;

    // ========== 繝ｦ繝九ャ繝磯・鄂ｮ邂｡逅・==========

    /// @brief 繝ｦ繝九ャ繝磯・鄂ｮ
    /// @param unitEntity 繝ｦ繝九ャ繝医お繝ｳ繝・ぅ繝・ぅ
    /// @param gx 繧ｰ繝ｪ繝・ラX蠎ｧ讓・
    /// @param gy 繧ｰ繝ｪ繝・ラY蠎ｧ讓・
    /// @return 驟咲ｽｮ謌仙粥譎Ｕrue
    bool PlaceUnit(entt::entity unitEntity, int gx, int gy);

    /// @brief 繝ｦ繝九ャ繝亥炎髯､
    /// @param gx 繧ｰ繝ｪ繝・ラX蠎ｧ讓・
    /// @param gy 繧ｰ繝ｪ繝・ラY蠎ｧ讓・
    /// @return 蜑企勁謌仙粥譎Ｕrue
    bool RemoveUnit(int gx, int gy);

    /// @brief 謖・ｮ壻ｽ咲ｽｮ縺ｮ繝ｦ繝九ャ繝医ｒ蜿門ｾ・
    /// @param gx 繧ｰ繝ｪ繝・ラX蠎ｧ讓・
    /// @param gy 繧ｰ繝ｪ繝・ラY蠎ｧ讓・
    /// @return 繝ｦ繝九ャ繝医お繝ｳ繝・ぅ繝・ぅ・亥ｭ伜惠縺励↑縺・ｴ蜷医・entt::null・・
    entt::entity GetUnitAt(int gx, int gy) const;

    /// @brief 繧ｻ繝ｫ縺碁・鄂ｮ蜿ｯ閭ｽ縺狗｢ｺ隱・
    /// @param gx 繧ｰ繝ｪ繝・ラX蠎ｧ讓・
    /// @param gy 繧ｰ繝ｪ繝・ラY蠎ｧ讓・
    /// @return 驟咲ｽｮ蜿ｯ閭ｽ縺ｪ蝣ｴ蜷・rue
    bool IsPlaceable(int gx, int gy) const;

    // ========== 繧｢繧ｯ繧ｻ繧ｵ ==========

    /// @brief 繧ｰ繝ｪ繝・ラ蟷・ｒ蜿門ｾ・
    int GetWidth() const { return width_; }

    /// @brief 繧ｰ繝ｪ繝・ラ鬮倥＆繧貞叙蠕・
    int GetHeight() const { return height_; }

    /// @brief 繧ｻ繝ｫ繧ｵ繧､繧ｺ繧貞叙蠕・
    int GetCellSize() const { return cellSize_; }

    /// @brief 蜴溽せ蠎ｧ讓吶ｒ蜿門ｾ・
    Vector2 GetOrigin() const { return Vector2{originX_, originY_}; }

    /// @brief 繧ｿ繧､繝ｫ諠・ｱ繧貞叙蠕・
    const std::vector<MapTile>& GetTiles() const { return tiles_; }

    /// @brief 謨ｵ繝代せ繧貞叙蠕・
    const std::vector<Vector2>& GetEnemyPath() const { return enemyPath_; }

private:
    // 繧ｰ繝ｪ繝・ラ險ｭ螳・
    int width_;
    int height_;
    int cellSize_;
    float originX_;
    float originY_;

    // ECS繝ｬ繧ｸ繧ｹ繝医Μ・亥盾辣ｧ・・
    entt::registry* registry_;

    // 繝槭ャ繝励ョ繝ｼ繧ｿ
    std::vector<MapTile> tiles_;
    std::vector<Vector2> enemyPath_;

    // 繝ｦ繝九ャ繝磯・鄂ｮ繝槭ャ繝暦ｼ医げ繝ｪ繝・ラ蠎ｧ讓・-> 繧ｨ繝ｳ繝・ぅ繝・ぅ・・
    std::map<std::pair<int, int>, entt::entity> gridMap_;

    /// @brief 繝・ヵ繧ｩ繝ｫ繝医・繝・・繧堤函謌・
    void GenerateDefaultMap();

    /// @brief 繧ｰ繝ｪ繝・ラ邱壹ｒ謠冗判
    void DrawGrid();

    /// @brief 閭梧勹繧ｿ繧､繝ｫ繧呈緒逕ｻ
    void DrawTiles();

    /// @brief 謨ｵ繝代せ繧呈緒逕ｻ
    void DrawEnemyPath();
};

} // namespace gamescene
} // namespace core
} // namespace game
