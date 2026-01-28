#pragma once

// ???????
#include <string>
#include <unordered_map>
#include <vector>

// ???????E
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
    SpawnUnit,
    GiveUp
};

struct BattleHUDAction {
    BattleHUDActionType type = BattleHUDActionType::None;
    float speed = 1.0f;            // SetSpeed?
    std::string unitId;            // SpawnUnit?
};

/// @brief ???E???????HUD?E????????10????????E?E
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
                const std::unordered_map<std::string, float>& cooldownUntil,
                bool isInfiniteStage = false);

    /// @brief ??????E???HUD???????????????
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
                      float gameSpeed, bool isPaused,
                      bool isInfiniteStage = false);

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

