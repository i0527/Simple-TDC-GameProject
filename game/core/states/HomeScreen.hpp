#pragma once

#include "IScene.hpp"
#include "../api/BaseSystemAPI.hpp"
#include "../api/UISystemAPI.hpp"
#include "../config/SharedContext.hpp"
#include "../config/GameState.hpp"
#include "overlays/home/TabBarManager.hpp"
#include "overlays/home/ResourceHeader.hpp"
#include "overlays/home/TabContent.hpp"
#include <memory>

namespace game {
namespace core {
class InputSystemAPI;

namespace states {

class HomeScreen : public IScene {
public:
    HomeScreen();
    ~HomeScreen();

    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(float deltaTime) override;
    void Render() override;
    void RenderOverlay() override;
    void RenderHUD() override;
    void Shutdown() override;

    // トランジション管理
    bool RequestTransition(GameState& nextState) override;
    bool RequestQuit() override;

    // SharedContext設定（GameSystemから呼ばれる）
    void SetSharedContext(SharedContext* ctx) override {
        sharedContext_ = ctx;
        inputAPI_ = ctx ? ctx->inputAPI : nullptr;
    }

    // ImGui描画（EndFrame()内のImGuiフレーム内で呼ばれる）
    void RenderImGui() override;

private:
    // UI コンポーネント
    std::unique_ptr<overlays::home::ResourceHeader> header_;
    std::unique_ptr<overlays::home::TabContent> content_;
    std::unique_ptr<overlays::home::TabBarManager> tabbar_;

    // 状態管理
    BaseSystemAPI* systemAPI_;
    InputSystemAPI* inputAPI_;
    SharedContext* sharedContext_;
    mutable bool request_transition_;
    mutable GameState next_state_;
    mutable bool request_quit_;


    // タブ変更コールバック
    void OnTabChanged(overlays::home::HomeTab tab);
    
    // マウスイベント処理
    void HandleMouseInput();
};

} // namespace states
} // namespace core
} // namespace game
