#include "GachaOverlay.hpp"
#include "GachaOverlayInternal.hpp"

// 標準ライブラリ
#include <array>
#include <algorithm>
#include <cmath>

// 外部ライブラリ
#include <raylib.h>

// プロジェクト内
#include "../../api/GameplayDataAPI.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../config/RenderTypes.hpp"

namespace game {
namespace core {

using namespace gacha_internal;
using namespace ui;

// レアリティカラーをColorに変換
inline Color GetRarityBgColor(GachaRarity rarity, float alpha) {
    switch (rarity) {
    case GachaRarity::N:
        return Color{
            static_cast<unsigned char>(GACHA_RARITY_N_BG_R * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_N_BG_G * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_N_BG_B * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_N_BG_A * 255.0f * alpha)
        };
    case GachaRarity::R:
        return Color{
            static_cast<unsigned char>(GACHA_RARITY_R_BG_R * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_R_BG_G * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_R_BG_B * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_R_BG_A * 255.0f * alpha)
        };
    case GachaRarity::SR:
        return Color{
            static_cast<unsigned char>(GACHA_RARITY_SR_BG_R * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_SR_BG_G * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_SR_BG_B * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_SR_BG_A * 255.0f * alpha)
        };
    case GachaRarity::SSR:
        return Color{
            static_cast<unsigned char>(GACHA_RARITY_SSR_BG_R * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_SSR_BG_G * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_SSR_BG_B * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_SSR_BG_A * 255.0f * alpha)
        };
    }
    return OverlayColors::CARD_BG_NORMAL;
}

inline Color GetRarityBorderColor(GachaRarity rarity, float alpha) {
    switch (rarity) {
    case GachaRarity::N:
        return Color{
            static_cast<unsigned char>(GACHA_RARITY_N_BORDER_R * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_N_BORDER_G * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_N_BORDER_B * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_N_BORDER_A * 255.0f * alpha)
        };
    case GachaRarity::R:
        return Color{
            static_cast<unsigned char>(GACHA_RARITY_R_BORDER_R * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_R_BORDER_G * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_R_BORDER_B * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_R_BORDER_A * 255.0f * alpha)
        };
    case GachaRarity::SR:
        return Color{
            static_cast<unsigned char>(GACHA_RARITY_SR_BORDER_R * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_SR_BORDER_G * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_SR_BORDER_B * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_SR_BORDER_A * 255.0f * alpha)
        };
    case GachaRarity::SSR:
        return Color{
            static_cast<unsigned char>(GACHA_RARITY_SSR_BORDER_R * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_SSR_BORDER_G * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_SSR_BORDER_B * 255.0f),
            static_cast<unsigned char>(GACHA_RARITY_SSR_BORDER_A * 255.0f * alpha)
        };
    }
    return OverlayColors::BORDER_DEFAULT;
}

void GachaOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_ || !systemAPI_) {
        return;
    }

    auto& render = systemAPI_->Render();
    const float introEase = EaseOutCubic(introProgress_);
    const float alpha = introEase;
    const float pulse = 0.5f + 0.5f * std::sin(pulseTime_ * kPi * 2.0f * 0.6f);

    // パネルの背景
    const Color panelBg = OverlayColors::PANEL_BG;
    render.DrawRectangle(panelX_, panelY_, panelW_, panelH_, panelBg);
    
    // パネルのボーダー
    const Color borderColor = Color{
        static_cast<unsigned char>(GACHA_PANEL_BORDER_R * 255.0f),
        static_cast<unsigned char>(GACHA_PANEL_BORDER_G * 255.0f),
        static_cast<unsigned char>(GACHA_PANEL_BORDER_B * 255.0f),
        static_cast<unsigned char>(GACHA_PANEL_BORDER_A * 255.0f * alpha)
    };
    render.DrawRectangleLines(panelX_, panelY_, panelW_, panelH_, 2.0f, borderColor);

    // ヘッダー
    const float headerY = panelY_ + GACHA_HEADER_PADDING_Y;
    const float fontSize = 28.0f;
    const Color subColor = OverlayColors::TEXT_SECONDARY;
    render.DrawTextDefault("チケットで装備・アタッチメントを獲得します",
                          panelX_ + GACHA_HEADER_PADDING_X, headerY,
                          fontSize, subColor);
    
    // ヘッダーの区切り線
    const float dividerY = headerY + fontSize + 10.0f;
    render.DrawRectangle(panelX_ + GACHA_HEADER_PADDING_X, dividerY,
                        panelW_ - GACHA_HEADER_PADDING_X * 2.0f, 1.0f, borderColor);

    // タブ
    const float tabRowY = contentTop_ - TAB_BUTTON_H - GACHA_TAB_ROW_GAP;
    const float tabBgTop = panelY_ + tabRowY - (GACHA_TAB_BG_H - TAB_BUTTON_H) * 0.5f;
    const float tabBgBottom = tabBgTop + GACHA_TAB_BG_H;

    int activeTabIndex = 0;
    switch (currentTab_) {
    case GachaTab::Draw: activeTabIndex = 0; break;
    case GachaTab::Rates: activeTabIndex = 1; break;
    case GachaTab::History: activeTabIndex = 2; break;
    case GachaTab::Exchange: activeTabIndex = 3; break;
    }

    const std::array<std::string, 4> tabLabels = {"引く", "提供割合", "履歴", "交換"};
    
    for (int i = 0; i < 4; ++i) {
        const float tabX = panelX_ + contentLeft_ + i * (TAB_BUTTON_W + TAB_BUTTON_SPACING);
        const float tabY = panelY_ + tabRowY;
        
        const bool isActive = (i == activeTabIndex);
        const bool isHovered = (hoveredTabIndex_ == i);
        
        Color tabBg;
        if (isActive) {
            tabBg = Color{
                static_cast<unsigned char>(GACHA_TAB_ACTIVE_BG_R * 255.0f),
                static_cast<unsigned char>(GACHA_TAB_ACTIVE_BG_G * 255.0f),
                static_cast<unsigned char>(GACHA_TAB_ACTIVE_BG_B * 255.0f),
                static_cast<unsigned char>(GACHA_TAB_ACTIVE_BG_A * 255.0f * alpha)
            };
        } else if (isHovered) {
            tabBg = Color{
                static_cast<unsigned char>(GACHA_TAB_HOVER_BG_R * 255.0f),
                static_cast<unsigned char>(GACHA_TAB_HOVER_BG_G * 255.0f),
                static_cast<unsigned char>(GACHA_TAB_HOVER_BG_B * 255.0f),
                static_cast<unsigned char>(GACHA_TAB_HOVER_BG_A * 255.0f * alpha)
            };
        } else {
            tabBg = Color{
                static_cast<unsigned char>(GACHA_TAB_BG_R * 255.0f),
                static_cast<unsigned char>(GACHA_TAB_BG_G * 255.0f),
                static_cast<unsigned char>(GACHA_TAB_BG_B * 255.0f),
                static_cast<unsigned char>(GACHA_TAB_BG_A * 255.0f * alpha)
            };
        }
        
        render.DrawRectangle(tabX, tabY, TAB_BUTTON_W, TAB_BUTTON_H, tabBg);
        
        // タブのテキスト
        Vector2 textSize = render.MeasureTextDefault(tabLabels[i], fontSize * 0.85f);
        render.DrawTextDefault(tabLabels[i],
                              tabX + (TAB_BUTTON_W - textSize.x) * 0.5f,
                              tabY + (TAB_BUTTON_H - textSize.y) * 0.5f,
                              fontSize * 0.85f, subColor);
    }

    // アクティブタブの下線
    const float activeX = panelX_ + contentLeft_ +
                          activeTabIndex * (TAB_BUTTON_W + TAB_BUTTON_SPACING);
    const Color activeLine = Color{
        static_cast<unsigned char>(GACHA_ACCENT_R * 255.0f),
        static_cast<unsigned char>(GACHA_ACCENT_G * 255.0f),
        static_cast<unsigned char>(GACHA_ACCENT_B * 255.0f),
        static_cast<unsigned char>(GACHA_ACCENT_A * 255.0f * alpha)
    };
    render.DrawRectangle(activeX, tabBgBottom - GACHA_TAB_ACTIVE_UNDERLINE_H,
                        TAB_BUTTON_W, GACHA_TAB_ACTIVE_UNDERLINE_H, activeLine);

    // コンテンツエリアの背景
    const float contentBgX = panelX_ + contentLeft_ - GACHA_CONTENT_PADDING;
    const float contentBgY = panelY_ + contentTop_ - GACHA_CONTENT_PADDING;
    const float contentBgW = contentRight_ - contentLeft_ + GACHA_CONTENT_PADDING * 2.0f;
    const float contentBgH = contentBottom_ - contentTop_ + GACHA_CONTENT_PADDING * 2.0f;
    render.DrawRectangleLines(contentBgX, contentBgY, contentBgW, contentBgH,
                            1.0f, borderColor);

    // ステータスバッジ
    if (ctx.gameplayDataAPI) {
        const float badgeFontSize = fontSize * 0.85f;
        const float badgePadX = GACHA_STATUS_BADGE_PAD_X;
        const float badgePadY = GACHA_STATUS_BADGE_PAD_Y;
        const float badgeSpacing = 8.0f;
        const float headerCenterY = panelY_ + GACHA_HEADER_PADDING_Y + fontSize * 0.5f;

        const std::array<std::string, 3> badges = {
            "所持チケット: " + std::to_string(ctx.gameplayDataAPI->GetTickets()),
            "天井: " + std::to_string(ctx.gameplayDataAPI->GetGachaPityCounter()) +
                " / " + std::to_string(PITY_HARD),
            "ダスト: " + std::to_string(ctx.gameplayDataAPI->GetGachaDust())
        };

        const Color badgeBg = Color{
            static_cast<unsigned char>(GACHA_BADGE_BG_R * 255.0f),
            static_cast<unsigned char>(GACHA_BADGE_BG_G * 255.0f),
            static_cast<unsigned char>(GACHA_BADGE_BG_B * 255.0f),
            static_cast<unsigned char>(GACHA_BADGE_BG_A * 255.0f * alpha)
        };

        float badgeX = panelX_ + panelW_ - GACHA_HEADER_PADDING_X;
        for (auto it = badges.rbegin(); it != badges.rend(); ++it) {
            Vector2 textSize = render.MeasureTextDefault(*it, badgeFontSize);
            const float badgeW = textSize.x + badgePadX * 2.0f;
            const float badgeH = textSize.y + badgePadY * 2.0f;
            badgeX -= badgeW;
            render.DrawRectangle(badgeX, headerCenterY - badgeH * 0.5f,
                               badgeW, badgeH, badgeBg);
            render.DrawTextDefault(*it,
                                  badgeX + badgePadX,
                                  headerCenterY - textSize.y * 0.5f,
                                  badgeFontSize, subColor);
            badgeX -= badgeSpacing;
        }
    }

    // タブ別のコンテンツ描画
    if (currentTab_ == GachaTab::Draw) {
        // ガチャボタン
        const bool isRevealing = !pendingResults_.empty() && revealedCount_ < pendingResults_.size();
        const bool singleEnabled = !isRevealing && ctx.gameplayDataAPI && 
                                   ctx.gameplayDataAPI->GetTickets() >= COST_SINGLE;
        const bool tenEnabled = !isRevealing && ctx.gameplayDataAPI && 
                               ctx.gameplayDataAPI->GetTickets() >= COST_TEN;

        Color singleBg = hoveredSingleButton_ && singleEnabled ? 
                            OverlayColors::BUTTON_PRIMARY_HOVER : OverlayColors::BUTTON_PRIMARY;
        if (!singleEnabled) singleBg = OverlayColors::BUTTON_DISABLED;
        
        Color tenBg = hoveredTenButton_ && tenEnabled ? 
                         OverlayColors::BUTTON_PRIMARY_HOVER : OverlayColors::BUTTON_PRIMARY;
        if (!tenEnabled) tenBg = OverlayColors::BUTTON_DISABLED;

        render.DrawRectangle(panelX_ + singleButtonX_, panelY_ + singleButtonY_,
                           buttonW_, buttonH_, singleBg);
        render.DrawRectangle(panelX_ + tenButtonX_, panelY_ + tenButtonY_,
                           buttonW_, buttonH_, tenBg);

        Vector2 singleTextSize = render.MeasureTextDefault("単発", fontSize);
        render.DrawTextDefault("単発",
                              panelX_ + singleButtonX_ + (buttonW_ - singleTextSize.x) * 0.5f,
                              panelY_ + singleButtonY_ + (buttonH_ - singleTextSize.y) * 0.5f,
                              fontSize, OverlayColors::TEXT_DARK);

        Vector2 tenTextSize = render.MeasureTextDefault("10連", fontSize);
        render.DrawTextDefault("10連",
                              panelX_ + tenButtonX_ + (buttonW_ - tenTextSize.x) * 0.5f,
                              panelY_ + tenButtonY_ + (buttonH_ - tenTextSize.y) * 0.5f,
                              fontSize, OverlayColors::TEXT_DARK);

        // チケット数表示
        if (ctx.gameplayDataAPI) {
            const float buttonLabelY = singleButtonY_ + buttonH_ + 8.0f;
            const float singleCostX = singleButtonX_ + buttonW_ * 0.5f;
            const float tenCostX = tenButtonX_ + buttonW_ * 0.5f;
            
            const std::string singleCost = "(1チケット)";
            Vector2 singleCostSize = render.MeasureTextDefault(singleCost, fontSize * 0.8f);
            render.DrawTextDefault(singleCost,
                                  panelX_ + singleCostX - singleCostSize.x * 0.5f,
                                  panelY_ + buttonLabelY,
                                  fontSize * 0.8f, OverlayColors::TEXT_ACCENT);
            
            const std::string tenCost = "(9チケット)";
            Vector2 tenCostSize = render.MeasureTextDefault(tenCost, fontSize * 0.8f);
            render.DrawTextDefault(tenCost,
                                  panelX_ + tenCostX - tenCostSize.x * 0.5f,
                                  panelY_ + buttonLabelY,
                                  fontSize * 0.8f, OverlayColors::TEXT_ACCENT);
        }

        // スキップボタン
        if (isRevealing) {
            const float skipX = panelW_ - 200.0f;
            const float skipY = panelH_ - 160.0f;
            const float skipW = 160.0f;
            const float skipH = 46.0f;
            
            Color skipBg = hoveredSkipButton_ ? 
                              OverlayColors::BUTTON_SECONDARY_HOVER : OverlayColors::BUTTON_SECONDARY;
            render.DrawRectangle(panelX_ + skipX, panelY_ + skipY, skipW, skipH, skipBg);
            
            Vector2 skipTextSize = render.MeasureTextDefault("SKIP", fontSize * 0.9f);
            render.DrawTextDefault("SKIP",
                                  panelX_ + skipX + (skipW - skipTextSize.x) * 0.5f,
                                  panelY_ + skipY + (skipH - skipTextSize.y) * 0.5f,
                                  fontSize * 0.9f, OverlayColors::TEXT_DARK);
        }

        // 注意書き
        render.DrawTextDefault("※ 10連はSR以上1枚保証",
                              panelX_ + panelW_ - 360.0f,
                              panelY_ + panelH_ - GACHA_NOTICE_Y_OFFSET,
                              fontSize * 0.95f, OverlayColors::TEXT_SECONDARY);

        // カードの描画
        const float scrollOffset = scrollYDraw_;
        for (size_t i = 0; i < resultCardInfos_.size(); ++i) {
            const auto& cardInfo = resultCardInfos_[i];
            
            const float animProgress = cardInfo.animationProgress;
            const float scale = 0.7f + 0.3f * EaseOutCubic(animProgress);
            const float offsetY = (1.0f - animProgress) * 30.0f;
            
            const float cardX = panelX_ + cardInfo.x + (cardInfo.width * (1.0f - scale)) * 0.5f;
            const float cardW = cardInfo.width * scale;
            const float cardH = cardInfo.height * scale;
            const float cardY = panelY_ + cardInfo.y - offsetY - scrollOffset;
            
            // ウィンドウ外のカードは描画しない（クリッピング）
            // カードの下端がコンテンツエリアの上端より上、または上端がコンテンツエリアの下端より下の場合はスキップ
            if (cardY + cardH < panelY_ + contentTop_ || cardY > panelY_ + contentBottom_) {
                continue;
            }

            const Color bgColor = GetRarityBgColor(cardInfo.rarity, alpha);
            const Color borderColor = GetRarityBorderColor(cardInfo.rarity, alpha);

            render.DrawRectangle(cardX, cardY, cardW, cardH, bgColor);
            render.DrawRectangleLines(cardX, cardY, cardW, cardH, 2.5f, borderColor);

            // レアリティバッジ（メッセージカード以外）
            if (!cardInfo.isMessageCard) {
                const float badgeSize = 32.0f;
                const float badgeX = cardX + 8.0f;
                const float badgeY = cardY + 8.0f;
                render.DrawRectangle(badgeX, badgeY, badgeSize, badgeSize * 0.5f, borderColor);
                
                const std::string rarityText = RarityToString(cardInfo.rarity);
                Vector2 rarityTextSize = render.MeasureTextDefault(rarityText, fontSize * 0.7f);
                render.DrawTextDefault(rarityText,
                                      badgeX + (badgeSize - rarityTextSize.x) * 0.5f,
                                      badgeY + (badgeSize * 0.5f - rarityTextSize.y) * 0.5f,
                                      fontSize * 0.7f, OverlayColors::TEXT_PRIMARY);
            }

            // カードの内容
            const float padding = 12.0f;
            float textY = cardY + padding;
            if (!cardInfo.isMessageCard) {
                textY += 32.0f * 0.5f + padding; // バッジの高さ分
            }
            
            if (cardInfo.isMessageCard) {
                // メッセージカード
                if (!cardInfo.title.empty()) {
                    Vector2 titleSize = render.MeasureTextDefault(cardInfo.title, fontSize);
                    render.DrawTextDefault(cardInfo.title,
                                          cardX + (cardW - titleSize.x) * 0.5f, textY,
                                          fontSize, OverlayColors::TEXT_PRIMARY);
                    textY += fontSize * 1.5f;
                }
                if (!cardInfo.message.empty()) {
                    // メッセージを複数行に分割（簡易実装）
                    render.DrawTextDefault(cardInfo.message,
                                          cardX + padding, textY,
                                          fontSize * 0.85f, OverlayColors::TEXT_SECONDARY);
                }
            } else if (cardInfo.attachment) {
                // アタッチメントカード（タワー強化）
                render.DrawTextDefault(cardInfo.attachment->name,
                                      cardX + padding, textY,
                                      fontSize * 0.9f, OverlayColors::TEXT_PRIMARY);
                textY += fontSize * 1.2f;
                if (!cardInfo.attachment->description.empty()) {
                    render.DrawTextDefault(cardInfo.attachment->description,
                                          cardX + padding, textY,
                                          fontSize * 0.75f, OverlayColors::TEXT_SECONDARY);
                    textY += fontSize * 1.0f;
                }
                if (cardInfo.countAfter > 0) {
                    textY += 8.0f;
                    render.DrawTextDefault("所持数: " + std::to_string(cardInfo.countAfter),
                                          cardX + padding, textY,
                                          fontSize * 0.8f, OverlayColors::TEXT_SECONDARY);
                }
            } else if (cardInfo.equipment) {
                // 装備カード
                render.DrawTextDefault(cardInfo.equipment->name,
                                      cardX + padding, textY,
                                      fontSize * 0.9f, OverlayColors::TEXT_PRIMARY);
                textY += fontSize * 1.2f;
                if (!cardInfo.equipment->description.empty()) {
                    render.DrawTextDefault(cardInfo.equipment->description,
                                          cardX + padding, textY,
                                          fontSize * 0.75f, OverlayColors::TEXT_SECONDARY);
                    textY += fontSize * 1.0f;
                }
                textY += 8.0f;
                render.DrawTextDefault("ATK: " + std::to_string(static_cast<int>(cardInfo.equipment->attack_bonus)),
                                      cardX + padding, textY,
                                      fontSize * 0.8f, OverlayColors::TEXT_SECONDARY);
                textY += fontSize * 0.9f;
                render.DrawTextDefault("DEF: " + std::to_string(static_cast<int>(cardInfo.equipment->defense_bonus)),
                                      cardX + padding, textY,
                                      fontSize * 0.8f, OverlayColors::TEXT_SECONDARY);
                textY += fontSize * 0.9f;
                render.DrawTextDefault("HP: " + std::to_string(static_cast<int>(cardInfo.equipment->hp_bonus)),
                                      cardX + padding, textY,
                                      fontSize * 0.8f, OverlayColors::TEXT_SECONDARY);
                if (cardInfo.countAfter > 0) {
                    textY += fontSize * 0.9f;
                    render.DrawTextDefault("所持数: " + std::to_string(cardInfo.countAfter),
                                          cardX + padding, textY,
                                          fontSize * 0.8f, OverlayColors::TEXT_SECONDARY);
                }
            }

            // SSR/SRカードのエフェクト（メッセージカード以外）
            if (!cardInfo.isMessageCard && 
                (cardInfo.rarity == GachaRarity::SSR || cardInfo.rarity == GachaRarity::SR)) {
                const float glowIntensity = animProgress * (0.5f + 0.5f * pulse);
                const float glowThickness = 8.0f + 4.0f * pulse;
                Color glowColor = borderColor;
                glowColor.a = static_cast<unsigned char>(glowColor.a * 0.3f * glowIntensity);
                render.DrawRectangleLines(cardX - glowThickness * 0.5f, cardY - glowThickness * 0.5f,
                                         cardW + glowThickness, cardH + glowThickness,
                                         glowThickness, glowColor);
                
                if (cardInfo.rarity == GachaRarity::SSR) {
                    const float sparkleRadius = 12.0f + 6.0f * pulse;
                    const float centerX = cardX + cardW * 0.5f;
                    const float centerY = cardY + cardH * 0.5f;
                    Color sparkleColor = borderColor;
                    sparkleColor.a = static_cast<unsigned char>(sparkleColor.a * 0.2f * glowIntensity);
                    render.DrawCircleLines(centerX, centerY, sparkleRadius, 2.0f, sparkleColor);
                }
            }
        }


    } else if (currentTab_ == GachaTab::Rates) {
        // 提供割合のバーグラフ（固定位置、スクロールの影響を受けない）
        // 最上の座標を固定：panelY_とcontentTop_はInitializeで設定される固定値
        const float barStartX = panelX_ + contentLeft_ + 40.0f;
        // 固定位置：panelY_ + contentTop_ + 60.0f（スクロールオフセットは一切適用しない）
        const float barStartY = panelY_ + contentTop_ + 60.0f;
        const float barWidth = contentRight_ - contentLeft_ - 80.0f;
        const float barHeight = 32.0f;
        const float barSpacing = 12.0f;
        const float labelWidth = 80.0f;
        
        // 右端の位置を計算（パーセンテージ用）
        const float rightMargin = 20.0f;
        const float rightEdgeX = panelX_ + contentRight_ - rightMargin;
        
        auto drawRateBar = [&](const std::string& label, float rate, 
                              GachaRarity rarity, float y) {
            // ラベル（固定位置）
            render.DrawTextDefault(label,
                                  barStartX, y + (barHeight - fontSize * 0.9f) * 0.5f,
                                  fontSize * 0.9f, subColor);
            
            // バーの背景（パーセンテージのスペースを確保）
            const float barX = barStartX + labelWidth + 20.0f;
            const float percentTextWidth = 60.0f; // パーセンテージ用の幅（"100.0%" を想定）
            const float adjustedBarWidth = (rightEdgeX - barX) - percentTextWidth;
            const Color barBg = OverlayColors::PANEL_BG_DARK;
            render.DrawRectangle(barX, y, adjustedBarWidth, barHeight, barBg);
            
            // バーの値
            const float fillWidth = adjustedBarWidth * (rate / 100.0f);
            if (fillWidth > 0.0f) {
                const Color barFill = GetRarityBorderColor(rarity, alpha * 0.8f);
                render.DrawRectangle(barX, y, fillWidth, barHeight, barFill);
            }
            
            // パーセンテージテキスト（右端に揃える、固定位置）
            const std::string percentText = FormatPercent(rate) + "%";
            Vector2 textSize = render.MeasureTextDefault(percentText, fontSize * 0.85f);
            const float percentX = rightEdgeX - textSize.x;
            render.DrawTextDefault(percentText,
                                  percentX,
                                  y + (barHeight - textSize.y) * 0.5f,
                                  fontSize * 0.85f, subColor);
        };
        
        // バーグラフは固定位置に描画（スクロールオフセットは適用しない）
        float currentY = barStartY;
        drawRateBar("N", rateN_, GachaRarity::N, currentY);
        currentY += barHeight + barSpacing;
        drawRateBar("R", rateR_, GachaRarity::R, currentY);
        currentY += barHeight + barSpacing;
        drawRateBar("SR", rateSR_, GachaRarity::SR, currentY);
        currentY += barHeight + barSpacing;
        drawRateBar("SSR", rateSSR_, GachaRarity::SSR, currentY);
        
        // 提供割合リスト
        // バーグラフの下端を計算（SSRバーの下端）
        const float barGraphBottom = currentY + barHeight;
        const float listStartY = barGraphBottom + 32.0f; // バーグラフとの間隔（20.0f → 32.0fに変更、12px追加）
        const float listItemHeight = 28.0f;
        const float ratesScrollOffset = scrollYRates_;
        const float ratesListStartY = listStartY;
        const float listItemX = barStartX; // リストアイテムの開始X位置（固定）
        const float listItemRightX = rightEdgeX; // 右端の位置（バーグラフと同じ）
        
        // スクロール可能な領域の上端（バーグラフの下）
        const float scrollAreaTop = barGraphBottom;
        // スクロール可能な領域の下端
        const float scrollAreaBottom = panelY_ + contentBottom_;
        
        for (size_t i = 0; i < poolItemInfos_.size(); ++i) {
            const float itemY = ratesListStartY + static_cast<float>(i) * listItemHeight - ratesScrollOffset;
            
            // ウィンドウ外のアイテムは描画しない（クリッピング）
            // バーグラフの下から、コンテンツエリアの下端まで
            if (itemY + listItemHeight < scrollAreaTop || itemY > scrollAreaBottom) {
                continue;
            }
            
            const auto& info = poolItemInfos_[i];
            
            // 名前とレアリティを左側に描画（固定位置）
            const std::string nameAndRarity = info.name + " " + info.rarity;
            render.DrawTextDefault(nameAndRarity,
                                  listItemX, itemY + (listItemHeight - fontSize * 0.8f) * 0.5f,
                                  fontSize * 0.8f, subColor);
            
            // パーセンテージを右側に揃えて描画（はみ出さないように）
            const std::string percentText = FormatPercent(info.percent) + "%";
            Vector2 percentSize = render.MeasureTextDefault(percentText, fontSize * 0.8f);
            const float percentX = listItemRightX - percentSize.x;
            render.DrawTextDefault(percentText,
                                  percentX, itemY + (listItemHeight - fontSize * 0.8f) * 0.5f,
                                  fontSize * 0.8f, subColor);
        }
        
        // 説明テキスト
        render.DrawTextDefault("提供割合（装備・アタッチメントの重みに基づく）",
                              panelX_ + 40.0f,
                              panelY_ + panelH_ - GACHA_FOOTNOTE_Y_OFFSET,
                              fontSize * 0.85f, subColor);

    } else if (currentTab_ == GachaTab::History) {
        // 履歴リスト
        const float listX = panelX_ + contentLeft_;
        const float listY = panelY_ + contentTop_;
        const float itemHeight = 34.0f;
        const float historyScrollOffset = scrollYHistory_;
        
        for (size_t i = 0; i < historyItemInfos_.size() && i < static_cast<size_t>(HISTORY_DISPLAY_LIMIT); ++i) {
            const auto& info = historyItemInfos_[i];
            const float itemY = listY + static_cast<float>(i) * itemHeight - historyScrollOffset;
            
            // ウィンドウ外のアイテムは描画しない（クリッピング）
            if (itemY + itemHeight < panelY_ + contentTop_ || itemY > panelY_ + contentBottom_) {
                continue;
            }
            
            // レアリティ色のバー
            const float barWidth = 4.0f;
            const Color barColor = GetRarityBorderColor(info.rarity, alpha);
            render.DrawRectangle(listX, itemY, barWidth, itemHeight, barColor);
            
            // アイテムテキスト
            render.DrawTextDefault(info.label,
                                  listX + barWidth + 8.0f, itemY + (itemHeight - fontSize * 0.85f) * 0.5f,
                                  fontSize * 0.85f, subColor);
            
            // 値
            Vector2 valueSize = render.MeasureTextDefault(info.value, fontSize * 0.85f);
            render.DrawTextDefault(info.value,
                                  panelX_ + contentRight_ - valueSize.x - 8.0f,
                                  itemY + (itemHeight - fontSize * 0.85f) * 0.5f,
                                  fontSize * 0.85f, OverlayColors::TEXT_SECONDARY);
        }

    } else if (currentTab_ == GachaTab::Exchange) {
        // 交換ボタン
        const float exchange1X = panelW_ / 2.0f - 220.0f;
        const float exchange10X = panelW_ / 2.0f + 20.0f;
        const float exchangeY = contentTop_ + 40.0f;
        const float exchangeW = 200.0f;
        const float exchangeH = 56.0f;
        
        const bool exchange1Enabled = ctx.gameplayDataAPI && 
                                     ctx.gameplayDataAPI->GetGachaDust() >= DUST_FOR_TICKET;
        const bool exchange10Enabled = ctx.gameplayDataAPI && 
                                      ctx.gameplayDataAPI->GetGachaDust() >= DUST_FOR_TEN_TICKETS;
        
        Color exchange1Bg = hoveredExchange1Button_ && exchange1Enabled ?
                               OverlayColors::BUTTON_PRIMARY_HOVER : OverlayColors::BUTTON_PRIMARY;
        if (!exchange1Enabled) exchange1Bg = OverlayColors::BUTTON_DISABLED;
        
        Color exchange10Bg = hoveredExchange10Button_ && exchange10Enabled ?
                                OverlayColors::BUTTON_PRIMARY_HOVER : OverlayColors::BUTTON_PRIMARY;
        if (!exchange10Enabled) exchange10Bg = OverlayColors::BUTTON_DISABLED;
        
        render.DrawRectangle(panelX_ + exchange1X, panelY_ + exchangeY,
                           exchangeW, exchangeH, exchange1Bg);
        render.DrawRectangle(panelX_ + exchange10X, panelY_ + exchangeY,
                           exchangeW, exchangeH, exchange10Bg);
        
        Vector2 exchange1TextSize = render.MeasureTextDefault("チケット x1", fontSize * 0.9f);
        render.DrawTextDefault("チケット x1",
                              panelX_ + exchange1X + (exchangeW - exchange1TextSize.x) * 0.5f,
                              panelY_ + exchangeY + (exchangeH - exchange1TextSize.y) * 0.5f,
                              fontSize * 0.9f, OverlayColors::TEXT_PRIMARY);
        
        Vector2 exchange10TextSize = render.MeasureTextDefault("チケット x10", fontSize * 0.9f);
        render.DrawTextDefault("チケット x10",
                              panelX_ + exchange10X + (exchangeW - exchange10TextSize.x) * 0.5f,
                              panelY_ + exchangeY + (exchangeH - exchange10TextSize.y) * 0.5f,
                              fontSize * 0.9f, OverlayColors::TEXT_PRIMARY);
        
        // 説明テキスト
        const std::string exchange = "ダスト交換  x1(" + std::to_string(DUST_FOR_TICKET) +
                                    ")  x10(" + std::to_string(DUST_FOR_TEN_TICKETS) + ")";
        render.DrawTextDefault(exchange,
                              panelX_ + 40.0f,
                              panelY_ + panelH_ - GACHA_FOOTNOTE_Y_OFFSET,
                              fontSize * 0.95f, subColor);
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

    // メッセージカード情報をCardInfoとして保存
    CardInfo cardInfo;
    cardInfo.x = x;
    cardInfo.y = y;
    cardInfo.width = cardW;
    cardInfo.height = cardH;
    cardInfo.rarity = GachaRarity::R;
    cardInfo.revealTime = cardAnimationTimer_;
    cardInfo.animationProgress = 1.0f;
    cardInfo.equipment = nullptr;
    cardInfo.title = title;
    cardInfo.message = message;
    cardInfo.isMessageCard = true;
    resultCardInfos_.push_back(cardInfo);
}

void GachaOverlay::ShowEquipmentResults(
    float contentWidth, float contentHeight,
    const std::vector<const entities::Equipment*>& results) {
    ClearResultCards();

    if (results.empty()) {
        ShowMessageCard(contentWidth, contentHeight, "結果", "排出結果がありません。");
        return;
    }

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
    if ((!result.equipment && !result.attachment) || total <= 0) {
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

    CardInfo cardInfo;
    cardInfo.x = x;
    cardInfo.y = y;
    cardInfo.width = cardW;
    cardInfo.height = cardH;
    cardInfo.rarity = result.rarity;
    cardInfo.revealTime = cardAnimationTimer_;
    cardInfo.animationProgress = 0.0f;
    cardInfo.equipment = result.equipment;
    cardInfo.attachment = result.attachment;
    cardInfo.countAfter = result.countAfter;
    resultCardInfos_.push_back(cardInfo);
}

} // namespace core
} // namespace game
