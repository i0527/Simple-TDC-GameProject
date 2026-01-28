#include "EnhancementOverlay.hpp"
#include "EnhancementOverlayInternal.hpp"

// 標準ライブラリ
#include <algorithm>
#include <string>
#include <vector>

// プロジェクト内
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/GameplayDataAPI.hpp"
#include "../../config/RenderTypes.hpp"
#include "../../ecs/entities/TowerAttachment.hpp"
#include "../../ui/OverlayColors.hpp"

namespace game {
namespace core {

namespace hi = enhancement_overlay_internal;

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

    InitializePanels();

    isInitialized_ = true;
    LOG_INFO("EnhancementOverlay initialized");
    return true;
}

void EnhancementOverlay::InitializePanels() {
    const float SCREEN_WIDTH = 1920.0f;
    const float SCREEN_HEIGHT = 1080.0f;
    const float HEADER_HEIGHT = 90.0f;
    const float TAB_BAR_HEIGHT = 0.0f;
    const float CONTENT_START_Y = HEADER_HEIGHT;
    const float CONTENT_END_Y = SCREEN_HEIGHT;

    const float MARGIN = 5.0f;
    const float PANEL_GAP = 10.0f;

    const float available_width = SCREEN_WIDTH - (MARGIN * 2);
    const float available_height = CONTENT_END_Y - CONTENT_START_Y - (MARGIN * 2);

    const float total_ratio = 1.0f + 1.3f + 2.0f;
    const float list_width =
        (available_width - PANEL_GAP * 2) * (1.0f / total_ratio);
    const float status_width =
        (available_width - PANEL_GAP * 2) * (1.3f / total_ratio);
    const float operation_width =
        (available_width - PANEL_GAP * 2) * (2.0f / total_ratio);

    item_list_panel_.x = MARGIN;
    item_list_panel_.y = CONTENT_START_Y + MARGIN;
    item_list_panel_.width = list_width;
    item_list_panel_.height = available_height;
    item_list_panel_.selected_index = 0;
    item_list_panel_.scroll_offset = 0;
    item_list_panel_.item_height = 60.0f;

    status_panel_.x = item_list_panel_.x + item_list_panel_.width + PANEL_GAP;
    status_panel_.y = CONTENT_START_Y + MARGIN;
    status_panel_.width = status_width;
    status_panel_.height = available_height;
    status_panel_.padding = 30.0f;
    status_panel_.line_height = 45.0f;
    status_panel_.font_size = 24;

    operation_panel_.x = status_panel_.x + status_panel_.width + PANEL_GAP;
    operation_panel_.y = CONTENT_START_Y + MARGIN;
    operation_panel_.width = operation_width;
    operation_panel_.height = available_height;

    // 右パネル: 3スロット横並び（CharacterEnhancementOverlay と同様）
    const float slot_margin = 15.0f;
    const float slot_spacing = 15.0f;
    const float slot_width =
        (operation_panel_.width - slot_margin * 2.0f - slot_spacing * 2.0f) / 3.0f;
    const float slot_height = 180.0f;
    const float slot_start_x = slot_margin;
    const float slot_y = 92.0f;  // 12px 下げて表示
    for (int i = 0; i < 3; ++i) {
        operation_panel_.attachment_slots[i].slot_id = i;
        operation_panel_.attachment_slots[i].assigned_attachment = nullptr;
        operation_panel_.attachment_slots[i].position.x = slot_start_x + static_cast<float>(i) * (slot_width + slot_spacing);
        operation_panel_.attachment_slots[i].position.y = slot_y;
        operation_panel_.attachment_slots[i].width = slot_width;
        operation_panel_.attachment_slots[i].height = slot_height;
        operation_panel_.attachment_slots[i].is_hovered = false;
    }

    LOG_INFO("EnhancementOverlay: Panel layout calculated");
    LOG_INFO("  Item list panel: x={:.1f} y={:.1f} w={:.1f} h={:.1f}",
             item_list_panel_.x, item_list_panel_.y, item_list_panel_.width,
             item_list_panel_.height);
    LOG_INFO("  Status panel: x={:.1f} y={:.1f} w={:.1f} h={:.1f}",
             status_panel_.x, status_panel_.y, status_panel_.width,
             status_panel_.height);
    LOG_INFO("  Operation panel: x={:.1f} y={:.1f} w={:.1f} h={:.1f}",
             operation_panel_.x, operation_panel_.y, operation_panel_.width,
             operation_panel_.height);
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

// アタッチメントレアリティは 1=R, 2=SR, 3=SSR のみ
Color EnhancementOverlay::GetRarityColor(int rarity) const {
    switch (rarity) {
    case 1: return ui::OverlayColors::ACCENT_BLUE;   // R
    case 2: return ui::OverlayColors::WARNING_ORANGE; // SR
    case 3: return ui::OverlayColors::ACCENT_GOLD;   // SSR
    default: return ui::OverlayColors::TEXT_SECONDARY;
    }
}

std::string EnhancementOverlay::GetRarityName(int rarity) const {
    switch (rarity) {
    case 1: return "R";
    case 2: return "SR";
    case 3: return "SSR";
    default: return "不明";
    }
}

std::vector<const entities::TowerAttachment*> EnhancementOverlay::SortAttachmentsByName(
    const std::vector<const entities::TowerAttachment*>& allAttachments) const {

    std::vector<const entities::TowerAttachment*> result = allAttachments;
    std::sort(result.begin(), result.end(),
        [](const entities::TowerAttachment* a, const entities::TowerAttachment* b) {
            return a->name < b->name;
        });
    return result;
}

int EnhancementOverlay::GetAttachmentSlotAtPosition(Vec2 position) const {
    for (int i = 0; i < 3; ++i) {
        const auto& slot = operation_panel_.attachment_slots[i];
        const float abs_x = operation_panel_.x + slot.position.x;
        const float abs_y = operation_panel_.y + slot.position.y;
        if (position.x >= abs_x && position.x < abs_x + slot.width &&
            position.y >= abs_y && position.y < abs_y + slot.height) {
            return i;
        }
    }
    return -1;
}

void EnhancementOverlay::DrawTooltip(const std::string& text, float x, float y) {
    if (!systemAPI_) {
        return;
    }

    auto& render = systemAPI_->Render();
    const float padding = 8.0f;
    Vector2 textSize = render.MeasureTextDefault(text, hi::FONT_CAPTION);
    const float tooltipW = textSize.x + padding * 2.0f;
    const float tooltipH = textSize.y + padding * 2.0f;

    const float tooltipX = x - tooltipW * 0.5f;
    const float tooltipY = y - tooltipH;

    render.DrawRectangle(tooltipX, tooltipY, tooltipW, tooltipH,
                         ui::OverlayColors::PANEL_BG_PRIMARY);
    render.DrawRectangleLines(tooltipX, tooltipY, tooltipW, tooltipH,
                              1.0f, ui::OverlayColors::BORDER_DEFAULT);

    render.DrawTextDefault(text.c_str(), tooltipX + padding, tooltipY + padding,
                           hi::FONT_CAPTION, ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
}

} // namespace core
} // namespace game
