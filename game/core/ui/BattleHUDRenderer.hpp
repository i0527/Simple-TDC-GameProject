#pragma once

// 標準ライブラリ
#include <string>
#include <unordered_map>
#include <vector>

// 外部ライブラリ
#include <raylib.h>

// プロジェクト内
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
    float speed = 1.0f;            // SetSpeed用
    std::string unitId;            // SpawnUnit用
};

/// @brief 画像寄せのシンプルHUD（上部操作＋下部10枠ユニットバー）
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

    /// @brief マウスクリックをHUDとして解釈し、アクションを返す
    BattleHUDAction HandleClick(const SharedContext& ctx,
                                Vector2 mousePos,
                                int gold,
                                float currentTime,
                                const std::unordered_map<std::string, float>& cooldownUntil);

private:
    BaseSystemAPI* sysAPI_;

    struct RectButton {
        Rectangle rect{};
        BattleHUDAction action{};
    };

    struct UnitSlotButton {
        Rectangle slotRect{};
        Rectangle spawnRect{};
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

    static bool IsMouseInRect(Vector2 mouse, Rectangle rect);
    static float SafePct(int current, int max);
};

} // namespace ui
} // namespace core
} // namespace game

