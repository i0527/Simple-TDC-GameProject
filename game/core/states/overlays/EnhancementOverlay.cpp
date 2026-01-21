#include "EnhancementOverlay.hpp"

// 標準ライブラリ
#include <algorithm>
#include <array>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

// プロジェクト内
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/GameplayDataAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../config/RenderPrimitives.hpp"
#include "../../ecs/entities/TowerAttachment.hpp"
#include "../../system/TowerEnhancementEffects.hpp"
#include "../../ui/OverlayColors.hpp"

namespace game {
namespace core {

namespace {

const char* ToAttachmentTargetLabel(entities::TowerAttachmentTargetStat stat) {
    switch (stat) {
    case entities::TowerAttachmentTargetStat::TowerHp:
        return "城HP";
    case entities::TowerAttachmentTargetStat::WalletGrowth:
        return "お金成長/秒";
    case entities::TowerAttachmentTargetStat::CostRegen:
        return "コスト回復/秒";
    case entities::TowerAttachmentTargetStat::AllyAttack:
        return "味方攻撃";
    case entities::TowerAttachmentTargetStat::AllyHp:
        return "味方HP";
    case entities::TowerAttachmentTargetStat::EnemyHp:
        return "敵HP";
    case entities::TowerAttachmentTargetStat::EnemyAttack:
        return "敵攻撃";
    case entities::TowerAttachmentTargetStat::EnemyMoveSpeed:
        return "敵移動速度";
    default:
        return "不明";
    }
}

std::string BuildAttachmentEffectText(const entities::TowerAttachment& attachment, int level) {
    const float raw = attachment.value_per_level * static_cast<float>(level);
    const float value = raw * 100.0f;
    std::ostringstream oss;
    if (value >= 0.0f) {
        oss << "+";
    }
    oss.setf(std::ios::fixed);
    oss.precision(1);
    oss << value;
    if (attachment.effect_type == entities::TowerAttachmentEffectType::Percentage) {
        oss << "%";
    }
    return oss.str();
}

std::string FormatFloat(float value, int precision) {
    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss.precision(precision);
    oss << value;
    return oss.str();
}

} // namespace

EnhancementOverlay::EnhancementOverlay()
    : systemAPI_(nullptr)
    , isInitialized_(false)
    , requestClose_(false)
    , hasTransitionRequest_(false)
    , requestedNextState_(GameState::Title)
{
}

bool EnhancementOverlay::Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) {
    if (isInitialized_) {
        LOG_ERROR("EnhancementOverlay already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("EnhancementOverlay: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    requestClose_ = false;
    hasTransitionRequest_ = false;

    isInitialized_ = true;
    LOG_INFO("EnhancementOverlay initialized");
    return true;
}

void EnhancementOverlay::Update(SharedContext& ctx, float deltaTime) {
    if (!isInitialized_) {
        return;
    }

    (void)deltaTime;

    if (ctx.inputAPI && ctx.inputAPI->IsEscapePressed()) {
        requestClose_ = true;
        return;
    }

    if (!ctx.gameplayDataAPI || !ctx.inputAPI) {
        return;
    }

    auto st = ctx.gameplayDataAPI->GetTowerEnhancements();
    auto attachments = ctx.gameplayDataAPI->GetTowerAttachments();
    const auto& masters = ctx.gameplayDataAPI->GetAllTowerAttachmentMasters();

    std::vector<const entities::TowerAttachment*> allAttachments;
    allAttachments.reserve(masters.size());
    for (const auto& [id, attachment] : masters) {
        (void)id;
        allAttachments.push_back(&attachment);
    }
    std::sort(allAttachments.begin(), allAttachments.end(),
              [](const entities::TowerAttachment* a, const entities::TowerAttachment* b) {
                  return a->name < b->name;
              });

    if (selectedSlotIndex_ < 0 || selectedSlotIndex_ >= 3) {
        selectedSlotIndex_ = 0;
    }
    if (selectedAttachmentId_.empty() && !allAttachments.empty()) {
        selectedAttachmentId_ = allAttachments.front()->id;
    }

    auto findAttachment = [&](const std::string& id) -> const entities::TowerAttachment* {
        auto it = masters.find(id);
        if (it != masters.end()) {
            return &it->second;
        }
        return nullptr;
    };

    struct Layout {
        Rect window;
        Rect content;
        Rect left;
        Rect right;
        Rect leftInner;
        Rect rightInner;
        Rect slotCard;
        Rect detailCard;
        Rect listCard;
        Rect listInner;
    };

    auto computeLayout = []() -> Layout {
        Layout layout{};
        constexpr float marginLeft = 20.0f;
        constexpr float marginRight = 20.0f;
        constexpr float marginTop = 90.0f;
        constexpr float marginBottom = 90.0f;
        constexpr float screenW = 1920.0f;
        constexpr float screenH = 1080.0f;
        const float contentWidth = screenW - marginLeft - marginRight;
        const float contentHeight = screenH - marginTop - marginBottom;
        layout.window = {marginLeft, marginTop, contentWidth, contentHeight};

        constexpr float headerHeight = 54.0f;
        constexpr float contentPadding = 20.0f;
        layout.content = {
            layout.window.x + contentPadding,
            layout.window.y + headerHeight,
            layout.window.width - contentPadding * 2.0f,
            layout.window.height - headerHeight - contentPadding
        };

        constexpr float panelGap = 16.0f;
        const float leftWidth = layout.content.width * 0.55f;
        const float rightWidth = layout.content.width - leftWidth - panelGap;
        layout.left = {layout.content.x, layout.content.y, leftWidth, layout.content.height};
        layout.right = {
            layout.content.x + leftWidth + panelGap,
            layout.content.y,
            rightWidth,
            layout.content.height
        };

        constexpr float panelPadding = 16.0f;
        layout.leftInner = {
            layout.left.x + panelPadding,
            layout.left.y + panelPadding,
            layout.left.width - panelPadding * 2.0f,
            layout.left.height - panelPadding * 2.0f
        };
        layout.rightInner = {
            layout.right.x + panelPadding,
            layout.right.y + panelPadding,
            layout.right.width - panelPadding * 2.0f,
            layout.right.height - panelPadding * 2.0f
        };

        const float slotHeight = 260.0f;
        const float detailHeight = 320.0f;
        const float listHeight =
            layout.rightInner.height - slotHeight - detailHeight - 24.0f;

        layout.slotCard = {
            layout.rightInner.x,
            layout.rightInner.y,
            layout.rightInner.width,
            slotHeight
        };
        layout.detailCard = {
            layout.rightInner.x,
            layout.slotCard.y + layout.slotCard.height + 12.0f,
            layout.rightInner.width,
            detailHeight
        };
        layout.listCard = {
            layout.rightInner.x,
            layout.detailCard.y + layout.detailCard.height + 12.0f,
            layout.rightInner.width,
            std::max(140.0f, listHeight)
        };
        layout.listInner = {
            layout.listCard.x + 8.0f,
            layout.listCard.y + 56.0f,
            layout.listCard.width - 16.0f,
            std::max(120.0f, layout.listCard.height - 64.0f)
        };
        return layout;
    };

    const Layout layout = computeLayout();
    const Vec2 mouse = ctx.inputAPI->GetMousePosition();
    auto inRect = [&](const Rect& r) {
        return mouse.x >= r.x && mouse.x <= r.x + r.width &&
               mouse.y >= r.y && mouse.y <= r.y + r.height;
    };

    // スクロール（アタッチメント一覧）
    if (inRect(layout.listInner)) {
        const float wheel = ctx.inputAPI->GetMouseWheelMove();
        if (wheel != 0.0f) {
            const float itemHeight = 72.0f;
            const int visibleCount = std::max(1, static_cast<int>(layout.listInner.height / itemHeight));
            const int maxOffset = std::max(0, static_cast<int>(allAttachments.size()) - visibleCount);
            attachmentListScroll_ = std::clamp(attachmentListScroll_ - wheel, 0.0f, static_cast<float>(maxOffset));
        }
    }

    if (!ctx.inputAPI->IsLeftClickPressed()) {
        return;
    }

    // スロット選択
    const float slotRowH = 64.0f;
    const float slotStartY = layout.slotCard.y + 64.0f;
    for (int i = 0; i < 3; ++i) {
        Rect row{layout.slotCard.x + 8.0f, slotStartY + slotRowH * i,
                 layout.slotCard.width - 16.0f, slotRowH};
        if (inRect(row)) {
            selectedSlotIndex_ = i;
            const auto* slotAttachment = findAttachment(attachments[i].id);
            if (slotAttachment) {
                selectedAttachmentId_ = slotAttachment->id;
            }
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
    }

    // 左パネル: 基礎強化ボタン
    constexpr int MAX_LEVEL = 50;
    struct BaseRow {
        const char* name;
        int* levelRef;
        float perLevel;
    };
    std::array<BaseRow, 5> rows = {{
        {"城HP最大値", &st.towerHpLevel, 0.05f},
        {"お金成長/秒", &st.walletGrowthLevel, 0.05f},
        {"コスト回復/秒", &st.costRegenLevel, 0.05f},
        {"味方攻撃", &st.allyAttackLevel, 0.02f},
        {"味方HP", &st.allyHpLevel, 0.02f}
    }};

    const float tableY = layout.leftInner.y + 110.0f;
    const float rowHeight = 88.0f;
    const float baseButtonAreaW = 270.0f;
    const float baseButtonGapX = 8.0f;
    const float baseButtonGapY = 6.0f;
    const float baseButtonH = 36.0f;
    const float baseButtonW = (baseButtonAreaW - baseButtonGapX * 2.0f) / 3.0f;
    const float baseButtonAreaX = layout.leftInner.x + layout.leftInner.width - baseButtonAreaW;
    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
        const float rowY = tableY + rowHeight * i;
        const float rowTop = rowY + 6.0f;
        const float rowBottom = rowTop + baseButtonH + baseButtonGapY;
        Rect down1Rect{baseButtonAreaX, rowTop, baseButtonW, baseButtonH};
        Rect up1Rect{baseButtonAreaX + baseButtonW + baseButtonGapX, rowTop, baseButtonW, baseButtonH};
        Rect downMaxRect{baseButtonAreaX + (baseButtonW + baseButtonGapX) * 2.0f, rowTop, baseButtonW, baseButtonH};
        Rect down5Rect{baseButtonAreaX, rowBottom, baseButtonW, baseButtonH};
        Rect up5Rect{baseButtonAreaX + baseButtonW + baseButtonGapX, rowBottom, baseButtonW, baseButtonH};
        Rect upMaxRect{baseButtonAreaX + (baseButtonW + baseButtonGapX) * 2.0f, rowBottom, baseButtonW, baseButtonH};

        if (inRect(down1Rect)) {
            OnBaseEnhancementDown(ctx, i);
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
        if (inRect(up1Rect)) {
            OnBaseEnhancementUp(ctx, i);
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
        if (inRect(down5Rect)) {
            OnBaseEnhancementDownBatch(ctx, i, 5);
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
        if (inRect(up5Rect)) {
            OnBaseEnhancementUpBatch(ctx, i, 5);
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
        if (inRect(downMaxRect)) {
            OnBaseEnhancementDownMax(ctx, i);
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
        if (inRect(upMaxRect)) {
            OnBaseEnhancementUpMax(ctx, i);
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
    }

    // 右パネル: スロット操作
    const auto* slotAttachment = findAttachment(attachments[selectedSlotIndex_].id);
    if (slotAttachment) {
        const float actionPadding = 12.0f;
        const float actionGap = 8.0f;
        const float actionButtonH = 40.0f;
        const float levelButtonAreaH = actionButtonH * 2.0f + actionGap;
        const float actionAreaH = levelButtonAreaH + actionButtonH + actionGap;
        const float actionStartY = layout.detailCard.y + layout.detailCard.height - actionAreaH - 12.0f;
        const float levelButtonW = (layout.detailCard.width - actionPadding * 2.0f - actionGap * 2.0f) / 3.0f;
        const float levelButtonX = layout.detailCard.x + actionPadding;
        const float levelRowTop = actionStartY;
        const float levelRowBottom = levelRowTop + actionButtonH + actionGap;

        Rect down1Rect{levelButtonX, levelRowTop, levelButtonW, actionButtonH};
        Rect up1Rect{levelButtonX + levelButtonW + actionGap, levelRowTop, levelButtonW, actionButtonH};
        Rect downMaxRect{levelButtonX + (levelButtonW + actionGap) * 2.0f, levelRowTop, levelButtonW, actionButtonH};
        Rect down5Rect{levelButtonX, levelRowBottom, levelButtonW, actionButtonH};
        Rect up5Rect{levelButtonX + levelButtonW + actionGap, levelRowBottom, levelButtonW, actionButtonH};
        Rect upMaxRect{levelButtonX + (levelButtonW + actionGap) * 2.0f, levelRowBottom, levelButtonW, actionButtonH};

        if (inRect(down1Rect)) {
            OnAttachmentLevelDown(ctx, selectedSlotIndex_);
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
        if (inRect(up1Rect)) {
            OnAttachmentLevelUp(ctx, selectedSlotIndex_);
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
        if (inRect(down5Rect)) {
            OnAttachmentLevelDownBatch(ctx, selectedSlotIndex_, 5);
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
        if (inRect(up5Rect)) {
            OnAttachmentLevelUpBatch(ctx, selectedSlotIndex_, 5);
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
        if (inRect(downMaxRect)) {
            OnAttachmentLevelDownMax(ctx, selectedSlotIndex_);
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
        if (inRect(upMaxRect)) {
            OnAttachmentLevelUpMax(ctx, selectedSlotIndex_);
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }

        Rect removeBtn{layout.detailCard.x + actionPadding,
                       actionStartY + levelButtonAreaH + actionGap,
                       (layout.detailCard.width - actionPadding * 2.0f - actionGap) / 2.0f,
                       actionButtonH};
        if (inRect(removeBtn)) {
            attachments[selectedSlotIndex_].id.clear();
            attachments[selectedSlotIndex_].level = 1;
            ctx.gameplayDataAPI->SetTowerAttachments(attachments);
            ctx.gameplayDataAPI->Save();
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
    }

    // アタッチメント一覧: 選択
    const float itemHeight = 72.0f;
    const int startIndex = std::max(0, static_cast<int>(attachmentListScroll_));
    const int visibleCount = std::max(1, static_cast<int>(layout.listInner.height / itemHeight));
    for (int i = 0; i < visibleCount; ++i) {
        const int idx = startIndex + i;
        if (idx >= static_cast<int>(allAttachments.size())) break;
        Rect itemRect{
            layout.listInner.x,
            layout.listInner.y + itemHeight * i,
            layout.listInner.width,
            itemHeight
        };
        if (inRect(itemRect)) {
            selectedAttachmentId_ = allAttachments[idx]->id;
            ctx.inputAPI->ConsumeLeftClick();
            return;
        }
    }

    // 装着ボタン
    const float attachButtonH = 40.0f;
    const float attachButtonGap = 8.0f;
    const float attachPadding = 12.0f;
    const float attachActionAreaH = attachButtonH * 3.0f + attachButtonGap * 2.0f;
    const float attachRowY = layout.detailCard.y + layout.detailCard.height - attachActionAreaH - 12.0f
                             + (attachButtonH * 2.0f + attachButtonGap * 2.0f);
    Rect attachBtn{
        layout.detailCard.x + attachPadding +
            (layout.detailCard.width - attachPadding * 2.0f - attachButtonGap) / 2.0f + attachButtonGap,
        attachRowY,
        (layout.detailCard.width - attachPadding * 2.0f - attachButtonGap) / 2.0f,
        attachButtonH
    };
    if (inRect(attachBtn) && !selectedAttachmentId_.empty()) {
        attachments[selectedSlotIndex_].id = selectedAttachmentId_;
        attachments[selectedSlotIndex_].level = 1;
        ctx.gameplayDataAPI->SetTowerAttachments(attachments);
        ctx.gameplayDataAPI->Save();
        ctx.inputAPI->ConsumeLeftClick();
        return;
    }

    ctx.inputAPI->ConsumeLeftClick();
}

void EnhancementOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) {
        return;
    }

    if (!ctx.gameplayDataAPI || !systemAPI_) {
        return;
    }

    auto st = ctx.gameplayDataAPI->GetTowerEnhancements();
    auto attachments = ctx.gameplayDataAPI->GetTowerAttachments();
    const auto& masters = ctx.gameplayDataAPI->GetAllTowerAttachmentMasters();

    std::vector<const entities::TowerAttachment*> allAttachments;
    allAttachments.reserve(masters.size());
    for (const auto& [id, attachment] : masters) {
        (void)id;
        allAttachments.push_back(&attachment);
    }
    std::sort(allAttachments.begin(), allAttachments.end(),
              [](const entities::TowerAttachment* a, const entities::TowerAttachment* b) {
                  return a->name < b->name;
              });

    if (selectedSlotIndex_ < 0 || selectedSlotIndex_ >= 3) {
        selectedSlotIndex_ = 0;
    }
    if (selectedAttachmentId_.empty() && !allAttachments.empty()) {
        selectedAttachmentId_ = allAttachments.front()->id;
    }

    auto findAttachment = [&](const std::string& id) -> const entities::TowerAttachment* {
        auto it = masters.find(id);
        if (it != masters.end()) {
            return &it->second;
        }
        return nullptr;
    };

    struct Layout {
        Rect window;
        Rect content;
        Rect left;
        Rect right;
        Rect leftInner;
        Rect rightInner;
        Rect slotArea;
        Rect detailArea;
        Rect listArea;
    };

    auto computeLayout = []() -> Layout {
        Layout layout{};
        constexpr float marginLeft = 20.0f;
        constexpr float marginRight = 20.0f;
        constexpr float marginTop = 90.0f;
        constexpr float marginBottom = 90.0f;
        constexpr float screenW = 1920.0f;
        constexpr float screenH = 1080.0f;
        const float contentWidth = screenW - marginLeft - marginRight;
        const float contentHeight = screenH - marginTop - marginBottom;
        layout.window = {marginLeft, marginTop, contentWidth, contentHeight};

        constexpr float headerHeight = 54.0f;
        constexpr float contentPadding = 20.0f;
        layout.content = {
            layout.window.x + contentPadding,
            layout.window.y + headerHeight,
            layout.window.width - contentPadding * 2.0f,
            layout.window.height - headerHeight - contentPadding
        };

        constexpr float panelGap = 16.0f;
        const float leftWidth = layout.content.width * 0.55f;
        const float rightWidth = layout.content.width - leftWidth - panelGap;
        layout.left = {layout.content.x, layout.content.y, leftWidth, layout.content.height};
        layout.right = {
            layout.content.x + leftWidth + panelGap,
            layout.content.y,
            rightWidth,
            layout.content.height
        };

        constexpr float panelPadding = 16.0f;
        layout.leftInner = {
            layout.left.x + panelPadding,
            layout.left.y + panelPadding,
            layout.left.width - panelPadding * 2.0f,
            layout.left.height - panelPadding * 2.0f
        };
        layout.rightInner = {
            layout.right.x + panelPadding,
            layout.right.y + panelPadding,
            layout.right.width - panelPadding * 2.0f,
            layout.right.height - panelPadding * 2.0f
        };

        constexpr float slotAreaHeight = 140.0f;
        constexpr float listAreaHeight = 320.0f;
        const float remainingHeight =
            layout.rightInner.height - slotAreaHeight - listAreaHeight - 16.0f;
        const float detailHeight = std::max(140.0f, remainingHeight);

        layout.slotArea = {
            layout.rightInner.x,
            layout.rightInner.y,
            layout.rightInner.width,
            slotAreaHeight
        };
        layout.detailArea = {
            layout.rightInner.x,
            layout.slotArea.y + layout.slotArea.height + 8.0f,
            layout.rightInner.width,
            detailHeight
        };
        layout.listArea = {
            layout.rightInner.x,
            layout.detailArea.y + layout.detailArea.height + 8.0f,
            layout.rightInner.width,
            listAreaHeight
        };
        if (layout.listArea.y + layout.listArea.height >
            layout.rightInner.y + layout.rightInner.height) {
            layout.listArea.height =
                std::max(120.0f, layout.rightInner.y + layout.rightInner.height - layout.listArea.y);
        }
        return layout;
    };

    const Layout layout = computeLayout();
    const Vec2 mouse = ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};
    auto inRect = [&](const Rect& r) {
        return mouse.x >= r.x && mouse.x <= r.x + r.width &&
               mouse.y >= r.y && mouse.y <= r.y + r.height;
    };

    auto& render = systemAPI_->Render();
    render.DrawRectangle(layout.window.x, layout.window.y, layout.window.width, layout.window.height,
                         ui::OverlayColors::PANEL_BG_SECONDARY);
    render.DrawRectangleLines(layout.window.x, layout.window.y, layout.window.width, layout.window.height,
                              2.0f, ui::OverlayColors::BORDER_DEFAULT);

    render.DrawTextDefault("タワー強化", layout.window.x + 24.0f, layout.window.y + 12.0f,
                           68.0f, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    render.DrawRectangle(layout.left.x, layout.left.y, layout.left.width, layout.left.height,
                         ui::OverlayColors::PANEL_BG_PRIMARY);
    render.DrawRectangleLines(layout.left.x, layout.left.y, layout.left.width, layout.left.height,
                              2.0f, ui::OverlayColors::BORDER_DEFAULT);
    render.DrawRectangle(layout.right.x, layout.right.y, layout.right.width, layout.right.height,
                         ui::OverlayColors::PANEL_BG_PRIMARY);
    render.DrawRectangleLines(layout.right.x, layout.right.y, layout.right.width, layout.right.height,
                              2.0f, ui::OverlayColors::BORDER_DEFAULT);

    // 左: 基礎強化
    auto drawCard = [&](const Rect& rect, const char* title) {
        render.DrawRectangle(rect.x, rect.y, rect.width, rect.height,
                             ui::OverlayColors::CARD_BG_NORMAL);
        render.DrawRectangleLines(rect.x, rect.y, rect.width, rect.height,
                                  2.0f, ui::OverlayColors::BORDER_DEFAULT);
        render.DrawRectangle(rect.x, rect.y, rect.width, 56.0f,
                             ui::OverlayColors::CARD_BG_SELECTED);
        render.DrawTextDefault(title, rect.x + 12.0f, rect.y + 10.0f, 36.0f,
                               ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
    };

    const Rect baseCard{
        layout.leftInner.x,
        layout.leftInner.y,
        layout.leftInner.width,
        layout.leftInner.height * 0.70f
    };
    const Rect summaryCard{
        layout.leftInner.x,
        baseCard.y + baseCard.height + 12.0f,
        layout.leftInner.width,
        layout.leftInner.height - baseCard.height - 12.0f
    };

    drawCard(baseCard, "基礎強化");
    render.DrawTextDefault("※ v1は無料で強化できます（通貨消費なし）",
                           baseCard.x + 12.0f, baseCard.y + 62.0f,
                           28.0f, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));

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

    const float tableY = baseCard.y + 98.0f;
    const float rowHeight = 78.0f;
    const float colNameX = layout.leftInner.x;
    const float colLvX = layout.leftInner.x + 220.0f;
    const float colCurX = layout.leftInner.x + 330.0f;
    const float colNextX = layout.leftInner.x + 440.0f;
    render.DrawTextDefault("項目", colNameX, tableY - 34.0f, 28.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_MUTED));
    render.DrawTextDefault("現在", colLvX, tableY - 34.0f, 28.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_MUTED));
    render.DrawTextDefault("効果", colCurX, tableY - 34.0f, 28.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_MUTED));
    render.DrawTextDefault("次", colNextX, tableY - 34.0f, 28.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_MUTED));

    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
        const float rowY = tableY + rowHeight * i;
        const int level = std::max(0, std::min(MAX_LEVEL, rows[i].level));
        const float cur = rows[i].perLevel * static_cast<float>(level) * 100.0f;
        const float next = rows[i].perLevel * static_cast<float>(std::min(MAX_LEVEL, level + 1)) * 100.0f;

        render.DrawTextDefault(rows[i].name, colNameX, rowY + 10.0f, 30.0f,
                               ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        render.DrawTextDefault(("Lv " + std::to_string(level)).c_str(), colLvX, rowY + 10.0f,
                               30.0f, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
        render.DrawTextDefault(("+" + FormatFloat(cur, 1) + "%").c_str(), colCurX, rowY + 10.0f,
                               30.0f, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
        render.DrawTextDefault(("-> +" + FormatFloat(next, 1) + "%").c_str(), colNextX, rowY + 10.0f,
                               30.0f, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));

        const float baseButtonAreaW = 240.0f;
        const float baseButtonGapX = 8.0f;
        const float baseButtonGapY = 4.0f;
        const float baseButtonH = 32.0f;
        const float baseButtonW = (baseButtonAreaW - baseButtonGapX * 2.0f) / 3.0f;
        const float baseButtonAreaX = layout.leftInner.x + layout.leftInner.width - baseButtonAreaW;
        const float rowTop = rowY + 6.0f;
        const float rowBottom = rowTop + baseButtonH + baseButtonGapY;

        Rect down1Rect{baseButtonAreaX, rowTop, baseButtonW, baseButtonH};
        Rect up1Rect{baseButtonAreaX + baseButtonW + baseButtonGapX, rowTop, baseButtonW, baseButtonH};
        Rect downMaxRect{baseButtonAreaX + (baseButtonW + baseButtonGapX) * 2.0f, rowTop, baseButtonW, baseButtonH};
        Rect down5Rect{baseButtonAreaX, rowBottom, baseButtonW, baseButtonH};
        Rect up5Rect{baseButtonAreaX + baseButtonW + baseButtonGapX, rowBottom, baseButtonW, baseButtonH};
        Rect upMaxRect{baseButtonAreaX + (baseButtonW + baseButtonGapX) * 2.0f, rowBottom, baseButtonW, baseButtonH};

        auto drawBaseButton = [&](const Rect& rect, const char* label, bool hovered, bool isPositive) {
            const Color btnColor = hovered
                                       ? (isPositive ? ui::OverlayColors::BUTTON_PRIMARY_HOVER
                                                     : ui::OverlayColors::BUTTON_RESET_HOVER)
                                       : (isPositive ? ui::OverlayColors::BUTTON_PRIMARY
                                                     : ui::OverlayColors::BUTTON_RESET);
            render.DrawRectangle(rect.x, rect.y, rect.width, rect.height, btnColor);
            render.DrawRectangleLines(rect.x, rect.y, rect.width, rect.height,
                                      2.0f, ui::OverlayColors::BORDER_DEFAULT);
            render.DrawTextDefault(label, rect.x + 8.0f, rect.y + 2.0f, 28.0f,
                                   ToCoreColor(ui::OverlayColors::TEXT_DARK));
        };

        drawBaseButton(down1Rect, "-1", inRect(down1Rect), false);
        drawBaseButton(up1Rect, "+1", inRect(up1Rect), true);
        drawBaseButton(downMaxRect, "一括-", inRect(downMaxRect), false);
        drawBaseButton(down5Rect, "-5", inRect(down5Rect), false);
        drawBaseButton(up5Rect, "+5", inRect(up5Rect), true);
        drawBaseButton(upMaxRect, "一括+", inRect(upMaxRect), true);
    }

    const auto mul = system::CalculateTowerEnhancementMultipliers(st, attachments, masters);
    drawCard(summaryCard, "現在倍率");
    const float summaryY = summaryCard.y + 70.0f;
    render.DrawTextDefault("現在倍率", summaryCard.x + 12.0f, summaryCard.y + 10.0f, 36.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
    render.DrawTextDefault(("城HP x" + FormatFloat(mul.playerTowerHpMul, 2)).c_str(),
                           summaryCard.x + 12.0f, summaryY + 18.0f, 36.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    render.DrawTextDefault(("お金成長 x" + FormatFloat(mul.walletGrowthMul, 2)).c_str(),
                           summaryCard.x + 260.0f, summaryY + 18.0f, 36.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    render.DrawTextDefault(("コスト回復 x" + FormatFloat(mul.costRegenMul, 2)).c_str(),
                           summaryCard.x + 520.0f, summaryY + 18.0f, 36.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    render.DrawTextDefault(("味方ATK x" + FormatFloat(mul.allyAttackMul, 2)).c_str(),
                           summaryCard.x + 12.0f, summaryY + 60.0f, 36.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    render.DrawTextDefault(("味方HP x" + FormatFloat(mul.allyHpMul, 2)).c_str(),
                           summaryCard.x + 260.0f, summaryY + 60.0f, 36.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    render.DrawTextDefault(("敵HP x" + FormatFloat(mul.enemyHpMul, 2)).c_str(),
                           summaryCard.x + 12.0f, summaryY + 102.0f, 36.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    render.DrawTextDefault(("敵ATK x" + FormatFloat(mul.enemyAttackMul, 2)).c_str(),
                           summaryCard.x + 260.0f, summaryY + 102.0f, 36.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    render.DrawTextDefault(("敵速度 x" + FormatFloat(mul.enemyMoveSpeedMul, 2)).c_str(),
                           summaryCard.x + 520.0f, summaryY + 102.0f, 36.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));

    // 右: アタッチメント
    const Rect slotCard{
        layout.rightInner.x,
        layout.rightInner.y,
        layout.rightInner.width,
        260.0f
    };
    const Rect detailCard{
        layout.rightInner.x,
        slotCard.y + slotCard.height + 12.0f,
        layout.rightInner.width,
        320.0f
    };
    const Rect listCard{
        layout.rightInner.x,
        detailCard.y + detailCard.height + 12.0f,
        layout.rightInner.width,
        layout.rightInner.height - slotCard.height - detailCard.height - 24.0f
    };

    drawCard(slotCard, "スロット");

    const float slotRowH = 64.0f;
    const float slotStartY = slotCard.y + 64.0f;
    for (int i = 0; i < 3; ++i) {
        const auto* slotAttachment = findAttachment(attachments[i].id);
        Rect row{slotCard.x + 8.0f, slotStartY + slotRowH * i,
                 slotCard.width - 16.0f, slotRowH};
        if (selectedSlotIndex_ == i) {
            render.DrawRectangle(row.x, row.y, row.width, row.height,
                                 ui::OverlayColors::CARD_BG_SELECTED);
        }
        std::string label = "スロット " + std::to_string(i + 1) + ": " +
                            (slotAttachment ? slotAttachment->name : "空");
        render.DrawTextDefault(label.c_str(), row.x + 8.0f, row.y + 14.0f, 36.0f,
                               ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
        if (slotAttachment) {
            render.DrawTextDefault(("Lv " + std::to_string(attachments[i].level)).c_str(),
                                   row.x + row.width - 140.0f, row.y + 14.0f, 36.0f,
                                   ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
        }
    }

    // 右: スロット詳細
    const auto* slotAttachment = findAttachment(attachments[selectedSlotIndex_].id);
    drawCard(detailCard, "スロット詳細");
    if (slotAttachment) {
        render.DrawTextDefault(("装着: " + slotAttachment->name).c_str(),
                               detailCard.x + 12.0f, detailCard.y + 52.0f, 36.0f,
                               ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
        render.DrawTextDefault(
            ("効果: " + std::string(ToAttachmentTargetLabel(slotAttachment->target_stat)) + " " +
             BuildAttachmentEffectText(*slotAttachment, attachments[selectedSlotIndex_].level)).c_str(),
            detailCard.x + 12.0f, detailCard.y + 96.0f, 36.0f,
            ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
        render.DrawTextDefault(
            ("Lv " + std::to_string(attachments[selectedSlotIndex_].level) + " / " +
             std::to_string(slotAttachment->max_level)).c_str(),
            detailCard.x + 12.0f, detailCard.y + 140.0f, 36.0f,
            ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    } else {
        render.DrawTextDefault("装着: 空",
                               detailCard.x + 12.0f, detailCard.y + 52.0f, 36.0f,
                               ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    }

    const float actionPadding = 12.0f;
    const float actionGap = 8.0f;
    const float actionButtonH = 40.0f;
    const float levelButtonAreaH = actionButtonH * 2.0f + actionGap;
    const float actionAreaH = levelButtonAreaH + actionButtonH + actionGap;
    const float actionStartY = detailCard.y + detailCard.height - actionAreaH - 12.0f;
    const float levelButtonW = (detailCard.width - actionPadding * 2.0f - actionGap * 2.0f) / 3.0f;
    const float levelButtonX = detailCard.x + actionPadding;
    const float levelRowTop = actionStartY;
    const float levelRowBottom = levelRowTop + actionButtonH + actionGap;

    Rect down1Rect{levelButtonX, levelRowTop, levelButtonW, actionButtonH};
    Rect up1Rect{levelButtonX + levelButtonW + actionGap, levelRowTop, levelButtonW, actionButtonH};
    Rect downMaxRect{levelButtonX + (levelButtonW + actionGap) * 2.0f, levelRowTop, levelButtonW, actionButtonH};
    Rect down5Rect{levelButtonX, levelRowBottom, levelButtonW, actionButtonH};
    Rect up5Rect{levelButtonX + levelButtonW + actionGap, levelRowBottom, levelButtonW, actionButtonH};
    Rect upMaxRect{levelButtonX + (levelButtonW + actionGap) * 2.0f, levelRowBottom, levelButtonW, actionButtonH};

    Rect removeBtn{detailCard.x + actionPadding,
                   actionStartY + levelButtonAreaH + actionGap,
                   (detailCard.width - actionPadding * 2.0f - actionGap) / 2.0f,
                   actionButtonH};
    Rect attachBtn{detailCard.x + actionPadding +
                       (detailCard.width - actionPadding * 2.0f - actionGap) / 2.0f + actionGap,
                   actionStartY + levelButtonAreaH + actionGap,
                   (detailCard.width - actionPadding * 2.0f - actionGap) / 2.0f,
                   actionButtonH};

    auto drawLevelButton = [&](const Rect& rect, const char* label, bool hovered, bool isPositive) {
        const Color btnColor = hovered
                                   ? (isPositive ? ui::OverlayColors::BUTTON_PRIMARY_HOVER
                                                 : ui::OverlayColors::BUTTON_RESET_HOVER)
                                   : (isPositive ? ui::OverlayColors::BUTTON_PRIMARY
                                                 : ui::OverlayColors::BUTTON_RESET);
        render.DrawRectangle(rect.x, rect.y, rect.width, rect.height, btnColor);
        render.DrawRectangleLines(rect.x, rect.y, rect.width, rect.height,
                                  2.0f, ui::OverlayColors::BORDER_DEFAULT);
        render.DrawTextDefault(label, rect.x + 10.0f, rect.y + 2.0f, 32.0f,
                               ToCoreColor(ui::OverlayColors::TEXT_DARK));
    };

    drawLevelButton(down1Rect, "-1", inRect(down1Rect), false);
    drawLevelButton(up1Rect, "+1", inRect(up1Rect), true);
    drawLevelButton(downMaxRect, "一括-", inRect(downMaxRect), false);
    drawLevelButton(down5Rect, "-5", inRect(down5Rect), false);
    drawLevelButton(up5Rect, "+5", inRect(up5Rect), true);
    drawLevelButton(upMaxRect, "一括+", inRect(upMaxRect), true);

    const Color removeColor = inRect(removeBtn) ? ui::OverlayColors::BUTTON_RESET_HOVER
                                                : ui::OverlayColors::BUTTON_RESET;
    const Color attachColor = inRect(attachBtn) ? ui::OverlayColors::BUTTON_BLUE_HOVER
                                                : ui::OverlayColors::BUTTON_BLUE;

    render.DrawRectangle(removeBtn.x, removeBtn.y, removeBtn.width, removeBtn.height, removeColor);
    render.DrawRectangleLines(removeBtn.x, removeBtn.y, removeBtn.width, removeBtn.height,
                              2.0f, ui::OverlayColors::BORDER_DEFAULT);
    render.DrawTextDefault("解除", removeBtn.x + 32.0f, removeBtn.y + 4.0f, 32.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));

    render.DrawRectangle(attachBtn.x, attachBtn.y, attachBtn.width, attachBtn.height, attachColor);
    render.DrawRectangleLines(attachBtn.x, attachBtn.y, attachBtn.width, attachBtn.height,
                              2.0f, ui::OverlayColors::BORDER_DEFAULT);
    render.DrawTextDefault("装着", attachBtn.x + 32.0f, attachBtn.y + 4.0f, 32.0f,
                           ToCoreColor(ui::OverlayColors::TEXT_DARK));

    // 右: アタッチメント一覧
    drawCard(listCard, "アタッチメント一覧");
    Rect listInner{
        listCard.x + 8.0f,
        listCard.y + 56.0f,
        listCard.width - 16.0f,
        listCard.height - 64.0f
    };
    render.DrawRectangle(listInner.x, listInner.y, listInner.width,
                         listInner.height, ui::OverlayColors::PANEL_BG_SECONDARY);
    render.DrawRectangleLines(listInner.x, listInner.y, listInner.width,
                              listInner.height, 2.0f, ui::OverlayColors::BORDER_DEFAULT);

    const float itemHeight = 72.0f;
    const int startIndex = std::max(0, static_cast<int>(attachmentListScroll_));
    const int visibleCount = std::max(1, static_cast<int>(listInner.height / itemHeight));
    for (int i = 0; i < visibleCount; ++i) {
        const int idx = startIndex + i;
        if (idx >= static_cast<int>(allAttachments.size())) break;
        const auto* attachment = allAttachments[idx];
        Rect itemRect{
            listInner.x,
            listInner.y + itemHeight * i,
            listInner.width,
            itemHeight
        };
        if (attachment && selectedAttachmentId_ == attachment->id) {
            render.DrawRectangle(itemRect.x, itemRect.y, itemRect.width, itemRect.height,
                                 ui::OverlayColors::CARD_BG_SELECTED);
        } else if (inRect(itemRect)) {
            render.DrawRectangle(itemRect.x, itemRect.y, itemRect.width, itemRect.height,
                                 ui::OverlayColors::CARD_BG_NORMAL);
        }
        if (attachment) {
            render.DrawTextDefault(attachment->name.c_str(), itemRect.x + 8.0f, itemRect.y + 16.0f,
                                   36.0f, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
            render.DrawTextDefault(ToAttachmentTargetLabel(attachment->target_stat),
                                   itemRect.x + itemRect.width - 260.0f, itemRect.y + 16.0f,
                                   36.0f, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
        }
    }

    const auto* selectedAttachment = findAttachment(selectedAttachmentId_);
    if (selectedAttachment) {
        render.DrawTextDefault(("選択: " + selectedAttachment->name).c_str(),
                               listCard.x + 12.0f, listCard.y + listCard.height - 48.0f,
                               36.0f, ToCoreColor(ui::OverlayColors::TEXT_SECONDARY));
    }
}

void EnhancementOverlay::OnBaseEnhancementUp(SharedContext& ctx, int rowIndex) {
    OnBaseEnhancementUpBatch(ctx, rowIndex, 1);
}

void EnhancementOverlay::OnBaseEnhancementDown(SharedContext& ctx, int rowIndex) {
    OnBaseEnhancementDownBatch(ctx, rowIndex, 1);
}

void EnhancementOverlay::OnBaseEnhancementUpBatch(SharedContext& ctx, int rowIndex, int levels) {
    if (!ctx.gameplayDataAPI || levels <= 0) {
        return;
    }

    auto st = ctx.gameplayDataAPI->GetTowerEnhancements();
    int* levelRef = nullptr;
    switch (rowIndex) {
    case 0: levelRef = &st.towerHpLevel; break;
    case 1: levelRef = &st.walletGrowthLevel; break;
    case 2: levelRef = &st.costRegenLevel; break;
    case 3: levelRef = &st.allyAttackLevel; break;
    case 4: levelRef = &st.allyHpLevel; break;
    default: return;
    }

    constexpr int MAX_LEVEL = 50;
    const int nextLevel = std::min(MAX_LEVEL, *levelRef + levels);
    if (nextLevel == *levelRef) {
        return;
    }
    *levelRef = nextLevel;
    ctx.gameplayDataAPI->SetTowerEnhancements(st);
    ctx.gameplayDataAPI->Save();
}

void EnhancementOverlay::OnBaseEnhancementDownBatch(SharedContext& ctx, int rowIndex, int levels) {
    if (!ctx.gameplayDataAPI || levels <= 0) {
        return;
    }

    auto st = ctx.gameplayDataAPI->GetTowerEnhancements();
    int* levelRef = nullptr;
    switch (rowIndex) {
    case 0: levelRef = &st.towerHpLevel; break;
    case 1: levelRef = &st.walletGrowthLevel; break;
    case 2: levelRef = &st.costRegenLevel; break;
    case 3: levelRef = &st.allyAttackLevel; break;
    case 4: levelRef = &st.allyHpLevel; break;
    default: return;
    }

    constexpr int MIN_LEVEL = 0;
    const int nextLevel = std::max(MIN_LEVEL, *levelRef - levels);
    if (nextLevel == *levelRef) {
        return;
    }
    *levelRef = nextLevel;
    ctx.gameplayDataAPI->SetTowerEnhancements(st);
    ctx.gameplayDataAPI->Save();
}

void EnhancementOverlay::OnBaseEnhancementUpMax(SharedContext& ctx, int rowIndex) {
    constexpr int MAX_LEVEL = 50;
    if (!ctx.gameplayDataAPI) {
        return;
    }

    auto st = ctx.gameplayDataAPI->GetTowerEnhancements();
    int* levelRef = nullptr;
    switch (rowIndex) {
    case 0: levelRef = &st.towerHpLevel; break;
    case 1: levelRef = &st.walletGrowthLevel; break;
    case 2: levelRef = &st.costRegenLevel; break;
    case 3: levelRef = &st.allyAttackLevel; break;
    case 4: levelRef = &st.allyHpLevel; break;
    default: return;
    }

    if (*levelRef >= MAX_LEVEL) {
        return;
    }
    *levelRef = MAX_LEVEL;
    ctx.gameplayDataAPI->SetTowerEnhancements(st);
    ctx.gameplayDataAPI->Save();
}

void EnhancementOverlay::OnBaseEnhancementDownMax(SharedContext& ctx, int rowIndex) {
    if (!ctx.gameplayDataAPI) {
        return;
    }

    auto st = ctx.gameplayDataAPI->GetTowerEnhancements();
    int* levelRef = nullptr;
    switch (rowIndex) {
    case 0: levelRef = &st.towerHpLevel; break;
    case 1: levelRef = &st.walletGrowthLevel; break;
    case 2: levelRef = &st.costRegenLevel; break;
    case 3: levelRef = &st.allyAttackLevel; break;
    case 4: levelRef = &st.allyHpLevel; break;
    default: return;
    }

    if (*levelRef <= 0) {
        return;
    }
    *levelRef = 0;
    ctx.gameplayDataAPI->SetTowerEnhancements(st);
    ctx.gameplayDataAPI->Save();
}

void EnhancementOverlay::OnAttachmentLevelUp(SharedContext& ctx, int slotIndex) {
    OnAttachmentLevelUpBatch(ctx, slotIndex, 1);
}

void EnhancementOverlay::OnAttachmentLevelDown(SharedContext& ctx, int slotIndex) {
    OnAttachmentLevelDownBatch(ctx, slotIndex, 1);
}

void EnhancementOverlay::OnAttachmentLevelUpBatch(SharedContext& ctx, int slotIndex, int levels) {
    if (!ctx.gameplayDataAPI || levels <= 0) {
        return;
    }

    auto attachments = ctx.gameplayDataAPI->GetTowerAttachments();
    if (slotIndex < 0 || slotIndex >= static_cast<int>(attachments.size())) {
        return;
    }

    const auto& masters = ctx.gameplayDataAPI->GetAllTowerAttachmentMasters();
    const auto& slot = attachments[slotIndex];
    if (slot.id.empty()) {
        return;
    }
    auto it = masters.find(slot.id);
    if (it == masters.end()) {
        return;
    }

    const int maxLevel = std::max(1, it->second.max_level);
    const int nextLevel = std::min(maxLevel, std::max(1, slot.level + levels));
    if (nextLevel == slot.level) {
        return;
    }
    attachments[slotIndex].level = nextLevel;
    ctx.gameplayDataAPI->SetTowerAttachments(attachments);
    ctx.gameplayDataAPI->Save();
}

void EnhancementOverlay::OnAttachmentLevelDownBatch(SharedContext& ctx, int slotIndex, int levels) {
    if (!ctx.gameplayDataAPI || levels <= 0) {
        return;
    }

    auto attachments = ctx.gameplayDataAPI->GetTowerAttachments();
    if (slotIndex < 0 || slotIndex >= static_cast<int>(attachments.size())) {
        return;
    }

    const auto& masters = ctx.gameplayDataAPI->GetAllTowerAttachmentMasters();
    const auto& slot = attachments[slotIndex];
    if (slot.id.empty()) {
        return;
    }
    auto it = masters.find(slot.id);
    if (it == masters.end()) {
        return;
    }

    const int maxLevel = std::max(1, it->second.max_level);
    const int nextLevel = std::max(1, std::min(maxLevel, slot.level - levels));
    if (nextLevel == slot.level) {
        return;
    }
    attachments[slotIndex].level = nextLevel;
    ctx.gameplayDataAPI->SetTowerAttachments(attachments);
    ctx.gameplayDataAPI->Save();
}

void EnhancementOverlay::OnAttachmentLevelUpMax(SharedContext& ctx, int slotIndex) {
    if (!ctx.gameplayDataAPI) {
        return;
    }

    auto attachments = ctx.gameplayDataAPI->GetTowerAttachments();
    if (slotIndex < 0 || slotIndex >= static_cast<int>(attachments.size())) {
        return;
    }

    const auto& masters = ctx.gameplayDataAPI->GetAllTowerAttachmentMasters();
    const auto& slot = attachments[slotIndex];
    if (slot.id.empty()) {
        return;
    }
    auto it = masters.find(slot.id);
    if (it == masters.end()) {
        return;
    }

    const int maxLevel = std::max(1, it->second.max_level);
    if (slot.level >= maxLevel) {
        return;
    }
    attachments[slotIndex].level = maxLevel;
    ctx.gameplayDataAPI->SetTowerAttachments(attachments);
    ctx.gameplayDataAPI->Save();
}

void EnhancementOverlay::OnAttachmentLevelDownMax(SharedContext& ctx, int slotIndex) {
    if (!ctx.gameplayDataAPI) {
        return;
    }

    auto attachments = ctx.gameplayDataAPI->GetTowerAttachments();
    if (slotIndex < 0 || slotIndex >= static_cast<int>(attachments.size())) {
        return;
    }

    const auto& masters = ctx.gameplayDataAPI->GetAllTowerAttachmentMasters();
    const auto& slot = attachments[slotIndex];
    if (slot.id.empty()) {
        return;
    }
    auto it = masters.find(slot.id);
    if (it == masters.end()) {
        return;
    }

    if (slot.level <= 1) {
        return;
    }
    attachments[slotIndex].level = 1;
    ctx.gameplayDataAPI->SetTowerAttachments(attachments);
    ctx.gameplayDataAPI->Save();
}

void EnhancementOverlay::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    isInitialized_ = false;
    systemAPI_ = nullptr;
    LOG_INFO("EnhancementOverlay shutdown");
}

bool EnhancementOverlay::RequestClose() const {
    if (requestClose_) {
        requestClose_ = false;
        return true;
    }
    return false;
}

bool EnhancementOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}

} // namespace core
} // namespace game
