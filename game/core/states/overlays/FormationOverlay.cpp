#include "FormationOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../ecs/entities/Character.hpp"
#include "../../api/GameplayDataAPI.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UIEffects.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

namespace game {
namespace core {

// ========== コンストラクタ ==========

FormationOverlay::FormationOverlay()
    : systemAPI_(nullptr), isInitialized_(false), requestClose_(false),
      hasTransitionRequest_(false), requestedNextState_(GameState::Title),
      dragging_character_(nullptr), dragging_source_slot_(-1),
      drag_position_({0.0f, 0.0f}), is_dragging_(false),
      drag_start_pos_({0.0f, 0.0f}), drag_started_(false),
      selected_slot_index_(-1), selected_character_(nullptr),
      animation_time_(0.0f), restored_from_context_(false),
      formation_dirty_(false) {
  InitializeSlots();
}

// ========== IOverlay 実裁E==========

bool FormationOverlay::Initialize(BaseSystemAPI *systemAPI, UISystemAPI* uiAPI) {
  if (isInitialized_) {
    LOG_ERROR("FormationOverlay already initialized");
    return false;
  }

  if (!systemAPI) {
    LOG_ERROR("FormationOverlay: systemAPI is null");
    return false;
  }

  systemAPI_ = systemAPI;
  requestClose_ = false;
  hasTransitionRequest_ = false;

  // スロチE��初期匁E
  InitializeSlots();
  restored_from_context_ = false;
  formation_dirty_ = false;

  // ドラチE��状態リセチE��
  dragging_character_ = nullptr;
  dragging_source_slot_ = -1;
  is_dragging_ = false;
  drag_start_pos_ = {0.0f, 0.0f};
  drag_started_ = false;
  selected_slot_index_ = -1;
  selected_character_ = nullptr;

  // ボタン状態リセチE��
  complete_button_ = ButtonState();
  cancel_button_ = ButtonState();
  reset_button_ = ButtonState();

  isInitialized_ = true;
  LOG_INFO("FormationOverlay initialized");
  return true;
}

void FormationOverlay::Update(SharedContext &ctx, float deltaTime) {
  if (!isInitialized_) {
    return;
  }

  // 既存�ESharedContext編成を最初�E1回だけ復允E
  if (!restored_from_context_) {
    RestoreFormationFromContext(ctx);
    restored_from_context_ = true;
  }

  // アニメーション時間更新
  animation_time_ += deltaTime;

  // キャラクター一覧の更新�E��E回�Eみ�E�E
  if (m_characterList.available_characters.empty() && ctx.gameplayDataAPI) {
    FilterAvailableCharacters(ctx);
  }

  // ESCキーで閉じめE
  if (ctx.inputAPI && ctx.inputAPI->IsEscapePressed()) { // ESC key
    requestClose_ = true;
  }

  // マウス入力�E琁E
  ProcessMouseInput(ctx);

  // キーボ�Eド�E力�E琁E
  ProcessKeyboardInput(ctx);

  // スクロール処琁E
  float wheel_delta = ctx.inputAPI ? ctx.inputAPI->GetMouseWheelMove() : 0.0f;
  if (wheel_delta != 0.0f) {
    ProcessScrollInput(wheel_delta);
  }

  // パ�EチE��ーサマリー更新
  UpdatePartySummary();

  if (formation_dirty_) {
    SaveFormationToContext(ctx);
    formation_dirty_ = false;
  }
}

void FormationOverlay::Render(SharedContext &ctx) {
  if (!isInitialized_) {
    return;
  }

  using namespace ui;

  // オーバ�Eレイ背景�E�グラチE�Eション�E�E
  UIEffects::DrawGradientPanel(systemAPI_, 100.0f, 90.0f, 1720.0f, 900.0f);

  // 背景粒子エフェクチE
  UIEffects::DrawParticles(systemAPI_, animation_time_, 100.0f, 90.0f, 1720.0f,
                           900.0f, 15);

  // タイトルバ�E�E�最初に描画して確実に上に表示�E�E

  // 区刁E��緁E
  RenderDividers();

  // 編成スロチE��
  RenderSquadSlots();

  // パ�EチE��ーサマリーパネル
  RenderPartySummary();

  // 詳細パネル
  RenderDetailsPanel(ctx);

  // キャラクター一覧
  RenderCharacterList(ctx);

  // ドラチE��中のキャラクター
  if (is_dragging_ && dragging_character_) {
    RenderDraggingCharacter();
  }
}

void FormationOverlay::Shutdown() {
  if (!isInitialized_) {
    return;
  }

  // スロチE��クリア
  for (int i = 0; i < 10; ++i) {
    squad_slots_[i].assigned_character = nullptr;
  }

  m_characterList.available_characters.clear();
  dragging_character_ = nullptr;

  isInitialized_ = false;
  systemAPI_ = nullptr;
  LOG_INFO("FormationOverlay shutdown");
}

bool FormationOverlay::RequestClose() const {
  if (requestClose_) {
    requestClose_ = false;
    return true;
  }
  return false;
}

bool FormationOverlay::RequestTransition(GameState &nextState) const {
  if (hasTransitionRequest_) {
    nextState = requestedNextState_;
    hasTransitionRequest_ = false;
    return true;
  }
  return false;
}

// ========== 初期化�EクリーンアチE�E ==========

void FormationOverlay::InitializeSlots() {
  for (int i = 0; i < 10; ++i) {
    squad_slots_[i].slot_id = i;
    squad_slots_[i].assigned_character = nullptr;
    squad_slots_[i].position = GetSlotPosition(i);
    squad_slots_[i].is_hovered = false;
    squad_slots_[i].is_dragging = false;
  }
}

void FormationOverlay::RestoreFormationFromContext(SharedContext& ctx) {
  // 一旦クリア
  for (int i = 0; i < 10; ++i) {
    squad_slots_[i].assigned_character = nullptr;
  }

  if (!ctx.gameplayDataAPI) {
    return;
  }

  if (ctx.formationData.IsEmpty()) {
    return;
  }

  const auto& masters = ctx.gameplayDataAPI->GetAllCharacterMasters();
  for (const auto& s : ctx.formationData.slots) {
    const int slotId = s.first;
    const std::string& characterId = s.second;
    if (slotId < 0 || slotId >= 10) continue;
    if (characterId.empty()) continue;

    auto it = masters.find(characterId);
    if (it == masters.end()) continue;

    squad_slots_[slotId].assigned_character = &it->second;
  }

  LOG_INFO("FormationOverlay: Restored formation from SharedContext: {} slots", ctx.formationData.slots.size());

  formation_dirty_ = false;
}

void FormationOverlay::SaveFormationToContext(SharedContext& ctx) {
  ctx.formationData.Clear();
  for (int i = 0; i < 10; ++i) {
    const auto* ch = squad_slots_[i].assigned_character;
    if (!ch) {
      continue;
    }
    ctx.formationData.slots.emplace_back(i, ch->id);
  }
}

void FormationOverlay::FilterAvailableCharacters(SharedContext &ctx) {
  if (!ctx.gameplayDataAPI) {
    return;
  }

  m_characterList.available_characters.clear();

  // 全キャラクターを表示対象に（ロック状態は表示時に反映）
  const auto &masters = ctx.gameplayDataAPI->GetAllCharacterMasters();
  for (const auto &[id, ch] : masters) {
    m_characterList.available_characters.push_back(&ch);
  }

  SortAvailableCharacters(ctx.gameplayDataAPI);

  LOG_INFO("FormationOverlay: Loaded {} available characters",
           m_characterList.available_characters.size());
}

void FormationOverlay::SortAvailableCharacters(const GameplayDataAPI* gameplayDataAPI) {
  std::sort(m_characterList.available_characters.begin(),
            m_characterList.available_characters.end(),
            [this, gameplayDataAPI](const entities::Character *a,
                                    const entities::Character *b) {
              if (!a || !b)
                return false;

              auto cmpInt = [this](int lhs, int rhs) {
                return sortAscending_ ? (lhs < rhs) : (lhs > rhs);
              };
              auto cmpStr = [this](const std::string& lhs, const std::string& rhs) {
                return sortAscending_ ? (lhs < rhs) : (rhs < lhs);
              };

              switch (currentSortKey_) {
                case SortKey::Name:
                  if (a->name != b->name) return cmpStr(a->name, b->name);
                  break;
                case SortKey::Rarity:
                  if (a->rarity != b->rarity) return cmpInt(a->rarity, b->rarity);
                  break;
                case SortKey::Cost:
                  if (a->cost != b->cost) return cmpInt(a->cost, b->cost);
                  break;
                case SortKey::Level: {
                  int levelA = 1;
                  int levelB = 1;
                  if (gameplayDataAPI) {
                    levelA = gameplayDataAPI->GetCharacterState(a->id).level;
                    levelB = gameplayDataAPI->GetCharacterState(b->id).level;
                  }
                  if (levelA != levelB) return cmpInt(levelA, levelB);
                  break;
                }
                case SortKey::Owned: {
                  bool ownedA = true;
                  bool ownedB = true;
                  if (gameplayDataAPI) {
                    ownedA = gameplayDataAPI->GetCharacterState(a->id).unlocked;
                    ownedB = gameplayDataAPI->GetCharacterState(b->id).unlocked;
                  }
                  if (ownedA != ownedB) {
                    // 所持している方を先に（降順の場合はtrue > false）
                    return sortAscending_ ? (!ownedA && ownedB) : (ownedA && !ownedB);
                  }
                  break;
                }
              }
              // タイブレーカー
              if (a->rarity != b->rarity) return a->rarity > b->rarity;
              if (a->cost != b->cost) return a->cost < b->cost;
              return a->name < b->name;
            });
}

// ========== 描画メソチE�� ==========

  // タイトルバ�E背景�E�タブバーの下に配置�E�E

  // タイトル


  void FormationOverlay::RenderSortUI() {
  using namespace ui;

  // ユニット一覧のヘッダ位置（list_y=535.0fの上）
  const float sort_x = 100.0f;
  const float sort_y = 470.0f;
  const float sort_w = 1100.0f;
  const float sort_bar_h = 36.0f;
  const float sort_bar_y = sort_y;

  // ソートラベル
  systemAPI_->Render().DrawTextDefault(
      "ソート", sort_x, sort_bar_y - 26.0f, 24.0f, OverlayColors::TEXT_GOLD);
  systemAPI_->Render().DrawRectangle(
      sort_x, sort_bar_y, sort_w, sort_bar_h, OverlayColors::PANEL_BG_SECONDARY);
  systemAPI_->Render().DrawRectangleLines(
      sort_x, sort_bar_y, sort_w, sort_bar_h, 2.0f,
      OverlayColors::BORDER_DEFAULT);

  auto sortKeyLabel = [](SortKey k) -> const char* {
    switch (k) {
      case SortKey::Name: return "名前";
      case SortKey::Rarity: return "レア";
      case SortKey::Cost: return "コスト";
      case SortKey::Level: return "レベル";
      case SortKey::Owned: return "所持";
      default: return "SORT";
    }
  };

  float btn_h = sort_bar_h - 8.0f;
  float sort_btn_y = sort_bar_y + 4.0f;
  float btn_gap = 8.0f;
  float toggle_w = 90.0f;
  float btn_w = (sort_w - toggle_w - btn_gap * 6.0f) / 5.0f;

  SortKey keys[5] = {
      SortKey::Name,
      SortKey::Rarity,
      SortKey::Cost,
      SortKey::Level,
      SortKey::Owned
  };

  for (int i = 0; i < 5; ++i) {
    float x = sort_x + btn_gap + i * (btn_w + btn_gap);
    bool active = (currentSortKey_ == keys[i]);
    systemAPI_->Render().DrawRectangle(
        x, sort_btn_y, btn_w, btn_h,
        active ? OverlayColors::CARD_BG_SELECTED : OverlayColors::CARD_BG_NORMAL);
    systemAPI_->Render().DrawRectangleLines(
        x, sort_btn_y, btn_w, btn_h, active ? 3.0f : 2.0f,
        active ? OverlayColors::BORDER_GOLD : OverlayColors::BORDER_DEFAULT);
    Vector2 ts = systemAPI_->Render().MeasureTextDefault(
        sortKeyLabel(keys[i]), 24.0f);
    systemAPI_->Render().DrawTextDefault(
        sortKeyLabel(keys[i]), x + (btn_w - ts.x) / 2.0f,
        sort_btn_y + (btn_h - ts.y) / 2.0f, 24.0f,
        OverlayColors::TEXT_PRIMARY);
  }

  // 昇順/降順トグル
  const float toggle_x = sort_x + sort_w - toggle_w - btn_gap;
  const bool asc = sortAscending_;
  systemAPI_->Render().DrawRectangle(
      toggle_x, sort_btn_y, toggle_w, btn_h, OverlayColors::CARD_BG_NORMAL);
  systemAPI_->Render().DrawRectangleLines(
      toggle_x, sort_btn_y, toggle_w, btn_h, 2.0f,
      OverlayColors::BORDER_DEFAULT);
  // テキストを中央揃えに変更
  const char* toggle_text = asc ? "↑昇順" : "↓降順";
  Vector2 toggle_text_size = systemAPI_->Render().MeasureTextDefault(toggle_text, 24.0f);
  systemAPI_->Render().DrawTextDefault(
      toggle_text, toggle_x + (toggle_w - toggle_text_size.x) / 2.0f,
      sort_btn_y + (btn_h - toggle_text_size.y) / 2.0f, 24.0f,
      OverlayColors::TEXT_SECONDARY);

}

void FormationOverlay::RenderDividers() {
  // 垂直区刁E��線（左側エリアと詳細パネルの間！E
  systemAPI_->Render().DrawRectangle(1205.0f, 155.0f, 3.0f, 825.0f,
                            ui::OverlayColors::DIVIDER);
  // 水平区刁E��線（編成スロチE��とソートUIの間）
  systemAPI_->Render().DrawRectangle(100.0f, 450.0f, 1100.0f, 2.0f,
                            ui::OverlayColors::DIVIDER);
  // 水平区刁E��線（ソートUIとキャラクター一覧の間）
  systemAPI_->Render().DrawRectangle(100.0f, 508.0f, 1100.0f, 2.0f,
                            ui::OverlayColors::DIVIDER);
}

void FormationOverlay::RenderSquadSlots() {
  for (int i = 0; i < 10; ++i) {
    RenderSlot(squad_slots_[i]);
  }
}

void FormationOverlay::RenderSlot(const SquadSlot &slot) {
  using namespace ui;

  Color bg_color = GetSlotColor(slot);
  bool is_selected = (slot.assigned_character != nullptr);

  // 立体カード描画�E�影 + 冁E�E光沢�E�E
  UIEffects::DrawCard3D(systemAPI_, slot.position.x, slot.position.y,
                        slot.width, slot.height, bg_color, is_selected,
                        slot.is_hovered);

  if (slot.assigned_character) {
    const entities::Character *ch = slot.assigned_character;

    // portrait を薄く背景に敷く（誰が誰か判別しやすくする�E�E
    if (!ch->icon_path.empty()) {
      void *texturePtr = systemAPI_->Resource().GetTexture(ch->icon_path);
      if (texturePtr) {
        Texture2D *texture = static_cast<Texture2D *>(texturePtr);
        if (texture && texture->id != 0) {
          Rectangle src{0.0f, 0.0f, static_cast<float>(texture->width),
                        static_cast<float>(texture->height)};
          Rectangle dst{slot.position.x, slot.position.y, slot.width,
                        slot.height};
          // 未選択時に編成に含まれているいる場合は不透明度を上げる
          Color tint{255, 255, 255, static_cast<unsigned char>(slot.is_hovered ? 70 : 120)};
          systemAPI_->Render().DrawTexturePro(*texture, src, dst, {0.0f, 0.0f},
                                              0.0f, tint);
        }
      }
    }

    systemAPI_->Render().DrawTextDefault(
        ch->name, slot.position.x + 10.0f, slot.position.y + 8.0f, 26.0f,
                                OverlayColors::TEXT_PRIMARY);

    std::string rarity_str = "";
    for (int i = 0; i < ch->rarity; ++i)
      rarity_str += "★";
    systemAPI_->Render().DrawTextDefault(
        rarity_str, slot.position.x + 10.0f, slot.position.y + 38.0f, 22.0f,
                                OverlayColors::TEXT_GOLD);

    std::string cost_str = "COST " + std::to_string(ch->cost);
    Vector2 cost_size = systemAPI_->Render().MeasureTextDefault(cost_str, 24.0f);
    systemAPI_->Render().DrawTextDefault(
        cost_str, slot.position.x + slot.width - cost_size.x - 10.0f,
        slot.position.y + slot.height - 28.0f, 24.0f,
        OverlayColors::TEXT_ACCENT);
  } else {
    systemAPI_->Render().DrawTextDefault(
        "Empty", slot.position.x + slot.width / 2.0f - 30.0f,
        slot.position.y + slot.height / 2.0f - 10.0f, 20.0f,
        OverlayColors::TEXT_DISABLED);
  }
}

void FormationOverlay::RenderResetButton() {
  using namespace ui;

  // スロットエリアの下部に配置（最後のスロットの下）
  // スロットは最大2行（0-4が1行目、5-9が2行目）
  // 2行目のスロットの下に配置
  float slot_bottom_y = 0.0f;
  for (int i = 5; i < 10; ++i) {
    float slot_y = squad_slots_[i].position.y + squad_slots_[i].height;
    if (slot_y > slot_bottom_y) {
      slot_bottom_y = slot_y;
    }
  }
  
  float button_y = slot_bottom_y + 20.0f;  // スロットの下に20pxの余白
  float button_h = 50.0f;
  float button_w = 180.0f;
  float button_x = 170.0f + 2.0f * 200.0f;  // 3列目（インデックス2）の位置に配置

  UIEffects::DrawModernButton(systemAPI_, button_x, button_y, button_w, button_h,
                              OverlayColors::BUTTON_RESET,
                              OverlayColors::CARD_BG_SELECTED,
                              reset_button_.is_hovered, false);
  Vector2 reset_text_size =
      systemAPI_->Render().MeasureTextDefault("リセット", 28.0f);
  systemAPI_->Render().DrawTextDefault(
      "リセット", button_x + (button_w - reset_text_size.x) / 2.0f,
      button_y + (button_h - reset_text_size.y) / 2.0f, 28.0f,
      OverlayColors::TEXT_PRIMARY);
}

void FormationOverlay::RenderPartySummary() {
  // 編成サマリーの数値表示は廃止し、ソートUIの利用領域を優先
}

void FormationOverlay::RenderCharacterList(SharedContext& ctx) {
  float list_x = 100.0f;
  float list_y = 510.0f;
  float list_w = 1100.0f;
  float list_h = 450.0f;

  // ソートUIをユニット一覧のヘッダに表示
  RenderSortUI();

  systemAPI_->Render().DrawRectangle(list_x, list_y, list_w, list_h,
                            ui::OverlayColors::PANEL_BG);

  int start_row = m_characterList.scroll_offset;
  int start_index = start_row * m_characterList.visible_columns;
  int max_visible =
      m_characterList.visible_rows * m_characterList.visible_columns;
  int end_index =
      std::min(start_index + max_visible,
               static_cast<int>(m_characterList.available_characters.size()));

  for (int i = start_index; i < end_index; ++i) {
    RenderCharacterCard(m_characterList.available_characters[i],
                        i - start_index, ctx);
  }

  int total_rows =
      (static_cast<int>(m_characterList.available_characters.size()) +
       m_characterList.visible_columns - 1) /
      m_characterList.visible_columns;
  if (total_rows > m_characterList.visible_rows) {
    float scrollbar_x = list_x + list_w - 15.0f;
    float scrollbar_y = list_y;
    float scrollbar_w = 12.0f;
    float scrollbar_h = list_h;
    systemAPI_->Render().DrawRectangle(
        scrollbar_x, scrollbar_y, scrollbar_w, scrollbar_h,
        ui::OverlayColors::PANEL_BG_DARK);

    float thumb_height =
        scrollbar_h *
        (static_cast<float>(m_characterList.visible_rows) / total_rows);
    float thumb_y =
        scrollbar_y + (scrollbar_h - thumb_height) *
                          (static_cast<float>(m_characterList.scroll_offset) /
                           (total_rows - m_characterList.visible_rows));
    systemAPI_->Render().DrawRectangle(
        scrollbar_x, thumb_y, scrollbar_w, thumb_height,
                              ui::OverlayColors::BORDER_DEFAULT);
  }
}

void FormationOverlay::RenderCharacterCard(const entities::Character *character,
                                           int card_index, SharedContext& ctx) {
  if (!character)
    return;

  using namespace ui;

  // ロック状態を取得
  bool is_locked = false;
  if (ctx.gameplayDataAPI) {
    const auto st = ctx.gameplayDataAPI->GetCharacterState(character->id);
    is_locked = !st.unlocked;
  }

  Vec2 pos = GetCardPosition(card_index);
  bool is_in_squad = IsCharacterInSquad(character);
  bool is_selected = (selected_character_ == character);
  bool is_hovered = (is_dragging_ && dragging_character_ == character);

  Color bg_color = is_in_squad ? OverlayColors::SLOT_ASSIGNED
                               : OverlayColors::CARD_BG_NORMAL;

  // ロックされたキャラは半透明で描画
  if (is_locked) {
    bg_color.a = static_cast<unsigned char>(bg_color.a * 0.5f);
  }

  // 立体カード描画
  UIEffects::DrawCard3D(systemAPI_, pos.x, pos.y, m_characterList.CARD_WIDTH,
                        m_characterList.CARD_HEIGHT, bg_color, is_selected,
                        is_hovered);

  // portrait を薄く背景に敷く（誰が誰か判別しやすくする）
  if (!is_locked && !character->icon_path.empty()) {
    void *texturePtr = systemAPI_->Resource().GetTexture(character->icon_path);
    if (texturePtr) {
      Texture2D *texture = static_cast<Texture2D *>(texturePtr);
      if (texture && texture->id != 0) {
        Rectangle src{0.0f, 0.0f, static_cast<float>(texture->width),
                      static_cast<float>(texture->height)};
        Rectangle dst{pos.x, pos.y, m_characterList.CARD_WIDTH,
                      m_characterList.CARD_HEIGHT};
        // 未選択時に編成に含まれている場合は不透明度を上げる
        int alpha = is_in_squad ? 70 : 120;
        Color tint{255, 255, 255, static_cast<unsigned char>(alpha)};
        systemAPI_->Render().DrawTexturePro(*texture, src, dst, {0.0f, 0.0f},
                                            0.0f, tint);
      }
    }
  }

  // 発光効果�Eーダー�E�選択時�E�E
  if (is_selected && !is_locked) {
    float pulse_alpha = UIEffects::CalculatePulseAlpha(animation_time_);
    UIEffects::DrawGlowingBorder(
        systemAPI_, pos.x, pos.y, m_characterList.CARD_WIDTH,
        m_characterList.CARD_HEIGHT, pulse_alpha, is_hovered);
  }

  if (is_locked) {
    const char *locked_text = "未所有";
    Vector2 label_size =
        systemAPI_->Render().MeasureTextDefault(locked_text, 26.0f);
    systemAPI_->Render().DrawTextDefault(
        locked_text,
        pos.x + (m_characterList.CARD_WIDTH - label_size.x) / 2.0f,
        pos.y + (m_characterList.CARD_HEIGHT - label_size.y) / 2.0f,
        26.0f, OverlayColors::TEXT_MUTED);
  } else {
    Color text_color = OverlayColors::TEXT_PRIMARY;
    if (is_in_squad) {
      text_color = OverlayColors::TEXT_DISABLED;
    }

    systemAPI_->Render().DrawTextDefault(character->name, pos.x + 5.0f,
                                         pos.y + 5.0f, 28.0f, text_color);

    // レア度を右下に表示（★マーク）
    std::string rarity_stars = "";
    for (int i = 0; i < character->rarity; ++i)
      rarity_stars += "★";
    Vector2 rarity_size = systemAPI_->Render().MeasureTextDefault(rarity_stars, 28.0f);
    systemAPI_->Render().DrawTextDefault(
        rarity_stars, 
        pos.x + m_characterList.CARD_WIDTH - rarity_size.x - 5.0f,
        pos.y + m_characterList.CARD_HEIGHT - 30.0f, 28.0f,
        OverlayColors::TEXT_GOLD);

    // コストは左下に表示（変更なし）
    systemAPI_->Render().DrawTextDefault(
        "C " + std::to_string(character->cost), pos.x + 5.0f,
        pos.y + m_characterList.CARD_HEIGHT - 30.0f, 28.0f,
        OverlayColors::TEXT_ACCENT);
  }
}

void FormationOverlay::RenderButtons() {
  // 下部の3つのボタンは削除（空実装）
}

void FormationOverlay::RenderDetailsPanel(SharedContext& ctx) {
  float panel_x = m_detailsPanel.x;
  float panel_y = m_detailsPanel.y;
  float panel_width = m_detailsPanel.width;
  float panel_height = m_detailsPanel.height;

  systemAPI_->Render().DrawRectangle(panel_x, panel_y, panel_width,
                                     panel_height,
                            ui::OverlayColors::PANEL_BG_BROWN);
  systemAPI_->Render().DrawRectangleLines(
      panel_x, panel_y, panel_width, panel_height, 2.0f,
      ui::OverlayColors::BORDER_GOLD);

  const entities::Character *display_char = is_dragging_ && dragging_character_
                                                ? dragging_character_
                                                : selected_character_;
  if (!display_char) {
    systemAPI_->Render().DrawTextDefault(
        "キャラクターを選択してください", panel_x + m_detailsPanel.padding,
        panel_y + panel_height / 2.0f - 18.0f, 32.0f,
        ui::OverlayColors::TEXT_DISABLED);
    return;
  }

  // ロック状態のキャラクターの場合は詳細を表示しない
  bool is_locked = false;
  if (ctx.gameplayDataAPI) {
    is_locked = !ctx.gameplayDataAPI->GetCharacterState(display_char->id).unlocked;
  }
  if (is_locked) {
    systemAPI_->Render().DrawTextDefault(
        "キャラクターを選択してください", panel_x + m_detailsPanel.padding,
        panel_y + panel_height / 2.0f - 18.0f, 32.0f,
        ui::OverlayColors::TEXT_DISABLED);
    return;
  }

  float x = panel_x + m_detailsPanel.padding;
  float y = panel_y + m_detailsPanel.padding;
  systemAPI_->Render().DrawTextDefault(display_char->name, x, y, 42.0f,
                              ui::OverlayColors::TEXT_PRIMARY);

  std::string rarity_str =
      "Rarity: " + (display_char->rarity_name.empty()
                        ? std::to_string(display_char->rarity)
                        : display_char->rarity_name);
  systemAPI_->Render().DrawTextDefault(rarity_str, x, y + 45.0f, 28.0f,
                              ui::OverlayColors::TEXT_GOLD);
  systemAPI_->Render().DrawRectangle(
      x, y + 80.0f, panel_width - m_detailsPanel.padding * 2.0f, 2.0f,
                            ui::OverlayColors::DIVIDER);

  float stats_y = y + 110.0f;
  auto attackTypeToString = [](entities::AttackType type) -> std::string {
    if (type == entities::AttackType::Single)
      return "単体";
    if (type == entities::AttackType::Range)
      return "範囲";
    if (type == entities::AttackType::Line)
      return "直線";
    return "不明";
  };
  auto effectTypeToString = [](entities::EffectType type) -> std::string {
    if (type == entities::EffectType::Normal)
      return "通常";
    if (type == entities::EffectType::Fire)
      return "炎";
    if (type == entities::EffectType::Ice)
      return "氷";
    if (type == entities::EffectType::Lightning)
      return "雷";
    if (type == entities::EffectType::Heal)
      return "回復";
    return "不明";
  };

  std::ostringstream frequencyText;
  const float attackSpan = display_char->attack_span;
  const float frequency = (attackSpan > 0.0f) ? (1.0f / attackSpan) : 0.0f;
  frequencyText << std::fixed << std::setprecision(2) << frequency << "回/秒";

  int level = 1;
  if (ctx.gameplayDataAPI) {
    level = ctx.gameplayDataAPI->GetCharacterState(display_char->id).level;
  }

  std::vector<std::pair<std::string, std::string>> stats = {
      {"Level", std::to_string(level)},
      {"HP", std::to_string(display_char->hp)},
      {"Attack", std::to_string(display_char->attack)},
      {"Defense", std::to_string(display_char->defense)},
      {"Speed", std::to_string(static_cast<int>(display_char->move_speed))},
      {"攻撃速度", frequencyText.str()},
      {"Cost", std::to_string(display_char->cost)},
      {"Type", attackTypeToString(display_char->attack_type)},
      {"Element", effectTypeToString(display_char->effect_type)}};

  for (size_t i = 0; i < stats.size(); ++i) {
    float line_y = stats_y + static_cast<float>(i) * m_detailsPanel.line_height;
    systemAPI_->Render().DrawTextDefault(
        stats[i].first, x, line_y,
                                static_cast<float>(m_detailsPanel.font_size),
                                ui::OverlayColors::TEXT_SECONDARY);
    Vector2 text_size = systemAPI_->Render().MeasureTextDefault(
        stats[i].second, static_cast<float>(m_detailsPanel.font_size));
    systemAPI_->Render().DrawTextDefault(
        stats[i].second,
        x + panel_width - m_detailsPanel.padding * 2.0f - text_size.x, line_y,
        static_cast<float>(m_detailsPanel.font_size),
        ui::OverlayColors::TEXT_PRIMARY);
  }

  if (!display_char->description.empty()) {
    float desc_y =
        stats_y +
        static_cast<float>(stats.size()) * m_detailsPanel.line_height + 20.0f;
    systemAPI_->Render().DrawRectangle(
        x, desc_y - 10.0f, panel_width - m_detailsPanel.padding * 2.0f, 1.0f,
                              ui::OverlayColors::DIVIDER);
    systemAPI_->Render().DrawTextDefault(
        display_char->description, x, desc_y, 26.0f,
                                ui::OverlayColors::TEXT_SECONDARY);
  }
}

void FormationOverlay::RenderDraggingCharacter() {
  if (!dragging_character_)
    return;
  float card_w = m_characterList.CARD_WIDTH;
  float card_h = m_characterList.CARD_HEIGHT;
  float card_x = drag_position_.x - card_w / 2.0f;
  float card_y = drag_position_.y - card_h / 2.0f;
  Rectangle rec = {card_x, card_y, card_w, card_h};

  Color drag_bg = ui::OverlayColors::SLOT_EMPTY;
  drag_bg.a = 180;
  systemAPI_->Render().DrawRectangleRounded(rec, 0.1f, 8, drag_bg);
  Color drag_border = ui::OverlayColors::BORDER_HOVER;
  drag_border.a = 180;
  systemAPI_->Render().DrawRectangleRoundedLines(rec, 0.1f, 8, drag_border);

  Color nameColor = ui::OverlayColors::TEXT_PRIMARY;
  nameColor.a = 180;
  systemAPI_->Render().DrawTextDefault(dragging_character_->name, card_x + 5.0f,
                              card_y + 5.0f, 14.0f, nameColor);

  Color costColor = ui::OverlayColors::TEXT_ACCENT;
  costColor.a = 180;
  systemAPI_->Render().DrawTextDefault(
      "C " + std::to_string(dragging_character_->cost), card_x + 5.0f,
      card_y + card_h - 20.0f, 14.0f, costColor);
}

// ========== 座標計算�Eルパ�E ==========

Vec2 FormationOverlay::GetSlotPosition(int slot_id) {
  int row = slot_id / 5;
  int col = slot_id % 5;
  // 下部ボタンを削除した分、スペースを拡大
  float start_x = 170.0f;
  float start_y = 150.0f;  // 少し上に移動
  float spacing_x = 200.0f;  // 190.0f → 200.0f
  float spacing_y = 170.0f;  // 150.0f → 170.0f
  return {start_x + col * spacing_x, start_y + row * spacing_y};
}

Vec2 FormationOverlay::GetCardPosition(int card_index) const {
  int row = card_index / m_characterList.visible_columns;
  int col = card_index % m_characterList.visible_columns;
  return {120.0f + col * m_characterList.CARD_SPACING_X,
          530.0f + row * m_characterList.CARD_SPACING_Y};
}

int FormationOverlay::GetSlotAtPosition(Vec2 position) const {
  for (int i = 0; i < 10; ++i) {
    const SquadSlot &slot = squad_slots_[i];
    if (position.x >= slot.position.x &&
        position.x < slot.position.x + slot.width &&
        position.y >= slot.position.y &&
        position.y < slot.position.y + slot.height) {
      return i;
    }
  }
  return -1;
}

int FormationOverlay::GetCardAtPosition(Vec2 position) const {
  if (position.x < 100.0f || position.x >= 1200.0f || position.y < 510.0f ||
      position.y >= 960.0f)
    return -1;

  int start_row = m_characterList.scroll_offset;
  int start_index = start_row * m_characterList.visible_columns;
  int end_index =
      std::min(start_index + m_characterList.visible_rows *
                                 m_characterList.visible_columns,
               static_cast<int>(m_characterList.available_characters.size()));

  for (int i = start_index; i < end_index; ++i) {
    Vec2 card_pos = GetCardPosition(i - start_index);
    if (position.x >= card_pos.x &&
        position.x < card_pos.x + m_characterList.CARD_WIDTH &&
        position.y >= card_pos.y &&
        position.y < card_pos.y + m_characterList.CARD_HEIGHT) {
      return i;
    }
  }
  return -1;
}

// ========== キャラクター管琁E==========

void FormationOverlay::AssignCharacter(int slot_id,
                                       const entities::Character *character,
                                       SharedContext& ctx) {
  if (slot_id < 0 || slot_id >= 10 || !character)
    return;
  
  // ロックされたキャラは配置できない
  if (ctx.gameplayDataAPI) {
    const auto st = ctx.gameplayDataAPI->GetCharacterState(character->id);
    if (!st.unlocked) {
      return; // ロックされている場合は配置しない
    }
  }
  
  squad_slots_[slot_id].assigned_character = character;
  UpdatePartySummary();
  formation_dirty_ = true;
}

void FormationOverlay::RemoveCharacter(int slot_id) {
  if (slot_id < 0 || slot_id >= 10)
    return;
  squad_slots_[slot_id].assigned_character = nullptr;
  UpdatePartySummary();
  formation_dirty_ = true;
}

void FormationOverlay::SwapCharacters(int slot1_id, int slot2_id) {
  if (slot1_id < 0 || slot1_id >= 10 || slot2_id < 0 || slot2_id >= 10)
    return;
  const entities::Character *temp = squad_slots_[slot1_id].assigned_character;
  squad_slots_[slot1_id].assigned_character =
      squad_slots_[slot2_id].assigned_character;
  squad_slots_[slot2_id].assigned_character = temp;
  UpdatePartySummary();
  formation_dirty_ = true;
}

// ========== パ�EチE��ー管琁E==========

void FormationOverlay::UpdatePartySummary() {
  m_partySummary.total_cost = 0;
  m_partySummary.character_count = 0;
  m_partySummary.total_hp = 0;
  m_partySummary.total_attack = 0;
  m_partySummary.total_defense = 0;

  for (int i = 0; i < 10; ++i) {
    if (squad_slots_[i].assigned_character) {
      const auto *ch = squad_slots_[i].assigned_character;
      m_partySummary.total_cost += ch->cost;
      m_partySummary.total_hp += ch->hp;
      m_partySummary.total_attack += ch->attack;
      m_partySummary.total_defense += ch->defense;
      m_partySummary.character_count++;
    }
  }
}

bool FormationOverlay::ValidateSquadComposition() {
  return m_partySummary.character_count > 0;
}

// ========== イベント�E琁E==========

void FormationOverlay::OnSlotClicked(int slot_id) {}
void FormationOverlay::OnCardClicked(int card_index) {}

void FormationOverlay::OnSlotRightClicked(int slot_id) {
  if (slot_id >= 0 && slot_id < 10)
    RemoveCharacter(slot_id);
}

void FormationOverlay::OnDragStart(int source_slot,
                                   const entities::Character *character,
                                   SharedContext& ctx) {
  // ロックされたキャラはドラッグできない
  if (character && ctx.gameplayDataAPI) {
    const auto st = ctx.gameplayDataAPI->GetCharacterState(character->id);
    if (!st.unlocked) {
      return; // ロックされている場合はドラッグを開始しない
    }
  }

  dragging_character_ = character;
  dragging_source_slot_ = source_slot;
  is_dragging_ = true;
  drag_position_ = ctx.inputAPI ? ctx.inputAPI->GetMousePositionInternal()
                                : Vec2{0.0f, 0.0f};
  selected_character_ = character;
}

void FormationOverlay::OnDragUpdate(Vec2 mouse_pos) {
  if (is_dragging_)
    drag_position_ = mouse_pos;
}

void FormationOverlay::OnDragEnd(Vec2 mouse_pos, SharedContext& ctx) {
  if (!is_dragging_ || !dragging_character_)
    return;

  int target_slot = GetSlotAtPosition(mouse_pos);
  if (target_slot >= 0) {
    if (dragging_source_slot_ >= 0) {
      if (target_slot != dragging_source_slot_)
        SwapCharacters(dragging_source_slot_, target_slot);
    } else {
      AssignCharacter(target_slot, dragging_character_, ctx);
    }
  }

  dragging_character_ = nullptr;
  dragging_source_slot_ = -1;
  is_dragging_ = false;
  drag_started_ = false;
}

void FormationOverlay::OnButtonClicked(const std::string &button_name, SharedContext& ctx) {
  // "complete"と"cancel"の処理は削除（下部ボタン削除に伴い）
  if (button_name == "reset") {
    for (int i = 0; i < 10; ++i)
      RemoveCharacter(i);
  }
}

// ========== マウス入力�E琁E==========

void FormationOverlay::ProcessMouseInput(SharedContext &ctx) {
  auto mouse_pos = ctx.inputAPI ? ctx.inputAPI->GetMousePositionInternal()
                                : Vec2{0.0f, 0.0f};
  UpdateHoverStates(mouse_pos, ctx);

  if (ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
    // ソートUIクリック処理（ユニット一覧のヘッダ位置）
    float sort_x = 100.0f;
    float sort_y = 470.0f;
    float sort_w = 1100.0f;
    float sort_bar_h = 36.0f;
    float sort_bar_y = sort_y;
    float btn_h = sort_bar_h - 8.0f;
    float sort_btn_y = sort_bar_y + 4.0f;
    float btn_gap = 8.0f;
    float toggle_w = 90.0f;
    float btn_w = (sort_w - toggle_w - btn_gap * 6.0f) / 5.0f;

    if (mouse_pos.y >= sort_btn_y && mouse_pos.y < sort_btn_y + btn_h &&
        mouse_pos.x >= sort_x && mouse_pos.x < sort_x + sort_w) {
      // ソートキーボタン
      SortKey keys[5] = {
          SortKey::Name,
          SortKey::Rarity,
          SortKey::Cost,
          SortKey::Level,
          SortKey::Owned
      };
      for (int i = 0; i < 5; ++i) {
        float x = sort_x + btn_gap + i * (btn_w + btn_gap);
        if (mouse_pos.x >= x && mouse_pos.x < x + btn_w) {
          if (currentSortKey_ == keys[i]) {
            // 同じキーをクリックした場合は昇順/降順を切り替え
            sortAscending_ = !sortAscending_;
          } else {
            currentSortKey_ = keys[i];
            sortAscending_ = false; // デフォルトは降順
          }
          SortAvailableCharacters(ctx.gameplayDataAPI);
          return;
        }
      }
      // 昇順/降順トグル
      float toggle_x = sort_x + sort_w - toggle_w - btn_gap;
      if (mouse_pos.x >= toggle_x && mouse_pos.x < toggle_x + toggle_w) {
        sortAscending_ = !sortAscending_;
        SortAvailableCharacters(ctx.gameplayDataAPI);
        return;
      }
    }

    // 下部の3つのボタンは削除済み
    
    // リセットボタンのクリック処理（編成枠の下部）
    float slot_bottom_y = 0.0f;
    for (int i = 5; i < 10; ++i) {
      float slot_y = squad_slots_[i].position.y + squad_slots_[i].height;
      if (slot_y > slot_bottom_y) {
        slot_bottom_y = slot_y;
      }
    }
    float button_y = slot_bottom_y + 20.0f;
    float button_h = 50.0f;
    float button_w = 180.0f;
    float button_x = 170.0f + 2.0f * 200.0f;
    
    if (mouse_pos.x >= button_x && mouse_pos.x < button_x + button_w &&
        mouse_pos.y >= button_y && mouse_pos.y < button_y + button_h) {
      OnButtonClicked("reset", ctx);
    }
    
    drag_start_pos_ = mouse_pos;
    drag_started_ = true;
  } else if (ctx.inputAPI && ctx.inputAPI->IsLeftClickDown() && drag_started_ &&
             !is_dragging_) {
    float dx = mouse_pos.x - drag_start_pos_.x,
          dy = mouse_pos.y - drag_start_pos_.y;
    if (std::sqrt(dx * dx + dy * dy) > 3.0f) {
      int slot_id = GetSlotAtPosition(drag_start_pos_);
      if (slot_id >= 0 && squad_slots_[slot_id].assigned_character)
        OnDragStart(slot_id, squad_slots_[slot_id].assigned_character, ctx);
      else {
        int card_index = GetCardAtPosition(drag_start_pos_);
        if (card_index >= 0) {
          const entities::Character *ch =
              m_characterList.available_characters[card_index];
          if (!IsCharacterInSquad(ch))
            OnDragStart(-1, ch, ctx);
        }
      }
    }
  } else if (ctx.inputAPI && ctx.inputAPI->IsLeftClickReleased()) {
    if (is_dragging_)
      OnDragEnd(mouse_pos, ctx);
    drag_started_ = false;
  }

  if (is_dragging_)
    OnDragUpdate(mouse_pos);
  if (ctx.inputAPI && ctx.inputAPI->IsRightClickPressed()) {
    int slot_id = GetSlotAtPosition(mouse_pos);
    if (slot_id >= 0)
      OnSlotRightClicked(slot_id);
  }
}

void FormationOverlay::UpdateHoverStates(Vec2 mouse_pos, SharedContext &ctx) {
  for (int i = 0; i < 10; ++i)
    squad_slots_[i].is_hovered = (GetSlotAtPosition(mouse_pos) == i);

  if (!is_dragging_) {
    selected_character_ = nullptr;
    int hovered_slot = GetSlotAtPosition(mouse_pos);
    if (hovered_slot >= 0 && squad_slots_[hovered_slot].assigned_character) {
      selected_character_ = squad_slots_[hovered_slot].assigned_character;
    } else {
      int card_index = GetCardAtPosition(mouse_pos);
      if (card_index >= 0) {
        const entities::Character* ch = m_characterList.available_characters[card_index];
        bool is_locked = false;
        if (ctx.gameplayDataAPI && ch) {
          is_locked = !ctx.gameplayDataAPI->GetCharacterState(ch->id).unlocked;
        }
        if (!is_locked) {
          selected_character_ = ch;
        }
      }
    }
  }

  // 下部の3つのボタンは削除済み
  
  // リセットボタンのホバー処理（編成枠の下部）
  float slot_bottom_y = 0.0f;
  for (int i = 5; i < 10; ++i) {
    float slot_y = squad_slots_[i].position.y + squad_slots_[i].height;
    if (slot_y > slot_bottom_y) {
      slot_bottom_y = slot_y;
    }
  }
  float button_y = slot_bottom_y + 20.0f;
  float button_h = 50.0f;
  float button_w = 180.0f;
  float button_x = 170.0f + 2.0f * 200.0f;
  
  reset_button_.is_hovered =
      (mouse_pos.x >= button_x && mouse_pos.x < button_x + button_w &&
       mouse_pos.y >= button_y && mouse_pos.y < button_y + button_h);
}

void FormationOverlay::ProcessKeyboardInput(SharedContext& ctx) {
  if (selected_slot_index_ >= 0 && selected_slot_index_ < 10) {
    if (ctx.inputAPI && (ctx.inputAPI->IsBackspacePressed() || ctx.inputAPI->IsDeletePressed()))
      RemoveCharacter(selected_slot_index_);
  }
}

void FormationOverlay::ProcessScrollInput(float wheel_delta) {
  int total_rows =
      (static_cast<int>(m_characterList.available_characters.size()) +
       m_characterList.visible_columns - 1) /
      m_characterList.visible_columns;
  int max_scroll = std::max(0, total_rows - m_characterList.visible_rows);
  // スクロールオフセットを更新（最上行（0）までスクロール可能）
  m_characterList.scroll_offset =
      std::max(0, std::min(max_scroll, m_characterList.scroll_offset -
                                           static_cast<int>(wheel_delta)));
  // 最上行まで確実にスクロールできるように、total_rows <= visible_rows の場合は0にリセット
  if (total_rows <= m_characterList.visible_rows) {
    m_characterList.scroll_offset = 0;
  }
}

bool FormationOverlay::IsCharacterInSquad(
    const entities::Character *character) const {
  if (!character)
    return false;
  for (int i = 0; i < 10; ++i)
    if (squad_slots_[i].assigned_character == character)
      return true;
  return false;
}

Color FormationOverlay::GetSlotColor(const SquadSlot &slot) const {
  if (slot.is_hovered)
    return ui::OverlayColors::SLOT_HOVER;
  if (slot.assigned_character)
    return ui::OverlayColors::SLOT_ASSIGNED;
  return ui::OverlayColors::SLOT_EMPTY;
}

Color FormationOverlay::GetPartySummaryColor() const {
  return ui::OverlayColors::PANEL_BG_DARK;
}

} // namespace core
} // namespace game
