#include "EnhancementOverlay.hpp"
#include "EnhancementOverlayInternal.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "../../api/BaseSystemAPI.hpp"
#include "../../api/GameplayDataAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../config/RenderPrimitives.hpp"
#include "../../config/RenderTypes.hpp"
#include "../../ecs/entities/TowerAttachment.hpp"
#include "../../system/TowerEnhancementEffects.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UIEffects.hpp"

namespace game {
namespace core {

namespace hi = enhancement_overlay_internal;


void EnhancementOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) {
        return;
    }

    if (!ctx.gameplayDataAPI || !systemAPI_) {
        return;
    }

    RenderItemListPanel(ctx);
    RenderCenterPanel(ctx);
    RenderRightPanel(ctx);

    // ドラッグ中のゴースト表示
    if (is_attachment_dragging_ && dragging_attachment_) {
        auto& render = systemAPI_->Render();
        const float ghostW = 160.0f;
        const float ghostH = 80.0f;
        const float gx = attachment_drag_pos_.x - ghostW * 0.5f;
        const float gy = attachment_drag_pos_.y - ghostH * 0.5f;
        render.DrawRectangle(gx, gy, ghostW, ghostH, ui::OverlayColors::PANEL_BG_SECONDARY);
        render.DrawRectangleLines(gx, gy, ghostW, ghostH, 2.0f, ui::OverlayColors::BORDER_GOLD);
        render.DrawTextDefault(dragging_attachment_->name.c_str(), gx + 8.0f, gy + 12.0f,
                               hi::FONT_BODY, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        const std::string effectStr = hi::BuildAttachmentEffectText(*dragging_attachment_, hi::ATTACHMENT_EFFECT_DISPLAY_LEVEL);
        render.DrawTextDefault(effectStr.c_str(), gx + 8.0f, gy + 36.0f,
                               hi::FONT_CAPTION, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    }
}

void EnhancementOverlay::RenderMultiplierPanel(SharedContext& ctx, const Rect& panelRect,
                                               const system::TowerEnhancementMultipliers& mul) {
    if (!systemAPI_) return;
    const Vec2 mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePositionInternal() : Vec2{0.0f, 0.0f};
    auto inRect = [&](const Rect& r) {
        return mouse.x >= r.x && mouse.x <= r.x + r.width &&
               mouse.y >= r.y && mouse.y <= r.y + r.height;
    };
    auto& render = systemAPI_->Render();

    const float cornerRadius = 12.0f;
    const int segments = 12;
    ::Rectangle rect{panelRect.x, panelRect.y, panelRect.width, panelRect.height};
    render.DrawRectangleRounded(rect, cornerRadius / panelRect.width, segments,
                                ui::OverlayColors::PANEL_BG_ORANGE);
    render.DrawRectangleRoundedLines(rect, cornerRadius / panelRect.width, segments,
                                     ui::OverlayColors::BORDER_GOLD);

    const bool compact = (panelRect.height < 420.0f);
    const float infoPadding = 25.0f;
    const float infoTitleY = panelRect.y + (compact ? 18.0f : 25.0f);
    const float titleFontSize = compact ? hi::FONT_HEADER : hi::FONT_SECTION;
    render.DrawTextDefault("現在倍率", panelRect.x + infoPadding, infoTitleY, titleFontSize,
                           ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    const float multiplierStartY = infoTitleY + (compact ? 42.0f : 55.0f);
    const float multiplierRowHeight = compact ? 34.0f : 45.0f;
    const float multiplierColWidth = panelRect.width / 2.0f;
    const float multiplierFontSize = hi::FONT_BODY;

    auto drawMultiplier = [&](float x, float y, const char* label, float value) {
        const bool isHighlighted = value != 1.0f;
        const Color textColor = isHighlighted ? ui::OverlayColors::TEXT_PRIMARY : ui::OverlayColors::TEXT_SECONDARY;
        const std::string displayText = std::string(label) + " x" + hi::FormatFloat(value, 2);
        render.DrawTextDefault(displayText.c_str(), x, y, multiplierFontSize, ToCoreColor(textColor));
        Rect multiplierRect{x, y, 200.0f, multiplierRowHeight};
        if (inRect(multiplierRect)) {
            const std::string tooltipText = std::string(label) + "の現在の倍率: " + hi::FormatFloat(value, 2) + "倍";
            DrawTooltip(tooltipText, x + 100.0f, y);
        }
    };

    drawMultiplier(panelRect.x + infoPadding, multiplierStartY, "城HP", mul.playerTowerHpMul);
    drawMultiplier(panelRect.x + infoPadding + multiplierColWidth, multiplierStartY, "お金成長", mul.walletGrowthMul);
    drawMultiplier(panelRect.x + infoPadding, multiplierStartY + multiplierRowHeight, "コスト回復", mul.costRegenMul);
    drawMultiplier(panelRect.x + infoPadding + multiplierColWidth, multiplierStartY + multiplierRowHeight, "味方ATK", mul.allyAttackMul);
    drawMultiplier(panelRect.x + infoPadding, multiplierStartY + multiplierRowHeight * 2.0f, "味方HP", mul.allyHpMul);
    drawMultiplier(panelRect.x + infoPadding + multiplierColWidth, multiplierStartY + multiplierRowHeight * 2.0f, "敵HP", mul.enemyHpMul);
    drawMultiplier(panelRect.x + infoPadding, multiplierStartY + multiplierRowHeight * 3.0f, "敵ATK", mul.enemyAttackMul);
    drawMultiplier(panelRect.x + infoPadding + multiplierColWidth, multiplierStartY + multiplierRowHeight * 3.0f, "敵速度", mul.enemyMoveSpeedMul);
}

void EnhancementOverlay::RenderItemListPanel(SharedContext& ctx) {
    if (!systemAPI_) return;
    auto& render = systemAPI_->Render();
    const float pad = 18.0f;
    const float titleH = 44.0f;
    const float listY = item_list_panel_.y + titleH + 12.0f;
    const float listH = item_list_panel_.height - (listY - item_list_panel_.y) - pad;

    ::Rectangle bgRect{item_list_panel_.x, item_list_panel_.y, item_list_panel_.width, item_list_panel_.height};
    render.DrawRectangleRounded(bgRect, 12.0f / item_list_panel_.width, 12, ui::OverlayColors::PANEL_BG_ORANGE);
    render.DrawRectangleRoundedLines(bgRect, 12.0f / item_list_panel_.width, 12, ui::OverlayColors::BORDER_GOLD);
    render.DrawTextDefault("項目", item_list_panel_.x + pad, item_list_panel_.y + 12.0f, hi::FONT_SECTION,
                           ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    static constexpr const char* kItemLabels[] = {
        "一覧",
        "城HP最大値", "お金成長/秒", "コスト回復/秒", "味方攻撃", "味方HP",
        "スロット1", "スロット2", "スロット3"
    };
    const int n = 9;
    for (int i = 0; i < n; ++i) {
        const float rowY = listY + static_cast<float>(i) * item_list_panel_.item_height;
        const bool selected = (item_list_panel_.selected_index == i);
        Color bgColor = selected ? ui::OverlayColors::CARD_BG_SELECTED : ui::OverlayColors::PANEL_BG_SECONDARY;
        Color borderColor = selected ? ui::OverlayColors::BORDER_BLUE : ui::OverlayColors::BORDER_DEFAULT;
        const float rowW = item_list_panel_.width - pad * 2.0f;
        ::Rectangle rowRect{item_list_panel_.x + pad, rowY, rowW, item_list_panel_.item_height - 4.0f};
        render.DrawRectangleRounded(rowRect, 6.0f / rowW, 6, bgColor);
        render.DrawRectangleRoundedLines(rowRect, 6.0f / rowW, 6, borderColor);
        const float textY = rowY + (item_list_panel_.item_height - 4.0f - 24.0f) * 0.5f;
        render.DrawTextDefault(kItemLabels[i], item_list_panel_.x + pad + 12.0f, textY, hi::FONT_BODY,
                               ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
    }
}

void EnhancementOverlay::RenderCenterPanel(SharedContext& ctx) {
    if (item_list_panel_.selected_index == 0) {
        RenderOverviewTab(ctx);
    } else if (item_list_panel_.selected_index >= 1 && item_list_panel_.selected_index <= 5) {
        RenderBaseEnhancementTab(ctx);
    } else if (item_list_panel_.selected_index >= 6 && item_list_panel_.selected_index <= 8) {
        const int slotIndex = item_list_panel_.selected_index - 6;
        RenderSlotDetailPanel(ctx, slotIndex);
    }
}

void EnhancementOverlay::RenderOverviewTab(SharedContext& ctx) {
    if (!ctx.gameplayDataAPI || !systemAPI_) return;
    auto st = ctx.gameplayDataAPI->GetTowerEnhancements();
    auto attachments = ctx.gameplayDataAPI->GetTowerAttachments();
    const auto& masters = ctx.gameplayDataAPI->GetAllTowerAttachmentMasters();
    const auto mul = system::CalculateTowerEnhancementMultipliers(st, attachments, masters);
    auto& render = systemAPI_->Render();

    constexpr float PANEL_GAP = 10.0f;
    const float compactHeight = 220.0f;
    Rect multiplierRect{status_panel_.x, status_panel_.y, status_panel_.width, compactHeight};
    RenderMultiplierPanel(ctx, multiplierRect, mul);

    const float basePanelX = status_panel_.x;
    const float basePanelW = status_panel_.width;
    const float basePanelY = status_panel_.y + compactHeight + PANEL_GAP;
    const float basePanelH = status_panel_.height - compactHeight - PANEL_GAP;

    ::Rectangle basePanelRect{basePanelX, basePanelY, basePanelW, basePanelH};
    render.DrawRectangleRounded(basePanelRect, 12.0f / basePanelW, 12,
                                ui::OverlayColors::PANEL_BG_SECONDARY);
    render.DrawRectangleRoundedLines(basePanelRect, 12.0f / basePanelW, 12,
                                     ui::OverlayColors::BORDER_DEFAULT);

    const float pad = 20.0f;
    constexpr float OVERVIEW_CONTENT_OFFSET = 24.0f;  // 一覧全体を下げる
    const float titleY = basePanelY + 12.0f + OVERVIEW_CONTENT_OFFSET;
    const float tableY = basePanelY + 26.0f + OVERVIEW_CONTENT_OFFSET;
    const float rowHeight = 40.0f;
    const float colNameX = basePanelX + pad;
    const float colLvX = colNameX + 140.0f;
    const float colEffectX = colLvX + 70.0f;

    render.DrawTextDefault("基礎強化（全項目）", basePanelX + pad, titleY,
                           hi::FONT_HEADER, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    constexpr int MAX_LEVEL = 50;
    struct BaseRow { const char* name; int level; float perLevel; };
    std::array<BaseRow, 5> rows = {{
        {"城HP最大値", st.towerHpLevel, 0.05f},
        {"お金成長/秒", st.walletGrowthLevel, 0.05f},
        {"コスト回復/秒", st.costRegenLevel, 0.05f},
        {"味方攻撃", st.allyAttackLevel, 0.02f},
        {"味方HP", st.allyHpLevel, 0.02f}
    }};

    for (size_t i = 0; i < rows.size(); ++i) {
        const float rowY = tableY + static_cast<float>(i) * rowHeight;
        const int level = std::max(0, std::min(MAX_LEVEL, rows[i].level));
        const float cur = rows[i].perLevel * static_cast<float>(level) * 100.0f;
        const Color levelColor = (level >= MAX_LEVEL) ? ui::OverlayColors::ACCENT_GOLD : ui::OverlayColors::TEXT_SECONDARY;
        const Color effectColor = (cur > 0.0f) ? ui::OverlayColors::SUCCESS_GREEN : ui::OverlayColors::TEXT_SECONDARY;
        render.DrawTextDefault(rows[i].name, colNameX, rowY + 8.0f, hi::FONT_BODY,
                               ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        render.DrawTextDefault(("Lv " + std::to_string(level)).c_str(), colLvX, rowY + 8.0f,
                               hi::FONT_BODY, ToCoreColor(levelColor));
        render.DrawTextDefault(("+" + hi::FormatFloat(cur, 1) + "%").c_str(), colEffectX, rowY + 8.0f,
                               hi::FONT_BODY, ToCoreColor(effectColor));
    }

    const float slotSectionY = tableY + static_cast<float>(rows.size()) * rowHeight + 12.0f;
    render.DrawTextDefault("アタッチメント装備", basePanelX + pad, slotSectionY,
                           hi::FONT_HEADER, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
    static constexpr const char* kSlotLabels[] = {"スロット1", "スロット2", "スロット3"};
    const float slotLineH = 28.0f;
    for (int i = 0; i < 3; ++i) {
        const float lineY = slotSectionY + 22.0f + static_cast<float>(i) * slotLineH;
        const auto& slot = attachments[i];
        std::string text = std::string(kSlotLabels[i]) + ": ";
        if (slot.id.empty()) {
            text += "未装着";
            render.DrawTextDefault(text.c_str(), basePanelX + pad, lineY, hi::FONT_BODY,
                                   ToCoreColor(ui::OverlayColors::TEXT_MUTED));
        } else {
            auto it = masters.find(slot.id);
            if (it != masters.end()) {
                text += it->second.name + " [" + GetRarityName(it->second.rarity) + "]";
                const Color rarityColor = GetRarityColor(it->second.rarity);
                render.DrawTextDefault(text.c_str(), basePanelX + pad, lineY, hi::FONT_BODY,
                                       ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
                const std::string effStr = hi::BuildAttachmentEffectText(it->second, hi::ATTACHMENT_EFFECT_DISPLAY_LEVEL);
                render.DrawTextDefault(effStr.c_str(), basePanelX + pad + 280.0f, lineY, hi::FONT_CAPTION,
                                       ToCoreColor(ui::OverlayColors::SUCCESS_GREEN));
            } else {
                text += "?";
                render.DrawTextDefault(text.c_str(), basePanelX + pad, lineY, hi::FONT_BODY,
                                       ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
            }
        }
    }
}

void EnhancementOverlay::RenderSlotDetailPanel(SharedContext& ctx, int slotIndex) {
    if (!ctx.gameplayDataAPI || !systemAPI_) return;
    auto attachments = ctx.gameplayDataAPI->GetTowerAttachments();
    const auto& masters = ctx.gameplayDataAPI->GetAllTowerAttachmentMasters();
    auto& render = systemAPI_->Render();

    const float cornerRadius = 12.0f;
    const int segments = 12;
    ::Rectangle panelRect{status_panel_.x, status_panel_.y, status_panel_.width, status_panel_.height};
    render.DrawRectangleRounded(panelRect, cornerRadius / status_panel_.width, segments,
                                ui::OverlayColors::PANEL_BG_SECONDARY);
    render.DrawRectangleRoundedLines(panelRect, cornerRadius / status_panel_.width, segments,
                                     ui::OverlayColors::BORDER_DEFAULT);

    static constexpr const char* kSlotLabels[] = {"スロット1", "スロット2", "スロット3"};
    const char* titleLabel = (slotIndex >= 0 && slotIndex < 3) ? kSlotLabels[slotIndex] : "";
    const float pad = 25.0f;
    const float titleY = status_panel_.y + pad;
    render.DrawTextDefault(titleLabel, status_panel_.x + pad, titleY, hi::FONT_SECTION,
                           ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    const auto& slot = attachments[slotIndex];
    const entities::TowerAttachment* att = nullptr;
    if (!slot.id.empty()) {
        auto it = masters.find(slot.id);
        if (it != masters.end()) att = &it->second;
    }

    float currentY = titleY + 50.0f;
    const float lineH = 36.0f;

    if (att) {
        const Color rarityColor = GetRarityColor(att->rarity);
        render.DrawTextDefault((att->name + " [" + GetRarityName(att->rarity) + "]").c_str(),
                               status_panel_.x + pad, currentY, hi::FONT_BODY, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        currentY += lineH;
        if (!att->description.empty()) {
            render.DrawTextDefault(("説明: " + att->description).c_str(), status_panel_.x + pad, currentY,
                                   hi::FONT_CAPTION, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
            currentY += lineH * 1.2f;
        }
        const std::string effectStr = hi::ToAttachmentTargetLabel(att->target_stat) + std::string(" ") +
            hi::BuildAttachmentEffectText(*att, hi::ATTACHMENT_EFFECT_DISPLAY_LEVEL);
        render.DrawTextDefault(("効果: " + effectStr).c_str(), status_panel_.x + pad, currentY,
                               hi::FONT_BODY, ToCoreColor(ui::OverlayColors::SUCCESS_GREEN));
        currentY += lineH + 20.0f;

        const Vec2 mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePositionInternal() : Vec2{0.0f, 0.0f};
        const float btnW = 120.0f;
        const float btnH = 40.0f;
        const float btnX = status_panel_.x + pad;
        const float btnY = currentY;
        const bool hoverRemove = (mouse.x >= btnX && mouse.x < btnX + btnW && mouse.y >= btnY && mouse.y < btnY + btnH);
        ui::UIEffects::DrawModernButton(systemAPI_, btnX, btnY, btnW, btnH,
                                        ui::OverlayColors::PANEL_BG_PRIMARY, ui::OverlayColors::PANEL_BG_SECONDARY,
                                        hoverRemove, false);
        render.DrawTextDefault("解除", btnX + (btnW - render.MeasureTextDefault("解除", hi::FONT_BUTTON).x) * 0.5f,
                               btnY + (btnH - 22.0f) * 0.5f, hi::FONT_BUTTON, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
    } else {
        render.DrawTextDefault("未装着", status_panel_.x + pad, currentY, hi::FONT_BODY,
                               ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    }
}

void EnhancementOverlay::RenderBaseEnhancementTab(SharedContext& ctx) {
    if (!ctx.gameplayDataAPI || !systemAPI_) {
        return;
    }

    auto st = ctx.gameplayDataAPI->GetTowerEnhancements();
    auto attachments = ctx.gameplayDataAPI->GetTowerAttachments();
    const auto& masters = ctx.gameplayDataAPI->GetAllTowerAttachmentMasters();
    const auto mul = system::CalculateTowerEnhancementMultipliers(st, attachments, masters);

    const Vec2 mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePositionInternal() : Vec2{0.0f, 0.0f};
    auto inRect = [&](const Rect& r) {
        return mouse.x >= r.x && mouse.x <= r.x + r.width &&
               mouse.y >= r.y && mouse.y <= r.y + r.height;
    };

    auto& render = systemAPI_->Render();

    // ========== 中央パネル: 現在倍率（コンパクト）＋ 基礎強化テーブル ==========
    const float compactHeight = 220.0f;
    Rect multiplierRect{status_panel_.x, status_panel_.y, status_panel_.width, compactHeight};
    RenderMultiplierPanel(ctx, multiplierRect, mul);

    // ========== 基礎強化テーブル＋ボタン（中央パネル内で倍率の下） ==========
    constexpr float PANEL_GAP = 10.0f;
    const float basePanelX = status_panel_.x;
    const float basePanelW = status_panel_.width;
    const float basePanelY = status_panel_.y + compactHeight + PANEL_GAP;
    const float basePanelH = status_panel_.height - compactHeight - PANEL_GAP;

    const float basePanelCornerRadius = 12.0f;
    const int basePanelSegments = 12;
    ::Rectangle basePanelRect{basePanelX, basePanelY, basePanelW, basePanelH};
    render.DrawRectangleRounded(basePanelRect, basePanelCornerRadius / basePanelW, basePanelSegments,
                                ui::OverlayColors::PANEL_BG_SECONDARY);
    render.DrawRectangleRoundedLines(basePanelRect, basePanelCornerRadius / basePanelW, basePanelSegments,
                                     ui::OverlayColors::BORDER_DEFAULT);

    constexpr int MAX_LEVEL = 50;
    struct BaseRow {
        const char* name;
        int level;
        float perLevel;
    };
    std::array<BaseRow, 5> rows = {{
        {"城HP最大値", st.towerHpLevel, 0.05f},
        {"お金成長/秒", st.walletGrowthLevel, 0.05f},
        {"コスト回復/秒", st.costRegenLevel, 0.05f},
        {"味方攻撃", st.allyAttackLevel, 0.02f},
        {"味方HP", st.allyHpLevel, 0.02f}
    }};

    const float pad = 20.0f;
    const float tableY = basePanelY + hi::BASE_TABLE_TOP_OFFSET;
    const float rowHeight = hi::BASE_TABLE_ROW_HEIGHT;

    // テーブルは全幅使用（ボタンは下段の中央2列×3行に配置・ユニットオーバーレイに合わせる）
    const float tableContentW = basePanelW - pad * 2.0f;
    const float colNameX = basePanelX + pad;
    const float colLvX = colNameX + 120.0f;
    const float colCurX = colLvX + 70.0f;
    const float colNextX = colCurX + 85.0f;
    const float colInfoX = colNextX + 90.0f;

    const int ownedGold = ctx.gameplayDataAPI ? ctx.gameplayDataAPI->GetGold() : 0;

    // ヘッダー（項目・現在・効果・次。説明・消費は行内表示）
    const float headerTop = tableY - hi::BASE_TABLE_HEADER_HEIGHT - 4.0f;
    const Rect headerRect{colNameX - 6.0f, headerTop, tableContentW + 12.0f, hi::BASE_TABLE_HEADER_HEIGHT};
    const float headerCornerRadius = 4.0f;
    ::Rectangle headerRectRounded{headerRect.x, headerRect.y, headerRect.width, headerRect.height};
    render.DrawRectangleRounded(headerRectRounded, headerCornerRadius / headerRect.width, 6,
                                ui::OverlayColors::CARD_BG_SELECTED);
    render.DrawRectangleRoundedLines(headerRectRounded, headerCornerRadius / headerRect.width, 6,
                                     ui::OverlayColors::BORDER_BLUE);

    const float headerTextY = headerTop + (hi::BASE_TABLE_HEADER_HEIGHT - 22.0f) * 0.5f;
    render.DrawTextDefault("項目", colNameX, headerTextY, hi::FONT_BODY,
                           ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
    render.DrawTextDefault("現在", colLvX, headerTextY, hi::FONT_BODY,
                           ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
    render.DrawTextDefault("効果", colCurX, headerTextY, hi::FONT_BODY,
                           ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
    render.DrawTextDefault("次", colNextX, headerTextY, hi::FONT_BODY,
                           ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    const float btnFontSize = hi::FONT_BUTTON;
    auto drawBaseButton = [&](const Rect& rect, const char* label, bool hovered, bool isPositive, bool isDisabled = false, const char* tooltip = nullptr) {
        if (isDisabled) {
            render.DrawRectangle(rect.x, rect.y, rect.width, rect.height,
                                 ui::OverlayColors::BUTTON_DISABLED);
            render.DrawRectangleLines(rect.x, rect.y, rect.width, rect.height,
                                      2.0f, ui::OverlayColors::BORDER_DEFAULT);
            Vector2 textSize = render.MeasureTextDefault(label, btnFontSize);
            const float textX = rect.x + (rect.width - textSize.x) * 0.5f;
            const float textY = rect.y + (rect.height - textSize.y) * 0.5f;
            render.DrawTextDefault(label, textX, textY, btnFontSize,
                                   ToCoreColor(ui::OverlayColors::TEXT_MUTED));
            return;
        }
        Color darkColor, brightColor, textColor;
        if (isPositive) {
            darkColor = ui::OverlayColors::BUTTON_PRIMARY_DARK;
            brightColor = ui::OverlayColors::BUTTON_PRIMARY_BRIGHT;
            textColor = ui::OverlayColors::TEXT_DARK;
        } else {
            darkColor = ui::OverlayColors::PANEL_BG_PRIMARY;
            brightColor = ui::OverlayColors::PANEL_BG_SECONDARY;
            textColor = ui::OverlayColors::TEXT_SECONDARY;
        }
        ui::UIEffects::DrawModernButton(systemAPI_, rect.x, rect.y, rect.width, rect.height,
                                        darkColor, brightColor, hovered, isDisabled);
        Vector2 textSize = render.MeasureTextDefault(label, btnFontSize);
        const float textX = rect.x + (rect.width - textSize.x) * 0.5f;
        const float textY = rect.y + (rect.height - textSize.y) * 0.5f;
        render.DrawTextDefault(label, textX, textY, btnFontSize, ToCoreColor(textColor));
        if (hovered && tooltip && !isDisabled) {
            DrawTooltip(tooltip, rect.x + rect.width * 0.5f, rect.y - 30.0f);
        }
    };

    // 左で選択した基礎強化項目の1行のみ表示（一覧=0 のときは呼ばれない。1～5 → 行 0～4）
    const int selectedRow = item_list_panel_.selected_index - 1;
    if (selectedRow < 0 || selectedRow >= static_cast<int>(rows.size())) {
        return;
    }
    const int i = selectedRow;
    const float rowY = tableY;
        const int level = std::max(0, std::min(MAX_LEVEL, rows[i].level));
        const float cur = rows[i].perLevel * static_cast<float>(level) * 100.0f;
        const float next = rows[i].perLevel * static_cast<float>(std::min(MAX_LEVEL, level + 1)) * 100.0f;

        // 行全体の背景（項目・現在・効果・次の情報のみ、ボタンは下段に配置）
        const Rect rowRect{colNameX - 6.0f, rowY, tableContentW + 12.0f, rowHeight};
        const bool isRowHovered = inRect(rowRect);
        if (isRowHovered) {
            const float rowCornerRadius = 4.0f;
            const int rowSegments = 4;
            ::Rectangle rowRectRounded{rowRect.x, rowRect.y, rowRect.width, rowRect.height};
            render.DrawRectangleRounded(rowRectRounded, rowCornerRadius / rowRect.width, rowSegments,
                                        ui::OverlayColors::PANEL_BG_SECONDARY);
            render.DrawRectangle(rowRect.x, rowRect.y, 4.0f, rowRect.height,
                                 ui::OverlayColors::ACCENT_BLUE);
        }

        const Color levelColor = (level >= MAX_LEVEL) ? ui::OverlayColors::ACCENT_GOLD : ui::OverlayColors::TEXT_SECONDARY;
        const Color effectColor = (cur > 0.0f) ? ui::OverlayColors::SUCCESS_GREEN : ui::OverlayColors::TEXT_SECONDARY;
        Vector2 nameTextSize = render.MeasureTextDefault(rows[i].name, hi::FONT_BODY);
        const float textBaselineY = rowY + (rowHeight - nameTextSize.y) * 0.5f;

        render.DrawTextDefault(rows[i].name, colNameX, textBaselineY, hi::FONT_BODY,
                               ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        render.DrawTextDefault(("Lv " + std::to_string(level)).c_str(), colLvX, textBaselineY,
                               hi::FONT_CAPTION, ToCoreColor(levelColor));
        render.DrawTextDefault(("+" + hi::FormatFloat(cur, 1) + "%").c_str(), colCurX, textBaselineY,
                               hi::FONT_BODY, ToCoreColor(effectColor));
        render.DrawTextDefault(("-> +" + hi::FormatFloat(next, 1) + "%").c_str(), colNextX, textBaselineY,
                               hi::FONT_CAPTION, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));

        // 情報列: 説明とレベルアップ消費量（次のLv: XXX G / 最大）
        const char* desc = hi::GetBaseEnhancementDescription(i);
        if (desc && *desc) {
            render.DrawTextDefault(desc, colInfoX, rowY + 6.0f, hi::FONT_BODY,
                                   ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
        }
        if (level >= MAX_LEVEL) {
            render.DrawTextDefault("最大", colInfoX, rowY + 24.0f, hi::FONT_CAPTION,
                                   ToCoreColor(ui::OverlayColors::TEXT_MUTED));
        } else {
            const int cost1 = hi::ComputeTowerBaseLevelUpCost(level, 1);
            const std::string costStr = "次のLv: " + std::to_string(cost1) + " G";
            render.DrawTextDefault(costStr.c_str(), colInfoX, rowY + 24.0f, hi::FONT_CAPTION,
                                   ToCoreColor(ui::OverlayColors::TEXT_ACCENT));
        }

        const float progressBarX = colLvX;
        const float progressBarY = rowY + rowHeight - 14.0f;
        const float progressBarW = 72.0f;
        const float progressBarH = 6.0f;
        const float progress = static_cast<float>(level) / static_cast<float>(MAX_LEVEL);
        render.DrawRectangle(progressBarX, progressBarY, progressBarW, progressBarH,
                             ui::OverlayColors::PANEL_BG_PRIMARY);
        render.DrawRectangleLines(progressBarX, progressBarY, progressBarW, progressBarH,
                                  1.0f, ui::OverlayColors::BORDER_DEFAULT);
        if (progress > 0.0f) {
            const float fillWidth = progressBarW * progress;
            const Color fillColor = (level >= MAX_LEVEL)
                ? ui::OverlayColors::ACCENT_GOLD : ui::OverlayColors::SUCCESS_GREEN;
            render.DrawRectangle(progressBarX, progressBarY, fillWidth, progressBarH, fillColor);
        }

        // 中央ボタンエリア: ユニットオーバーレイ同様 2列×3行（左: 下げる / 右: 上げる）
        const float centerButtonW = (basePanelW - pad * 2.0f - hi::BASE_CENTER_BUTTON_COL_GAP) / 2.0f;
        const float centerButtonH = hi::BASE_CENTER_BUTTON_H;
        const float bx = basePanelX + pad;
        const float buttonYTop = tableY + rowHeight + hi::BASE_CENTER_BUTTON_TOP_MARGIN;
        const float buttonYMid = buttonYTop + centerButtonH + hi::BASE_CENTER_BUTTON_ROW_GAP;
        const float buttonYBottom = buttonYMid + centerButtonH + hi::BASE_CENTER_BUTTON_ROW_GAP;
        const float rightColX = bx + centerButtonW + hi::BASE_CENTER_BUTTON_COL_GAP;

        Rect rb0{bx, buttonYTop, centerButtonW, centerButtonH};           // -1
        Rect rb1{bx, buttonYMid, centerButtonW, centerButtonH};           // -5
        Rect rb2{bx, buttonYBottom, centerButtonW, centerButtonH};        // 一括へ
        Rect rb3{rightColX, buttonYTop, centerButtonW, centerButtonH};    // +1
        Rect rb4{rightColX, buttonYMid, centerButtonW, centerButtonH};    // +5
        Rect rb5{rightColX, buttonYBottom, centerButtonW, centerButtonH}; // 一括

        const bool canDecrease = level > 0;
        const bool canIncrease = level < MAX_LEVEL;
        const bool canDecrease5 = level >= 5;
        const bool canIncrease5 = level <= MAX_LEVEL - 5;

        const int cost1 = hi::ComputeTowerBaseLevelUpCost(level, 1);
        const int cost5 = hi::ComputeTowerBaseLevelUpCost(level, 5);
        const int costMax = hi::ComputeTowerBaseLevelUpCost(level, MAX_LEVEL - level);
        const bool canAfford1 = ownedGold >= cost1;
        const bool canAfford5 = ownedGold >= cost5;
        const bool canAffordMax = ownedGold >= costMax;

        drawBaseButton(rb0, "レベル-1", inRect(rb0) && canDecrease, false, !canDecrease, "レベルを1下げる");
        drawBaseButton(rb1, "レベル-5", inRect(rb1) && canDecrease5, false, !canDecrease5, "レベルを5下げる");
        drawBaseButton(rb2, "一括へ", inRect(rb2) && canDecrease, false, !canDecrease, "レベルを0まで下げる");
        drawBaseButton(rb3, "レベル+1", inRect(rb3) && canIncrease && canAfford1, true, !canIncrease || !canAfford1, "レベルを1上げる");
        drawBaseButton(rb4, "レベル+5", inRect(rb4) && canIncrease5 && canAfford5, true, !canIncrease5 || !canAfford5, "レベルを5上げる");
        drawBaseButton(rb5, "一括", inRect(rb5) && canIncrease && canAffordMax, true, !canIncrease || !canAffordMax, "レベルを最大まで上げる");
}

void EnhancementOverlay::RenderRightPanel(SharedContext& ctx) {
    if (!ctx.gameplayDataAPI || !systemAPI_) {
        return;
    }

    const auto& masters = ctx.gameplayDataAPI->GetAllTowerAttachmentMasters();
    std::vector<const entities::TowerAttachment*> allAttachments;
    allAttachments.reserve(masters.size());
    for (const auto& [id, attachment] : masters) {
        if (ctx.gameplayDataAPI->GetOwnedTowerAttachmentCount(id) > 0) {
            allAttachments.push_back(&attachment);
        }
    }
    const auto filteredAttachments = SortAttachmentsByName(allAttachments);
    if (selectedAttachmentId_.empty() && !filteredAttachments.empty()) {
        selectedAttachmentId_ = filteredAttachments.front()->id;
    }

    const Vec2 mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePositionInternal() : Vec2{0.0f, 0.0f};
    auto inRect = [&](const Rect& r) {
        return mouse.x >= r.x && mouse.x <= r.x + r.width &&
               mouse.y >= r.y && mouse.y <= r.y + r.height;
    };

    auto& render = systemAPI_->Render();

    const float slotOpPanelCornerRadius = 12.0f;
    const int slotOpPanelSegments = 12;
    ::Rectangle operationPanelRect{operation_panel_.x, operation_panel_.y, operation_panel_.width, operation_panel_.height};
    render.DrawRectangleRounded(operationPanelRect, slotOpPanelCornerRadius / operation_panel_.width, slotOpPanelSegments,
                                ui::OverlayColors::PANEL_BG_SECONDARY);
    render.DrawRectangleRoundedLines(operationPanelRect, slotOpPanelCornerRadius / operation_panel_.width, slotOpPanelSegments,
                                     ui::OverlayColors::BORDER_DEFAULT);

    const float opPadding = 25.0f;
    const float opTitleY = operation_panel_.y + 25.0f;
    render.DrawTextDefault("アタッチメント装備", operation_panel_.x + opPadding, opTitleY, hi::FONT_SECTION,
                           ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    // 固定3スロット横並び
    for (int i = 0; i < 3; ++i) {
        operation_panel_.attachment_slots[i].is_hovered = (GetAttachmentSlotAtPosition(mouse) == i);
        RenderAttachmentSlot(ctx, operation_panel_.attachment_slots[i]);
    }

    const float slotBottomY = operation_panel_.y + operation_panel_.attachment_slots[0].position.y + operation_panel_.attachment_slots[0].height;
    const float listStartY = slotBottomY + 14.0f;
    const float listHeight = operation_panel_.height - (listStartY - operation_panel_.y) - opPadding;

    const float listTitleY = listStartY;
    render.DrawTextDefault("アタッチメント一覧", operation_panel_.x + opPadding, listTitleY, hi::FONT_HEADER,
                           ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
    render.DrawTextDefault("ドラッグでスロットに装着", operation_panel_.x + opPadding, listTitleY + 22.0f, hi::FONT_CAPTION,
                           ToCoreColor(ui::OverlayColors::TEXT_MUTED));
    
    const float listContentY = listTitleY + 44.0f;
    const float listContentHeight = listHeight - (listContentY - listStartY) - opPadding;
    
    Rect listInner{
        operation_panel_.x + opPadding,
        listContentY,
        operation_panel_.width - opPadding * 2.0f,
        listContentHeight
    };
    
    // 一覧の背景
    render.DrawRectangle(listInner.x, listInner.y, listInner.width, listInner.height,
                         ui::OverlayColors::PANEL_BG_SECONDARY);
    render.DrawRectangleLines(listInner.x, listInner.y, listInner.width, listInner.height,
                             2.0f, ui::OverlayColors::BORDER_DEFAULT);
    
    // アタッチメント一覧を描画
    const float itemHeight = 70.0f;
    const int startIndex = std::max(0, static_cast<int>(attachmentListScroll_));
    const int visibleCount = std::max(1, static_cast<int>(listInner.height / itemHeight));
    const int totalItems = static_cast<int>(filteredAttachments.size());
    const bool needsScrollbar = totalItems > visibleCount;
    
    // スクロールバーの描画
    if (needsScrollbar) {
        const float scrollbarWidth = 22.0f;
        const float scrollbarX = listInner.x + listInner.width - scrollbarWidth - 4.0f;
        const float scrollbarY = listInner.y + 4.0f;
        const float scrollbarHeight = listInner.height - 8.0f;
        const float scrollbarTrackHeight = scrollbarHeight;
        
        render.DrawRectangle(scrollbarX, scrollbarY, scrollbarWidth, scrollbarTrackHeight,
                             ui::OverlayColors::PANEL_BG_PRIMARY);
        render.DrawRectangleLines(scrollbarX, scrollbarY, scrollbarWidth, scrollbarTrackHeight,
                                  2.0f, ui::OverlayColors::BORDER_DEFAULT);
        
        const float scrollRatio = (totalItems > visibleCount) 
            ? (startIndex / static_cast<float>(totalItems - visibleCount))
            : 0.0f;
        const float thumbHeight = std::max(30.0f, (static_cast<float>(visibleCount) / static_cast<float>(totalItems)) * scrollbarTrackHeight);
        const float thumbY = scrollbarY + scrollRatio * (scrollbarTrackHeight - thumbHeight);
        
        render.DrawRectangle(scrollbarX + 2.0f, thumbY, scrollbarWidth - 4.0f, thumbHeight,
                             ui::OverlayColors::ACCENT_BLUE);
        render.DrawRectangleLines(scrollbarX + 2.0f, thumbY, scrollbarWidth - 4.0f, thumbHeight,
                                  2.0f, ui::OverlayColors::BORDER_BLUE);
    }
    
    // アタッチメントアイテムを描画
    for (int i = 0; i < visibleCount; ++i) {
        const int idx = startIndex + i;
        if (idx >= totalItems) break;
        const auto* attachment = filteredAttachments[idx];
        Rect itemRect{
            listInner.x,
            listInner.y + itemHeight * i,
            listInner.width - (needsScrollbar ? 26.0f : 0.0f),
            itemHeight
        };
        const bool isItemSelected = attachment && selectedAttachmentId_ == attachment->id;
        const bool isItemHovered = inRect(itemRect);
        
        // レアリティに応じた背景色と角丸デザイン
        Color itemBgColor = ui::OverlayColors::CARD_BG_NORMAL;
        Color itemBorderColor = ui::OverlayColors::BORDER_DEFAULT;
        const float itemCornerRadius = 6.0f;
        const int itemSegments = 6;
        ::Rectangle itemRectRounded{itemRect.x, itemRect.y, itemRect.width, itemRect.height};
        
        if (attachment) {
            const Color rarityColor = GetRarityColor(attachment->rarity);
            if (isItemSelected) {
                itemBgColor = ui::OverlayColors::CARD_BG_SELECTED;
                itemBorderColor = rarityColor;
            } else if (isItemHovered) {
                itemBgColor = ui::OverlayColors::PANEL_BG_SECONDARY;
                itemBorderColor = ui::OverlayColors::BORDER_HOVER;
            }
        }
        
        // 角丸の矩形を描画
        render.DrawRectangleRounded(itemRectRounded, itemCornerRadius / itemRect.width, itemSegments, itemBgColor);
        render.DrawRectangleRoundedLines(itemRectRounded, itemCornerRadius / itemRect.width, itemSegments,
                                         itemBorderColor);
        
        // 選択時は左側にアクセントラインを追加
        if (isItemSelected && attachment) {
            const Color rarityColor = GetRarityColor(attachment->rarity);
            render.DrawRectangle(itemRect.x, itemRect.y, 4.0f, itemRect.height, rarityColor);
        }
        
        if (attachment) {
            const Color nameColor = isItemSelected ? ui::OverlayColors::TEXT_PRIMARY : ui::OverlayColors::TEXT_SECONDARY;
            const Color rarityColor = GetRarityColor(attachment->rarity);
            
            // 名前
            render.DrawTextDefault(attachment->name.c_str(), itemRect.x + 8.0f, itemRect.y + 12.0f,
                                   hi::FONT_BODY, ToCoreColor(nameColor));
            
            // レアリティ
            render.DrawTextDefault(("[" + GetRarityName(attachment->rarity) + "]").c_str(),
                                   itemRect.x + 8.0f, itemRect.y + 36.0f,
                                   hi::FONT_CAPTION, ToCoreColor(rarityColor));
            
            // 対象ステータス・効果（見切れないよう十分な幅と FONT_BODY）
            const float effectColX = itemRect.x + itemRect.width - 220.0f;
            render.DrawTextDefault(hi::ToAttachmentTargetLabel(attachment->target_stat),
                                   effectColX, itemRect.y + 10.0f,
                                   hi::FONT_BODY, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
            const std::string effectText = hi::BuildAttachmentEffectText(*attachment, hi::ATTACHMENT_EFFECT_DISPLAY_LEVEL);
            render.DrawTextDefault((" " + effectText).c_str(),
                                   effectColX, itemRect.y + 34.0f,
                                   hi::FONT_BODY, ToCoreColor(ui::OverlayColors::SUCCESS_GREEN));
        }
    }
}

void EnhancementOverlay::RenderAttachmentSlot(SharedContext& ctx, const OperationPanel::AttachmentSlot& slot) {
    using namespace ui;

    const float abs_x = operation_panel_.x + slot.position.x;
    const float abs_y = operation_panel_.y + slot.position.y;

    Color bg_color = slot.is_hovered
                       ? OverlayColors::SLOT_ORANGE_SELECTED
                       : OverlayColors::SLOT_ORANGE_EMPTY;

    systemAPI_->Render().DrawRectangle(abs_x, abs_y, slot.width, slot.height, bg_color);
    systemAPI_->Render().DrawRectangleLines(abs_x, abs_y, slot.width, slot.height,
                                            2.0f, OverlayColors::BORDER_GOLD);

    if (slot.assigned_attachment) {
        systemAPI_->Render().DrawTextDefault(
            slot.assigned_attachment->name.c_str(), abs_x + 10.0f, abs_y + 16.0f, hi::FONT_HEADER, ToCoreColor(OverlayColors::TEXT_PRIMARY));

        const Color rarityColor = GetRarityColor(slot.assigned_attachment->rarity);
        const std::string rarityName = GetRarityName(slot.assigned_attachment->rarity);
        systemAPI_->Render().DrawTextDefault(
            ("[" + rarityName + "]").c_str(), abs_x + 10.0f, abs_y + 48.0f, hi::FONT_BODY, ToCoreColor(rarityColor));

        const std::string effectText = hi::BuildAttachmentEffectText(*slot.assigned_attachment, hi::ATTACHMENT_EFFECT_DISPLAY_LEVEL);
        systemAPI_->Render().DrawTextDefault(
            ("効果: " + effectText).c_str(), abs_x + 10.0f, abs_y + 76.0f, hi::FONT_BODY, ToCoreColor(OverlayColors::SUCCESS_GREEN));

        const std::string targetLabel = hi::ToAttachmentTargetLabel(slot.assigned_attachment->target_stat);
        systemAPI_->Render().DrawTextDefault(
            ("対象: " + targetLabel).c_str(), abs_x + 10.0f, abs_y + 104.0f, hi::FONT_BODY, ToCoreColor(OverlayColors::TEXT_SECONDARY));
    }
}
} // namespace core
} // namespace game
