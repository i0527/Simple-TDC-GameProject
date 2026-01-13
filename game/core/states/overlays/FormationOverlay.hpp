#pragma once

#include "IOverlay.hpp"
#include "../../entities/Character.hpp"
#include <vector>
#include <raylib.h>

namespace game {
namespace core {

/// @brief 編成オーバーレイ
///
/// 10キャラクター編成に対応した編成画面を表示するオーバーレイ。
/// FHD (1920x1080) 画面に最適化されています。
class FormationOverlay : public IOverlay {
public:
    FormationOverlay();
    ~FormationOverlay() = default;

    // IOverlay実装
    bool Initialize(BaseSystemAPI* systemAPI) override;
    void Update(SharedContext& ctx, float deltaTime) override;
    void Render(SharedContext& ctx) override;
    void Shutdown() override;

    OverlayState GetState() const override { return OverlayState::Formation; }
    bool RequestClose() const override;
    bool RequestTransition(GameState& nextState) const override;

private:
    // ========== 内部構造体 ==========
    
    /// @brief 編成スロット
    struct SquadSlot {
        int slot_id = 0;                                // 0-9
        const entities::Character* assigned_character = nullptr; // nullptr = empty
        Vector2 position = {0.0f, 0.0f};                // 画面座標
        float width = 140.0f;
        float height = 120.0f;
        bool is_hovered = false;
        bool is_dragging = false;
    };
    
    /// @brief パーティーサマリー情報
    struct PartySummaryInfo {
        int total_cost = 0;
        // 編成コスト上限は撤廃（表示用に残しているが判定には使わない）
        int max_cost = 0;
        int total_hp = 0;
        int total_attack = 0;
        int total_defense = 0;
        int character_count = 0;
        int max_character_count = 10;
        
        bool IsCostValid() const {
            // コスト上限なし
            return true;
        }
        
        bool IsComplete() const {
            // 1体以上いればOK（コスト上限チェックはしない）
            return character_count > 0;
        }
    };
    
    /// @brief キャラクター一覧ビュー
    struct CharacterListView {
        std::vector<const entities::Character*> available_characters;
        int scroll_offset = 0;
        int visible_columns = 8;
        int visible_rows = 2;
        int selected_character_index = -1;
        
        const float CARD_WIDTH = 100.0f;
        const float CARD_HEIGHT = 120.0f;
        const float CARD_SPACING_X = 115.0f;
        const float CARD_SPACING_Y = 140.0f;
    };
    
    /// @brief 詳細パネル情報
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
    
    // ========== メンバ変数 ==========
    BaseSystemAPI* systemAPI_;
    bool isInitialized_;
    mutable bool requestClose_;
    mutable bool hasTransitionRequest_;
    mutable GameState requestedNextState_;
    
    // 編成スロット（10個）
    SquadSlot squad_slots_[10];
    PartySummaryInfo m_partySummary;
    CharacterListView m_characterList;
    DetailsPanelInfo m_detailsPanel;
    
    // 選択中のキャラクター（ホバー/ドラッグ中）
    const entities::Character* selected_character_;
    
    // ドラッグ&ドロップ状態
    const entities::Character* dragging_character_;
    int dragging_source_slot_;  // -1 = キャラクター一覧から, 0-9 = スロットから
    Vector2 drag_position_;
    bool is_dragging_;
    Vector2 drag_start_pos_;
    bool drag_started_;
    
    // ボタン状態
    struct ButtonState {
        bool is_hovered = false;
        bool is_pressed = false;
    };
    ButtonState complete_button_;
    ButtonState cancel_button_;
    ButtonState reset_button_;
    
    // 選択中のスロット（キーボード操作用）
    int selected_slot_index_;
    
    // アニメーション時間（パルスエフェクト用）
    float animation_time_;
    
    // ========== プライベートメソッド ==========
    
    // 初期化・クリーンアップ
    void InitializeSlots();
    void FilterAvailableCharacters(SharedContext& ctx);
    void SortAvailableCharacters();
    
    // 描画メソッド
    void RenderTitleBar();
    void RenderSquadSlots();
    void RenderSlot(const SquadSlot& slot);
    void RenderPartySummary();
    void RenderCharacterList();
    void RenderCharacterCard(const entities::Character* character, int card_index);
    void RenderButtons();
    void RenderDividers();
    void RenderDraggingCharacter();
    void RenderDetailsPanel();
    
    // 座標計算ヘルパー
    static Vector2 GetSlotPosition(int slot_id);
    Vector2 GetCardPosition(int card_index) const;
    int GetSlotAtPosition(Vector2 position) const;
    int GetCardAtPosition(Vector2 position) const;
    
    // キャラクター管理
    void AssignCharacter(int slot_id, const entities::Character* character);
    void RemoveCharacter(int slot_id);
    void SwapCharacters(int slot1_id, int slot2_id);
    
    // パーティー管理
    void UpdatePartySummary();
    bool ValidateSquadComposition();
    
    // イベント処理
    void OnSlotClicked(int slot_id);
    void OnCardClicked(int card_index);
    void OnSlotRightClicked(int slot_id);
    void OnDragStart(int source_slot, const entities::Character* character);
    void OnDragUpdate(Vector2 mouse_pos);
    void OnDragEnd(Vector2 mouse_pos);
    void OnButtonClicked(const std::string& button_name, SharedContext& ctx);
    
    // マウス入力処理
    void ProcessMouseInput(SharedContext& ctx);
    void UpdateHoverStates(Vector2 mouse_pos);
    
    // キーボード入力処理
    void ProcessKeyboardInput();
    
    // スクロール処理
    void ProcessScrollInput(float wheel_delta);
    
    // ユーティリティ
    bool IsCharacterInSquad(const entities::Character* character) const;
    Color GetSlotColor(const SquadSlot& slot) const;
    Color GetPartySummaryColor() const;
};

} // namespace core
} // namespace game
