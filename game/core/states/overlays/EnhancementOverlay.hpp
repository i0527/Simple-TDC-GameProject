#pragma once

#include "IOverlay.hpp"
// 標準ライブラリ
#include <array>
#include <string>
#include <vector>

// プロジェクト内
#include "../../config/RenderPrimitives.hpp"
#include "../../config/RenderTypes.hpp"
#include "../../system/TowerEnhancementEffects.hpp"

// 前方宣言
namespace game {
namespace core {
namespace entities {
struct TowerAttachment;
} // namespace entities
} // namespace core
} // namespace game

namespace game {
namespace core {

/// @brief 強化タブ
enum class EnhancementTab {
    BaseEnhancement,  // 基礎強化
    SlotEnhancement   // スロット強化
};

/// @brief 強化オーバーレイ
///
/// タワー強化画面を表示するオーバーレイ。
class EnhancementOverlay : public IOverlay {
public:
    EnhancementOverlay();
    ~EnhancementOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Enhancement; }
    bool IsImGuiOverlay() const override { return false; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    // ========== パネル構造 ==========

    /// @brief 左パネル: 項目一覧（一覧＋基礎強化5項目＋スロット1～3を縦並び）
    /// 0=一覧  1～5=城HP,お金成長,コスト回復,味方攻撃,味方HP  6～8=スロット1～3
    struct ItemListPanel {
        float x = 5.0f, y = 95.0f;
        float width = 437.0f, height = 880.0f;
        int selected_index = 0;
        int scroll_offset = 0;
        float item_height = 60.0f;
    };

    /// @brief 中央パネル: 選択項目に応じた強化パネル
    struct StatusPanel {
        float x = 452.0f, y = 95.0f;
        float width = 568.0f, height = 880.0f;
        float padding = 30.0f;
        float line_height = 45.0f;
        int font_size = 24;
    };

    /// @brief 右パネル: 固定3スロット ＋ アタッチメント一覧（D&D）
    struct OperationPanel {
        float x = 1030.0f, y = 95.0f;
        float width = 875.0f, height = 880.0f;

        struct AttachmentSlot {
            int slot_id;
            const entities::TowerAttachment* assigned_attachment;
            Vector2 position;
            float width = 280.0f;
            float height = 180.0f;
            bool is_hovered = false;
        };
        std::array<AttachmentSlot, 3> attachment_slots;
    };

    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;
    std::string selectedAttachmentId_;
    float attachmentListScroll_ = 0.0f;

    // ドラッグ＆ドロップ状態（一覧→スロット／スロット→スロット）
    bool attachment_drag_started_ = false;
    bool is_attachment_dragging_ = false;
    int dragging_attachment_index_ = -1;   // 一覧インデックス（-1 のときはスロットからドラッグ）
    int dragging_from_slot_index_ = -1;    // スロットからドラッグ時
    const entities::TowerAttachment* dragging_attachment_ = nullptr;
    Vec2 attachment_drag_start_pos_{};
    Vec2 attachment_drag_pos_{};

    // パネル
    ItemListPanel item_list_panel_;
    StatusPanel status_panel_;
    OperationPanel operation_panel_;

    void OnBaseEnhancementUp(SharedContext& ctx, int rowIndex);
    void OnBaseEnhancementDown(SharedContext& ctx, int rowIndex);
    void OnBaseEnhancementUpBatch(SharedContext& ctx, int rowIndex, int levels);
    void OnBaseEnhancementDownBatch(SharedContext& ctx, int rowIndex, int levels);
    void OnBaseEnhancementUpMax(SharedContext& ctx, int rowIndex);
    void OnBaseEnhancementDownMax(SharedContext& ctx, int rowIndex);

    std::vector<const entities::TowerAttachment*> SortAttachmentsByName(
        const std::vector<const entities::TowerAttachment*>& allAttachments) const;

    // レアリティ表示用ヘルパー
    Color GetRarityColor(int rarity) const;
    std::string GetRarityName(int rarity) const;
    
    // 初期化・クリーンアップ
    void InitializePanels();
    
    void RenderItemListPanel(SharedContext& ctx);
    void RenderMultiplierPanel(SharedContext& ctx, const Rect& panelRect,
                               const system::TowerEnhancementMultipliers& mul);
    void RenderCenterPanel(SharedContext& ctx);
    void RenderOverviewTab(SharedContext& ctx);
    void RenderBaseEnhancementTab(SharedContext& ctx);
    void RenderSlotDetailPanel(SharedContext& ctx, int slotIndex);
    void RenderRightPanel(SharedContext& ctx);

    bool ProcessBaseEnhancementInput(SharedContext& ctx);
    bool ProcessItemListClick(SharedContext& ctx);
    bool ProcessSlotDetailRemoveClick(SharedContext& ctx);
    bool ProcessAttachmentDragAndDrop(SharedContext& ctx,
        const std::vector<const entities::TowerAttachment*>& filteredAttachments);
    
    // ツールチップ描画
    void DrawTooltip(const std::string& text, float x, float y);

    // スロット／一覧のヒットテスト
    int GetAttachmentSlotAtPosition(Vec2 position) const;

    // 装備スロット描画（1スロット分）
    void RenderAttachmentSlot(SharedContext& ctx, const OperationPanel::AttachmentSlot& slot);
};

} // namespace core
} // namespace game
