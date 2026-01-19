#include "GachaOverlay.hpp"
#include "GachaOverlayInternal.hpp"

// プロジェクト内
#include "../../../utils/Log.h"
#include "../../api/GameplayDataAPI.hpp"
#include "../../system/PlayerDataManager.hpp"
#include "../../api/InputSystemAPI.hpp"

namespace game {
namespace core {

using namespace gacha_internal;

void GachaOverlay::Update(SharedContext& ctx, float deltaTime) {
    if (!isInitialized_) {
        return;
    }

    cachedGameplayDataAPI_ = ctx.gameplayDataAPI;

    if (!poolBuilt_ && cachedGameplayDataAPI_) {
        BuildGachaPool(*cachedGameplayDataAPI_);
    }

    // クリック要求をUpdateで処理（ctxへアクセスできるように）
    if (pendingRollCount_ == 1 || pendingRollCount_ == 10) {
        const int rollCount = pendingRollCount_;
        pendingRollCount_ = 0;

        const int cost = GetCostByRollCount(rollCount);
        if (!ctx.gameplayDataAPI) {
            LOG_ERROR("GachaOverlay: gameplayDataAPI is null");
            ShowMessageCard(panelW_, panelH_, "エラー",
                            "必要なデータにアクセスできません。");
        } else if (ctx.gameplayDataAPI->GetTickets() < cost) {
            const int shortage = cost - ctx.gameplayDataAPI->GetTickets();
            ShowMessageCard(
                panelW_, panelH_, "チケット不足",
                "チケットが足りません。（不足: " + std::to_string(shortage) +
                    "）\n交換タブでチケットに交換できます。");
            currentTab_ = GachaTab::Exchange;
            UpdateTabVisibility();
        } else {
            if (!poolBuilt_ || pool_.empty()) {
                ShowMessageCard(panelW_, panelH_, "エラー",
                                "装備マスタが空です。");
            } else {
                std::vector<GachaResult> results;
                results.reserve(static_cast<size_t>(rollCount));
                int pityCounter = ctx.gameplayDataAPI->GetGachaPityCounter();

                for (int i = 0; i < rollCount; ++i) {
                    const bool forceAtLeastSr =
                        (rollCount == 10 && i == rollCount - 1);
                    const bool forceSsr = (pityCounter + 1 >= PITY_HARD);
                    const std::vector<GachaEntry>* rollPool = &pool_;
                    if (forceSsr && !poolSsr_.empty()) {
                        rollPool = &poolSsr_;
                    } else if (forceAtLeastSr && !poolSrUp_.empty()) {
                        rollPool = &poolSrUp_;
                    }

                    GachaResult result = RollFromPool(*rollPool);
                    if (!result.equipment) {
                        continue;
                    }

                    if (result.rarity == GachaRarity::SSR) {
                        pityCounter = 0;
                    } else {
                        pityCounter += 1;
                    }

                    const int countBefore = ctx.gameplayDataAPI->GetOwnedEquipmentCount(
                        result.equipment->id);
                    const int countAfter = countBefore + 1;
                    result.countAfter = countAfter;
                    ctx.gameplayDataAPI->SetOwnedEquipmentCount(
                        result.equipment->id, countAfter);

                    if (countBefore > 0) {
                        ctx.gameplayDataAPI->AddGachaDust(
                            GetDustRewardInternal(result.rarity));
                    }

                    PlayerDataManager::PlayerSaveData::GachaHistoryEntry entry;
                    entry.seq = ctx.gameplayDataAPI->NextGachaRollSequence();
                    entry.equipmentId = result.equipment->id;
                    entry.rarity = RarityToString(result.rarity);
                    entry.countAfter = countAfter;
                    ctx.gameplayDataAPI->AddGachaHistoryEntry(entry);

                    results.push_back(result);
                }

                ctx.gameplayDataAPI->SetGachaPityCounter(pityCounter);
                ctx.gameplayDataAPI->AddTickets(-cost);
                ctx.gameplayDataAPI->Save();

                ClearResultCards();
                pendingResults_ = results;
                revealedCount_ = 0;
                revealTimer_ = 0.0f;
                skipRevealRequested_ = false;
                showMessageOverlay_ = false;
            }
        }
    }

    const bool isRevealing =
        !pendingResults_.empty() && revealedCount_ < pendingResults_.size();
    if (isRevealing) {
        if (skipRevealRequested_) {
            const int total = static_cast<int>(pendingResults_.size());
            for (int i = static_cast<int>(revealedCount_); i < total; ++i) {
                AddEquipmentResultCard(panelW_, panelH_, pendingResults_[i], i, total);
            }
            revealedCount_ = pendingResults_.size();
        } else {
            revealTimer_ += deltaTime;
            while (revealTimer_ >= revealInterval_ &&
                   revealedCount_ < pendingResults_.size()) {
                const int index = static_cast<int>(revealedCount_);
                const int total = static_cast<int>(pendingResults_.size());
                AddEquipmentResultCard(panelW_, panelH_, pendingResults_[revealedCount_],
                                       index, total);
                revealedCount_++;
                revealTimer_ -= revealInterval_;
            }
        }
        if (revealedCount_ >= pendingResults_.size()) {
            pendingResults_.clear();
            skipRevealRequested_ = false;
            revealTimer_ = 0.0f;
        }
    }

    if (panel_) {
        panel_->Update(deltaTime);
    }

    if (singleGachaButton_ && tenGachaButton_ && ctx.gameplayDataAPI) {
        const int tickets = ctx.gameplayDataAPI->GetTickets();
        singleGachaButton_->SetEnabled(!isRevealing && tickets >= COST_SINGLE);
        tenGachaButton_->SetEnabled(!isRevealing && tickets >= COST_TEN);
    }

    if (skipRevealButton_) {
        skipRevealButton_->SetVisible(isRevealing);
        skipRevealButton_->SetEnabled(isRevealing);
    }

    if (exchangeTicketButton_ && exchangeTicketTenButton_ && ctx.gameplayDataAPI) {
        const int dust = ctx.gameplayDataAPI->GetGachaDust();
        exchangeTicketButton_->SetEnabled(dust >= DUST_FOR_TICKET);
        exchangeTicketTenButton_->SetEnabled(dust >= DUST_FOR_TEN_TICKETS);
    }

    if (currentTab_ == GachaTab::Rates && poolList_ &&
        poolList_->GetItemCount() == 0) {
        RefreshPoolList();
    }

    if (currentTab_ == GachaTab::History && historyList_ && ctx.gameplayDataAPI) {
        RefreshHistoryList(*ctx.gameplayDataAPI);
    }

    introProgress_ = std::min(1.0f, introProgress_ + deltaTime * 2.5f);
    pulseTime_ += deltaTime;

    if (ctx.inputAPI && ctx.inputAPI->IsEscapePressed()) {
        requestClose_ = true;
    }
}

bool GachaOverlay::RequestClose() const {
    if (requestClose_) {
        requestClose_ = false;
        return true;
    }
    return false;
}

bool GachaOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}

} // namespace core
} // namespace game
