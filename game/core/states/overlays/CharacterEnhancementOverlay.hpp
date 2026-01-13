#pragma once

#include "IOverlay.hpp"
#include "../../entities/Character.hpp"
#include <raylib.h>
#include <vector>
#include <random>
#include <array>
#include <string>

namespace game {
namespace core {

/// @brief ユニット強化オーバーレイ
///
/// ユニット強化・パッシブ付与・持ち物装備を管理するオーバーレイ。
/// 3分割レイアウト（ユニット情報・ステータス・操作）で実装。
class CharacterEnhancementOverlay : public IOverlay {
public:
    CharacterEnhancementOverlay();
    ~CharacterEnhancementOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Enhancement; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    // ========== パネル構造体 ==========
    
    /// @brief 左パネル: ユニット情報（幅は画面サイズから動的計算）
    /// コンテンツ領域（y=90～990）内に配置される
    struct UnitInfoPanel {
        float x = 5.0f, y = 95.0f;
        float width = 437.0f, height = 880.0f;
        
        // ユニット一覧
        std::vector<const entities::Character*> entries;
        int selected_index = -1;
        int scroll_offset = 0;
        float item_height = 60.0f;
        
        // 選択ユニット情報
        const entities::Character* selected_character = nullptr;
        float icon_size = 200.0f;
    };
    
    /// @brief 中央パネル: ステータス表示（幅は画面サイズから動的計算）
    /// コンテンツ領域（y=90～990）内に配置される
    struct StatusPanel {
        float x = 452.0f, y = 95.0f;
        float width = 568.0f, height = 880.0f;
        
        struct StatValue {
            int base = 0;
            int bonus = 0;
        };

        StatValue hp;
        StatValue attack;
        StatValue defense;
        StatValue speed;
        StatValue range;
        
        // レイアウト定数 (図鑑に合わせる)
        float padding = 30.0f;
        float line_height = 45.0f;
        int font_size = 24;
    };
    
    /// @brief 右パネル: 操作（幅は画面サイズから動的計算）
    /// コンテンツ領域（y=90～990）内に配置される
    struct OperationPanel {
        float x = 1030.0f, y = 95.0f;
        float width = 875.0f, height = 880.0f;
        
        // タブ切り替え
        enum class TabType {
            Enhancement,  // 強化
            Equipment     // 装備
        };
        TabType active_tab = TabType::Enhancement;
    
        
        // パッシブスロット (3つ、横並び)
        struct PassiveSlot {
            int slot_id;  // 0-2
            const entities::PassiveSkill* assigned_passive;  // nullptr = empty
            Vector2 position;  // パネル内の相対座標
            float width = 280.0f;  // 操作パネル幅から動的計算
            float height = 180.0f;
            bool is_hovered = false;
            int level = 1;  // パッシブレベル (1-3)
        };
        PassiveSlot passive_slots[3];
        
        // パッシブポップアップメニュー
        bool show_passive_popup = false;
        int popup_slot_id = -1;  // どのスロットのポップアップか
        Vector2 popup_position;
        
        // 利用可能なパッシブ一覧
        std::vector<const entities::PassiveSkill*> available_passives;
        
        // アイテムスロット（装備タブ用）
        struct ItemSlot {
            int slot_id;
            const entities::Equipment* assigned_item;
            Vector2 position;  // パネル内の相対座標
            float width = 280.0f;  // 操作パネル幅から動的計算
            float height = 180.0f;
            bool is_hovered = false;
        };
        ItemSlot item_slots[3];
        std::vector<const entities::Equipment*> available_items;
        int item_scroll_offset = 0;
    };
    
    /// @brief ポップアップメニュー項目
    struct PopupMenuItem {
        std::string label;
        Color color;
        int option_id;
        bool is_hovered;
    };
    
    // ========== メンバ変数 ==========
    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;
    
    // パネル
    UnitInfoPanel unit_info_panel_;
    StatusPanel status_panel_;
    OperationPanel operation_panel_;
    
    // 状態管理
    bool has_unsaved_changes_;
    
    // ランダム生成器（ガチャ用）
    std::mt19937 rng_;
    
    // ========== プライベートメソッド ==========
    
    // 初期化・クリーンアップ
    void InitializePanels();
    void LoadCharacterList(SharedContext& ctx);
    void SelectCharacter(const entities::Character* character);
    void UpdateStatusPanel();
    void FilterAvailablePassives(SharedContext& ctx);
    void FilterAvailableItems(SharedContext& ctx);
    
    // 描画メソッド
    void RenderUnitInfoPanel();
    void RenderStatusPanel();
    void RenderOperationPanel();
    void RenderUnitListButton();
    void RenderPassivePopup();
    
    // タブ別描画
    void RenderEnhancementTab();
    void RenderEquipmentTab();
    
    // パーツ描画
    void RenderPassiveSlot(const OperationPanel::PassiveSlot& slot);
    void RenderItemSlot(const OperationPanel::ItemSlot& slot);
    void RenderTabButton(float x, float y, float width, float height, const char* label, bool is_active);
    
    // イベント処理
    void OnUnitListItemClick(int index);
    void OnTabClick(OperationPanel::TabType tab);
    void OnPassiveSlotClick(int slot_id);
    void OnPassivePopupOption(int option);  // 0=ランダム付与/強化, 1=ランダム変更, 2=削除, 3=キャンセル
    void OnItemSlotClick(int slot_id);
    void OnItemListClick(int index);
    void OnConfirmButtonClick();
    void OnCancelButtonClick();
    
    // マウス入力処理
    void ProcessMouseInput(SharedContext& ctx);
    void UpdateHoverStates(Vector2 mouse_pos);
    
    // キーボード入力処理
    void ProcessKeyboardInput(SharedContext& ctx);
    
    // ユーティリティ
    const entities::Character* GetSelectedCharacter() const;
    int GetPassiveSlotAtPosition(Vector2 position) const;
    int GetItemSlotAtPosition(Vector2 position) const;
    
    // パッシブ操作
    void AssignRandomPassive(int slot_id);
    void UpgradePassive(int slot_id);
    void ReplacePassive(int slot_id);
    void RemovePassive(int slot_id);
    
    // 定数
    static constexpr float PANEL_GAP = 20.0f;
};

} // namespace core
} // namespace game
