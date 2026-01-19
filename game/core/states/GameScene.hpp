#pragma once

#include "IScene.hpp"
#include "../api/BaseSystemAPI.hpp"
#include "../config/SharedContext.hpp"
#include "../config/GameState.hpp"
#include "../game/BattleRenderer.hpp"
#include "../api/BattleProgressAPI.hpp"
#include "../ui/BattleHUDRenderer.hpp"
#include <memory>
#include <string>

namespace game {
namespace core {
class InputSystemAPI;
class BattleProgressAPI;

namespace states {

/// @brief 繧ｲ繝ｼ繝繧ｷ繝ｼ繝ｳ繧ｯ繝ｩ繧ｹ
///
/// 雋ｬ蜍・
/// - 1繝ｬ繝ｼ繝ｳ讓ｪ繧ｹ繧ｯ繝ｭ繝ｼ繝ｫ謌ｦ髣假ｼ医↓繧・ｓ縺灘梛・峨・邂｡逅・
/// - 蜈･蜉帙→UI縺ｮ邨ｱ蜷茨ｼ域ｮｵ髫守噪縺ｫ螳溯｣・ｼ・
/// - 繧ｲ繝ｼ繝繝ｭ繧ｸ繝・け縺ｮ騾ｲ陦悟宛蠕｡
class GameScene : public IScene {
public:
    GameScene();
    ~GameScene() override;

    // ========== IScene繧､繝ｳ繧ｿ繝ｼ繝輔ぉ繝ｼ繧ｹ螳溯｣・==========

    /// @brief 繧ｷ繝ｼ繝ｳ縺ｮ蛻晄悄蛹・
    bool Initialize(BaseSystemAPI* systemAPI) override;

    /// @brief 繧ｷ繝ｼ繝ｳ縺ｮ譖ｴ譁ｰ蜃ｦ逅・
    void Update(float deltaTime) override;

    /// @brief 繧ｷ繝ｼ繝ｳ縺ｮ謠冗判蜃ｦ逅・
    void Render() override;

    /// @brief HUD謠冗判
    void RenderHUD() override;

    /// @brief 繧ｷ繝ｼ繝ｳ縺ｮ繧ｯ繝ｪ繝ｼ繝ｳ繧｢繝・・
    void Shutdown() override;

    /// @brief 驕ｷ遘ｻ繝ｪ繧ｯ繧ｨ繧ｹ繝医ｒ蜿門ｾ・
    bool RequestTransition(GameState& nextState) override;

    /// @brief 邨ゆｺ・Μ繧ｯ繧ｨ繧ｹ繝医ｒ蜿門ｾ・
    bool RequestQuit() override;

    // ========== SharedContext險ｭ螳・==========

    /// @brief SharedContext繧定ｨｭ螳夲ｼ・ameSystem縺九ｉ蜻ｼ縺ｰ繧後ｋ・・
    void SetSharedContext(SharedContext* ctx) override {
        sharedContext_ = ctx;
        inputAPI_ = ctx ? ctx->inputAPI : nullptr;
        battleProgressAPI_ = ctx ? ctx->battleProgressAPI : nullptr;
        if (battleRenderer_) {
            battleRenderer_->SetEcsAPI(ctx ? ctx->ecsAPI : nullptr);
        }
    }

private:
    // 繧ｳ繧｢繧ｷ繧ｹ繝・Β
    BaseSystemAPI* systemAPI_;
    SharedContext* sharedContext_;

    // 蜈･蜉・
    InputSystemAPI* inputAPI_;
    std::unique_ptr<::game::core::ui::BattleHUDRenderer> battleHud_;

    // ECS邂｡逅・ｼ域姶髣倥Θ繝九ャ繝育畑・・
    std::unique_ptr<::game::core::game::BattleRenderer> battleRenderer_;

    // 謌ｦ髣倬ｲ陦窟PI
    BattleProgressAPI* battleProgressAPI_;

    // 驕ｷ遘ｻ繝ｪ繧ｯ繧ｨ繧ｹ繝・
    mutable bool requestTransition_;
    mutable GameState nextState_;
    mutable bool requestQuit_;

    // ========== 蜀・Κ蜃ｦ逅・==========

    /// @brief 蜈･蜉帛・逅・
    void ProcessInput();

    /// @brief 繝懊ち繝ｳ繧ｯ繝ｪ繝・け蜃ｦ逅・
    void HandleButtonClick(const std::string& buttonId);

    // 謠冗判・医↓繧・ｓ縺灘梛・・
    void RenderBattle();
    void HandleHUDAction(const ::game::core::ui::BattleHUDAction& action);
};

} // namespace states
} // namespace core
} // namespace game
