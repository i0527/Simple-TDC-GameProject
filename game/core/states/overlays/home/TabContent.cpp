#include "TabContent.hpp"
#include "ITabContent.hpp"
#include "../IOverlay.hpp"
#include "../FormationOverlay.hpp"
#include "../GachaOverlay.hpp"
#include "../StageSelectOverlay.hpp"
#include "../CodexOverlay.hpp"
#include "../EnhancementOverlay.hpp"
#include "../CharacterEnhancementOverlay.hpp"
#include "../SettingsOverlay.hpp"
#include "../../../../utils/Log.h"

namespace game {
namespace core {
namespace states {
namespace overlays {
namespace home {

namespace {

class OverlayTabContent : public ITabContent {
public:
    explicit OverlayTabContent(std::unique_ptr<IOverlay> overlay)
        : overlay_(std::move(overlay)) {}

    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) override {
        if (!overlay_) {
            return false;
        }
        return overlay_->Initialize(systemAPI, uiAPI);
    }

    void Update(SharedContext& ctx, float deltaTime) override {
        if (overlay_) {
            overlay_->Update(ctx, deltaTime);
        }
    }

    void Render(SharedContext& ctx) override {
        if (overlay_ && !overlay_->IsImGuiOverlay()) {
            overlay_->Render(ctx);
        }
    }

    void RenderImGui(SharedContext& ctx) override {
        if (overlay_ && overlay_->IsImGuiOverlay()) {
            overlay_->Render(ctx);
        }
    }

    void Shutdown() override {
        if (overlay_) {
            overlay_->Shutdown();
        }
    }

    bool RequestTransition(GameState& nextState) const override {
        if (!overlay_) {
            return false;
        }
        return overlay_->RequestTransition(nextState);
    }

    bool RequestQuit() const override {
        if (!overlay_) {
            return false;
        }
        return overlay_->RequestQuit();
    }

private:
    std::unique_ptr<IOverlay> overlay_;
};

} // namespace

TabContent::TabContent()
    : current_tab_(HomeTab::StageSelect),
      systemAPI_(nullptr),
      uiAPI_(nullptr) {}

TabContent::~TabContent() {
    Shutdown();
}

bool TabContent::Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) {
    systemAPI_ = systemAPI;
    uiAPI_ = uiAPI;

    if (!systemAPI) {
        LOG_ERROR("TabContent: systemAPI is null");
        return false;
    }

    for (int i = 0; i < static_cast<int>(HomeTab::COUNT); ++i) {
        auto content = CreateContent(static_cast<HomeTab>(i), systemAPI, uiAPI);
        if (content) {
            if (content->Initialize(systemAPI, uiAPI)) {
                contents_[i] = std::move(content);
                LOG_INFO("TabContent: Initialized content for tab {}", i);
            } else {
                LOG_ERROR("TabContent: Failed to initialize content for tab {}", i);
            }
        }
    }

    return true;
}

std::unique_ptr<ITabContent> TabContent::CreateContent(HomeTab tab, BaseSystemAPI* api, UISystemAPI* uiAPI) {
    static_cast<void>(api);
    static_cast<void>(uiAPI);
    std::unique_ptr<IOverlay> overlay;

    switch (tab) {
        case HomeTab::StageSelect:
            overlay = std::make_unique<StageSelectOverlay>();
            break;
        case HomeTab::Formation:
            overlay = std::make_unique<FormationOverlay>();
            break;
        case HomeTab::Unit:
            overlay = std::make_unique<CharacterEnhancementOverlay>();
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
            LOG_WARN("TabContent: Unknown tab {}", static_cast<int>(tab));
            return nullptr;
    }

    return std::make_unique<OverlayTabContent>(std::move(overlay));
}

void TabContent::SwitchTab(HomeTab tab) {
    if (current_tab_ == tab) {
        return;
    }
    current_tab_ = tab;
    LOG_INFO("TabContent: Switched to tab: {}", static_cast<int>(tab));
}

ITabContent* TabContent::GetCurrentContent() const {
    auto it = contents_.find(static_cast<int>(current_tab_));
    if (it != contents_.end() && it->second) {
        return it->second.get();
    }
    return nullptr;
}

void TabContent::Update(float deltaTime, SharedContext& ctx) {
    if (auto* content = GetCurrentContent()) {
        content->Update(ctx, deltaTime);
    }
}

void TabContent::Render(SharedContext& ctx) {
    if (auto* content = GetCurrentContent()) {
        content->Render(ctx);
    }
}

void TabContent::RenderImGui(SharedContext& ctx) {
    if (auto* content = GetCurrentContent()) {
        content->RenderImGui(ctx);
    }
}

bool TabContent::RequestTransition(GameState& nextState) const {
    if (auto* content = GetCurrentContent()) {
        return content->RequestTransition(nextState);
    }
    return false;
}

bool TabContent::RequestQuit() const {
    if (auto* content = GetCurrentContent()) {
        return content->RequestQuit();
    }
    return false;
}

void TabContent::Shutdown() {
    for (auto& pair : contents_) {
        if (pair.second) {
            pair.second->Shutdown();
        }
    }
    contents_.clear();
    LOG_INFO("TabContent: Shutdown");
}

} // namespace home
} // namespace overlays
} // namespace states
} // namespace core
} // namespace game
