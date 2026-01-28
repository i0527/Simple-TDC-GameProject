#pragma once

#include "IOverlay.hpp"
#include "../../ui/OverlayColors.hpp"
#include <memory>
#include <random>
#include <vector>

namespace game {
namespace core {

namespace entities {
struct Equipment;
struct TowerAttachment;
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
    bool IsImGuiOverlay() const override { return false; }
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
        std::string equipmentId;  // 装備時は装備ID、アタッチメント時はアタッチメントID
        const entities::Equipment* equipment = nullptr;
        const entities::TowerAttachment* attachment = nullptr;
        GachaRarity rarity = GachaRarity::R;
        int weight = 1;
    };

    struct GachaResult {
        const entities::Equipment* equipment = nullptr;
        const entities::TowerAttachment* attachment = nullptr;
        GachaRarity rarity = GachaRarity::R;
        int countAfter = 0;
    };

    BaseSystemAPI* systemAPI_;
    UISystemAPI* uiAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;

    // カードの位置とレアリティ情報
    struct CardInfo {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        GachaRarity rarity = GachaRarity::R;
        float revealTime = 0.0f; // カードが表示された時刻
        float animationProgress = 0.0f; // アニメーション進行度 (0.0-1.0)
        const entities::Equipment* equipment = nullptr;
        const entities::TowerAttachment* attachment = nullptr;
        int countAfter = 0;
        std::string title; // メッセージカード用
        std::string message; // メッセージカード用
        bool isMessageCard = false; // メッセージカードかどうか
    };
    std::vector<CardInfo> resultCardInfos_;
    float cardAnimationTimer_ = 0.0f;
    
    // 履歴アイテムのレアリティ情報
    struct HistoryItemInfo {
        std::string itemId;
        std::string label;
        std::string value;
        GachaRarity rarity = GachaRarity::R;
    };
    std::vector<HistoryItemInfo> historyItemInfos_;
    
    // 提供割合リストのアイテム情報
    struct PoolItemInfo {
        std::string equipmentId;
        std::string name;
        std::string rarity;
        float percent = 0.0f;
        std::string bar;
    };
    std::vector<PoolItemInfo> poolItemInfos_;
    
    // マウス入力状態
    Vec2 mousePos_ = {0.0f, 0.0f};
    bool mouseClicked_ = false;
    int hoveredTabIndex_ = -1;
    bool hoveredSingleButton_ = false;
    bool hoveredTenButton_ = false;
    bool hoveredSkipButton_ = false;
    bool hoveredExchange1Button_ = false;
    bool hoveredExchange10Button_ = false;

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
    
    // スクロール関連（タブ別に保持）
    float scrollYDraw_ = 0.0f;
    float scrollYRates_ = 0.0f;
    float scrollYHistory_ = 0.0f;
    bool hasAutoScrolled_ = false; // アニメーション完了後の自動スクロール実行済みフラグ

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
