#pragma once

#include "IOverlay.hpp"
#include "../../ui/components/Card.hpp"
#include "../../ui/components/Button.hpp"
#include "../../ui/components/List.hpp"
#include "../../ui/components/Panel.hpp"
#include <memory>
#include <random>
#include <vector>

namespace game {
namespace core {

namespace entities {
struct Equipment;
} // namespace entities

class GameplayDataAPI;

enum class GachaRarity {
    N,
    R,
    SR,
    SSR
};

/// @brief ガチャオーバーレイ
///
/// ガチャ画面を表示するオーバーレイ。
/// Cardコンポーネントを使用します。
class GachaOverlay : public IOverlay {
public:
    GachaOverlay();
    ~GachaOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Gacha; }
    bool IsImGuiOverlay() const override { return true; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    enum class GachaTab {
        Draw,
        Rates,
        History,
        Exchange
    };

    struct GachaEntry {
        std::string equipmentId;
        const entities::Equipment* equipment = nullptr;
        GachaRarity rarity = GachaRarity::R;
        int weight = 1;
    };

    struct GachaResult {
        const entities::Equipment* equipment = nullptr;
        GachaRarity rarity = GachaRarity::R;
        int countAfter = 0;
    };

    BaseSystemAPI* systemAPI_;
    UISystemAPI* uiAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;

    // UIコンポーネント
    std::shared_ptr<ui::Panel> panel_;
    std::vector<std::shared_ptr<ui::Card>> resultCards_;
    std::shared_ptr<ui::Button> singleGachaButton_;
    std::shared_ptr<ui::Button> tenGachaButton_;
    std::shared_ptr<ui::Button> skipRevealButton_;
    std::shared_ptr<ui::Button> tabDrawButton_;
    std::shared_ptr<ui::Button> tabRatesButton_;
    std::shared_ptr<ui::Button> tabHistoryButton_;
    std::shared_ptr<ui::Button> tabExchangeButton_;
    std::shared_ptr<ui::List> poolList_;
    std::shared_ptr<ui::List> historyList_;
    std::shared_ptr<ui::Button> exchangeTicketButton_;
    std::shared_ptr<ui::Button> exchangeTicketTenButton_;

    // ガチャ処理（クリック→Updateで実行）
    std::mt19937 rng_;
    int pendingRollCount_ = 0; // 0=なし、1 or 10
    bool poolBuilt_ = false;
    std::vector<GachaEntry> pool_;
    std::vector<GachaEntry> poolSrUp_;
    std::vector<GachaEntry> poolSsr_;
    GachaTab currentTab_ = GachaTab::Draw;
    float rateN_ = 0.0f;
    float rateR_ = 0.0f;
    float rateSR_ = 0.0f;
    float rateSSR_ = 0.0f;
    std::vector<GachaResult> pendingResults_;
    size_t revealedCount_ = 0;
    float revealTimer_ = 0.0f;
    float revealInterval_ = 0.08f;
    bool skipRevealRequested_ = false;
    bool showMessageOverlay_ = false;
    GameplayDataAPI* cachedGameplayDataAPI_ = nullptr;

    // レイアウト/アニメーション
    float panelX_ = 0.0f;
    float panelY_ = 0.0f;
    float panelW_ = 0.0f;
    float panelH_ = 0.0f;
    float singleButtonX_ = 0.0f;
    float singleButtonY_ = 0.0f;
    float tenButtonX_ = 0.0f;
    float tenButtonY_ = 0.0f;
    float buttonW_ = 0.0f;
    float buttonH_ = 0.0f;
    float introProgress_ = 0.0f;
    float pulseTime_ = 0.0f;
    float contentLeft_ = 0.0f;
    float contentTop_ = 0.0f;
    float contentRight_ = 0.0f;
    float contentBottom_ = 0.0f;

    void ClearResultCards();
    void ShowMessageCard(float contentWidth, float contentHeight, const std::string& title, const std::string& message);
    void ShowEquipmentResults(float contentWidth, float contentHeight, const std::vector<const entities::Equipment*>& results);
    void AddEquipmentResultCard(float contentWidth, float contentHeight, const GachaResult& result, int index, int total);
    void BuildGachaPool(const GameplayDataAPI& gameplayDataAPI);
    void RefreshPoolList();
    void RefreshHistoryList(const GameplayDataAPI& gameplayDataAPI);
    void UpdateTabVisibility();
    GachaResult RollFromPool(const std::vector<GachaEntry>& pool);
    std::string RarityToString(GachaRarity rarity) const;
};

} // namespace core
} // namespace game
