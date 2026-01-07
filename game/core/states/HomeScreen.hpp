#pragma once

#include "IScene.hpp"
#include "../api/BaseSystemAPI.hpp"
#include "../config/SharedContext.hpp"
#include "../config/GameState.hpp"
#include "../entities/CharacterManager.hpp"
#include "overlays/home/TabBarManager.hpp"
#include "overlays/home/ResourceHeader.hpp"
#include "overlays/home/ContentContainer.hpp"
#include <memory>

namespace game {
namespace core {
namespace states {

class HomeScreen : public IScene {
public:
    HomeScreen();
    ~HomeScreen();

    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

    // トランジション管理
    bool RequestTransition(GameState& nextState) override;
    bool RequestQuit() override { return false; }

    // SharedContext設定（GameSystemから呼ばれる）
    void SetSharedContext(SharedContext* ctx) { sharedContext_ = ctx; }

    // ImGui描画（EndFrame()内のImGuiフレーム内で呼ばれる）
    void RenderImGui();

private:
    // UI コンポーネント
    std::unique_ptr<overlays::home::ResourceHeader> header_;
    std::unique_ptr<overlays::home::ContentContainer> content_;
    std::unique_ptr<overlays::home::TabBarManager> tabbar_;

    // キャラクター管理
    std::unique_ptr<entities::CharacterManager> characterManager_;

    // 状態管理
    BaseSystemAPI* systemAPI_;
    SharedContext* sharedContext_;
    mutable bool request_transition_;
    mutable GameState next_state_;

    // タブ変更コールバック
    void OnTabChanged(overlays::home::HomeTab tab);
    
    // マウスイベント処理
    void HandleMouseInput();
};

} // namespace states
} // namespace core
} // namespace game
