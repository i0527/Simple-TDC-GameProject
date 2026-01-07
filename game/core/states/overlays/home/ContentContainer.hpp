#pragma once

#include "../IOverlay.hpp"
#include "TabBarManager.hpp"
#include "../../../api/BaseSystemAPI.hpp"
#include "../../../config/SharedContext.hpp"
#include <memory>
#include <unordered_map>

namespace game {
namespace core {
namespace entities {
    class CharacterManager;
}
namespace states {
namespace overlays {
namespace home {

// コンテンツコンテナ: タブ選択に応じて異なるオーバレイを表示
class ContentContainer {
public:
    ContentContainer();
    ~ContentContainer();

    // 初期化（既存オーバレイのインスタンス化）
    bool Initialize(BaseSystemAPI* systemAPI, 
                    entities::CharacterManager* characterManager);

    // UI更新・描画
    void Update(float deltaTime, SharedContext& ctx);
    void Render(SharedContext& ctx);

    // タブ切り替え
    void SwitchTab(HomeTab tab);

    // 現在のオーバレイを取得（ImGui描画用）
    IOverlay* GetCurrentOverlay() const;

    // 終了処理
    void Shutdown();

private:
    // 各タブに対応するオーバレイ
    std::unordered_map<int, std::unique_ptr<IOverlay>> overlays_;
    
    HomeTab current_tab_;
    BaseSystemAPI* systemAPI_;
    entities::CharacterManager* characterManager_;

    // オーバレイ生成ヘルパー
    std::unique_ptr<IOverlay> CreateOverlay(HomeTab tab, BaseSystemAPI* api);
};

} // namespace home
} // namespace overlays
} // namespace states
} // namespace core
} // namespace game
