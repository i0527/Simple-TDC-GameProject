#pragma once

// 讓呎ｺ悶Λ繧､繝悶Λ繝ｪ
#include <string>
#include <unordered_map>
#include <vector>

// 繝励Ο繧ｸ繧ｧ繧ｯ繝亥・
#include "../config/RenderPrimitives.hpp"
#include "../api/BaseSystemAPI.hpp"
#include "../config/SharedContext.hpp"

namespace game {
namespace core {
namespace ui {

enum class BattleHUDActionType {
    None,
    TogglePause,
    SetSpeed,
    SpawnUnit
};

struct BattleHUDAction {
    BattleHUDActionType type = BattleHUDActionType::None;
    float speed = 1.0f;            // SetSpeed逕ｨ
    std::string unitId;            // SpawnUnit逕ｨ
};

/// @brief 逕ｻ蜒丞ｯ・○縺ｮ繧ｷ繝ｳ繝励ΝHUD・井ｸ企Κ謫堺ｽ懶ｼ倶ｸ矩Κ10譫繝ｦ繝九ャ繝医ヰ繝ｼ・・
class BattleHUDRenderer {
public:
    explicit BattleHUDRenderer(BaseSystemAPI* sysAPI);
    ~BattleHUDRenderer() = default;

    void Render(const SharedContext& ctx,
                int playerTowerHp, int playerTowerMaxHp,
                int enemyTowerHp, int enemyTowerMaxHp,
                int gold,
                int goldMax,
                float gameSpeed,
                bool isPaused,
                float currentTime,
                const std::unordered_map<std::string, float>& cooldownUntil);

    /// @brief 繝槭え繧ｹ繧ｯ繝ｪ繝・け繧辿UD縺ｨ縺励※隗｣驥医＠縲√い繧ｯ繧ｷ繝ｧ繝ｳ繧定ｿ斐☆
    BattleHUDAction HandleClick(const SharedContext& ctx,
                                Vec2 mousePos,
                                int gold,
                                float currentTime,
                                const std::unordered_map<std::string, float>& cooldownUntil);

private:
    BaseSystemAPI* sysAPI_;

    struct RectButton {
        Rect rect{};
        BattleHUDAction action{};
    };

    struct UnitSlotButton {
        Rect slotRect{};
        std::string unitId;
        int costGold = 0;
        bool isEnabled = false;
    };

    std::vector<RectButton> topButtons_;
    std::vector<UnitSlotButton> unitSlotButtons_;

    void RenderTopBar(int playerHp, int playerMaxHp,
                      int enemyHp, int enemyMaxHp,
                      float gameSpeed, bool isPaused);

    void RenderBottomBar(const SharedContext& ctx,
                         int gold,
                         int goldMax,
                         float currentTime,
                         const std::unordered_map<std::string, float>& cooldownUntil);

    static bool IsMouseInRect(Vec2 mouse, Rect rect);
    static float SafePct(int current, int max);
};

} // namespace ui
} // namespace core
} // namespace game

