#pragma once

// プロジェクト内
#include "../../../config/GameState.hpp"
#include "../../../config/SharedContext.hpp"
#include "../../../api/BaseSystemAPI.hpp"
#include "../../../api/UISystemAPI.hpp"

namespace game {
namespace core {
namespace states {
namespace overlays {
namespace home {

/// @brief ホームタブ用コンテンツ基底インターフェース
class ITabContent {
public:
    virtual ~ITabContent() = default;

    /// @brief コンテンツの初期化
    virtual bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) = 0;

    /// @brief 更新処理
    virtual void Update(SharedContext& ctx, float deltaTime) = 0;

    /// @brief Raylib描画（ImGui以外）
    virtual void Render(SharedContext& ctx) = 0;

    /// @brief ImGui描画（ImGuiフレーム内）
    virtual void RenderImGui(SharedContext& ctx) = 0;

    /// @brief 終了処理
    virtual void Shutdown() = 0;

    /// @brief ステート遷移リクエストを取得
    virtual bool RequestTransition(GameState& nextState) const = 0;

    /// @brief 終了リクエストを取得
    virtual bool RequestQuit() const = 0;
};

} // namespace home
} // namespace overlays
} // namespace states
} // namespace core
} // namespace game
