#pragma once

#include "../../config/RenderPrimitives.hpp"
#include "../../config/RenderTypes.hpp"
#include "../../ecs/entities/Character.hpp"
#include "IOverlay.hpp"
#include <vector>

namespace game {
namespace core {

class GameplayDataAPI;

/// @brief 編成オーバーレイ
///
/// 10キャラクター編成に対応した編成画面を表示するオーバーレイ。
/// FHD (1920x1080) 画面に最適化されています。
class FormationOverlay : public IOverlay {
public:
  FormationOverlay();
  ~FormationOverlay() = default;

  // IOverlay実装
  bool Initialize(BaseSystemAPI *systemAPI, UISystemAPI *uiAPI) override;
  void Update(SharedContext &ctx, float deltaTime) override;
  void Render(SharedContext &ctx) override;
  void Shutdown() override;

  OverlayState GetState() const override { return OverlayState::Formation; }
  bool RequestClose() const override;
  bool RequestTransition(GameState &nextState) const override;

private:
  // ========== 構造体定義 ==========

  /// @brief 編成スロット
  struct SquadSlot {
    int slot_id = 0;                                         // 0-9
    const entities::Character *assigned_character = nullptr; // nullptr = empty
    Vec2 position = {0.0f, 0.0f};                            // 画面座標
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
      // 1体以上いれればOK。コスト上限チェックはしない。
      return character_count > 0;
    }
  };

  /// @brief キャラクター一覧ビュー
  struct CharacterListView {
    std::vector<const entities::Character *> available_characters;
    int scroll_offset = 0;
    int visible_columns = 6;  // 5 → 6 に変更
    int visible_rows = 5;
    int selected_character_index = -1;

    const float CARD_WIDTH = 140.0f;
    const float CARD_HEIGHT = 120.0f;
    const float CARD_SPACING_X = 183.0f;  // 180.0f → 183.0f に調整（6列で1100.0f幅に収まるように）
    const float CARD_SPACING_Y = 150.0f;
  };

  /// @brief 詳細パネル情報
  struct DetailsPanelInfo {
    float x = 1220.0f;
    float y = 160.0f;
    float width = 590.0f;
    float height = 745.0f;
    float padding = 30.0f;
    float line_height = 45.0f;
    int font_size = 32;

    const entities::Character *displayed_character = nullptr;
  };

  // ========== メンバ変数 ==========
  BaseSystemAPI *systemAPI_;
  bool isInitialized_;
  mutable bool requestClose_;
  mutable bool hasTransitionRequest_;
  mutable GameState requestedNextState_;

  // 編成スロット（10個）
  SquadSlot squad_slots_[10];
  PartySummaryInfo m_partySummary;
  CharacterListView m_characterList;
  DetailsPanelInfo m_detailsPanel;

  // 選択中のキャラクター（ホバー/ドラッグ中など）
  const entities::Character *selected_character_;

  // ドラッグ&ドロップ状態
  const entities::Character *dragging_character_;
  int dragging_source_slot_; // -1 = キャラクター一覧から, 0-9 = スロットから
  Vec2 drag_position_;
  bool is_dragging_;
  Vec2 drag_start_pos_;
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

  // SharedContextの編成を一度だけ復元するためのフラグ
  bool restored_from_context_ = false;
  bool formation_dirty_ = false;

  // ソート関連
  enum class SortKey { Name, Rarity, Cost, Level, Owned };
  SortKey currentSortKey_ = SortKey::Owned;
  bool sortAscending_ = false;

  // ========== プライベートメソッド ==========

  // 初期化・クリーンアップ
  void InitializeSlots();
  void RestoreFormationFromContext(SharedContext &ctx);
  void SaveFormationToContext(SharedContext &ctx);
  void FilterAvailableCharacters(SharedContext &ctx);
  void SortAvailableCharacters(const GameplayDataAPI *gameplayDataAPI);

  // 描画メソッド
  void RenderSortUI();
  void RenderSquadSlots();
  void RenderSlot(const SquadSlot &slot);
  void RenderResetButton();
  void RenderPartySummary();
  void RenderCharacterList(SharedContext &ctx);
  void RenderCharacterCard(const entities::Character *character, int card_index,
                           SharedContext &ctx);
  void RenderButtons();
  void RenderDividers();
  void RenderDraggingCharacter();
  void RenderDetailsPanel(SharedContext &ctx);

  // 座標計算・ヘルパー
  static Vec2 GetSlotPosition(int slot_id);
  Vec2 GetCardPosition(int card_index) const;
  int GetSlotAtPosition(Vec2 position) const;
  int GetCardAtPosition(Vec2 position) const;

  // キャラクター管理
  void AssignCharacter(int slot_id, const entities::Character *character,
                       SharedContext &ctx);
  void RemoveCharacter(int slot_id);
  void SwapCharacters(int slot1_id, int slot2_id);

  // パーティー管理
  void UpdatePartySummary();
  bool ValidateSquadComposition();

  // イベント処理
  void OnSlotClicked(int slot_id);
  void OnCardClicked(int card_index);
  void OnSlotRightClicked(int slot_id);
  void OnDragStart(int source_slot, const entities::Character *character,
                   SharedContext &ctx);
  void OnDragUpdate(Vec2 mouse_pos);
  void OnDragEnd(Vec2 mouse_pos, SharedContext &ctx);
  void OnButtonClicked(const std::string &button_name, SharedContext &ctx);

  // マウス入力処理
  void ProcessMouseInput(SharedContext &ctx);
  void UpdateHoverStates(Vec2 mouse_pos, SharedContext &ctx);

  // キーボード入力処理
  void ProcessKeyboardInput(SharedContext &ctx);

  // スクロール処理
  void ProcessScrollInput(float wheel_delta);

  // ユーティリティ
  bool IsCharacterInSquad(const entities::Character *character) const;
  Color GetSlotColor(const SquadSlot &slot) const;
  Color GetPartySummaryColor() const;
};

} // namespace core
} // namespace game
