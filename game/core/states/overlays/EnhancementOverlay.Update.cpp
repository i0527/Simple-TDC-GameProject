#include "EnhancementOverlay.hpp"
#include "EnhancementOverlayInternal.hpp"

#include <array>
#include <cmath>
#include <string>
#include <vector>

#include "../../api/GameplayDataAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../config/RenderTypes.hpp"
#include "../../ecs/entities/TowerAttachment.hpp"

namespace game {
namespace core {

namespace hi = enhancement_overlay_internal;

bool EnhancementOverlay::ProcessBaseEnhancementInput(SharedContext& ctx) {
    const int sel = item_list_panel_.selected_index;
    if (sel < 1 || sel > 5) return false;
    const int rowIndex = sel - 1;
    if (!ctx.inputAPI->IsLeftClickPressed()) {
        return false;
    }
    auto st = ctx.gameplayDataAPI->GetTowerEnhancements();
    const Vec2 mouse = ctx.inputAPI->GetMousePositionInternal();
    auto inRect = [&](const Rect& r) {
        return mouse.x >= r.x && mouse.x <= r.x + r.width &&
               mouse.y >= r.y && mouse.y <= r.y + r.height;
    };

    constexpr int MAX_LEVEL = 50;
    int* levelRef = nullptr;
    switch (rowIndex) {
    case 0: levelRef = &st.towerHpLevel; break;
    case 1: levelRef = &st.walletGrowthLevel; break;
    case 2: levelRef = &st.costRegenLevel; break;
    case 3: levelRef = &st.allyAttackLevel; break;
    case 4: levelRef = &st.allyHpLevel; break;
    default: return false;
    }

    const int currentLevel = levelRef ? *levelRef : 0;
    const bool canDecrease = currentLevel > 0;
    const bool canIncrease = currentLevel < MAX_LEVEL;
    const bool canDecrease5 = currentLevel >= 5;
    const bool canIncrease5 = currentLevel <= MAX_LEVEL - 5;

    const int cost1 = hi::ComputeTowerBaseLevelUpCost(currentLevel, 1);
    const int cost5 = hi::ComputeTowerBaseLevelUpCost(currentLevel, 5);
    const int costMax = hi::ComputeTowerBaseLevelUpCost(currentLevel, MAX_LEVEL - currentLevel);
    const int ownedGold = ctx.gameplayDataAPI->GetGold();
    const bool canAfford1 = ownedGold >= cost1;
    const bool canAfford5 = ownedGold >= cost5;
    const bool canAffordMax = ownedGold >= costMax;

    constexpr float PANEL_GAP = 10.0f;
    constexpr float COMPACT_HEIGHT = 220.0f;
    const float basePanelX = status_panel_.x;
    const float basePanelW = status_panel_.width;
    const float pad = 20.0f;
    const float tableY = status_panel_.y + COMPACT_HEIGHT + PANEL_GAP + hi::BASE_TABLE_TOP_OFFSET;
    const float rowHeight = hi::BASE_TABLE_ROW_HEIGHT;

    // 中央ボタンエリア: Render と同じ 2列×3行
    const float centerButtonW = (basePanelW - pad * 2.0f - hi::BASE_CENTER_BUTTON_COL_GAP) / 2.0f;
    const float centerButtonH = hi::BASE_CENTER_BUTTON_H;
    const float bx = basePanelX + pad;
    const float buttonYTop = tableY + rowHeight + hi::BASE_CENTER_BUTTON_TOP_MARGIN;
    const float buttonYMid = buttonYTop + centerButtonH + hi::BASE_CENTER_BUTTON_ROW_GAP;
    const float buttonYBottom = buttonYMid + centerButtonH + hi::BASE_CENTER_BUTTON_ROW_GAP;
    const float rightColX = bx + centerButtonW + hi::BASE_CENTER_BUTTON_COL_GAP;

    Rect r0{bx, buttonYTop, centerButtonW, centerButtonH};           // -1
    Rect r1{bx, buttonYMid, centerButtonW, centerButtonH};           // -5
    Rect r2{bx, buttonYBottom, centerButtonW, centerButtonH};        // 一括へ
    Rect r3{rightColX, buttonYTop, centerButtonW, centerButtonH};    // +1
    Rect r4{rightColX, buttonYMid, centerButtonW, centerButtonH};    // +5
    Rect r5{rightColX, buttonYBottom, centerButtonW, centerButtonH}; // 一括

    if (inRect(r0) && canDecrease) {
        OnBaseEnhancementDown(ctx, rowIndex);
        ctx.inputAPI->ConsumeLeftClick();
        return true;
    }
    if (inRect(r1) && canDecrease5) {
        OnBaseEnhancementDownBatch(ctx, rowIndex, 5);
        ctx.inputAPI->ConsumeLeftClick();
        return true;
    }
    if (inRect(r2) && canDecrease) {
        OnBaseEnhancementDownMax(ctx, rowIndex);
        ctx.inputAPI->ConsumeLeftClick();
        return true;
    }
    if (inRect(r3) && canIncrease && canAfford1) {
        OnBaseEnhancementUp(ctx, rowIndex);
        ctx.inputAPI->ConsumeLeftClick();
        return true;
    }
    if (inRect(r4) && canIncrease5 && canAfford5) {
        OnBaseEnhancementUpBatch(ctx, rowIndex, 5);
        ctx.inputAPI->ConsumeLeftClick();
        return true;
    }
    if (inRect(r5) && canIncrease && canAffordMax) {
        OnBaseEnhancementUpMax(ctx, rowIndex);
        ctx.inputAPI->ConsumeLeftClick();
        return true;
    }
    return false;
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

    // セーブと表示の同期: attachment_slots にマスターを反映
    auto attachments = ctx.gameplayDataAPI->GetTowerAttachments();
    const auto& masters = ctx.gameplayDataAPI->GetAllTowerAttachmentMasters();
    for (int i = 0; i < 3; ++i) {
        operation_panel_.attachment_slots[i].assigned_attachment = nullptr;
        if (!attachments[i].id.empty()) {
            auto it = masters.find(attachments[i].id);
            if (it != masters.end()) {
                operation_panel_.attachment_slots[i].assigned_attachment = &it->second;
            }
        }
    }

    std::vector<const entities::TowerAttachment*> allAttachments;
    allAttachments.reserve(masters.size());
    for (const auto& [id, attachment] : masters) {
        if (ctx.gameplayDataAPI->GetOwnedTowerAttachmentCount(id) > 0) {
            allAttachments.push_back(&attachment);
        }
    }
    const auto filteredAttachments = SortAttachmentsByName(allAttachments);

    if (ProcessItemListClick(ctx)) {
        return;
    }
    if (item_list_panel_.selected_index >= 1 && item_list_panel_.selected_index <= 5 && ProcessBaseEnhancementInput(ctx)) {
        return;
    }
    if (item_list_panel_.selected_index >= 6 && item_list_panel_.selected_index <= 8 && ProcessSlotDetailRemoveClick(ctx)) {
        return;
    }
    if (ProcessAttachmentDragAndDrop(ctx, filteredAttachments)) {
        return;
    }
}
bool EnhancementOverlay::ProcessItemListClick(SharedContext& ctx) {
    if (!ctx.inputAPI->IsLeftClickPressed()) return false;
    const Vec2 mouse = ctx.inputAPI->GetMousePositionInternal();
    const float pad = 18.0f;
    const float titleH = 44.0f;
    const float listY = item_list_panel_.y + titleH + 12.0f;
    for (int i = 0; i < 9; ++i) {
        const float rowY = listY + static_cast<float>(i) * item_list_panel_.item_height;
        const float rowW = item_list_panel_.width - pad * 2.0f;
        if (mouse.x >= item_list_panel_.x + pad && mouse.x < item_list_panel_.x + pad + rowW &&
            mouse.y >= rowY && mouse.y < rowY + item_list_panel_.item_height - 4.0f) {
            item_list_panel_.selected_index = i;
            ctx.inputAPI->ConsumeLeftClick();
            return true;
        }
    }
    return false;
}

bool EnhancementOverlay::ProcessSlotDetailRemoveClick(SharedContext& ctx) {
    const int sel = item_list_panel_.selected_index;
    if (sel < 6 || sel > 8) return false;
    const int slotIndex = sel - 6;
    auto attachments = ctx.gameplayDataAPI->GetTowerAttachments();
    const auto& masters = ctx.gameplayDataAPI->GetAllTowerAttachmentMasters();
    auto it = masters.find(attachments[slotIndex].id);
    if (it == masters.end()) return false;
    const auto* att = &it->second;
    if (!ctx.inputAPI->IsLeftClickPressed()) return false;
    const Vec2 mouse = ctx.inputAPI->GetMousePositionInternal();
    const float pad = 25.0f;
    const float lineH = 36.0f;
    const float titleY = status_panel_.y + pad;
    float btnY = titleY + 50.0f + lineH;  // name
    if (!att->description.empty()) btnY += lineH * 1.2f;
    btnY += lineH + 20.0f;  // effect + gap
    const float btnW = 120.0f;
    const float btnH = 40.0f;
    const float btnX = status_panel_.x + pad;
    if (mouse.x >= btnX && mouse.x < btnX + btnW && mouse.y >= btnY && mouse.y < btnY + btnH) {
        attachments[slotIndex].id.clear();
        attachments[slotIndex].level = 1;
        ctx.gameplayDataAPI->SetTowerAttachments(attachments);
        ctx.gameplayDataAPI->Save();
        ctx.inputAPI->ConsumeLeftClick();
        return true;
    }
    return false;
}

bool EnhancementOverlay::ProcessAttachmentDragAndDrop(SharedContext& ctx,
    const std::vector<const entities::TowerAttachment*>& filteredAttachments) {
    const Vec2 mouse = ctx.inputAPI->GetMousePositionInternal();
    auto inRect = [&](const Rect& r) {
        return mouse.x >= r.x && mouse.x <= r.x + r.width &&
               mouse.y >= r.y && mouse.y <= r.y + r.height;
    };

    const float opPadding = 25.0f;
    const float slotBottomY = operation_panel_.y + operation_panel_.attachment_slots[0].position.y + operation_panel_.attachment_slots[0].height;
    const float listStartY = slotBottomY + 14.0f;
    const float listTitleY = listStartY;
    const float listContentY = listTitleY + 44.0f;
    const float listHeight = operation_panel_.height - (listStartY - operation_panel_.y) - opPadding;
    const float listContentHeight = listHeight - (listContentY - listStartY) - opPadding;
    Rect listInner{
        operation_panel_.x + opPadding,
        listContentY,
        operation_panel_.width - opPadding * 2.0f,
        listContentHeight
    };

    if (!attachment_drag_started_ && !is_attachment_dragging_) {
        if (ctx.inputAPI->IsLeftClickPressed()) {
            for (int i = 0; i < 3; ++i) {
                const auto& slot = operation_panel_.attachment_slots[i];
                const float abs_x = operation_panel_.x + slot.position.x;
                const float abs_y = operation_panel_.y + slot.position.y;
                if (mouse.x >= abs_x && mouse.x < abs_x + slot.width && mouse.y >= abs_y && mouse.y < abs_y + slot.height && slot.assigned_attachment) {
                    attachment_drag_started_ = true;
                    is_attachment_dragging_ = false;
                    dragging_attachment_index_ = -1;
                    dragging_from_slot_index_ = i;
                    dragging_attachment_ = slot.assigned_attachment;
                    attachment_drag_start_pos_ = mouse;
                    attachment_drag_pos_ = mouse;
                    ctx.inputAPI->ConsumeLeftClick();
                    return true;
                }
            }
            if (inRect(listInner)) {
                const float itemHeight = 70.0f;
                const int startIndex = std::max(0, static_cast<int>(attachmentListScroll_));
                const int visibleCount = std::max(1, static_cast<int>(listInner.height / itemHeight));
                for (int i = 0; i < visibleCount; ++i) {
                    const int idx = startIndex + i;
                    if (idx >= static_cast<int>(filteredAttachments.size())) break;
                    Rect itemRect{listInner.x, listInner.y + itemHeight * i, listInner.width - 26.0f, itemHeight};
                    if (inRect(itemRect)) {
                        const auto* att = filteredAttachments[idx];
                        if (att) {
                            attachment_drag_started_ = true;
                            is_attachment_dragging_ = false;
                            dragging_attachment_index_ = idx;
                            dragging_from_slot_index_ = -1;
                            dragging_attachment_ = att;
                            attachment_drag_start_pos_ = mouse;
                            attachment_drag_pos_ = mouse;
                            ctx.inputAPI->ConsumeLeftClick();
                            return true;
                        }
                    }
                }
            }
        }
        float wheel = ctx.inputAPI->GetMouseWheelMove();
        if (wheel != 0.0f && inRect(listInner)) {
            const float itemHeight = 70.0f;
            const int visibleCount = std::max(1, static_cast<int>(listInner.height / itemHeight));
            const int maxOffset = std::max(0, static_cast<int>(filteredAttachments.size()) - visibleCount);
            attachmentListScroll_ = std::max(0.0f, std::min(attachmentListScroll_ - wheel, static_cast<float>(maxOffset)));
        }
        return false;
    }

    if (attachment_drag_started_ && dragging_attachment_) {
        if (!is_attachment_dragging_ && ctx.inputAPI->IsLeftClickDown()) {
            const float dx = mouse.x - attachment_drag_start_pos_.x;
            const float dy = mouse.y - attachment_drag_start_pos_.y;
            if (std::sqrt(dx * dx + dy * dy) > 5.0f) {
                is_attachment_dragging_ = true;
            }
        }
        if (is_attachment_dragging_) {
            attachment_drag_pos_ = mouse;
        }
        if (ctx.inputAPI->IsLeftClickReleased()) {
            if (is_attachment_dragging_) {
                const int targetSlot = GetAttachmentSlotAtPosition(mouse);
                if (targetSlot >= 0 && dragging_attachment_) {
                    auto attachments = ctx.gameplayDataAPI->GetTowerAttachments();
                    const std::string& equipId = dragging_attachment_->id;
                    bool alreadyEquippedElsewhere = false;
                    for (int i = 0; i < 3; ++i) {
                        if (i != targetSlot && attachments[i].id == equipId) {
                            alreadyEquippedElsewhere = true;
                            break;
                        }
                    }
                    if (!alreadyEquippedElsewhere) {
                        attachments[targetSlot].id = equipId;
                        attachments[targetSlot].level = 20;
                        if (dragging_from_slot_index_ >= 0 && dragging_from_slot_index_ != targetSlot) {
                            attachments[dragging_from_slot_index_].id.clear();
                            attachments[dragging_from_slot_index_].level = 1;
                        }
                        ctx.gameplayDataAPI->SetTowerAttachments(attachments);
                        ctx.gameplayDataAPI->Save();
                    }
                }
            }
            attachment_drag_started_ = false;
            is_attachment_dragging_ = false;
            dragging_attachment_index_ = -1;
            dragging_from_slot_index_ = -1;
            dragging_attachment_ = nullptr;
            ctx.inputAPI->ConsumeLeftClick();
            return true;
        }
    }
    return false;
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
    const int currentLevel = *levelRef;
    const int nextLevel = std::min(MAX_LEVEL, currentLevel + levels);
    if (nextLevel == currentLevel) {
        return;
    }
    const int levelsToAdd = nextLevel - currentLevel;
    const int totalCost = hi::ComputeTowerBaseLevelUpCost(currentLevel, levelsToAdd);
    if (ctx.gameplayDataAPI->GetGold() < totalCost) {
        return;
    }
    ctx.gameplayDataAPI->AddGold(-totalCost);
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
    const int levelsRemoved = *levelRef - nextLevel;
    const int refund = hi::ComputeTowerBaseRefund(nextLevel, levelsRemoved);
    ctx.gameplayDataAPI->AddGold(refund);
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

    const int currentLevel = *levelRef;
    if (currentLevel >= MAX_LEVEL) {
        return;
    }
    const int levelsToAdd = MAX_LEVEL - currentLevel;
    const int totalCost = hi::ComputeTowerBaseLevelUpCost(currentLevel, levelsToAdd);
    if (ctx.gameplayDataAPI->GetGold() < totalCost) {
        return;
    }
    ctx.gameplayDataAPI->AddGold(-totalCost);
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
    const int levelsRemoved = *levelRef;
    const int refund = hi::ComputeTowerBaseRefund(0, levelsRemoved);
    ctx.gameplayDataAPI->AddGold(refund);
    *levelRef = 0;
    ctx.gameplayDataAPI->SetTowerEnhancements(st);
    ctx.gameplayDataAPI->Save();
}

} // namespace core
} // namespace game
