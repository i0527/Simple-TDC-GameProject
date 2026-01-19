#pragma once

#include "IOverlay.hpp"
#include "../../ecs/entities/Character.hpp"
#include "../../config/RenderPrimitives.hpp"
#include "../../config/RenderTypes.hpp"
#include <vector>

namespace game {
namespace core {

class GameplayDataAPI;

/// @brief 邱ｨ謌舌が繝ｼ繝舌・繝ｬ繧､
///
/// 10繧ｭ繝｣繝ｩ繧ｯ繧ｿ繝ｼ邱ｨ謌舌↓蟇ｾ蠢懊＠縺溽ｷｨ謌千判髱｢繧定｡ｨ遉ｺ縺吶ｋ繧ｪ繝ｼ繝舌・繝ｬ繧､縲・
/// FHD (1920x1080) 逕ｻ髱｢縺ｫ譛驕ｩ蛹悶＆繧後※縺・∪縺吶・
class FormationOverlay : public IOverlay {
public:
    FormationOverlay();
    ~FormationOverlay() = default;

    // IOverlay螳溯｣・
    bool Initialize(BaseSystemAPI* systemAPI, UISystemAPI* uiAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Formation; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    // ========== 蜀・Κ讒矩菴・==========
    
    /// @brief 邱ｨ謌舌せ繝ｭ繝・ヨ
    struct SquadSlot {
        int slot_id = 0;                                // 0-9
        const entities::Character* assigned_character = nullptr; // nullptr = empty
        Vec2 position = {0.0f, 0.0f};                // 逕ｻ髱｢蠎ｧ讓・
        float width = 140.0f;
        float height = 120.0f;
        bool is_hovered = false;
        bool is_dragging = false;
    };
    
    /// @brief 繝代・繝・ぅ繝ｼ繧ｵ繝槭Μ繝ｼ諠・ｱ
    struct PartySummaryInfo {
        int total_cost = 0;
        // 邱ｨ謌舌さ繧ｹ繝井ｸ企剞縺ｯ謦､蟒・ｼ郁｡ｨ遉ｺ逕ｨ縺ｫ谿九＠縺ｦ縺・ｋ縺悟愛螳壹↓縺ｯ菴ｿ繧上↑縺・ｼ・
        int max_cost = 0;
        int total_hp = 0;
        int total_attack = 0;
        int total_defense = 0;
        int character_count = 0;
        int max_character_count = 10;
        
        bool IsCostValid() const {
            // 繧ｳ繧ｹ繝井ｸ企剞縺ｪ縺・
            return true;
        }
        
        bool IsComplete() const {
            // 1菴謎ｻ･荳翫＞繧後・OK・医さ繧ｹ繝井ｸ企剞繝√ぉ繝・け縺ｯ縺励↑縺・ｼ・
            return character_count > 0;
        }
    };
    
    /// @brief 繧ｭ繝｣繝ｩ繧ｯ繧ｿ繝ｼ荳隕ｧ繝薙Η繝ｼ
    struct CharacterListView {
        std::vector<const entities::Character*> available_characters;
        int scroll_offset = 0;
        int visible_columns = 7;
        int visible_rows = 2;
        int selected_character_index = -1;
        
        const float CARD_WIDTH = 140.0f;
        const float CARD_HEIGHT = 120.0f;
        const float CARD_SPACING_X = 150.0f;
        const float CARD_SPACING_Y = 150.0f;
    };
    
    /// @brief 隧ｳ邏ｰ繝代ロ繝ｫ諠・ｱ
    struct DetailsPanelInfo {
        float x = 1220.0f;
        float y = 160.0f;
        float width = 590.0f;
        float height = 745.0f;
        float padding = 30.0f;
        float line_height = 40.0f;
        int font_size = 28;
        
        const entities::Character* displayed_character = nullptr;
    };
    
    // ========== 繝｡繝ｳ繝仙､画焚 ==========
    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;
    
    // 邱ｨ謌舌せ繝ｭ繝・ヨ・・0蛟具ｼ・
    SquadSlot squad_slots_[10];
    PartySummaryInfo m_partySummary;
    CharacterListView m_characterList;
    DetailsPanelInfo m_detailsPanel;
    
    // 驕ｸ謚樔ｸｭ縺ｮ繧ｭ繝｣繝ｩ繧ｯ繧ｿ繝ｼ・医・繝舌・/繝峨Λ繝・げ荳ｭ・・
    const entities::Character* selected_character_;
    
    // 繝峨Λ繝・げ&繝峨Ο繝・・迥ｶ諷・
    const entities::Character* dragging_character_;
    int dragging_source_slot_;  // -1 = 繧ｭ繝｣繝ｩ繧ｯ繧ｿ繝ｼ荳隕ｧ縺九ｉ, 0-9 = 繧ｹ繝ｭ繝・ヨ縺九ｉ
    Vec2 drag_position_;
    bool is_dragging_;
    Vec2 drag_start_pos_;
    bool drag_started_;
    
    // 繝懊ち繝ｳ迥ｶ諷・
    struct ButtonState {
        bool is_hovered = false;
        bool is_pressed = false;
    };
    ButtonState complete_button_;
    ButtonState cancel_button_;
    ButtonState reset_button_;
    
    // 驕ｸ謚樔ｸｭ縺ｮ繧ｹ繝ｭ繝・ヨ・医く繝ｼ繝懊・繝画桃菴懃畑・・
    int selected_slot_index_;
    
    // 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ譎る俣・医ヱ繝ｫ繧ｹ繧ｨ繝輔ぉ繧ｯ繝育畑・・
    float animation_time_;
    
    // SharedContext縺ｮ邱ｨ謌舌ｒ荳蠎ｦ縺縺大ｾｩ蜈・☆繧九◆繧√・繝輔Λ繧ｰ
    bool restored_from_context_ = false;
    
    // ========== 繝励Λ繧､繝吶・繝医Γ繧ｽ繝・ラ ==========
    
    // 蛻晄悄蛹悶・繧ｯ繝ｪ繝ｼ繝ｳ繧｢繝・・
    void InitializeSlots();
    void RestoreFormationFromContext(SharedContext& ctx);
    void FilterAvailableCharacters(SharedContext& ctx);
    void SortAvailableCharacters(const GameplayDataAPI* gameplayDataAPI);
    
    // 謠冗判繝｡繧ｽ繝・ラ
    void RenderTitleBar();
    void RenderSquadSlots();
    void RenderSlot(const SquadSlot& slot);
    void RenderPartySummary();
    void RenderCharacterList();
    void RenderCharacterCard(const entities::Character* character, int card_index);
    void RenderButtons();
    void RenderDividers();
    void RenderDraggingCharacter();
    void RenderDetailsPanel(SharedContext& ctx);
    
    // 蠎ｧ讓呵ｨ育ｮ励・繝ｫ繝代・
    static Vec2 GetSlotPosition(int slot_id);
    Vec2 GetCardPosition(int card_index) const;
    int GetSlotAtPosition(Vec2 position) const;
    int GetCardAtPosition(Vec2 position) const;
    
    // 繧ｭ繝｣繝ｩ繧ｯ繧ｿ繝ｼ邂｡逅・
    void AssignCharacter(int slot_id, const entities::Character* character);
    void RemoveCharacter(int slot_id);
    void SwapCharacters(int slot1_id, int slot2_id);
    
    // 繝代・繝・ぅ繝ｼ邂｡逅・
    void UpdatePartySummary();
    bool ValidateSquadComposition();
    
    // 繧､繝吶Φ繝亥・逅・
    void OnSlotClicked(int slot_id);
    void OnCardClicked(int card_index);
    void OnSlotRightClicked(int slot_id);
    void OnDragStart(int source_slot, const entities::Character* character, SharedContext& ctx);
    void OnDragUpdate(Vec2 mouse_pos);
    void OnDragEnd(Vec2 mouse_pos);
    void OnButtonClicked(const std::string& button_name, SharedContext& ctx);
    
    // 繝槭え繧ｹ蜈･蜉帛・逅・
    void ProcessMouseInput(SharedContext& ctx);
    void UpdateHoverStates(Vec2 mouse_pos);
    
    // 繧ｭ繝ｼ繝懊・繝牙・蜉帛・逅・
  void ProcessKeyboardInput(SharedContext& ctx);
    
    // 繧ｹ繧ｯ繝ｭ繝ｼ繝ｫ蜃ｦ逅・
    void ProcessScrollInput(float wheel_delta);
    
    // 繝ｦ繝ｼ繝・ぅ繝ｪ繝・ぅ
    bool IsCharacterInSquad(const entities::Character* character) const;
    Color GetSlotColor(const SquadSlot& slot) const;
    Color GetPartySummaryColor() const;
};

} // namespace core
} // namespace game
