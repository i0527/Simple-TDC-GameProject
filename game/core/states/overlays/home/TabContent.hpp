#pragma once

#include "ITabContent.hpp"
#include "TabBarManager.hpp"
#include "../../../api/BaseSystemAPI.hpp"
#include "../../../api/UISystemAPI.hpp"
#include "../../../config/SharedContext.hpp"
#include <memory>
#include <unordered_map>

namespace game {
namespace core {
namespace states {
namespace overlays {
namespace home {

// タブ選択に応じてコンテンツを切り替えるコンテナ
class TabContent {
public:
    TabContent();
    ~TabContent();

    // 初期化（各タブのコンテンツ生成）
    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI);

    // UI更新・描画
    void Update(float deltaTime, SharedContext& ctx);
    void Render(SharedContext& ctx);
    void RenderImGui(SharedContext& ctx);

    // タブ切り替え
    void SwitchTab(HomeTab tab);

    // 遷移リクエスト（現在タブ）
    bool RequestTransition(GameState& nextState) const;
    bool RequestQuit() const;

    // 終了処理
    void Shutdown();

private:
    ITabContent* GetCurrentContent() const;
    std::unique_ptr<ITabContent> CreateContent(HomeTab tab, BaseSystemAPI* api, UISystemAPI* uiAPI);

    std::unordered_map<int, std::unique_ptr<ITabContent>> contents_;
    HomeTab current_tab_;
    BaseSystemAPI* systemAPI_;
    UISystemAPI* uiAPI_;
};

} // namespace home
} // namespace overlays
} // namespace states
} // namespace core
} // namespace game
