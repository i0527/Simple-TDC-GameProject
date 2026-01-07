#include "ContentContainer.hpp"
#include "../FormationOverlay.hpp"
#include "../GachaOverlay.hpp"
#include "../StageSelectOverlay.hpp"
#include "../CodexOverlay.hpp"
#include "../EnhancementOverlay.hpp"
#include "../SettingsOverlay.hpp"
#include "../../../entities/CharacterManager.hpp"
#include "../../../../utils/Log.h"
#include <raylib.h>

namespace game {
namespace core {
namespace states {
namespace overlays {
namespace home {

ContentContainer::ContentContainer()
    : current_tab_(HomeTab::StageSelect)
    , systemAPI_(nullptr)
    , characterManager_(nullptr)
{
}

ContentContainer::~ContentContainer() {
    Shutdown();
}

bool ContentContainer::Initialize(BaseSystemAPI* systemAPI,
                                   entities::CharacterManager* characterManager) {
    systemAPI_ = systemAPI;
    characterManager_ = characterManager;
    
    if (!systemAPI) {
        LOG_ERROR("ContentContainer: systemAPI is null");
        return false;
    }
    
    // 全タブのオーバレイを事前生成
    for (int i = 0; i < static_cast<int>(HomeTab::COUNT); ++i) {
        auto overlay = CreateOverlay(static_cast<HomeTab>(i), systemAPI);
        if (overlay) {
            if (overlay->Initialize(systemAPI)) {
                overlays_[i] = std::move(overlay);
                LOG_INFO("ContentContainer: Initialized overlay for tab {}", i);
            } else {
                LOG_ERROR("ContentContainer: Failed to initialize overlay for tab {}", i);
            }
        }
    }
    
    return true;
}

std::unique_ptr<IOverlay> ContentContainer::CreateOverlay(HomeTab tab, 
                                                           BaseSystemAPI* api) {
    std::unique_ptr<IOverlay> overlay;
    
    switch (tab) {
        case HomeTab::StageSelect:
            overlay = std::make_unique<StageSelectOverlay>();
            break;
        case HomeTab::Formation:
            overlay = std::make_unique<FormationOverlay>();
            break;
        case HomeTab::Unit:
            // TODO: UnitOverlayを実装するまで、一旦FormationOverlayを使用
            // または、空のオーバレイを返す
            LOG_WARN("ContentContainer: UnitOverlay not implemented yet, using FormationOverlay as placeholder");
            overlay = std::make_unique<FormationOverlay>();
            break;
        case HomeTab::Enhancement:
            overlay = std::make_unique<EnhancementOverlay>();
            break;
        case HomeTab::Gacha:
            overlay = std::make_unique<GachaOverlay>();
            break;
        case HomeTab::Codex:
            overlay = std::make_unique<CodexOverlay>();
            break;
        case HomeTab::Settings:
            overlay = std::make_unique<SettingsOverlay>();
            break;
        default:
            LOG_WARN("ContentContainer: Unknown tab {}", static_cast<int>(tab));
            return nullptr;
    }
    
    return overlay;
}

void ContentContainer::SwitchTab(HomeTab tab) {
    if (current_tab_ == tab) return;
    
    current_tab_ = tab;
    LOG_INFO("ContentContainer: Switched to tab: {}", static_cast<int>(tab));
}

IOverlay* ContentContainer::GetCurrentOverlay() const {
    auto it = overlays_.find(static_cast<int>(current_tab_));
    if (it != overlays_.end() && it->second) {
        return it->second.get();
    }
    return nullptr;
}

void ContentContainer::Update(float deltaTime, SharedContext& ctx) {
    // 現在のタブのオーバレイのみ更新
    auto it = overlays_.find(static_cast<int>(current_tab_));
    if (it != overlays_.end() && it->second) {
        it->second->Update(ctx, deltaTime);
    }
}

void ContentContainer::Render(SharedContext& ctx) {
    // コンテンツ領域を確保（y: 90, height: 900）
    
    // コンテンツ領域の背景（BaseSystemAPI経由）
    if (systemAPI_) {
        systemAPI_->DrawRectangle(0, 90, 1920, 900, Color{20, 20, 20, 255});
    }
    
    // 注意: オーバレイの描画は行わない
    // オーバレイは既存の実装でImGuiを使用しているため、
    // EndFrame()内でImGuiフレームが開始された後に描画する必要があります
    // オーバレイの描画は、GameSystemのoverlayManager_->Render()で行われるか、
    // または別の方法でImGuiフレーム内で描画する必要があります
}

void ContentContainer::Shutdown() {
    for (auto& pair : overlays_) {
        if (pair.second) {
            pair.second->Shutdown();
        }
    }
    overlays_.clear();
    LOG_INFO("ContentContainer: Shutdown");
}

} // namespace home
} // namespace overlays
} // namespace states
} // namespace core
} // namespace game
