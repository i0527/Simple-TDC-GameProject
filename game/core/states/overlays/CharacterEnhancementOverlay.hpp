#pragma once

#include "IOverlay.hpp"
#include "../../ecs/entities/Character.hpp"
#include "../../system/PlayerDataManager.hpp"
#include "../../config/RenderPrimitives.hpp"
#include "../../config/RenderTypes.hpp"
#include <vector>
#include <random>
#include <array>
#include <string>

namespace game {
namespace core {

/// @brief ユニット強化オーバEレイ
///
/// ユニット強化Eパッシブ付与E持ち物裁Eを管琁EるオーバEレイ、E
/// 3刁Eレイアウト（ユニット情報・スチEEタス・操作）で実裁EE
class CharacterEnhancementOverlay : public IOverlay {
public:
    CharacterEnhancementOverlay();
    ~CharacterEnhancementOverlay() = default;

    // IOverlay実裁E
    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Enhancement; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    // ========== パネル構造佁E==========
    
    /// @brief 左パネル: ユニット情報EE画面サイズから動的計算！E
    /// コンチEチE域EE=90EE90EEに配置されめE
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
    };
    
    /// @brief 中央パネル: スチEEタス表示EE画面サイズから動的計算！E
    /// コンチEチE域EE=90EE90EEに配置されめE
    struct StatusPanel {
        float x = 452.0f, y = 95.0f;
        float width = 568.0f, height = 880.0f;
        
        struct StatValue {
            int base = 0;
            int bonus = 0;
        };
        
        // 元の状態（saved_character_state_）からの変化量計算用
        struct OriginalStatValue {
            int total = 0;  // 元の状態での合計値
        };
        
        OriginalStatValue original_hp;
        OriginalStatValue original_attack;
        OriginalStatValue original_defense;
        OriginalStatValue original_speed;
        OriginalStatValue original_range;
        int original_level = 1;

        StatValue hp;
        StatValue attack;
        StatValue defense;
        StatValue speed;
        StatValue range;
        
        // 追加表示EE編成相当！E
        int level = 1;
        int cost = 0;
        entities::AttackType attack_type = entities::AttackType::Single;
        entities::EffectType effect_type = entities::EffectType::Normal;
        float attack_span = 0.0f;
        int rarity = 1;
        std::string rarity_name;
        
        // レイアウト定数 (図鑑に合わせる)
        float padding = 30.0f;
        float line_height = 45.0f;
        int font_size = 24;
    };
    
    /// @brief 右パネル: 操作（画面サイズから動的計算！E
    /// コンチEチE域EE=90EE90EEに配置されめE
    struct OperationPanel {
        float x = 1030.0f, y = 95.0f;
        float width = 875.0f, height = 880.0f;
        
        // タブEり替ぁE
        enum class TabType {
            BaseEnhancement,  // 基礎強化
            Slot               // スロット
        };
        TabType active_tab = TabType::BaseEnhancement;
        
        // スロットタブ用のサブタブ
        enum class SlotSubTabType {
            Passive,   // パッシブスキル
            Equipment  // 装備
        };
        SlotSubTabType active_slot_subtab = SlotSubTabType::Passive;
    
        
        // パッシブスロチE (3つ、横並び)
        struct PassiveSlot {
            int slot_id;  // 0-2
            const entities::PassiveSkill* assigned_passive;  // nullptr = empty
            Vector2 position;  // パネル冁EE相対座樁E
            float width = 280.0f;  // 操作パネル画面から動的に計算
            float height = 180.0f;
            bool is_hovered = false;
            int level = 1;  // パッシブレベル (1-3)
        };
        PassiveSlot passive_slots[3];
        
        // パッシブEチEアチEEメニュー
        bool show_passive_popup = false;
        int popup_slot_id = -1;  // どのスロチEのポップアチEEぁE
        Vector2 popup_position;
        
        // 利用可能なパッシブ一覧
        std::vector<const entities::PassiveSkill*> available_passives;
        
        // パッシブ一覧スクロールEE下部パネル用EE
        int passive_scroll_offset = 0;
        
        // アイチEスロチEEE裁Eタブ用EE
        struct ItemSlot {
            int slot_id;
            const entities::Equipment* assigned_item;
            Vector2 position;  // パネル冁EE相対座樁E
            float width = 280.0f;  // 操作パネル画面から動的に計算
            float height = 180.0f;
            bool is_hovered = false;
        };
        ItemSlot item_slots[3];
        std::vector<const entities::Equipment*> available_items;
        int item_scroll_offset = 0;

        // ソーチEIEE裁Eタブ！E
        enum class ItemSortKey {
            Name,
            OwnedCount,
            Attack,
            Defense,
            Hp
        };
        ItemSortKey item_sort_key = ItemSortKey::Name;
        bool item_sort_ascending = true;  // 初期: 名前は昇順

        // 裁ED&D用
        std::string dragging_item_id; // ドラチE中のアイチEID
        Vector2 drag_start_mouse_pos; // ドラチE開始時のマウス座樁E
        bool is_dragging_item = false; // アイチEをドラチE中ぁE
        int selected_item_slot_id = -1; // 選択中のアイチEスロチEID

        // アイチEポップアチEEメニュー
        bool show_item_popup = false;
        int popup_item_slot_id = -1; // どのスロチEのポップアチEEぁE
    };
    
    /// @brief ポップアチEEメニュー頁E
    struct PopupMenuItem {
        std::string label;
        Color color;
        int option_id;
        bool is_hovered;
    };
    
    // ========== メンバ変数 ==========
    BaseSystemAPI* systemAPI_;
    SharedContext* sharedContext_ = nullptr;  // Render冁EEも参照したぁE報があるため保持EE所有権なし！E
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;
    
    // パネル
    UnitInfoPanel unit_info_panel_;
    StatusPanel status_panel_;
    OperationPanel operation_panel_;
    
    // 状態管琁E
    bool has_unsaved_changes_;
    PlayerDataManager::CharacterState saved_character_state_{};    // 直近セーブ状態（差刁E算E基準！E
    PlayerDataManager::CharacterState editing_character_state_{};  // 現在の編雁E態！EvなどEE
    std::string editing_character_id_;

    // 裁ED&D状態
    bool item_drag_started_ = false;
    bool is_item_dragging_ = false;
    int dragging_item_index_ = -1;
    const entities::Equipment* dragging_item_ = nullptr;
    Vec2 item_drag_start_pos_{};
    Vec2 item_drag_pos_{};
    
    // ランダム生E器Eガチャ用EE
    std::mt19937 rng_;
    
    // ソート関連
    enum class SortKey {
        Name,
        Rarity,
        Cost,
        Level,
        Owned
    };
    SortKey currentSortKey_ = SortKey::Owned;
    bool sortAscending_ = false;
    bool showOwnedOnly_ = false;  // 保持（所持）のみ表示するか
    
    // ========== プライベEトメソチE ==========
    
    // 初期化EクリーンアチEE
    void InitializePanels();
    void LoadCharacterList(SharedContext& ctx);
    void SortCharacterList(SharedContext& ctx);
    void SelectCharacter(SharedContext& ctx, const entities::Character* character);
    void UpdateStatusPanel(SharedContext& ctx);
    PlayerDataManager::CharacterState BuildCurrentEditingState() const;
    bool TryCommitEditingState(SharedContext& ctx, const PlayerDataManager::CharacterState& newState);
    void ApplyStateToUI(SharedContext& ctx, const PlayerDataManager::CharacterState& state);
    void FilterAvailablePassives(SharedContext& ctx);
    void FilterAvailableItems(SharedContext& ctx);
    
    // 描画メソチE
    void RenderUnitInfoPanel();
    void RenderSortUI();
    void RenderStatusPanel(SharedContext& ctx);
    void RenderOperationPanel(SharedContext& ctx);
    void RenderUnitListButton();
    void RenderPassivePopup(SharedContext& ctx);
    void RenderItemPopup(SharedContext& ctx);
    
    // タブ別描画
    void RenderBaseEnhancementTab(SharedContext& ctx);
    void RenderSlotTab(SharedContext& ctx);
    void RenderPassiveSubTab(SharedContext& ctx);
    void RenderEquipmentSubTab(SharedContext& ctx);
    
    // パEチE画
    void RenderPassiveSlot(SharedContext& ctx, const OperationPanel::PassiveSlot& slot);
    void RenderItemSlot(const OperationPanel::ItemSlot& slot);
    void RenderTabButton(float x, float y, float width, float height, const char* label, bool is_active);
    
    // イベントE琁E
    void OnUnitListItemClick(SharedContext& ctx, int index);
    void OnTabClick(OperationPanel::TabType tab);
    void OnSlotSubTabClick(OperationPanel::SlotSubTabType subtab);
    void OnPassiveSlotClick(int slot_id);
    void OnPassivePopupOption(SharedContext& ctx, int option);  // 0=ランダム付丁E強匁E 1=ランダム変更, 2=削除, 3=キャンセル
    void OnItemSlotClick(int slot_id);
    void OnItemListClick(SharedContext& ctx, int index);
    void OnItemPopupOption(SharedContext& ctx, int option);  // 0=こEスロチEに裁E, 1=外す, 2=キャンセル

    // レベル操佁E
    void OnLevelUpClick(SharedContext& ctx);
    void OnLevelDownClick(SharedContext& ctx);
    void OnLevelDownBatchClick(SharedContext& ctx, int levels);
    void OnLevelUpBatchClick(SharedContext& ctx, int levels);
    void OnLevelDownMaxClick(SharedContext& ctx);
    void OnLevelUpMaxClick(SharedContext& ctx);

    // パッシブ一括操佁E
    bool ResetAllPassives(SharedContext& ctx);
    bool RerollAllPassives(SharedContext& ctx);

    // 裁E一括操佁E
    void RemoveAllEquipment(SharedContext& ctx);
    
    // 裁ED&D操佁E
    void OnDragStartItem(SharedContext& ctx, int item_index);
    void OnDragUpdateItem();
    void OnDragEndItem(SharedContext& ctx);
    
    // マウス入力E琁E
    void ProcessMouseInput(SharedContext& ctx);
    void UpdateHoverStates(Vec2 mouse_pos);
    
    // キーボEドE力E琁E
    void ProcessKeyboardInput(SharedContext& ctx);
    
    // ユーチEEリチE
    const entities::Character* GetSelectedCharacter() const;
    int GetPassiveSlotAtPosition(Vector2 position) const;
    int GetItemSlotAtPosition(Vector2 position) const;
    
    // パッシブ操佁E
    bool AssignRandomPassive(int slot_id);
    bool UpgradePassive(int slot_id);
    bool ReplacePassive(SharedContext& ctx, int slot_id);
    bool RemovePassive(int slot_id);
    
    // 定数
    static constexpr float PANEL_GAP = 20.0f;
};

} // namespace core
} // namespace game
