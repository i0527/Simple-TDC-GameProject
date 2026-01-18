#include "FormationOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../entities/Character.hpp"
#include "../../entities/CharacterManager.hpp"
#include "../../system/PlayerDataManager.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UIEffects.hpp"
#include <algorithm>
#include <cmath>
#include <raylib.h>
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
      animation_time_(0.0f), restored_from_context_(false) {
  InitializeSlots();
}

// ========== IOverlay 実装 ==========

bool FormationOverlay::Initialize(BaseSystemAPI *systemAPI) {
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

  // スロット初期化
  InitializeSlots();
  restored_from_context_ = false;

  // ドラッグ状態リセット
  dragging_character_ = nullptr;
  dragging_source_slot_ = -1;
  is_dragging_ = false;
  drag_start_pos_ = {0.0f, 0.0f};
  drag_started_ = false;
  selected_slot_index_ = -1;
  selected_character_ = nullptr;

  // ボタン状態リセット
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

  // 既存のSharedContext編成を最初の1回だけ復元
  if (!restored_from_context_) {
    RestoreFormationFromContext(ctx);
    restored_from_context_ = true;
  }

  // アニメーション時間更新
  animation_time_ += deltaTime;

  // キャラクター一覧の更新（初回のみ）
  if (m_characterList.available_characters.empty() && ctx.characterManager) {
    FilterAvailableCharacters(ctx);
  }

  // ESCキーで閉じる
  if (systemAPI_->IsKeyPressed(256)) { // ESC key
    requestClose_ = true;
  }

  // マウス入力処理
  ProcessMouseInput(ctx);

  // キーボード入力処理
  ProcessKeyboardInput();

  // スクロール処理
  float wheel_delta = systemAPI_->GetMouseWheelMove();
  if (wheel_delta != 0.0f) {
    ProcessScrollInput(wheel_delta);
  }

  // パーティーサマリー更新
  UpdatePartySummary();
}

void FormationOverlay::Render(SharedContext &ctx) {
  if (!isInitialized_) {
    return;
  }

  using namespace ui;

  // オーバーレイ背景（グラデーション）
  UIEffects::DrawGradientPanel(systemAPI_, 100.0f, 90.0f, 1720.0f, 900.0f);

  // 背景粒子エフェクト
  UIEffects::DrawParticles(systemAPI_, animation_time_, 100.0f, 90.0f, 1720.0f,
                           900.0f, 15);

  // タイトルバー（最初に描画して確実に上に表示）
  RenderTitleBar();

  // 区切り線
  RenderDividers();

  // 編成スロット
  RenderSquadSlots();

  // パーティーサマリーパネル
  RenderPartySummary();

  // 詳細パネル
  RenderDetailsPanel();

  // キャラクター一覧
  RenderCharacterList();

  // ボタン
  RenderButtons();

  // ドラッグ中のキャラクター
  if (is_dragging_ && dragging_character_) {
    RenderDraggingCharacter();
  }
}

void FormationOverlay::Shutdown() {
  if (!isInitialized_) {
    return;
  }

  // スロットクリア
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

// ========== 初期化・クリーンアップ ==========

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

  if (!ctx.characterManager) {
    return;
  }

  if (ctx.formationData.IsEmpty()) {
    return;
  }

  const auto& masters = ctx.characterManager->GetAllMasters();
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
}

void FormationOverlay::FilterAvailableCharacters(SharedContext &ctx) {
  if (!ctx.characterManager) {
    return;
  }

  m_characterList.available_characters.clear();

  const auto &masters = ctx.characterManager->GetAllMasters();
  for (const auto &[id, ch] : masters) {
    if (ch.is_discovered) {
      m_characterList.available_characters.push_back(&ch);
    }
  }

  SortAvailableCharacters();

  LOG_INFO("FormationOverlay: Loaded {} available characters",
           m_characterList.available_characters.size());
}

void FormationOverlay::SortAvailableCharacters() {
  std::sort(m_characterList.available_characters.begin(),
            m_characterList.available_characters.end(),
            [](const entities::Character *a, const entities::Character *b) {
              if (!a || !b)
                return false;
              if (a->rarity != b->rarity)
                return a->rarity > b->rarity;
              if (a->level != b->level)
                return a->level > b->level;
              if (a->cost != b->cost)
                return a->cost < b->cost;
              return a->name < b->name;
            });
}

// ========== 描画メソッド ==========

void FormationOverlay::RenderTitleBar() {
  // タイトルバー背景（タブバーの下に配置）
  systemAPI_->DrawRectangle(100.0f, 90.0f, 1720.0f, 60.0f,
                            ui::OverlayColors::HEADER_BG);

  // タイトル
  systemAPI_->DrawTextDefault("編成画面", 120.0f, 105.0f, 32.0f,
                              ui::OverlayColors::TEXT_PRIMARY);
}

void FormationOverlay::RenderDividers() {
  // 垂直区切り線（左側エリアと詳細パネルの間）
  systemAPI_->DrawRectangle(1205.0f, 155.0f, 3.0f, 825.0f,
                            ui::OverlayColors::DIVIDER);
  // 水平区切り線（編成スロットとサマリーの間）
  systemAPI_->DrawRectangle(100.0f, 500.0f, 1100.0f, 2.0f,
                            ui::OverlayColors::DIVIDER);
  // 水平区切り線（サマリーとキャラクター一覧の間）
  systemAPI_->DrawRectangle(100.0f, 590.0f, 1100.0f, 2.0f,
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

  // 立体カード描画（影 + 内側光沢）
  UIEffects::DrawCard3D(systemAPI_, slot.position.x, slot.position.y,
                        slot.width, slot.height, bg_color, is_selected,
                        slot.is_hovered);

  if (slot.assigned_character) {
    const entities::Character *ch = slot.assigned_character;
    systemAPI_->DrawTextDefault(ch->name, slot.position.x + 10.0f,
                                slot.position.y + 10.0f, 20.0f,
                                OverlayColors::TEXT_PRIMARY);

    std::string rarity_str = "";
    for (int i = 0; i < ch->rarity; ++i)
      rarity_str += "★";
    systemAPI_->DrawTextDefault(rarity_str, slot.position.x + 10.0f,
                                slot.position.y + 40.0f, 16.0f,
                                OverlayColors::TEXT_GOLD);

    std::string cost_str = "COST " + std::to_string(ch->cost);
    Vector2 cost_size = systemAPI_->MeasureTextDefault(cost_str, 18.0f);
    systemAPI_->DrawTextDefault(
        cost_str, slot.position.x + slot.width - cost_size.x - 10.0f,
        slot.position.y + slot.height - 25.0f, 18.0f,
        OverlayColors::TEXT_ACCENT);
  } else {
    systemAPI_->DrawTextDefault("Empty",
                                slot.position.x + slot.width / 2.0f - 30.0f,
                                slot.position.y + slot.height / 2.0f - 10.0f,
                                20.0f, OverlayColors::TEXT_DISABLED);
  }
}

void FormationOverlay::RenderPartySummary() {
  using namespace ui;

  float summary_x = 100.0f;
  float summary_y = 505.0f;
  float summary_w = 1100.0f;
  float summary_h = 80.0f;

  // グラデーションパネル
  UIEffects::DrawGradientPanel(systemAPI_, summary_x, summary_y, summary_w,
                               summary_h);

  // ボーダー
  Rectangle rect = {summary_x, summary_y, summary_w, summary_h};
  systemAPI_->DrawRectangleRoundedLines(rect, 0.1f, 8,
                                        OverlayColors::BORDER_GOLD);

  float text_y = summary_y + 15.0f;
  float font_size = 22.0f;

  // 編成コスト上限なし
  std::string cost_text =
      "COST: " + std::to_string(m_partySummary.total_cost);
  Color cost_color = ui::OverlayColors::TEXT_PRIMARY;
  systemAPI_->DrawTextDefault(cost_text, summary_x + 20.0f, text_y, font_size,
                              cost_color);

  std::string count_text =
      "UNIT: " + std::to_string(m_partySummary.character_count) + " / " +
      std::to_string(m_partySummary.max_character_count);
  systemAPI_->DrawTextDefault(count_text, summary_x + 280.0f, text_y, font_size,
                              ui::OverlayColors::TEXT_PRIMARY);

  std::string hp_text = "TOTAL HP: " + std::to_string(m_partySummary.total_hp);
  systemAPI_->DrawTextDefault(hp_text, summary_x + 520.0f, text_y, font_size,
                              ui::OverlayColors::TEXT_PRIMARY);

  std::string atk_text =
      "TOTAL ATK: " + std::to_string(m_partySummary.total_attack);
  systemAPI_->DrawTextDefault(atk_text, summary_x + 800.0f, text_y, font_size,
                              ui::OverlayColors::TEXT_PRIMARY);
}

void FormationOverlay::RenderCharacterList() {
  float list_x = 100.0f;
  float list_y = 595.0f;
  float list_w = 1100.0f;
  float list_h = 310.0f;

  systemAPI_->DrawRectangle(list_x, list_y, list_w, list_h,
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
                        i - start_index);
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
    systemAPI_->DrawRectangle(scrollbar_x, scrollbar_y, scrollbar_w,
                              scrollbar_h, ui::OverlayColors::PANEL_BG_DARK);

    float thumb_height =
        scrollbar_h *
        (static_cast<float>(m_characterList.visible_rows) / total_rows);
    float thumb_y =
        scrollbar_y + (scrollbar_h - thumb_height) *
                          (static_cast<float>(m_characterList.scroll_offset) /
                           (total_rows - m_characterList.visible_rows));
    systemAPI_->DrawRectangle(scrollbar_x, thumb_y, scrollbar_w, thumb_height,
                              ui::OverlayColors::BORDER_DEFAULT);
  }
}

void FormationOverlay::RenderCharacterCard(const entities::Character *character,
                                           int card_index) {
  if (!character)
    return;

  using namespace ui;

  Vector2 pos = GetCardPosition(card_index);
  bool is_in_squad = IsCharacterInSquad(character);
  bool is_selected = (selected_character_ == character);
  bool is_hovered = (is_dragging_ && dragging_character_ == character);

  Color bg_color = is_in_squad ? OverlayColors::SLOT_ASSIGNED
                               : OverlayColors::CARD_BG_NORMAL;

  // 立体カード描画
  UIEffects::DrawCard3D(systemAPI_, pos.x, pos.y, m_characterList.CARD_WIDTH,
                        m_characterList.CARD_HEIGHT, bg_color, is_selected,
                        is_hovered);

  // 発光効果ボーダー（選択時）
  if (is_selected) {
    float pulse_alpha = UIEffects::CalculatePulseAlpha(animation_time_);
    UIEffects::DrawGlowingBorder(
        systemAPI_, pos.x, pos.y, m_characterList.CARD_WIDTH,
        m_characterList.CARD_HEIGHT, pulse_alpha, is_hovered);
  }

  Color text_color =
      is_in_squad ? OverlayColors::TEXT_DISABLED : OverlayColors::TEXT_PRIMARY;
  systemAPI_->DrawTextDefault(character->name, pos.x + 5.0f, pos.y + 5.0f,
                              24.0f, text_color);
  systemAPI_->DrawTextDefault("C " + std::to_string(character->cost),
                              pos.x + 5.0f,
                              pos.y + m_characterList.CARD_HEIGHT - 30.0f,
                              24.0f, OverlayColors::TEXT_ACCENT);

  std::string rarity_stars = "";
  for (int i = 0; i < character->rarity; ++i)
    rarity_stars += "★";
  systemAPI_->DrawTextDefault(rarity_stars, pos.x + 5.0f, pos.y + 30.0f, 24.0f,
                              OverlayColors::TEXT_GOLD);
}

void FormationOverlay::RenderButtons() {
  using namespace ui;

  float button_y = 920.0f;
  float button_h = 55.0f;
  float button_w = 180.0f;

  // キャンセルボタン
  float cancel_x = 120.0f;
  UIEffects::DrawModernButton(systemAPI_, cancel_x, button_y, button_w,
                              button_h, OverlayColors::BUTTON_SECONDARY,
                              OverlayColors::BUTTON_SECONDARY_BRIGHT,
                              cancel_button_.is_hovered, false);
  Vector2 cancel_text_size =
      systemAPI_->MeasureTextDefault("キャンセル", 30.0f);
  systemAPI_->DrawTextDefault("キャンセル",
                              cancel_x + (button_w - cancel_text_size.x) / 2.0f,
                              button_y + (button_h - cancel_text_size.y) / 2.0f,
                              30.0f, OverlayColors::TEXT_PRIMARY);

  // リセットボタン
  float reset_x = cancel_x + button_w + 20.0f;
  UIEffects::DrawModernButton(systemAPI_, reset_x, button_y, button_w, button_h,
                              OverlayColors::BUTTON_RESET,
                              OverlayColors::CARD_BG_SELECTED,
                              reset_button_.is_hovered, false);
  Vector2 reset_text_size = systemAPI_->MeasureTextDefault("リセット", 30.0f);
  systemAPI_->DrawTextDefault("リセット",
                              reset_x + (button_w - reset_text_size.x) / 2.0f,
                              button_y + (button_h - reset_text_size.y) / 2.0f,
                              30.0f, OverlayColors::TEXT_PRIMARY);

  // 編成完了ボタン
  float complete_x = 1620.0f;
  bool is_disabled = !m_partySummary.IsComplete();
  UIEffects::DrawModernButton(
      systemAPI_, complete_x, button_y, button_w, button_h,
      OverlayColors::BUTTON_PRIMARY_DARK, OverlayColors::BUTTON_PRIMARY_BRIGHT,
      complete_button_.is_hovered && !is_disabled, is_disabled);
  Vector2 complete_text_size =
      systemAPI_->MeasureTextDefault("編成完了", 30.0f);
  Color text_color =
      is_disabled ? OverlayColors::TEXT_DISABLED : OverlayColors::TEXT_DARK;
  systemAPI_->DrawTextDefault(
      "編成完了", complete_x + (button_w - complete_text_size.x) / 2.0f,
      button_y + (button_h - complete_text_size.y) / 2.0f, 30.0f, text_color);
}

void FormationOverlay::RenderDetailsPanel() {
  float panel_x = m_detailsPanel.x;
  float panel_y = m_detailsPanel.y;
  float panel_width = m_detailsPanel.width;
  float panel_height = m_detailsPanel.height;

  systemAPI_->DrawRectangle(panel_x, panel_y, panel_width, panel_height,
                            ui::OverlayColors::PANEL_BG_BROWN);
  systemAPI_->DrawRectangleLines(panel_x, panel_y, panel_width, panel_height,
                                 2.0f, ui::OverlayColors::BORDER_GOLD);

  const entities::Character *display_char = is_dragging_ && dragging_character_
                                                ? dragging_character_
                                                : selected_character_;
  if (!display_char) {
    systemAPI_->DrawTextDefault("キャラクターを選択",
                                panel_x + m_detailsPanel.padding,
                                panel_y + panel_height / 2.0f - 18.0f, 32.0f,
                                ui::OverlayColors::TEXT_DISABLED);
    return;
  }

  float x = panel_x + m_detailsPanel.padding;
  float y = panel_y + m_detailsPanel.padding;
  systemAPI_->DrawTextDefault(display_char->name, x, y, 36.0f,
                              ui::OverlayColors::TEXT_PRIMARY);

  std::string rarity_str =
      "Rarity: " + (display_char->rarity_name.empty()
                        ? std::to_string(display_char->rarity)
                        : display_char->rarity_name);
  systemAPI_->DrawTextDefault(rarity_str, x, y + 45.0f, 24.0f,
                              ui::OverlayColors::TEXT_GOLD);
  systemAPI_->DrawRectangle(x, y + 80.0f,
                            panel_width - m_detailsPanel.padding * 2.0f, 2.0f,
                            ui::OverlayColors::DIVIDER);

  float stats_y = y + 100.0f;
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

  std::vector<std::pair<std::string, std::string>> stats = {
      {"Level", std::to_string(display_char->level)},
      {"HP", std::to_string(display_char->hp)},
      {"Attack", std::to_string(display_char->attack)},
      {"Defense", std::to_string(display_char->defense)},
      {"Speed", std::to_string(static_cast<int>(display_char->move_speed))},
      {"Interval", std::to_string(display_char->attack_span) + "s"},
      {"Cost", std::to_string(display_char->cost)},
      {"Type", attackTypeToString(display_char->attack_type)},
      {"Element", effectTypeToString(display_char->effect_type)}};

  for (size_t i = 0; i < stats.size(); ++i) {
    float line_y = stats_y + static_cast<float>(i) * m_detailsPanel.line_height;
    systemAPI_->DrawTextDefault(stats[i].first, x, line_y,
                                static_cast<float>(m_detailsPanel.font_size),
                                ui::OverlayColors::TEXT_SECONDARY);
    Vector2 text_size = systemAPI_->MeasureTextDefault(
        stats[i].second, static_cast<float>(m_detailsPanel.font_size));
    systemAPI_->DrawTextDefault(
        stats[i].second,
        x + panel_width - m_detailsPanel.padding * 2.0f - text_size.x, line_y,
        static_cast<float>(m_detailsPanel.font_size),
        ui::OverlayColors::TEXT_PRIMARY);
  }

  if (!display_char->description.empty()) {
    float desc_y =
        stats_y +
        static_cast<float>(stats.size()) * m_detailsPanel.line_height + 20.0f;
    systemAPI_->DrawRectangle(x, desc_y - 10.0f,
                              panel_width - m_detailsPanel.padding * 2.0f, 1.0f,
                              ui::OverlayColors::DIVIDER);
    systemAPI_->DrawTextDefault(display_char->description, x, desc_y, 20.0f,
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
  systemAPI_->DrawRectangleRounded(rec, 0.1f, 8, drag_bg);
  Color drag_border = ui::OverlayColors::BORDER_HOVER;
  drag_border.a = 180;
  systemAPI_->DrawRectangleRoundedLines(rec, 0.1f, 8, drag_border);

  Color nameColor = ui::OverlayColors::TEXT_PRIMARY;
  nameColor.a = 180;
  systemAPI_->DrawTextDefault(dragging_character_->name, card_x + 5.0f,
                              card_y + 5.0f, 14.0f, nameColor);

  Color costColor = ui::OverlayColors::TEXT_ACCENT;
  costColor.a = 180;
  systemAPI_->DrawTextDefault("C " + std::to_string(dragging_character_->cost),
                              card_x + 5.0f, card_y + card_h - 20.0f, 14.0f,
                              costColor);
}

// ========== 座標計算ヘルパー ==========

Vector2 FormationOverlay::GetSlotPosition(int slot_id) {
  int row = slot_id / 5;
  int col = slot_id % 5;
  float start_x = 170.0f;
  float start_y = 170.0f;
  float spacing_x = 190.0f;
  float spacing_y = 150.0f;
  return {start_x + col * spacing_x, start_y + row * spacing_y};
}

Vector2 FormationOverlay::GetCardPosition(int card_index) const {
  int row = card_index / m_characterList.visible_columns;
  int col = card_index % m_characterList.visible_columns;
  return {120.0f + col * m_characterList.CARD_SPACING_X,
          615.0f + row * m_characterList.CARD_SPACING_Y};
}

int FormationOverlay::GetSlotAtPosition(Vector2 position) const {
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

int FormationOverlay::GetCardAtPosition(Vector2 position) const {
  if (position.x < 100.0f || position.x >= 1200.0f || position.y < 595.0f ||
      position.y >= 905.0f)
    return -1;

  int start_row = m_characterList.scroll_offset;
  int start_index = start_row * m_characterList.visible_columns;
  int end_index =
      std::min(start_index + m_characterList.visible_rows *
                                 m_characterList.visible_columns,
               static_cast<int>(m_characterList.available_characters.size()));

  for (int i = start_index; i < end_index; ++i) {
    Vector2 card_pos = GetCardPosition(i - start_index);
    if (position.x >= card_pos.x &&
        position.x < card_pos.x + m_characterList.CARD_WIDTH &&
        position.y >= card_pos.y &&
        position.y < card_pos.y + m_characterList.CARD_HEIGHT) {
      return i;
    }
  }
  return -1;
}

// ========== キャラクター管理 ==========

void FormationOverlay::AssignCharacter(int slot_id,
                                       const entities::Character *character) {
  if (slot_id < 0 || slot_id >= 10 || !character)
    return;
  squad_slots_[slot_id].assigned_character = character;
  UpdatePartySummary();
}

void FormationOverlay::RemoveCharacter(int slot_id) {
  if (slot_id < 0 || slot_id >= 10)
    return;
  squad_slots_[slot_id].assigned_character = nullptr;
  UpdatePartySummary();
}

void FormationOverlay::SwapCharacters(int slot1_id, int slot2_id) {
  if (slot1_id < 0 || slot1_id >= 10 || slot2_id < 0 || slot2_id >= 10)
    return;
  const entities::Character *temp = squad_slots_[slot1_id].assigned_character;
  squad_slots_[slot1_id].assigned_character =
      squad_slots_[slot2_id].assigned_character;
  squad_slots_[slot2_id].assigned_character = temp;
  UpdatePartySummary();
}

// ========== パーティー管理 ==========

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

// ========== イベント処理 ==========

void FormationOverlay::OnSlotClicked(int slot_id) {}
void FormationOverlay::OnCardClicked(int card_index) {}

void FormationOverlay::OnSlotRightClicked(int slot_id) {
  if (slot_id >= 0 && slot_id < 10)
    RemoveCharacter(slot_id);
}

void FormationOverlay::OnDragStart(int source_slot,
                                   const entities::Character *character) {
  dragging_character_ = character;
  dragging_source_slot_ = source_slot;
  is_dragging_ = true;
  drag_position_ = systemAPI_->GetMousePosition();
  selected_character_ = character;
}

void FormationOverlay::OnDragUpdate(Vector2 mouse_pos) {
  if (is_dragging_)
    drag_position_ = mouse_pos;
}

void FormationOverlay::OnDragEnd(Vector2 mouse_pos) {
  if (!is_dragging_ || !dragging_character_)
    return;

  int target_slot = GetSlotAtPosition(mouse_pos);
  if (target_slot >= 0) {
    if (dragging_source_slot_ >= 0) {
      if (target_slot != dragging_source_slot_)
        SwapCharacters(dragging_source_slot_, target_slot);
    } else {
      AssignCharacter(target_slot, dragging_character_);
    }
  }

  dragging_character_ = nullptr;
  dragging_source_slot_ = -1;
  is_dragging_ = false;
  drag_started_ = false;
}

void FormationOverlay::OnButtonClicked(const std::string &button_name, SharedContext& ctx) {
  if (button_name == "complete") {
    if (ValidateSquadComposition()) {
      // 編成データをSharedContextに保存
      ctx.formationData.Clear();
      for (int i = 0; i < 10; ++i) {
        if (squad_slots_[i].assigned_character != nullptr) {
          ctx.formationData.slots.push_back({i, squad_slots_[i].assigned_character->id});
        }
      }
      LOG_INFO("Formation data saved: {} characters in formation", ctx.formationData.slots.size());

      // 単一JSONへ保存
      if (ctx.playerDataManager) {
        ctx.playerDataManager->SetFormationFromSharedContext(ctx.formationData);
        ctx.playerDataManager->Save();
      }

      requestClose_ = true;
    }
  } else if (button_name == "cancel") {
    requestClose_ = true;
  } else if (button_name == "reset") {
    for (int i = 0; i < 10; ++i)
      RemoveCharacter(i);
  }
}

// ========== マウス入力処理 ==========

void FormationOverlay::ProcessMouseInput(SharedContext &ctx) {
  Vector2 mouse_pos = systemAPI_->GetMousePosition();
  UpdateHoverStates(mouse_pos);

  if (systemAPI_->IsMouseButtonPressed(0)) {
    float button_y = 920.0f, button_h = 55.0f, button_w = 180.0f;
    float cancel_x = 120.0f, reset_x = 320.0f, complete_x = 1620.0f;

    if (mouse_pos.x >= cancel_x && mouse_pos.x < cancel_x + button_w &&
        mouse_pos.y >= button_y && mouse_pos.y < button_y + button_h)
      OnButtonClicked("cancel", ctx);
    else if (mouse_pos.x >= reset_x && mouse_pos.x < reset_x + button_w &&
             mouse_pos.y >= button_y && mouse_pos.y < button_y + button_h)
      OnButtonClicked("reset", ctx);
    else if (mouse_pos.x >= complete_x && mouse_pos.x < complete_x + button_w &&
             mouse_pos.y >= button_y && mouse_pos.y < button_y + button_h) {
      if (m_partySummary.IsComplete())
        OnButtonClicked("complete", ctx);
    }

    drag_start_pos_ = mouse_pos;
    drag_started_ = true;
  } else if (systemAPI_->IsMouseButtonDown(0) && drag_started_ &&
             !is_dragging_) {
    float dx = mouse_pos.x - drag_start_pos_.x,
          dy = mouse_pos.y - drag_start_pos_.y;
    if (std::sqrt(dx * dx + dy * dy) > 3.0f) {
      int slot_id = GetSlotAtPosition(drag_start_pos_);
      if (slot_id >= 0 && squad_slots_[slot_id].assigned_character)
        OnDragStart(slot_id, squad_slots_[slot_id].assigned_character);
      else {
        int card_index = GetCardAtPosition(drag_start_pos_);
        if (card_index >= 0) {
          const entities::Character *ch =
              m_characterList.available_characters[card_index];
          if (!IsCharacterInSquad(ch))
            OnDragStart(-1, ch);
        }
      }
    }
  } else if (systemAPI_->IsMouseButtonReleased(0)) {
    if (is_dragging_)
      OnDragEnd(mouse_pos);
    drag_started_ = false;
  }

  if (is_dragging_)
    OnDragUpdate(mouse_pos);
  if (systemAPI_->IsMouseButtonPressed(1)) {
    int slot_id = GetSlotAtPosition(mouse_pos);
    if (slot_id >= 0)
      OnSlotRightClicked(slot_id);
  }
}

void FormationOverlay::UpdateHoverStates(Vector2 mouse_pos) {
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
        selected_character_ = m_characterList.available_characters[card_index];
      }
    }
  }

  float button_y = 920.0f, button_h = 55.0f, button_w = 180.0f;
  float cancel_x = 120.0f, reset_x = 320.0f, complete_x = 1620.0f;
  cancel_button_.is_hovered =
      (mouse_pos.x >= cancel_x && mouse_pos.x < cancel_x + button_w &&
       mouse_pos.y >= button_y && mouse_pos.y < button_y + button_h);
  reset_button_.is_hovered =
      (mouse_pos.x >= reset_x && mouse_pos.x < reset_x + button_w &&
       mouse_pos.y >= button_y && mouse_pos.y < button_y + button_h);
  complete_button_.is_hovered =
      (mouse_pos.x >= complete_x && mouse_pos.x < complete_x + button_w &&
       mouse_pos.y >= button_y && mouse_pos.y < button_y + button_h);
}

void FormationOverlay::ProcessKeyboardInput() {
  if (selected_slot_index_ >= 0 && selected_slot_index_ < 10) {
    if (systemAPI_->IsKeyPressed(259) || systemAPI_->IsKeyPressed(261))
      RemoveCharacter(selected_slot_index_);
  }
}

void FormationOverlay::ProcessScrollInput(float wheel_delta) {
  int total_rows =
      (static_cast<int>(m_characterList.available_characters.size()) +
       m_characterList.visible_columns - 1) /
      m_characterList.visible_columns;
  int max_scroll = std::max(0, total_rows - m_characterList.visible_rows);
  m_characterList.scroll_offset =
      std::max(0, std::min(max_scroll, m_characterList.scroll_offset -
                                           static_cast<int>(wheel_delta)));
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
