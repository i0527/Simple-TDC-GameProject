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
        } else {
            if (!poolBuilt_ || pool_.empty()) {
                ShowMessageCard(panelW_, panelH_, "エラー",
                    "ガチャのラインナップが空です。");
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
                    if (!result.equipment && !result.attachment) {
                        continue;
                    }

                    if (result.rarity == GachaRarity::SSR) {
                        pityCounter = 0;
                    } else {
                        pityCounter += 1;
                    }

                    if (result.attachment) {
                        const std::string& id = result.attachment->id;
                        const int countBefore = ctx.gameplayDataAPI->GetOwnedTowerAttachmentCount(id);
                        const int countAfter = countBefore + 1;
                        result.countAfter = countAfter;
                        ctx.gameplayDataAPI->SetOwnedTowerAttachmentCount(id, countAfter);
                        if (countBefore > 0) {
                            ctx.gameplayDataAPI->AddTickets(1);
                        }
                        PlayerDataManager::PlayerSaveData::GachaHistoryEntry entry;
                        entry.seq = ctx.gameplayDataAPI->NextGachaRollSequence();
                        entry.equipmentId = id;
                        entry.rarity = RarityToString(result.rarity);
                        entry.countAfter = countAfter;
                        ctx.gameplayDataAPI->AddGachaHistoryEntry(entry);
                    } else {
                        const int countBefore = ctx.gameplayDataAPI->GetOwnedEquipmentCount(
                            result.equipment->id);
                        const int countAfter = countBefore + 1;
                        result.countAfter = countAfter;
                        ctx.gameplayDataAPI->SetOwnedEquipmentCount(
                            result.equipment->id, countAfter);
                        if (countBefore > 0) {
                            ctx.gameplayDataAPI->AddTickets(1);
                        }
                        PlayerDataManager::PlayerSaveData::GachaHistoryEntry entry;
                        entry.seq = ctx.gameplayDataAPI->NextGachaRollSequence();
                        entry.equipmentId = result.equipment->id;
                        entry.rarity = RarityToString(result.rarity);
                        entry.countAfter = countAfter;
                        ctx.gameplayDataAPI->AddGachaHistoryEntry(entry);
                    }

                    results.push_back(result);
                }

                ctx.gameplayDataAPI->SetGachaPityCounter(pityCounter);
                ctx.gameplayDataAPI->AddTickets(-cost);
                ctx.gameplayDataAPI->Save();

                // 新しいガチャ結果を表示する前に、スクロール位置とフラグをリセット
                scrollYDraw_ = 0.0f;
                hasAutoScrolled_ = false;
                ClearResultCards();
                pendingResults_ = results;
                revealedCount_ = 0;
                revealTimer_ = 0.0f;
                cardAnimationTimer_ = 0.0f;
                skipRevealRequested_ = false;
                showMessageOverlay_ = false;
            }
        }
    }

    const bool isRevealing =
        !pendingResults_.empty() && revealedCount_ < pendingResults_.size();
    
    // カードアニメーションタイマーは常に更新（アニメーション中のカードがある限り）
    if (isRevealing || !resultCardInfos_.empty()) {
        cardAnimationTimer_ += deltaTime;
    }
    
    if (isRevealing) {
        if (skipRevealRequested_) {
            const int total = static_cast<int>(pendingResults_.size());
            for (int i = static_cast<int>(revealedCount_); i < total; ++i) {
                AddEquipmentResultCard(panelW_, panelH_, pendingResults_[i], i, total);
            }
            revealedCount_ = pendingResults_.size();
            // すべてのカードのアニメーションを完了状態に
            for (auto& cardInfo : resultCardInfos_) {
                cardInfo.animationProgress = 1.0f;
            }
        } else {
            revealTimer_ += deltaTime;
            while (revealTimer_ >= revealInterval_ &&
                   revealedCount_ < pendingResults_.size()) {
                const int index = static_cast<int>(revealedCount_);
                const int total = static_cast<int>(pendingResults_.size());
                AddEquipmentResultCard(panelW_, panelH_, pendingResults_[revealedCount_],
                                       index, total);
                // カードの表示時刻を記録
                if (index < static_cast<int>(resultCardInfos_.size())) {
                    resultCardInfos_[index].revealTime = cardAnimationTimer_;
                }
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
    
    // カードアニメーションの更新（常に実行）
    bool allAnimationsComplete = true;
    for (auto& cardInfo : resultCardInfos_) {
        if (cardInfo.revealTime > 0.0f) {
            const float elapsed = cardAnimationTimer_ - cardInfo.revealTime;
            const float animationDuration = 0.4f; // アニメーション時間
            cardInfo.animationProgress = std::min(1.0f, elapsed / animationDuration);
            if (cardInfo.animationProgress < 1.0f) {
                allAnimationsComplete = false;
            }
        } else if (cardInfo.animationProgress < 1.0f) {
            // revealTimeが設定されていない場合でも、アニメーションを進行させる
            cardInfo.animationProgress = 1.0f;
        }
    }
    
    // アニメーション終了時は表示座標を変えない（自動スクロールを無効化）
    // ユーザーが手動でスクロールできるようにする

    // マウス入力処理
    if (ctx.inputAPI) {
        mousePos_ = ctx.inputAPI->GetMousePositionInternal();
        mouseClicked_ = ctx.inputAPI->IsMouseButtonPressed(0); // 左クリック
        
        // タブのホバー判定
        hoveredTabIndex_ = -1;
        const float tabRowY = contentTop_ - TAB_BUTTON_H - GACHA_TAB_ROW_GAP;
        for (int i = 0; i < 4; ++i) {
            const float tabX = panelX_ + contentLeft_ + i * (TAB_BUTTON_W + TAB_BUTTON_SPACING);
            const float tabY = panelY_ + tabRowY;
            if (mousePos_.x >= tabX && mousePos_.x < tabX + TAB_BUTTON_W &&
                mousePos_.y >= tabY && mousePos_.y < tabY + TAB_BUTTON_H) {
                hoveredTabIndex_ = i;
                if (mouseClicked_) {
                    // タブ切り替え時、アニメーション中のカードがあれば即座に完了させる
                    GachaTab previousTab = currentTab_;
                    switch (i) {
                    case 0: currentTab_ = GachaTab::Draw; break;
                    case 1: currentTab_ = GachaTab::Rates; break;
                    case 2: currentTab_ = GachaTab::History; break;
                    case 3: currentTab_ = GachaTab::Exchange; break;
                    }
                    
                    // タブが切り替わった場合、アニメーション中のカードを即座に完了させる
                    if (previousTab != currentTab_) {
                        for (auto& cardInfo : resultCardInfos_) {
                            if (cardInfo.animationProgress < 1.0f) {
                                cardInfo.animationProgress = 1.0f;
                            }
                        }
                    }
                }
                break;
            }
        }

        // ガチャボタンのホバーとクリック判定
        hoveredSingleButton_ = false;
        hoveredTenButton_ = false;
        if (currentTab_ == GachaTab::Draw && !isRevealing) {
            const float singleX = panelX_ + singleButtonX_;
            const float singleY = panelY_ + singleButtonY_;
            if (mousePos_.x >= singleX && mousePos_.x < singleX + buttonW_ &&
                mousePos_.y >= singleY && mousePos_.y < singleY + buttonH_) {
                hoveredSingleButton_ = true;
                if (mouseClicked_ && ctx.gameplayDataAPI && 
                    ctx.gameplayDataAPI->GetTickets() >= COST_SINGLE) {
                    pendingRollCount_ = 1;
                }
            }
            
            const float tenX = panelX_ + tenButtonX_;
            const float tenY = panelY_ + tenButtonY_;
            if (mousePos_.x >= tenX && mousePos_.x < tenX + buttonW_ &&
                mousePos_.y >= tenY && mousePos_.y < tenY + buttonH_) {
                hoveredTenButton_ = true;
                if (mouseClicked_ && ctx.gameplayDataAPI && 
                    ctx.gameplayDataAPI->GetTickets() >= COST_TEN) {
                    pendingRollCount_ = 10;
                }
            }
        }

        // スキップボタンのホバーとクリック判定
        hoveredSkipButton_ = false;
        if (isRevealing) {
            const float skipX = panelW_ - 200.0f;
            const float skipY = panelH_ - 160.0f;
            const float skipW = 160.0f;
            const float skipH = 46.0f;
            if (mousePos_.x >= panelX_ + skipX && mousePos_.x < panelX_ + skipX + skipW &&
                mousePos_.y >= panelY_ + skipY && mousePos_.y < panelY_ + skipY + skipH) {
                hoveredSkipButton_ = true;
                if (mouseClicked_) {
                    skipRevealRequested_ = true;
                }
            }
        }

        // 交換ボタンのホバーとクリック判定
        hoveredExchange1Button_ = false;
        hoveredExchange10Button_ = false;
        if (currentTab_ == GachaTab::Exchange && ctx.gameplayDataAPI) {
            const float exchange1X = panelW_ / 2.0f - 220.0f;
            const float exchange10X = panelW_ / 2.0f + 20.0f;
            const float exchangeY = contentTop_ + 40.0f;
            const float exchangeW = 200.0f;
            const float exchangeH = 56.0f;
            
            if (mousePos_.x >= panelX_ + exchange1X && mousePos_.x < panelX_ + exchange1X + exchangeW &&
                mousePos_.y >= panelY_ + exchangeY && mousePos_.y < panelY_ + exchangeY + exchangeH) {
                hoveredExchange1Button_ = true;
                if (mouseClicked_) {
                    const int dust = ctx.gameplayDataAPI->GetGachaDust();
                    if (dust >= DUST_FOR_TICKET) {
                        ctx.gameplayDataAPI->AddGachaDust(-DUST_FOR_TICKET);
                        ctx.gameplayDataAPI->AddTickets(1);
                        ctx.gameplayDataAPI->Save();
                        ShowMessageCard(panelW_, panelH_, "交換完了", "チケット x1 を交換しました。");
                    } else {
                        ShowMessageCard(panelW_, panelH_, "交換不可", "ダストが不足しています。");
                    }
                }
            }
            
            if (mousePos_.x >= panelX_ + exchange10X && mousePos_.x < panelX_ + exchange10X + exchangeW &&
                mousePos_.y >= panelY_ + exchangeY && mousePos_.y < panelY_ + exchangeY + exchangeH) {
                hoveredExchange10Button_ = true;
                if (mouseClicked_) {
                    const int dust = ctx.gameplayDataAPI->GetGachaDust();
                    if (dust >= DUST_FOR_TEN_TICKETS) {
                        ctx.gameplayDataAPI->AddGachaDust(-DUST_FOR_TEN_TICKETS);
                        ctx.gameplayDataAPI->AddTickets(10);
                        ctx.gameplayDataAPI->Save();
                        ShowMessageCard(panelW_, panelH_, "交換完了", "チケット x10 を交換しました。");
                    } else {
                        ShowMessageCard(panelW_, panelH_, "交換不可", "ダストが不足しています。");
                    }
                }
            }
        }
        
        // スクロール処理
        float wheelMove = ctx.inputAPI->GetMouseWheelMove();
        if (wheelMove != 0.0f) {
            // マウス位置がコンテンツエリア内かチェック
            bool isInScrollArea = false;
            if (currentTab_ == GachaTab::Rates) {
                // 提供割合タブの場合、バーグラフの下のスクロール可能領域のみ
                const float barHeight = 32.0f;
                const float barSpacing = 12.0f;
                const float barGraphHeight = 4.0f * barHeight + 3.0f * barSpacing;
                const float barGraphBottom = panelY_ + contentTop_ + 60.0f + barGraphHeight; // 40.0f → 60.0fに変更
                const float scrollAreaTop = barGraphBottom;
                isInScrollArea = (mousePos_.x >= panelX_ + contentLeft_ && mousePos_.x <= panelX_ + contentRight_ &&
                                 mousePos_.y >= scrollAreaTop && mousePos_.y <= panelY_ + contentBottom_);
            } else {
                // その他のタブは通常のコンテンツエリア全体
                isInScrollArea = (mousePos_.x >= panelX_ + contentLeft_ && mousePos_.x <= panelX_ + contentRight_ &&
                                 mousePos_.y >= panelY_ + contentTop_ && mousePos_.y <= panelY_ + contentBottom_);
            }
            
            if (isInScrollArea) {
                
                float& scrollY = scrollYDraw_;
                if (currentTab_ == GachaTab::Rates) {
                    scrollY = scrollYRates_;
                } else if (currentTab_ == GachaTab::History) {
                    scrollY = scrollYHistory_;
                }
                
                scrollY -= wheelMove * 30.0f;
                scrollY = std::max(0.0f, scrollY);
                
                // 最大スクロール位置を計算して制限
                float maxScroll = 0.0f;
                if (currentTab_ == GachaTab::Draw) {
                    // カードの最大高さを計算
                    if (!resultCardInfos_.empty()) {
                        float maxCardBottom = 0.0f;
                        for (const auto& card : resultCardInfos_) {
                            // card.yはcontentTop_からの相対位置（startY = contentTop_ + 16.0fから始まる）
                            // card.y + card.heightが最大のカードの下端位置
                            maxCardBottom = std::max(maxCardBottom, card.y + card.height);
                        }
                        const float contentHeight = contentBottom_ - contentTop_;
                        // コンテンツエリアの高さを超える分だけスクロール可能
                        // maxCardBottomはcontentTop_からの相対位置なので、contentHeightと直接比較可能
                        // 最後のカードが完全に見えるように、少し余裕を持たせる
                        maxScroll = std::max(0.0f, maxCardBottom - contentHeight + 20.0f);
                    }
                } else if (currentTab_ == GachaTab::Rates) {
                    // 提供割合リストの最大高さを計算
                    // バーグラフの高さを計算（N, R, SR, SSRの4つ + 間隔）
                    const float barHeight = 32.0f;
                    const float barSpacing = 12.0f;
                    const float barGraphHeight = 4.0f * barHeight + 3.0f * barSpacing; // 4つのバー + 3つの間隔
                    const float barGraphBottom = contentTop_ + 60.0f + barGraphHeight; // barStartY + barGraphHeight（40.0f → 60.0fに変更）
                    const float listStartY = barGraphBottom + 20.0f; // バーグラフとの間隔
                    
                    const float listItemHeight = 28.0f;
                    const float totalHeight = static_cast<float>(poolItemInfos_.size()) * listItemHeight;
                    // スクロール可能な領域の高さ = コンテンツエリアの下端 - バーグラフの下端
                    const float scrollableHeight = contentBottom_ - barGraphBottom;
                    maxScroll = std::max(0.0f, totalHeight - scrollableHeight + 20.0f); // 少し余裕を持たせる
                } else if (currentTab_ == GachaTab::History) {
                    // 履歴リストの最大高さを計算
                    const float itemHeight = 34.0f;
                    const float totalHeight = static_cast<float>(std::min(historyItemInfos_.size(), static_cast<size_t>(HISTORY_DISPLAY_LIMIT))) * itemHeight;
                    const float contentHeight = contentBottom_ - contentTop_;
                    maxScroll = std::max(0.0f, totalHeight - contentHeight);
                }
                
                scrollY = std::min(scrollY, maxScroll);
                
                // タブ別のスクロール位置を更新
                if (currentTab_ == GachaTab::Rates) {
                    scrollYRates_ = scrollY;
                } else if (currentTab_ == GachaTab::History) {
                    scrollYHistory_ = scrollY;
                } else {
                    scrollYDraw_ = scrollY;
                }
            }
        }
    }

    if (currentTab_ == GachaTab::Rates && poolItemInfos_.empty()) {
        RefreshPoolList();
    }

    if (currentTab_ == GachaTab::History && historyItemInfos_.empty() && ctx.gameplayDataAPI) {
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
