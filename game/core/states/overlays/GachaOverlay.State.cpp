#include "GachaOverlay.hpp"
#include "GachaOverlayInternal.hpp"

// 標準ライブラリ
#include <algorithm>
#include <unordered_map>

// プロジェクト内
#include "../../api/GameplayDataAPI.hpp"

namespace game {
namespace core {

using namespace gacha_internal;

void GachaOverlay::ClearResultCards() {
    if (!panel_) {
        resultCards_.clear();
        showMessageOverlay_ = false;
        return;
    }

    for (auto& card : resultCards_) {
        if (!card) {
            continue;
        }
        panel_->RemoveChild(card);
        card->Shutdown();
    }
    resultCards_.clear();
    showMessageOverlay_ = false;
}

void GachaOverlay::BuildGachaPool(const GameplayDataAPI& gameplayDataAPI) {
    pool_.clear();
    poolSrUp_.clear();
    poolSsr_.clear();
    rateN_ = 0.0f;
    rateR_ = 0.0f;
    rateSR_ = 0.0f;
    rateSSR_ = 0.0f;

    const std::unordered_map<std::string, GachaRarity> overrides = {
        {"eq_sword_001", GachaRarity::N},
        {"eq_shield_001", GachaRarity::N},
        {"eq_armor_001", GachaRarity::N},
    };

    int totalWeight = 0;
    int weightN = 0;
    int weightR = 0;
    int weightSR = 0;
    int weightSSR = 0;

    for (const auto* eq : gameplayDataAPI.GetAllEquipment()) {
        if (!eq) {
            continue;
        }
        GachaRarity rarity = GachaRarity::R;
        const auto it = overrides.find(eq->id);
        if (it != overrides.end()) {
            rarity = it->second;
        } else if (eq->id.find("ssr") != std::string::npos ||
                   eq->id.find("legend") != std::string::npos) {
            rarity = GachaRarity::SSR;
        } else if (eq->id.find("sr") != std::string::npos ||
                   eq->id.find("epic") != std::string::npos) {
            rarity = GachaRarity::SR;
        }

        const int weight = GetRarityWeightInternal(rarity);
        GachaEntry entry;
        entry.equipmentId = eq->id;
        entry.equipment = eq;
        entry.rarity = rarity;
        entry.weight = weight;
        pool_.push_back(entry);

        totalWeight += weight;
        switch (rarity) {
        case GachaRarity::N:
            weightN += weight;
            break;
        case GachaRarity::R:
            weightR += weight;
            break;
        case GachaRarity::SR:
            weightSR += weight;
            break;
        case GachaRarity::SSR:
            weightSSR += weight;
            break;
        }

        if (rarity == GachaRarity::SR || rarity == GachaRarity::SSR) {
            poolSrUp_.push_back(entry);
        }
        if (rarity == GachaRarity::SSR) {
            poolSsr_.push_back(entry);
        }
    }

    if (totalWeight > 0) {
        rateN_ = static_cast<float>(weightN) * 100.0f /
                 static_cast<float>(totalWeight);
        rateR_ = static_cast<float>(weightR) * 100.0f /
                 static_cast<float>(totalWeight);
        rateSR_ = static_cast<float>(weightSR) * 100.0f /
                  static_cast<float>(totalWeight);
        rateSSR_ = static_cast<float>(weightSSR) * 100.0f /
                   static_cast<float>(totalWeight);
    }

    poolBuilt_ = true;
    RefreshPoolList();
}

void GachaOverlay::RefreshPoolList() {
    if (!poolList_) {
        return;
    }
    poolList_->ClearItems();

    std::vector<GachaEntry> sorted = pool_;
    std::sort(sorted.begin(), sorted.end(),
              [](const GachaEntry& a, const GachaEntry& b) {
                  return static_cast<int>(a.rarity) >
                         static_cast<int>(b.rarity);
              });

    for (const auto& entry : sorted) {
        if (!entry.equipment) {
            continue;
        }
        ui::ListItem item;
        item.id = entry.equipmentId;
        item.label = entry.equipment->name;
        item.value = RarityToString(entry.rarity);
        poolList_->AddItem(item);
    }
}

void GachaOverlay::RefreshHistoryList(const GameplayDataAPI& gameplayDataAPI) {
    if (!historyList_) {
        return;
    }
    historyList_->ClearItems();
    const auto& history = gameplayDataAPI.GetGachaHistory();
    if (history.empty()) {
        return;
    }

    int count = 0;
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        if (count >= HISTORY_DISPLAY_LIMIT) {
            break;
        }
        const auto& entry = *it;
        ui::ListItem item;
        item.id = std::to_string(entry.seq);
        item.label = entry.rarity + " " + entry.equipmentId;
        if (cachedGameplayDataAPI_) {
            if (const auto* eq =
                    cachedGameplayDataAPI_->GetEquipment(entry.equipmentId)) {
                item.label = entry.rarity + " " + eq->name;
            }
        }
        item.value = "所持: " + std::to_string(entry.countAfter);
        historyList_->AddItem(item);
        count++;
    }
}

void GachaOverlay::UpdateTabVisibility() {
    const bool drawVisible = currentTab_ == GachaTab::Draw;
    const bool ratesVisible = currentTab_ == GachaTab::Rates;
    const bool historyVisible = currentTab_ == GachaTab::History;
    const bool exchangeVisible = currentTab_ == GachaTab::Exchange;
    const bool cardsVisible = drawVisible || showMessageOverlay_;

    if (singleGachaButton_) {
        singleGachaButton_->SetVisible(drawVisible);
    }
    if (tenGachaButton_) {
        tenGachaButton_->SetVisible(drawVisible);
    }
    if (skipRevealButton_) {
        skipRevealButton_->SetVisible(drawVisible);
    }

    for (auto& card : resultCards_) {
        if (card) {
            card->SetVisible(cardsVisible);
        }
    }

    if (poolList_) {
        poolList_->SetVisible(ratesVisible);
    }
    if (historyList_) {
        historyList_->SetVisible(historyVisible);
    }
    if (exchangeTicketButton_) {
        exchangeTicketButton_->SetVisible(exchangeVisible);
    }
    if (exchangeTicketTenButton_) {
        exchangeTicketTenButton_->SetVisible(exchangeVisible);
    }
}

GachaOverlay::GachaResult
GachaOverlay::RollFromPool(const std::vector<GachaEntry>& pool) {
    GachaResult result;
    if (pool.empty()) {
        return result;
    }
    int totalWeight = 0;
    for (const auto& entry : pool) {
        totalWeight += std::max(1, entry.weight);
    }
    std::uniform_int_distribution<int> dist(1, totalWeight);
    const int roll = dist(rng_);
    int acc = 0;
    for (const auto& entry : pool) {
        acc += std::max(1, entry.weight);
        if (roll <= acc) {
            result.equipment = entry.equipment;
            result.rarity = entry.rarity;
            return result;
        }
    }
    result.equipment = pool.back().equipment;
    result.rarity = pool.back().rarity;
    return result;
}

std::string GachaOverlay::RarityToString(GachaRarity rarity) const {
    switch (rarity) {
    case GachaRarity::N:
        return "N";
    case GachaRarity::R:
        return "R";
    case GachaRarity::SR:
        return "SR";
    case GachaRarity::SSR:
        return "SSR";
    }
    return "R";
}

} // namespace core
} // namespace game
