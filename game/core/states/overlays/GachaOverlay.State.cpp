#include "GachaOverlay.hpp"
#include "GachaOverlayInternal.hpp"

// 標準ライブラリ
#include <algorithm>
#include <unordered_map>

// プロジェクト内
#include "../../api/GameplayDataAPI.hpp"
#include "../../ecs/entities/TowerAttachment.hpp"

namespace game {
namespace core {

using namespace gacha_internal;

void GachaOverlay::ClearResultCards() {
    resultCardInfos_.clear();
    showMessageOverlay_ = false;
    cardAnimationTimer_ = 0.0f;
    // スクロール位置とフラグもリセット
    scrollYDraw_ = 0.0f;
    hasAutoScrolled_ = false;
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

    // アタッチメントを統合プールに追加
    const auto& masters = gameplayDataAPI.GetAllTowerAttachmentMasters();
    for (const auto& [id, att] : masters) {
        GachaRarity rarity = GachaRarity::R;
        if (att.rarity == 2) {
            rarity = GachaRarity::SR;
        } else if (att.rarity == 3) {
            rarity = GachaRarity::SSR;
        }
        const int weight = GetRarityWeightInternal(rarity);
        GachaEntry entry;
        entry.equipmentId = id;
        entry.equipment = nullptr;
        entry.attachment = &att;
        entry.rarity = rarity;
        entry.weight = weight;
        pool_.push_back(entry);

        totalWeight += weight;
        switch (rarity) {
        case GachaRarity::R: weightR += weight; break;
        case GachaRarity::SR: weightSR += weight; break;
        case GachaRarity::SSR: weightSSR += weight; break;
        default: break;
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
    poolItemInfos_.clear();

    int totalWeight = 0;
    int maxWeight = 1;
    for (const auto& entry : pool_) {
        totalWeight += std::max(1, entry.weight);
        maxWeight = std::max(maxWeight, entry.weight);
    }

    std::vector<GachaEntry> sorted = pool_;
    std::sort(sorted.begin(), sorted.end(),
              [](const GachaEntry& a, const GachaEntry& b) {
                  return static_cast<int>(a.rarity) >
                         static_cast<int>(b.rarity);
              });

    for (const auto& entry : sorted) {
        const bool hasItem = entry.equipment || entry.attachment;
        if (!hasItem) {
            continue;
        }
        const float percent = totalWeight > 0
                                  ? static_cast<float>(entry.weight) * 100.0f /
                                        static_cast<float>(totalWeight)
                                  : 0.0f;
        const float ratio = maxWeight > 0
                                ? static_cast<float>(entry.weight) /
                                      static_cast<float>(maxWeight)
                                : 0.0f;
        const int barCells = 10;
        const int filledCells =
            std::clamp(static_cast<int>(std::round(ratio * barCells)),
                       0, barCells);
        const std::string bar =
            "[" + std::string(filledCells, '#') +
            std::string(barCells - filledCells, '-') + "]";

        PoolItemInfo info;
        info.equipmentId = entry.equipmentId;
        info.name = entry.equipment ? entry.equipment->name : (entry.attachment ? entry.attachment->name : "");
        info.rarity = RarityToString(entry.rarity);
        info.percent = percent;
        info.bar = bar;
        poolItemInfos_.push_back(info);
    }
}

void GachaOverlay::RefreshHistoryList(const GameplayDataAPI& gameplayDataAPI) {
    historyItemInfos_.clear();
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
        
        // レアリティを文字列から判定
        GachaRarity rarity = GachaRarity::R;
        if (entry.rarity == "N") {
            rarity = GachaRarity::N;
        } else if (entry.rarity == "R") {
            rarity = GachaRarity::R;
        } else if (entry.rarity == "SR") {
            rarity = GachaRarity::SR;
        } else if (entry.rarity == "SSR") {
            rarity = GachaRarity::SSR;
        }
        
        std::string label = entry.rarity + " " + entry.equipmentId;
        if (cachedGameplayDataAPI_) {
            if (const auto* eq = cachedGameplayDataAPI_->GetEquipment(entry.equipmentId)) {
                label = entry.rarity + " " + eq->name;
            } else if (const auto* att = cachedGameplayDataAPI_->GetTowerAttachment(entry.equipmentId)) {
                label = entry.rarity + " " + att->name;
            }
        }
        
        HistoryItemInfo info;
        info.itemId = std::to_string(entry.seq);
        info.label = label;
        info.value = "所持: " + std::to_string(entry.countAfter);
        info.rarity = rarity;
        historyItemInfos_.push_back(info);
        
        count++;
    }
}

void GachaOverlay::UpdateTabVisibility() {
    // UIコンポーネント非依存のため、この関数は不要になったが、
    // 他のコードから呼ばれる可能性があるため空実装として残す
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
            result.attachment = entry.attachment;
            result.rarity = entry.rarity;
            return result;
        }
    }
    const auto& last = pool.back();
    result.equipment = last.equipment;
    result.attachment = last.attachment;
    result.rarity = last.rarity;
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
