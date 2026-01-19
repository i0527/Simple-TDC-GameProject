#include "GachaOverlay.hpp"
#include "GachaOverlayInternal.hpp"

// 外部ライブラリ
#include <imgui.h>

// プロジェクト内
#include "../../api/GameplayDataAPI.hpp"

namespace game {
namespace core {

using namespace gacha_internal;

void GachaOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) {
        return;
    }

    const float introEase = EaseOutCubic(introProgress_);
    const float alpha = introEase;
    const float pulse = 0.5f + 0.5f * std::sin(pulseTime_ * kPi * 2.0f * 0.6f);

    if (const ImGuiViewport* viewport = ImGui::GetMainViewport()) {
        ImDrawList* bg = ImGui::GetBackgroundDrawList();
        const ImVec2 vpPos = viewport->Pos;
        const ImVec2 vpSize = viewport->Size;
        const ImU32 dimColor =
            ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.35f * alpha));
        bg->AddRectFilled(vpPos, ImVec2(vpPos.x + vpSize.x, vpPos.y + vpSize.y),
                          dimColor);
    }

    ImDrawList* fg = ImGui::GetForegroundDrawList();
    const ImVec2 panelMin(panelX_, panelY_);
    const ImVec2 panelMax(panelX_ + panelW_, panelY_ + panelH_);
    const ImU32 borderColor = ImGui::GetColorU32(
        ImVec4(GACHA_PANEL_BORDER_R, GACHA_PANEL_BORDER_G, GACHA_PANEL_BORDER_B,
               GACHA_PANEL_BORDER_A * alpha));
    fg->AddRect(panelMin, panelMax, borderColor, 6.0f, 0, 2.0f);

    const ImVec2 headerMax(panelX_ + panelW_, panelY_ + GACHA_HEADER_H);

    ImFont* font = ImGui::GetFont();
    const float fontSize = ImGui::GetFontSize();
    const ImU32 titleColor =
        ImGui::GetColorU32(ImVec4(0.95f, 0.95f, 0.95f, alpha));
    const ImU32 subColor =
        ImGui::GetColorU32(ImVec4(0.7f, 0.7f, 0.7f, alpha));
    const ImVec2 titlePos(panelX_ + GACHA_HEADER_PADDING_X,
                           panelY_ + GACHA_HEADER_PADDING_Y);
    fg->AddText(font, fontSize * 1.6f, titlePos, titleColor, "ガチャ");
    fg->AddText(font, fontSize,
                ImVec2(titlePos.x, titlePos.y + fontSize * 1.6f + 6.0f),
                subColor, "チケットで装備を獲得します");
    fg->AddLine(ImVec2(panelX_ + GACHA_HEADER_PADDING_X,
                       panelY_ + GACHA_HEADER_DIVIDER_Y),
                ImVec2(panelX_ + panelW_ - GACHA_HEADER_PADDING_X,
                       panelY_ + GACHA_HEADER_DIVIDER_Y),
                borderColor, 1.0f);

    const float tabRowY = contentTop_ - TAB_BUTTON_H - GACHA_TAB_ROW_GAP;
    const float tabBgTop =
        panelY_ + tabRowY - (GACHA_TAB_BG_H - TAB_BUTTON_H) * 0.5f;
    const float tabBgBottom = tabBgTop + GACHA_TAB_BG_H;
    (void)tabBgBottom;

    int activeTabIndex = 0;
    switch (currentTab_) {
    case GachaTab::Draw:
        activeTabIndex = 0;
        break;
    case GachaTab::Rates:
        activeTabIndex = 1;
        break;
    case GachaTab::History:
        activeTabIndex = 2;
        break;
    case GachaTab::Exchange:
        activeTabIndex = 3;
        break;
    }

    const float activeX = panelX_ + contentLeft_ +
                          activeTabIndex * (TAB_BUTTON_W + TAB_BUTTON_SPACING);
    const ImU32 activeLine = ImGui::GetColorU32(
        ImVec4(GACHA_ACCENT_R, GACHA_ACCENT_G, GACHA_ACCENT_B,
               GACHA_ACCENT_A * alpha));
    fg->AddRectFilled(
        ImVec2(activeX, tabBgBottom - GACHA_TAB_ACTIVE_UNDERLINE_H),
        ImVec2(activeX + TAB_BUTTON_W, tabBgBottom), activeLine, 3.0f);

    const ImVec2 contentBgMin(panelX_ + contentLeft_ - GACHA_CONTENT_PADDING,
                              panelY_ + contentTop_ - GACHA_CONTENT_PADDING);
    const ImVec2 contentBgMax(panelX_ + contentRight_ + GACHA_CONTENT_PADDING,
                              panelY_ + contentBottom_ + GACHA_CONTENT_PADDING);
    fg->AddRect(contentBgMin, contentBgMax, borderColor, GACHA_CONTENT_RADIUS,
                0, 1.0f);

    if (ctx.gameplayDataAPI) {
        const ImU32 badgeBg = ImGui::GetColorU32(
            ImVec4(GACHA_BADGE_BG_R, GACHA_BADGE_BG_G, GACHA_BADGE_BG_B,
                   GACHA_BADGE_BG_A * alpha));
        const float rightX = panelX_ + panelW_ - GACHA_HEADER_PADDING_X;
        float badgeY = panelY_ + GACHA_HEADER_PADDING_Y + 2.0f;
        auto drawBadge = [&](const std::string& text) {
            const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
            const float badgeW = textSize.x + GACHA_STATUS_BADGE_PAD_X * 2.0f;
            const float badgeH = textSize.y + GACHA_STATUS_BADGE_PAD_Y * 2.0f;
            const ImVec2 badgeMin(rightX - badgeW, badgeY);
            const ImVec2 badgeMax(rightX, badgeY + badgeH);
            fg->AddRectFilled(badgeMin, badgeMax, badgeBg, GACHA_STATUS_BADGE_RADIUS);
            fg->AddText(font, fontSize,
                        ImVec2(badgeMin.x + GACHA_STATUS_BADGE_PAD_X,
                               badgeMin.y + GACHA_STATUS_BADGE_PAD_Y),
                        subColor, text.c_str());
            badgeY += badgeH + GACHA_STATUS_BADGE_SPACING;
        };

        drawBadge("所持チケット: " +
                  std::to_string(ctx.gameplayDataAPI->GetTickets()));
        drawBadge("天井: " +
                  std::to_string(ctx.gameplayDataAPI->GetGachaPityCounter()) +
                  " / " + std::to_string(PITY_HARD));
        drawBadge("ダスト: " +
                  std::to_string(ctx.gameplayDataAPI->GetGachaDust()));
    }

    if (currentTab_ == GachaTab::Draw) {
        const ImVec2 noticePos(panelX_ + panelW_ - 360.0f,
                               panelY_ + panelH_ - GACHA_NOTICE_Y_OFFSET);
        const ImU32 noticeColor =
            ImGui::GetColorU32(ImVec4(0.85f, 0.85f, 0.85f, alpha));
        fg->AddText(font, fontSize * 0.95f, noticePos, noticeColor,
                    "※ 10連はSR以上1枚保証");
    } else if (currentTab_ == GachaTab::Rates) {
        const std::string rates =
            "提供割合  N " + FormatPercent(rateN_) + "% / R " +
            FormatPercent(rateR_) + "% / SR " + FormatPercent(rateSR_) +
            "% / SSR " + FormatPercent(rateSSR_) + "%";
        fg->AddText(font, fontSize * 0.95f,
                    ImVec2(panelX_ + 40.0f,
                           panelY_ + panelH_ - GACHA_FOOTNOTE_Y_OFFSET),
                    subColor, rates.c_str());
    } else if (currentTab_ == GachaTab::Exchange) {
        const std::string exchange =
            "ダスト交換  x1(" + std::to_string(DUST_FOR_TICKET) +
            ")  x10(" + std::to_string(DUST_FOR_TEN_TICKETS) + ")";
        fg->AddText(font, fontSize * 0.95f,
                    ImVec2(panelX_ + 40.0f,
                           panelY_ + panelH_ - GACHA_FOOTNOTE_Y_OFFSET),
                    subColor, exchange.c_str());
    }

    const bool isRevealing =
        !pendingResults_.empty() && revealedCount_ < pendingResults_.size();
    if (isRevealing && skipRevealButton_) {
        const float bx = panelW_ - 200.0f;
        const float by = panelH_ - 160.0f;
        skipRevealButton_->SetPosition(bx, by);
        skipRevealButton_->SetSize(160.0f, 46.0f);
        skipRevealButton_->SetLabel("SKIP");
        skipRevealButton_->SetVisible(true);
        skipRevealButton_->SetOnClickCallback([this]() { skipRevealRequested_ = true; });
    }

    if (panel_) {
        panel_->Render();
    }

    if (showMessageOverlay_) {
        const ImVec2 center(panelX_ + panelW_ * 0.5f, panelY_ + panelH_ * 0.5f);
        const float radius = 120.0f + 14.0f * pulse;
        const ImU32 ringColor =
            ImGui::GetColorU32(ImVec4(1.0f, 0.9f, 0.2f, 0.3f * alpha));
        fg->AddCircle(center, radius, ringColor, 48, 3.0f);
    }
}

void GachaOverlay::ShowMessageCard(float contentWidth, float contentHeight,
                                   const std::string& title,
                                   const std::string& message) {
    ClearResultCards();
    showMessageOverlay_ = true;
    pendingResults_.clear();
    revealedCount_ = 0;
    revealTimer_ = 0.0f;
    skipRevealRequested_ = false;

    const float cardW = 520.0f;
    const float cardH = 220.0f;
    const float x = (contentWidth - cardW) / 2.0f;
    const float availableH = contentBottom_ - contentTop_;
    const float y = contentTop_ + (availableH - cardH) / 2.0f;

    auto card = std::make_shared<ui::Card>();
    card->SetId("gacha_message");
    card->SetPosition(x, y);
    card->SetSize(cardW, cardH);

    ui::CardContent content;
    content.title = title;
    content.description = message;
    content.imageId = "";
    card->SetContent(content);
    card->Initialize();

    resultCards_.push_back(card);
    if (panel_) {
        panel_->AddChild(card);
    }
    UpdateTabVisibility();
}

void GachaOverlay::ShowEquipmentResults(
    float contentWidth, float contentHeight,
    const std::vector<const entities::Equipment*>& results) {
    ClearResultCards();

    if (results.empty()) {
        ShowMessageCard(contentWidth, contentHeight, "結果", "排出結果がありません。");
        return;
    }

    const int cols =
        results.size() >= 10 ? 5 : static_cast<int>(results.size());
    const int rows = (static_cast<int>(results.size()) + cols - 1) / cols;

    const float cardW = 240.0f;
    const float cardH = 260.0f;
    const float spacingX = 18.0f;
    const float spacingY = 18.0f;

    const float totalW = cols * cardW + (cols - 1) * spacingX;
    const float totalH = rows * cardH + (rows - 1) * spacingY;
    const float startX = (contentWidth - totalW) / 2.0f;
    const float startY = contentTop_ + 16.0f;

    for (size_t i = 0; i < results.size(); ++i) {
        const auto* eq = results[i];
        if (!eq) {
            continue;
        }

        GachaResult result;
        result.equipment = eq;
        result.rarity = GachaRarity::R;
        result.countAfter = 0;
        AddEquipmentResultCard(contentWidth, contentHeight, result,
                               static_cast<int>(i),
                               static_cast<int>(results.size()));
    }
}

void GachaOverlay::AddEquipmentResultCard(float contentWidth, float contentHeight,
                                          const GachaResult& result, int index,
                                          int total) {
    if (!result.equipment || total <= 0) {
        return;
    }

    const int cols = total >= 10 ? 5 : total;
    const int rows = (total + cols - 1) / cols;

    const float cardW = 240.0f;
    const float cardH = 260.0f;
    const float spacingX = 18.0f;
    const float spacingY = 18.0f;

    const float totalW = cols * cardW + (cols - 1) * spacingX;
    const float totalH = rows * cardH + (rows - 1) * spacingY;
    const float startX = (contentWidth - totalW) / 2.0f;
    const float startY = contentTop_ + 16.0f;

    const int c = index % cols;
    const int r = index / cols;
    const float x = startX + c * (cardW + spacingX);
    const float y = startY + r * (cardH + spacingY);

    auto card = std::make_shared<ui::Card>();
    card->SetId("gacha_result_" + std::to_string(index));
    card->SetPosition(x, y);
    card->SetSize(cardW, cardH);

    ui::CardContent content;
    content.title = result.equipment->name;
    content.description = result.equipment->description;
    content.metadata["レア"] = RarityToString(result.rarity);
    content.metadata["ATK"] =
        std::to_string(static_cast<int>(result.equipment->attack_bonus));
    content.metadata["DEF"] =
        std::to_string(static_cast<int>(result.equipment->defense_bonus));
    content.metadata["HP"] =
        std::to_string(static_cast<int>(result.equipment->hp_bonus));
    if (result.countAfter > 0) {
        content.metadata["所持数"] = std::to_string(result.countAfter);
    }
    content.imageId = "";
    card->SetContent(content);
    card->Initialize();

    resultCards_.push_back(card);
    if (panel_) {
        panel_->AddChild(card);
    }
}

} // namespace core
} // namespace game
