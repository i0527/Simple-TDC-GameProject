#include "CharacterEnhancementOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/GameplayDataAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../ecs/entities/CharacterStatCalculator.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UIEffects.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <unordered_map>
#include <unordered_set>

namespace {

std::unordered_set<std::string>
BuildEquippedEquipmentNames(const game::core::SharedContext &ctx) {
  std::unordered_set<std::string> names;
  if (!ctx.gameplayDataAPI) {
    return names;
  }

  const auto &save = ctx.gameplayDataAPI->GetSaveData();
  for (const auto &[_, st] : save.characters) {
    (void)_;
    for (int i = 0; i < 3; ++i) {
      const std::string &eid = st.equipment[i];
      if (eid.empty())
        continue;
      const auto *eq = ctx.gameplayDataAPI->GetEquipment(eid);
      if (!eq)
        continue;
      if (!eq->name.empty()) {
        names.insert(eq->name);
      }
    }
  }
  return names;
}

} // namespace

namespace game {
namespace core {

CharacterEnhancementOverlay::CharacterEnhancementOverlay()
    : systemAPI_(nullptr), isInitialized_(false), requestClose_(false),
      hasTransitionRequest_(false), requestedNextState_(GameState::Title),
      has_unsaved_changes_(false), rng_(std::random_device{}()) {}

bool CharacterEnhancementOverlay::Initialize(BaseSystemAPI *systemAPI,
                                             UISystemAPI *uiAPI) {
  if (isInitialized_) {
    LOG_ERROR("CharacterEnhancementOverlay already initialized");
    return false;
  }

  if (!systemAPI) {
    LOG_ERROR("CharacterEnhancementOverlay: systemAPI is null");
    return false;
  }

  systemAPI_ = systemAPI;
  requestClose_ = false;
  hasTransitionRequest_ = false;
  has_unsaved_changes_ = false;

  // パネル初期匁E
  InitializePanels();

  isInitialized_ = true;
  LOG_INFO("CharacterEnhancementOverlay initialized");
  return true;
}

void CharacterEnhancementOverlay::Update(SharedContext &ctx, float deltaTime) {
  if (!isInitialized_) {
    return;
  }
  (void)deltaTime;

  // Render側でも参照したぁE��報があるため保持�E�所有権なし！E
  sharedContext_ = &ctx;

  // キャラクターリスト�E読み込み�E��E回�Eみ�E�E
  if (unit_info_panel_.entries.empty() && ctx.gameplayDataAPI) {
    LoadCharacterList(ctx);
  }

  // パッシブ�EアイチE��リスト�E読み込み�E��E回�Eみ�E�E
  if (operation_panel_.available_passives.empty() && ctx.gameplayDataAPI) {
    FilterAvailablePassives(ctx);
  }
  if (operation_panel_.available_items.empty() && ctx.gameplayDataAPI) {
    FilterAvailableItems(ctx);
  }

  // マウス入力�E琁E
  ProcessMouseInput(ctx);

  // キーボ�Eド�E力�E琁E
  ProcessKeyboardInput(ctx);

  // ホバー状態更新
  auto mouse_pos =
      ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};
  UpdateHoverStates(mouse_pos);
}

void CharacterEnhancementOverlay::Render(SharedContext &ctx) {
  if (!isInitialized_) {
    return;
  }

  using namespace ui;

  // 吁E��ネルの描画�E�EnitializePanelsで設定された座標をそ�Eまま使用�E�E
  RenderUnitInfoPanel();
  RenderSortUI();
  RenderStatusPanel(ctx);
  RenderOperationPanel(ctx);
}

void CharacterEnhancementOverlay::Shutdown() {
  if (!isInitialized_) {
    return;
  }

  unit_info_panel_.entries.clear();
  operation_panel_.available_passives.clear();
  operation_panel_.available_items.clear();

  isInitialized_ = false;
  systemAPI_ = nullptr;
  sharedContext_ = nullptr;
  LOG_INFO("CharacterEnhancementOverlay shutdown");
}

bool CharacterEnhancementOverlay::RequestClose() const { return requestClose_; }

bool CharacterEnhancementOverlay::RequestTransition(
    GameState &nextState) const {
  if (hasTransitionRequest_) {
    nextState = requestedNextState_;
    return true;
  }
  return false;
}

// ========== 初期化メソチE�� ==========

void CharacterEnhancementOverlay::InitializePanels() {
  // 画面レイアウト定数
  const float SCREEN_WIDTH = 1920.0f;
  const float SCREEN_HEIGHT = 1080.0f;
  const float HEADER_HEIGHT = 90.0f; // ヘッダー�E�Eold/Gems/Tickets表示�E�E
  const float TAB_BAR_HEIGHT = 90.0f; // タブバー�E�下部、y=990�E�E080�E�E
  const float CONTENT_START_Y = HEADER_HEIGHT; // コンチE��チE��始位置 y=90
  const float CONTENT_END_Y =
      SCREEN_HEIGHT - TAB_BAR_HEIGHT; // コンチE��チE��亁E��置 y=990

  // マ�EジンとギャチE�E
  const float MARGIN = 5.0f;     // 画面端からのマ�Eジン
  const float PANEL_GAP = 10.0f; // パネル間�EギャチE�E

  // 利用可能な描画領域を計算！E=90�E�E90の900px篁E��冁E��E
  const float available_width = SCREEN_WIDTH - (MARGIN * 2);
  const float available_height = CONTENT_END_Y - CONTENT_START_Y - (MARGIN * 2);

  // パネル幁E�E配�E�E�比率: ユニッチEスチE�Eタス:操佁E= 1:1.3:2�E�E
  const float total_ratio = 1.0f + 1.3f + 2.0f; // 4.3
  const float unit_width =
      (available_width - PANEL_GAP * 2) * (1.0f / total_ratio);
  const float status_width =
      (available_width - PANEL_GAP * 2) * (1.3f / total_ratio);
  const float operation_width =
      (available_width - PANEL_GAP * 2) * (2.0f / total_ratio);

  // ユニット情報パネル�E�左�E�E コンチE��チE��域冁E��配置
  unit_info_panel_.x = MARGIN;
  unit_info_panel_.y = CONTENT_START_Y + MARGIN;
  unit_info_panel_.width = unit_width;
  unit_info_panel_.height = available_height;
  unit_info_panel_.selected_index = -1;
  unit_info_panel_.scroll_offset = 0;
  unit_info_panel_.item_height = 60.0f;
  unit_info_panel_.selected_character = nullptr;

  // スチE�Eタスパネル�E�中央�E�E コンチE��チE��域冁E��配置
  status_panel_.x = unit_info_panel_.x + unit_info_panel_.width + PANEL_GAP;
  status_panel_.y = CONTENT_START_Y + MARGIN;
  status_panel_.width = status_width;
  status_panel_.height = available_height;
  status_panel_.padding = 30.0f;
  status_panel_.line_height = 45.0f;
  status_panel_.font_size = 24;

  // 操作パネル�E�右�E�E コンチE��チE��域冁E��配置
  operation_panel_.x = status_panel_.x + status_panel_.width + PANEL_GAP;
  operation_panel_.y = CONTENT_START_Y + MARGIN;
  operation_panel_.width = operation_width;
  operation_panel_.height = available_height;
  operation_panel_.active_tab = OperationPanel::TabType::Enhancement;
  operation_panel_.show_passive_popup = false;
  operation_panel_.popup_slot_id = -1;
  operation_panel_.item_scroll_offset = 0;
  operation_panel_.passive_scroll_offset = 0;
  operation_panel_.selected_item_slot_id = -1;
  operation_panel_.show_item_popup = false;
  operation_panel_.popup_item_slot_id = -1;
  operation_panel_.dragging_item_id = "";
  operation_panel_.is_dragging_item = false;
  operation_panel_.drag_start_mouse_pos = {0, 0};

  // パッシブスロチE���E�横並び�E�E 操作パネルの相対位置で配置
  const float slot_margin = 15.0f;  // スロチE��左右のマ�Eジン
  const float slot_spacing = 15.0f; // スロチE��間�E間隔
  const float slot_width =
      (operation_panel_.width - slot_margin * 2 - slot_spacing * 2) / 3.0f;
  const float slot_height = 180.0f;
  const float slot_start_x = slot_margin; // パネル冁E�E相対位置
  const float slot_y = 80.0f; // パネル冁E�E相対位置�E�タブ�E下！E

  for (int i = 0; i < 3; ++i) {
    operation_panel_.passive_slots[i].slot_id = i;
    operation_panel_.passive_slots[i].assigned_passive = nullptr;
    operation_panel_.passive_slots[i].position.x =
        slot_start_x + i * (slot_width + slot_spacing);
    operation_panel_.passive_slots[i].position.y = slot_y;
    operation_panel_.passive_slots[i].width = slot_width;
    operation_panel_.passive_slots[i].height = slot_height;
    operation_panel_.passive_slots[i].is_hovered = false;
    operation_panel_.passive_slots[i].level = 1;
  }

  // アイチE��スロチE���E�横並び�E�E パッシブスロチE��と同じ配置
  for (int i = 0; i < 3; ++i) {
    operation_panel_.item_slots[i].slot_id = i;
    operation_panel_.item_slots[i].assigned_item = nullptr;
    operation_panel_.item_slots[i].position.x =
        slot_start_x + i * (slot_width + slot_spacing);
    operation_panel_.item_slots[i].position.y = slot_y;
    operation_panel_.item_slots[i].width = slot_width;
    operation_panel_.item_slots[i].height = slot_height;
    operation_panel_.item_slots[i].is_hovered = false;
  }

  LOG_INFO("CharacterEnhancementOverlay: Panel layout calculated");
  LOG_INFO("  Unit panel: x={:.1f} y={:.1f} w={:.1f} h={:.1f}",
           unit_info_panel_.x, unit_info_panel_.y, unit_info_panel_.width,
           unit_info_panel_.height);
  LOG_INFO("  Status panel: x={:.1f} y={:.1f} w={:.1f} h={:.1f}",
           status_panel_.x, status_panel_.y, status_panel_.width,
           status_panel_.height);
  LOG_INFO("  Operation panel: x={:.1f} y={:.1f} w={:.1f} h={:.1f}",
           operation_panel_.x, operation_panel_.y, operation_panel_.width,
           operation_panel_.height);
  LOG_INFO("  Slot size: {:.1f}x{:.1f}", slot_width, slot_height);
}

void CharacterEnhancementOverlay::LoadCharacterList(SharedContext &ctx) {
  if (!ctx.gameplayDataAPI) {
    return;
  }

  unit_info_panel_.entries.clear();
  const auto &masters = ctx.gameplayDataAPI->GetAllCharacterMasters();
  for (const auto &[id, ch] : masters) {
    unit_info_panel_.entries.push_back(&ch);
  }

  // ソート処理
  SortCharacterList(ctx);

  if (!unit_info_panel_.entries.empty()) {
    unit_info_panel_.selected_index = 0;
    SelectCharacter(ctx, unit_info_panel_.entries[0]);
  }

  LOG_INFO("CharacterEnhancementOverlay: Loaded {} characters",
           unit_info_panel_.entries.size());
}

void CharacterEnhancementOverlay::SortCharacterList(SharedContext &ctx) {
  if (!ctx.gameplayDataAPI) {
    return;
  }

  std::sort(
      unit_info_panel_.entries.begin(), unit_info_panel_.entries.end(),
      [this, &ctx](const entities::Character *a, const entities::Character *b) {
        if (!a || !b)
          return false;

        auto cmpInt = [this](int lhs, int rhs) {
          return sortAscending_ ? (lhs < rhs) : (lhs > rhs);
        };
        auto cmpStr = [this](const std::string &lhs, const std::string &rhs) {
          return sortAscending_ ? (lhs < rhs) : (rhs < lhs);
        };

        switch (currentSortKey_) {
        case SortKey::Name:
          if (a->name != b->name)
            return cmpStr(a->name, b->name);
          break;
        case SortKey::Rarity:
          if (a->rarity != b->rarity)
            return cmpInt(a->rarity, b->rarity);
          break;
        case SortKey::Cost:
          if (a->cost != b->cost)
            return cmpInt(a->cost, b->cost);
          break;
        case SortKey::Level: {
          int levelA = 1;
          int levelB = 1;
          if (ctx.gameplayDataAPI) {
            levelA = ctx.gameplayDataAPI->GetCharacterState(a->id).level;
            levelB = ctx.gameplayDataAPI->GetCharacterState(b->id).level;
          }
          if (levelA != levelB)
            return cmpInt(levelA, levelB);
          break;
        }
        case SortKey::Owned: {
          bool ownedA = true;
          bool ownedB = true;
          if (ctx.gameplayDataAPI) {
            ownedA = ctx.gameplayDataAPI->GetCharacterState(a->id).unlocked;
            ownedB = ctx.gameplayDataAPI->GetCharacterState(b->id).unlocked;
          }
          if (ownedA != ownedB) {
            // 所持している方を先に（降順の場合はtrue > false）
            return sortAscending_ ? (!ownedA && ownedB) : (ownedA && !ownedB);
          }
          break;
        }
        }
        // タイブレーカー
        if (a->rarity != b->rarity)
          return a->rarity > b->rarity;
        if (a->cost != b->cost)
          return a->cost < b->cost;
        return a->name < b->name;
      });
}

void CharacterEnhancementOverlay::SelectCharacter(
    SharedContext &ctx, const entities::Character *character) {
  if (!character) {
    return;
  }

  // ロックされたキャラは選択できない
  if (ctx.gameplayDataAPI) {
    const auto st = ctx.gameplayDataAPI->GetCharacterState(character->id);
    if (!st.unlocked) {
      LOG_WARN("CharacterEnhancementOverlay: Attempted to select locked "
               "character: {}",
               character->id);
      return; // ロックされている場合は選択しない
    }
  }

  unit_info_panel_.selected_character = character;

  // セーブからロードアウトを復允E��なければ空�E�E
  PlayerDataManager::CharacterState st;
  if (ctx.gameplayDataAPI) {
    st = ctx.gameplayDataAPI->GetCharacterState(character->id);
  }
  saved_character_state_ = st;
  editing_character_state_ = st;
  editing_character_id_ = character->id;

  ApplyStateToUI(ctx, editing_character_state_);

  // 候補リストを現在キャラに合わせて更新�E�裁E��中/付与中が所持Eでも表示されるよぁE���E�E
  FilterAvailablePassives(ctx);
  FilterAvailableItems(ctx);

  // スチE�Eタスパネルを更新�E�ロードアウト反映後！E
  UpdateStatusPanel(ctx);

  has_unsaved_changes_ = false;

  LOG_INFO("CharacterEnhancementOverlay: Selected character: {}",
           character->id);
}

void CharacterEnhancementOverlay::UpdateStatusPanel(SharedContext &ctx) {
  const auto *character = GetSelectedCharacter();
  if (!character) {
    return;
  }

  // 共通計算！EI/戦闘で一致させる！E
  status_panel_.hp = {};
  status_panel_.attack = {};
  status_panel_.defense = {};
  status_panel_.speed = {};
  status_panel_.range = {};
  status_panel_.attack_span = character->attack_span;
  status_panel_.level = std::max(1, editing_character_state_.level);
  status_panel_.cost = character->cost;
  status_panel_.attack_type = character->attack_type;
  status_panel_.effect_type = character->effect_type;
  status_panel_.rarity = std::max(1, character->rarity);
  status_panel_.rarity_name = character->rarity_name;

  // GameplayDataAPI が無ぁE��合�Eベ�Eス値のみ
  if (!ctx.gameplayDataAPI) {
    status_panel_.hp.base = character->hp;
    status_panel_.attack.base = character->attack;
    status_panel_.defense.base = character->defense;
    status_panel_.speed.base = static_cast<int>(character->move_speed);
    status_panel_.range.base = static_cast<int>(character->attack_size.x);
    return;
  }

  const auto editingState = BuildCurrentEditingState();
  const auto *itemPassiveManager = ctx.gameplayDataAPI->GetItemPassiveManager();
  if (!itemPassiveManager) {
    return;
  }
  const auto calc = entities::CharacterStatCalculator::Calculate(
      *character, editingState, *itemPassiveManager);

  status_panel_.hp.base = calc.hp.base;
  status_panel_.hp.bonus = calc.hp.bonus;
  status_panel_.attack.base = calc.attack.base;
  status_panel_.attack.bonus = calc.attack.bonus;
  status_panel_.defense.base = calc.defense.base;
  status_panel_.defense.bonus = calc.defense.bonus;

  status_panel_.speed.base = static_cast<int>(std::round(calc.moveSpeed.base));
  status_panel_.speed.bonus =
      static_cast<int>(std::round(calc.moveSpeed.final - calc.moveSpeed.base));
  status_panel_.range.base = static_cast<int>(std::round(calc.range.base));
  status_panel_.range.bonus =
      static_cast<int>(std::round(calc.range.final - calc.range.base));

  status_panel_.attack_span = calc.attackSpan.final;
  status_panel_.level = std::max(1, std::min(10, editingState.level));
}

PlayerDataManager::CharacterState
CharacterEnhancementOverlay::BuildCurrentEditingState() const {
  PlayerDataManager::CharacterState st;
  st.unlocked = editing_character_state_.unlocked;
  st.level = editing_character_state_.level;

  for (int i = 0; i < 3; ++i) {
    const auto *p = operation_panel_.passive_slots[i].assigned_passive;
    st.passives[i].id = p ? p->id : "";
    st.passives[i].level = std::max(1, operation_panel_.passive_slots[i].level);

    const auto *e = operation_panel_.item_slots[i].assigned_item;
    st.equipment[i] = e ? e->id : "";
  }
  return st;
}

void CharacterEnhancementOverlay::ApplyStateToUI(
    SharedContext &ctx, const PlayerDataManager::CharacterState &state) {
  (void)ctx;
  // パッシチE
  for (int i = 0; i < 3; ++i) {
    operation_panel_.passive_slots[i].assigned_passive = nullptr;
    operation_panel_.passive_slots[i].level = 1;

    if (ctx.gameplayDataAPI) {
      const std::string pid = state.passives[i].id;
      if (!pid.empty()) {
        operation_panel_.passive_slots[i].assigned_passive =
            ctx.gameplayDataAPI->GetPassiveSkill(pid);
        operation_panel_.passive_slots[i].level =
            std::max(1, state.passives[i].level);
      }
    }
  }

  // 裁E��
  for (int i = 0; i < 3; ++i) {
    operation_panel_.item_slots[i].assigned_item = nullptr;
    if (ctx.gameplayDataAPI) {
      const std::string eid = state.equipment[i];
      if (!eid.empty()) {
        operation_panel_.item_slots[i].assigned_item =
            ctx.gameplayDataAPI->GetEquipment(eid);
      }
    }
  }
}

bool CharacterEnhancementOverlay::TryCommitEditingState(
    SharedContext &ctx, const PlayerDataManager::CharacterState &newState) {
  const auto *character = GetSelectedCharacter();
  if (!character)
    return false;
  if (!ctx.gameplayDataAPI) {
    // セーブ無し�E場合でも編雁E��態だけ更新
    editing_character_state_ = newState;
    UpdateStatusPanel(ctx);
    return true;
  }

  const auto oldState = saved_character_state_;

  // ===== inventory整合（このキャラの付け替え差刁E�Eみ�E�E====
  std::unordered_map<std::string, int> oldEqCount;
  std::unordered_map<std::string, int> newEqCount;
  std::unordered_map<std::string, int> oldPsCount;
  std::unordered_map<std::string, int> newPsCount;

  for (int i = 0; i < 3; ++i) {
    if (!oldState.equipment[i].empty())
      oldEqCount[oldState.equipment[i]]++;
    if (!newState.equipment[i].empty())
      newEqCount[newState.equipment[i]]++;
    if (!oldState.passives[i].id.empty())
      oldPsCount[oldState.passives[i].id]++;
    if (!newState.passives[i].id.empty())
      newPsCount[newState.passives[i].id]++;
  }

  // 裁E��チェチE��
  for (const auto &[id, required] : newEqCount) {
    const int available = ctx.gameplayDataAPI->GetOwnedEquipmentCount(id) +
                          (oldEqCount.count(id) ? oldEqCount[id] : 0);
    if (required > available) {
      LOG_WARN("CharacterEnhancementOverlay: Not enough equipment '{}' "
               "required={} available={}",
               id, required, available);
      return false;
    }
  }
  // パッシブチェチE��
  for (const auto &[id, required] : newPsCount) {
    const int available = ctx.gameplayDataAPI->GetOwnedPassiveCount(id) +
                          (oldPsCount.count(id) ? oldPsCount[id] : 0);
    if (required > available) {
      LOG_WARN("CharacterEnhancementOverlay: Not enough passive '{}' "
               "required={} available={}",
               id, required, available);
      return false;
    }
  }

  // 裁E��の残数更新�E�このキャラ刁E��け戻して→消費�E�E
  for (const auto &[id, cnt] : oldEqCount) {
    ctx.gameplayDataAPI->SetOwnedEquipmentCount(
        id, ctx.gameplayDataAPI->GetOwnedEquipmentCount(id) + cnt);
  }
  for (const auto &[id, cnt] : newEqCount) {
    ctx.gameplayDataAPI->SetOwnedEquipmentCount(
        id, ctx.gameplayDataAPI->GetOwnedEquipmentCount(id) - cnt);
  }
  // パッシブ�E残数更新
  for (const auto &[id, cnt] : oldPsCount) {
    ctx.gameplayDataAPI->SetOwnedPassiveCount(
        id, ctx.gameplayDataAPI->GetOwnedPassiveCount(id) + cnt);
  }
  for (const auto &[id, cnt] : newPsCount) {
    ctx.gameplayDataAPI->SetOwnedPassiveCount(
        id, ctx.gameplayDataAPI->GetOwnedPassiveCount(id) - cnt);
  }

  ctx.gameplayDataAPI->SetCharacterState(character->id, newState);
  ctx.gameplayDataAPI->Save();

  saved_character_state_ = newState;
  editing_character_state_ = newState;

  // 候補リストを更新�E�裁E��中/付与中をpin�E�E
  FilterAvailablePassives(ctx);
  FilterAvailableItems(ctx);
  UpdateStatusPanel(ctx);
  has_unsaved_changes_ = false;
  return true;
}

void CharacterEnhancementOverlay::FilterAvailablePassives(SharedContext &ctx) {
  operation_panel_.available_passives.clear();

  if (!ctx.gameplayDataAPI) {
    return;
  }

  // 現在付与中のも�Eは所持Eでも候補に残す
  std::unordered_set<std::string> pinned;
  for (int i = 0; i < 3; ++i) {
    if (operation_panel_.passive_slots[i].assigned_passive) {
      pinned.insert(operation_panel_.passive_slots[i].assigned_passive->id);
    }
  }

  const auto all = ctx.gameplayDataAPI->GetAllPassiveSkills();
  operation_panel_.available_passives.reserve(all.size());
  for (const auto *p : all) {
    if (!p)
      continue;
    if (pinned.find(p->id) != pinned.end()) {
      operation_panel_.available_passives.push_back(p);
      continue;
    }
    if (!ctx.gameplayDataAPI) {
      operation_panel_.available_passives.push_back(p);
      continue;
    }
    if (ctx.gameplayDataAPI->GetOwnedPassiveCount(p->id) > 0) {
      operation_panel_.available_passives.push_back(p);
    }
  }

  LOG_INFO("CharacterEnhancementOverlay: Loaded {} available passives",
           operation_panel_.available_passives.size());
}

void CharacterEnhancementOverlay::FilterAvailableItems(SharedContext &ctx) {
  operation_panel_.available_items.clear();

  if (!ctx.gameplayDataAPI) {
    return;
  }

  std::unordered_set<std::string> pinned;
  for (int i = 0; i < 3; ++i) {
    if (operation_panel_.item_slots[i].assigned_item) {
      pinned.insert(operation_panel_.item_slots[i].assigned_item->id);
    }
  }
  // グローバルに裁E��中�E�同名）�E裁E��も候補に残す�E�所持Eでも「裁E��中」が見えるよぁE���E�E
  const auto equippedNames = BuildEquippedEquipmentNames(ctx);

  const auto all = ctx.gameplayDataAPI->GetAllEquipment();
  operation_panel_.available_items.reserve(all.size());
  for (const auto *e : all) {
    if (!e)
      continue;
    if (pinned.find(e->id) != pinned.end()) {
      operation_panel_.available_items.push_back(e);
      continue;
    }
    if (!equippedNames.empty() &&
        equippedNames.find(e->name) != equippedNames.end()) {
      operation_panel_.available_items.push_back(e);
      continue;
    }
    if (!ctx.gameplayDataAPI) {
      operation_panel_.available_items.push_back(e);
      continue;
    }
    if (ctx.gameplayDataAPI->GetOwnedEquipmentCount(e->id) > 0) {
      operation_panel_.available_items.push_back(e);
    }
  }

  LOG_INFO("CharacterEnhancementOverlay: Loaded {} available items",
           operation_panel_.available_items.size());
}

// ========== 描画メソチE�� ==========

void CharacterEnhancementOverlay::RenderUnitInfoPanel() {
  using namespace ui;

  // 背景
  systemAPI_->Render().DrawRectangle(
      unit_info_panel_.x, unit_info_panel_.y, unit_info_panel_.width,
      unit_info_panel_.height, OverlayColors::PANEL_BG_ORANGE);

  // ゴールド�Eーダー
  systemAPI_->Render().DrawRectangleLines(
      unit_info_panel_.x, unit_info_panel_.y, unit_info_panel_.width,
      unit_info_panel_.height, 2.0f, OverlayColors::BORDER_GOLD);

  // タイトル
  systemAPI_->Render().DrawTextDefault(
      "ユニット選択", unit_info_panel_.x + 10.0f, unit_info_panel_.y + 10.0f,
      24.0f, OverlayColors::TEXT_GOLD);

  // ユニット一覧�E�スクロール可能、�E高さ使用�E�E
  const float sort_y = unit_info_panel_.y + 40.0f;
  const float sort_bar_h = 32.0f;
  const float list_top = sort_y + sort_bar_h + 10.0f;
  const float list_height =
      unit_info_panel_.height - (list_top - unit_info_panel_.y) - 10.0f;

  for (int i = 0; i < static_cast<int>(unit_info_panel_.entries.size()); ++i) {
    float item_y = list_top + (i - unit_info_panel_.scroll_offset) *
                                  unit_info_panel_.item_height;

    if (item_y < list_top || item_y >= list_top + list_height) {
      continue;
    }

    const auto *entry = unit_info_panel_.entries[i];
    if (!entry)
      continue;

    bool is_selected = (i == unit_info_panel_.selected_index);

    // 選択状態�E背景
    if (is_selected) {
      systemAPI_->Render().DrawRectangle(
          unit_info_panel_.x, item_y, unit_info_panel_.width,
          unit_info_panel_.item_height, OverlayColors::PANEL_BG_ORANGE_LIGHT);
    }

    // 名前: Lv{level}:{name}
    int level = 1;
    if (sharedContext_ && sharedContext_->gameplayDataAPI) {
      const auto st =
          sharedContext_->gameplayDataAPI->GetCharacterState(entry->id);
      level = std::max(1, st.level);
    }
    // ロック状態を取得
    bool is_locked = false;
    if (sharedContext_ && sharedContext_->gameplayDataAPI) {
      const auto st =
          sharedContext_->gameplayDataAPI->GetCharacterState(entry->id);
      is_locked = !st.unlocked;
    }

    const std::string label =
        is_locked ? "未所持"
                  : ("Lv" + std::to_string(level) + ":" + entry->name);

    // ロックされたキャラはグレーアウト
    Color text_color = OverlayColors::TEXT_SECONDARY;
    if (is_selected && !is_locked) {
      text_color = WHITE;
    } else if (is_locked) {
      text_color = OverlayColors::TEXT_MUTED;
    }

    systemAPI_->Render().DrawTextDefault(label, unit_info_panel_.x + 15.0f,
                                         item_y + 15.0f, 20.0f, text_color);

    // ロックアイコン表示
    if (is_locked) {
      systemAPI_->Render().DrawTextDefault(
          "🔒", unit_info_panel_.x + unit_info_panel_.width - 30.0f,
          item_y + 15.0f, 18.0f, OverlayColors::TEXT_MUTED);
    }
  }
}

void CharacterEnhancementOverlay::RenderSortUI() {
  using namespace ui;

  // ユニット一覧パネルの上部（タイトルの下）にソートUIを配置
  const float sort_x = unit_info_panel_.x + 10.0f;
  const float sort_y = unit_info_panel_.y + 40.0f;
  const float sort_w = unit_info_panel_.width - 20.0f;
  const float sort_bar_h = 32.0f;
  const float sort_bar_y = sort_y;

  // ソートラベル
  systemAPI_->Render().DrawTextDefault("ソート", sort_x, sort_bar_y - 20.0f,
                                       16.0f, OverlayColors::TEXT_GOLD);
  systemAPI_->Render().DrawRectangle(sort_x, sort_bar_y, sort_w, sort_bar_h,
                                     OverlayColors::PANEL_BG_SECONDARY);
  systemAPI_->Render().DrawRectangleLines(sort_x, sort_bar_y, sort_w,
                                          sort_bar_h, 2.0f,
                                          OverlayColors::BORDER_DEFAULT);

  auto sortKeyLabel = [](SortKey k) -> const char * {
    switch (k) {
    case SortKey::Name:
      return "名前";
    case SortKey::Rarity:
      return "レア";
    case SortKey::Cost:
      return "コスト";
    case SortKey::Level:
      return "レベル";
    case SortKey::Owned:
      return "所持";
    default:
      return "SORT";
    }
  };

  float btn_h = sort_bar_h - 6.0f;
  float sort_btn_y = sort_bar_y + 3.0f;
  float btn_gap = 6.0f;
  float toggle_w = 70.0f;
  float btn_w = (sort_w - toggle_w - btn_gap * 6.0f) / 5.0f;

  SortKey keys[5] = {SortKey::Name, SortKey::Rarity, SortKey::Cost,
                     SortKey::Level, SortKey::Owned};

  for (int i = 0; i < 5; ++i) {
    float x = sort_x + btn_gap + i * (btn_w + btn_gap);
    bool active = (currentSortKey_ == keys[i]);
    systemAPI_->Render().DrawRectangle(x, sort_btn_y, btn_w, btn_h,
                                       active ? OverlayColors::CARD_BG_SELECTED
                                              : OverlayColors::CARD_BG_NORMAL);
    systemAPI_->Render().DrawRectangleLines(
        x, sort_btn_y, btn_w, btn_h, active ? 3.0f : 2.0f,
        active ? OverlayColors::BORDER_GOLD : OverlayColors::BORDER_DEFAULT);
    Vector2 ts =
        systemAPI_->Render().MeasureTextDefault(sortKeyLabel(keys[i]), 16.0f);
    systemAPI_->Render().DrawTextDefault(
        sortKeyLabel(keys[i]), x + (btn_w - ts.x) / 2.0f,
        sort_btn_y + (btn_h - ts.y) / 2.0f, 16.0f, OverlayColors::TEXT_PRIMARY);
  }

  // 昇順/降順トグル
  const float toggle_x = sort_x + sort_w - toggle_w - btn_gap;
  const bool asc = sortAscending_;
  systemAPI_->Render().DrawRectangle(toggle_x, sort_btn_y, toggle_w, btn_h,
                                     OverlayColors::CARD_BG_NORMAL);
  systemAPI_->Render().DrawRectangleLines(toggle_x, sort_btn_y, toggle_w, btn_h,
                                          2.0f, OverlayColors::BORDER_DEFAULT);
  systemAPI_->Render().DrawTextDefault(asc ? "↑昇順" : "↓降順", toggle_x + 8.0f,
                                       sort_btn_y + 6.0f, 14.0f,
                                       OverlayColors::TEXT_SECONDARY);

  // 保持チェックボックスは廃止
}

void CharacterEnhancementOverlay::RenderStatusPanel(SharedContext &ctx) {
  using namespace ui;

  // 背景
  systemAPI_->Render().DrawRectangle(status_panel_.x, status_panel_.y,
                                     status_panel_.width, status_panel_.height,
                                     OverlayColors::ORANGE_PANEL_BG_DARK);

  // ゴールド�Eーダー
  systemAPI_->Render().DrawRectangleLines(
      status_panel_.x, status_panel_.y, status_panel_.width,
      status_panel_.height, 2.0f, OverlayColors::BORDER_GOLD);

  // タイトル
  systemAPI_->Render().DrawTextDefault(
      "ステータス", status_panel_.x + status_panel_.padding,
      status_panel_.y + 20.0f, 24.0f, OverlayColors::TEXT_GOLD);

  float start_y = status_panel_.y + 70.0f;
  float x = status_panel_.x + status_panel_.padding;
  float width = status_panel_.width - status_panel_.padding * 2.0f;

  // ロック状態を取得
  const auto *character = GetSelectedCharacter();
  bool is_locked = false;
  if (character && ctx.gameplayDataAPI) {
    const auto st = ctx.gameplayDataAPI->GetCharacterState(character->id);
    is_locked = !st.unlocked;
  }

  // ロックされたキャラの場合は「ロック中」メッセージを表示
  if (is_locked) {
    systemAPI_->Render().DrawTextDefault("🔒 ロック中", x, start_y, 28.0f,
                                         OverlayColors::TEXT_MUTED);
    start_y += status_panel_.line_height * 2.0f;
    systemAPI_->Render().DrawTextDefault(
        "このキャラクターはまだ獲得していません。", x, start_y, 22.0f,
        OverlayColors::TEXT_SECONDARY);
    start_y += status_panel_.line_height;
    systemAPI_->Render().DrawTextDefault(
        "ステージをクリアして獲得してください。", x, start_y, 22.0f,
        OverlayColors::TEXT_SECONDARY);
    return; // ロックされたキャラはステータス表示と編集UIをスキップ
  }

  auto renderStatLine = [&](const std::string &label,
                            const StatusPanel::StatValue &stat, int index) {
    float line_y = start_y + index * status_panel_.line_height;

    // ラベル
    systemAPI_->Render().DrawTextDefault(label, x, line_y,
                                         (float)status_panel_.font_size,
                                         OverlayColors::TEXT_SECONDARY);

    // 合計値 (強化済み)
    int total = stat.base + stat.bonus;
    std::string total_str = std::to_string(total);
    Vector2 total_size = systemAPI_->Render().MeasureTextDefault(
        total_str, (float)status_panel_.font_size);

    systemAPI_->Render().DrawTextDefault(total_str, x + width - total_size.x,
                                         line_y, (float)status_panel_.font_size,
                                         ui::OverlayColors::TEXT_PRIMARY);

    // 強化�E裁E��冁E�� (+-)
    if (stat.bonus != 0) {
      std::string bonus_str =
          (stat.bonus > 0 ? "+" : "") + std::to_string(stat.bonus);
      Color bonus_color = stat.bonus > 0 ? OverlayColors::TEXT_SUCCESS
                                         : OverlayColors::TEXT_ERROR;
      Vector2 bonus_size = systemAPI_->Render().MeasureTextDefault(
          bonus_str, (float)status_panel_.font_size * 0.8f);

      systemAPI_->Render().DrawTextDefault(
          bonus_str, x + width - total_size.x - bonus_size.x - 10.0f,
          line_y + 2.0f, (float)status_panel_.font_size * 0.8f, bonus_color);
    }
  };

  // 編成相当�E惁E��量へ拡張�E�上部チE��スト！E
  auto renderKV = [&](const std::string &label, const std::string &value,
                      int index) {
    float line_y = start_y + index * status_panel_.line_height;
    systemAPI_->Render().DrawTextDefault(label, x, line_y,
                                         (float)status_panel_.font_size,
                                         OverlayColors::TEXT_SECONDARY);
    Vector2 vs = systemAPI_->Render().MeasureTextDefault(
        value, (float)status_panel_.font_size);
    systemAPI_->Render().DrawTextDefault(value, x + width - vs.x, line_y,
                                         (float)status_panel_.font_size,
                                         OverlayColors::TEXT_PRIMARY);
  };

  int row = 0;
  renderKV("Level", std::to_string(std::max(1, status_panel_.level)), row++);

  std::string rarityStr;
  const int rarity = std::max(1, status_panel_.rarity);
  for (int i = 0; i < rarity; ++i)
    rarityStr += "★";
  if (!status_panel_.rarity_name.empty()) {
    rarityStr += " (" + status_panel_.rarity_name + ")";
  }
  renderKV("Rarity", rarityStr.empty() ? "★" : rarityStr, row++);

  renderStatLine("HP (体力)", status_panel_.hp, row++);
  renderStatLine("ATK (攻撃)", status_panel_.attack, row++);
  renderStatLine("DEF (防御)", status_panel_.defense, row++);
  renderStatLine("SPD (速度)", status_panel_.speed, row++);
  renderStatLine("RNG (射程)", status_panel_.range, row++);
  const float frequency = (status_panel_.attack_span > 0.0f)
                              ? (1.0f / status_panel_.attack_span)
                              : 0.0f;
  renderKV("攻撃速度", TextFormat("%.2f回/秒", frequency), row++);
  renderKV("Cost", std::to_string(status_panel_.cost), row++);

  auto attackTypeToString = [](entities::AttackType type) -> const char * {
    if (type == entities::AttackType::Single)
      return "単体";
    if (type == entities::AttackType::Range)
      return "範囲";
    if (type == entities::AttackType::Line)
      return "直線";
    return "不明";
  };
  auto effectTypeToString = [](entities::EffectType type) -> const char * {
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
  renderKV("Type", attackTypeToString(status_panel_.attack_type), row++);
  renderKV("Element", effectTypeToString(status_panel_.effect_type), row++);

  // ===== Lv操佁EUI�E�下部�E�E====
  const float button_h = 44.0f;
  const float row_gap = 8.0f;
  const float button_w =
      (status_panel_.width - status_panel_.padding * 2.0f - 10.0f) / 2.0f;
  const float bx = status_panel_.x + status_panel_.padding;
  const float button_y_top = status_panel_.y + status_panel_.height -
                             (button_h * 3.0f + row_gap * 2.0f + 16.0f);
  const float button_y_mid = button_y_top + button_h + row_gap;
  const float button_y_bottom = button_y_mid + button_h + row_gap;
  auto mouse =
      ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};

  Rectangle downRect{bx, button_y_top, button_w, button_h};
  Rectangle upRect{bx + button_w + 10.0f, button_y_top, button_w, button_h};
  Rectangle down5Rect{bx, button_y_mid, button_w, button_h};
  Rectangle up5Rect{bx + button_w + 10.0f, button_y_mid, button_w, button_h};
  Rectangle downMaxRect{bx, button_y_bottom, button_w, button_h};
  Rectangle upMaxRect{bx + button_w + 10.0f, button_y_bottom, button_w,
                      button_h};
  bool hoverDown =
      (mouse.x >= downRect.x && mouse.x < downRect.x + downRect.width &&
       mouse.y >= downRect.y && mouse.y < downRect.y + downRect.height);
  bool hoverUp = (mouse.x >= upRect.x && mouse.x < upRect.x + upRect.width &&
                  mouse.y >= upRect.y && mouse.y < upRect.y + upRect.height);
  bool hoverDown5 =
      (mouse.x >= down5Rect.x && mouse.x < down5Rect.x + down5Rect.width &&
       mouse.y >= down5Rect.y && mouse.y < down5Rect.y + down5Rect.height);
  bool hoverUp5 =
      (mouse.x >= up5Rect.x && mouse.x < up5Rect.x + up5Rect.width &&
       mouse.y >= up5Rect.y && mouse.y < up5Rect.y + up5Rect.height);
  bool hoverDownMax = (mouse.x >= downMaxRect.x &&
                       mouse.x < downMaxRect.x + downMaxRect.width &&
                       mouse.y >= downMaxRect.y &&
                       mouse.y < downMaxRect.y + downMaxRect.height);
  bool hoverUpMax =
      (mouse.x >= upMaxRect.x && mouse.x < upMaxRect.x + upMaxRect.width &&
       mouse.y >= upMaxRect.y && mouse.y < upMaxRect.y + upMaxRect.height);

  // 所持Goldと増減量を表示�E�所持Goldは右上タブ�Eに表示�E�E
  int ownedGold = 0;
  if (sharedContext_ && sharedContext_->gameplayDataAPI) {
    ownedGold = sharedContext_->gameplayDataAPI->GetGold();
  }
  const int curLevel = std::max(1, status_panel_.level);
  const bool canDown = (curLevel > 1);
  const bool canUpBase = (curLevel < 10);
  const int upCost = 200 * curLevel;
  const bool canUp = canUpBase && (ownedGold >= upCost);

  int possibleLevels = 0;
  int tempLevel = curLevel;
  int tempGold = ownedGold;
  while (tempLevel < 10 && tempGold >= 200 * tempLevel) {
    tempGold -= 200 * tempLevel;
    tempLevel++;
    possibleLevels++;
  }
  const bool canUpBatch = (possibleLevels > 0);
  const bool canDown5 = canDown;
  const bool canDownBatch = canDown;

  UIEffects::DrawModernButton(
      systemAPI_, downRect.x, downRect.y, downRect.width, downRect.height,
      OverlayColors::BUTTON_SECONDARY_DARK,
      OverlayColors::BUTTON_SECONDARY_BRIGHT, hoverDown, !canDown);
  UIEffects::DrawModernButton(systemAPI_, upRect.x, upRect.y, upRect.width,
                              upRect.height, OverlayColors::BUTTON_PRIMARY_DARK,
                              OverlayColors::BUTTON_PRIMARY_BRIGHT, hoverUp,
                              !canUp);
  UIEffects::DrawModernButton(
      systemAPI_, down5Rect.x, down5Rect.y, down5Rect.width, down5Rect.height,
      OverlayColors::BUTTON_SECONDARY_DARK,
      OverlayColors::BUTTON_SECONDARY_BRIGHT, hoverDown5, !canDown5);
  UIEffects::DrawModernButton(
      systemAPI_, up5Rect.x, up5Rect.y, up5Rect.width, up5Rect.height,
      OverlayColors::BUTTON_PRIMARY_DARK, OverlayColors::BUTTON_PRIMARY_BRIGHT,
      hoverUp5, !canUpBatch);
  UIEffects::DrawModernButton(
      systemAPI_, downMaxRect.x, downMaxRect.y, downMaxRect.width,
      downMaxRect.height, OverlayColors::BUTTON_SECONDARY_DARK,
      OverlayColors::BUTTON_SECONDARY_BRIGHT, hoverDownMax, !canDownBatch);
  UIEffects::DrawModernButton(
      systemAPI_, upMaxRect.x, upMaxRect.y, upMaxRect.width, upMaxRect.height,
      OverlayColors::BUTTON_PRIMARY_DARK, OverlayColors::BUTTON_PRIMARY_BRIGHT,
      hoverUpMax, !canUpBatch);

  const Color downText =
      (canDown ? OverlayColors::TEXT_DARK : OverlayColors::TEXT_PRIMARY);
  const Color upText =
      (canUp ? OverlayColors::TEXT_DARK : OverlayColors::TEXT_PRIMARY);
  const Color downBatchText =
      (canDownBatch ? OverlayColors::TEXT_DARK : OverlayColors::TEXT_PRIMARY);
  const Color upBatchText =
      (canUpBatch ? OverlayColors::TEXT_DARK : OverlayColors::TEXT_PRIMARY);

  systemAPI_->Render().DrawTextDefault("レベルダウン", downRect.x + 18.0f,
                                       downRect.y + 10.0f, 18.0f, downText);
  systemAPI_->Render().DrawTextDefault("レベルアップ", upRect.x + 18.0f,
                                       upRect.y + 10.0f, 18.0f, upText);
  systemAPI_->Render().DrawTextDefault("レベル-5", down5Rect.x + 24.0f,
                                       down5Rect.y + 10.0f, 18.0f, downText);
  systemAPI_->Render().DrawTextDefault("レベル+5", up5Rect.x + 24.0f,
                                       up5Rect.y + 10.0f, 18.0f, upBatchText);
  systemAPI_->Render().DrawTextDefault("一括ダウン", downMaxRect.x + 18.0f,
                                       downMaxRect.y + 10.0f, 18.0f,
                                       downBatchText);
  systemAPI_->Render().DrawTextDefault("一括アップ", upMaxRect.x + 18.0f,
                                       upMaxRect.y + 10.0f, 18.0f, upBatchText);
}

void CharacterEnhancementOverlay::RenderOperationPanel(SharedContext &ctx) {
  using namespace ui;

  // 背景
  systemAPI_->Render().DrawRectangle(
      operation_panel_.x, operation_panel_.y, operation_panel_.width,
      operation_panel_.height, OverlayColors::PANEL_BG_ORANGE);

  // ゴールド�Eーダー
  systemAPI_->Render().DrawRectangleLines(
      operation_panel_.x, operation_panel_.y, operation_panel_.width,
      operation_panel_.height, 2.0f, OverlayColors::BORDER_GOLD);

  // タブ�Eタン
  const float tab_width = operation_panel_.width / 2.0f;
  const float tab_height = 50.0f;
  const float tab_y = operation_panel_.y + 10.0f;

  RenderTabButton(
      operation_panel_.x, tab_y, tab_width, tab_height, "パッシブスキル",
      operation_panel_.active_tab == OperationPanel::TabType::Enhancement);
  RenderTabButton(
      operation_panel_.x + tab_width, tab_y, tab_width, tab_height, "装備",
      operation_panel_.active_tab == OperationPanel::TabType::Equipment);

  // 所持Goldの表示はホ�Eムヘッダに一本化する（このオーバ�Eレイ冁E��は表示しなぁE��E

  // タブ�E容
  if (operation_panel_.active_tab == OperationPanel::TabType::Enhancement) {
    RenderEnhancementTab(ctx);
  } else {
    RenderEquipmentTab(ctx);
  }

  // 決宁E取消�Eタンは廁E���E�即時セーブ！E
}

void CharacterEnhancementOverlay::RenderEnhancementTab(SharedContext &ctx) {
  using namespace ui;

  // パッシブスロチE��3つを描画
  for (int i = 0; i < 3; ++i) {
    RenderPassiveSlot(ctx, operation_panel_.passive_slots[i]);
  }

  // ===== Reset / Reroll ボタン =====
  const float buttons_y = operation_panel_.y +
                          operation_panel_.passive_slots[0].position.y +
                          operation_panel_.passive_slots[0].height + 18.0f;
  const float buttons_h = 44.0f;
  const float buttons_w = (operation_panel_.width - 40.0f - 10.0f) / 2.0f;
  const float bx = operation_panel_.x + 20.0f;
  Rectangle resetRect{bx, buttons_y, buttons_w, buttons_h};
  Rectangle rerollRect{bx + buttons_w + 10.0f, buttons_y, buttons_w, buttons_h};

  auto mouse_pos =
      ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};
  const bool hoverReset = (mouse_pos.x >= resetRect.x &&
                           mouse_pos.x < resetRect.x + resetRect.width &&
                           mouse_pos.y >= resetRect.y &&
                           mouse_pos.y < resetRect.y + resetRect.height);
  const bool hoverReroll = (mouse_pos.x >= rerollRect.x &&
                            mouse_pos.x < rerollRect.x + rerollRect.width &&
                            mouse_pos.y >= rerollRect.y &&
                            mouse_pos.y < rerollRect.y + rerollRect.height);
  const int ownedGold =
      (ctx.gameplayDataAPI ? ctx.gameplayDataAPI->GetGold() : 0);
  const bool canReroll = (ownedGold >= 50);

  UIEffects::DrawModernButton(
      systemAPI_, resetRect.x, resetRect.y, resetRect.width, resetRect.height,
      OverlayColors::BUTTON_SECONDARY_DARK,
      OverlayColors::BUTTON_SECONDARY_BRIGHT, hoverReset, false);
  UIEffects::DrawModernButton(
      systemAPI_, rerollRect.x, rerollRect.y, rerollRect.width,
      rerollRect.height, OverlayColors::BUTTON_PRIMARY_DARK,
      OverlayColors::BUTTON_PRIMARY_BRIGHT, hoverReroll, !canReroll);

  systemAPI_->Render().DrawTextDefault("全リセット", resetRect.x + 16.0f,
                                       resetRect.y + 10.0f, 22.0f,
                                       OverlayColors::TEXT_DARK);
  const Color rerollText =
      (canReroll ? OverlayColors::TEXT_DARK : OverlayColors::TEXT_PRIMARY);
  systemAPI_->Render().DrawTextDefault("全リロール (-50G)",
                                       rerollRect.x + 12.0f,
                                       rerollRect.y + 10.0f, 20.0f, rerollText);

  // ===== 下部: パッシブ一覧�E�効果表�E�スクロール�E�E====
  // タイトルはボタン直下に置き、リスト枠と重ならなぁE��ぁE��する
  const float list_title_y = buttons_y + buttons_h + 10.0f;
  const float list_y = list_title_y + 26.0f;
  const float list_x = operation_panel_.x + 20.0f;
  const float list_w = operation_panel_.width - 40.0f;
  const float list_h =
      operation_panel_.y + operation_panel_.height - list_y - 20.0f;
  const float row_h = 58.0f;

  systemAPI_->Render().DrawTextDefault("パッシブ一覧 / Lv別効极E(Lv1-3)",
                                       list_x, list_title_y, 18.0f,
                                       OverlayColors::TEXT_GOLD);

  // 背景枠
  systemAPI_->Render().DrawRectangle(list_x, list_y, list_w, list_h,
                                     OverlayColors::ORANGE_PANEL_BG_DARK);
  systemAPI_->Render().DrawRectangleLines(list_x, list_y, list_w, list_h, 2.0f,
                                          OverlayColors::BORDER_DEFAULT);

  auto targetStatToShort = [](entities::PassiveTargetStat st) -> const char * {
    switch (st) {
    case entities::PassiveTargetStat::Attack:
      return "ATK";
    case entities::PassiveTargetStat::Defense:
      return "DEF";
    case entities::PassiveTargetStat::Hp:
      return "HP";
    case entities::PassiveTargetStat::MoveSpeed:
      return "SPD";
    case entities::PassiveTargetStat::Range:
      return "RNG";
    case entities::PassiveTargetStat::AttackSpeed:
      return "AS";
    default:
      return "-";
    }
  };

  auto formatLvEffect = [&](const entities::PassiveSkill &p,
                            int lv) -> std::string {
    const float v = p.value * static_cast<float>(lv);
    const char *stat = targetStatToShort(p.target_stat);
    if (p.effect_type == entities::PassiveEffectType::Percentage) {
      // MoveSpeed は基本皁E��「上�E」前提�E表示�E�負符号が�Eるケースを吸収！E
      if (p.target_stat == entities::PassiveTargetStat::MoveSpeed) {
        const int pct = static_cast<int>(std::round(std::abs(v) * 100.0f));
        return TextFormat("%s +%d%%", stat, pct);
      }
      return TextFormat("%s %+d%%", stat,
                        static_cast<int>(std::round(v * 100.0f)));
    }
    if (p.target_stat == entities::PassiveTargetStat::AttackSpeed) {
      // flat は攻撁E��隔短縮�E�秒！E
      return TextFormat("%s -%.2fs", stat, v);
    }
    if (p.target_stat == entities::PassiveTargetStat::MoveSpeed) {
      const int iv = static_cast<int>(std::round(std::abs(v)));
      return TextFormat("%s +%d", stat, iv);
    }
    return TextFormat("%s %+d", stat, static_cast<int>(std::round(v)));
  };

  // ヘッダー衁E
  systemAPI_->Render().DrawTextDefault("Name", list_x + 10.0f, list_y + 8.0f,
                                       32.0f, OverlayColors::TEXT_SECONDARY);
  systemAPI_->Render().DrawTextDefault("R", list_x + list_w * 0.45f,
                                       list_y + 8.0f, 32.0f,
                                       OverlayColors::TEXT_SECONDARY);
  systemAPI_->Render().DrawTextDefault("Lv1", list_x + list_w * 0.52f,
                                       list_y + 8.0f, 32.0f,
                                       OverlayColors::TEXT_SECONDARY);
  systemAPI_->Render().DrawTextDefault("Lv2", list_x + list_w * 0.70f,
                                       list_y + 8.0f, 32.0f,
                                       OverlayColors::TEXT_SECONDARY);
  systemAPI_->Render().DrawTextDefault("Lv3", list_x + list_w * 0.86f,
                                       list_y + 8.0f, 32.0f,
                                       OverlayColors::TEXT_SECONDARY);

  // 一覧�E�オフセチE��はヘッダーの1行�E�E�E
  const float content_y0 = list_y + 30.0f;
  const float content_h = list_h - 36.0f;

  for (int i = 0;
       i < static_cast<int>(operation_panel_.available_passives.size()); ++i) {
    const float y =
        content_y0 + (i - operation_panel_.passive_scroll_offset) * row_h;
    if (y < content_y0 || y >= content_y0 + content_h - row_h) {
      continue;
    }
    const auto *p = operation_panel_.available_passives[i];
    if (!p)
      continue;

    // 行背景
    systemAPI_->Render().DrawRectangle(list_x + 6.0f, y, list_w - 12.0f,
                                       row_h - 6.0f,
                                       OverlayColors::PANEL_BG_ORANGE);
    systemAPI_->Render().DrawRectangleLines(list_x + 6.0f, y, list_w - 12.0f,
                                            row_h - 6.0f, 1.0f,
                                            OverlayColors::BORDER_DEFAULT);

    // 名前
    systemAPI_->Render().DrawTextDefault(p->name, list_x + 12.0f, y + 8.0f,
                                         32.0f, WHITE);
    // レア
    std::string stars;
    const int passive_rarity = std::max(1, p->rarity);
    for (int s = 0; s < passive_rarity; ++s)
      stars += "★";
    systemAPI_->Render().DrawTextDefault(stars, list_x + list_w * 0.45f,
                                         y + 10.0f, 28.0f,
                                         OverlayColors::TEXT_GOLD);

    // 効极E
    systemAPI_->Render().DrawTextDefault(formatLvEffect(*p, 1),
                                         list_x + list_w * 0.52f, y + 10.0f,
                                         28.0f, OverlayColors::TEXT_SECONDARY);
    systemAPI_->Render().DrawTextDefault(formatLvEffect(*p, 2),
                                         list_x + list_w * 0.70f, y + 10.0f,
                                         28.0f, OverlayColors::TEXT_SECONDARY);
    systemAPI_->Render().DrawTextDefault(formatLvEffect(*p, 3),
                                         list_x + list_w * 0.86f, y + 10.0f,
                                         28.0f, OverlayColors::TEXT_SECONDARY);
  }
}

void CharacterEnhancementOverlay::RenderEquipmentTab(SharedContext &ctx) {
  using namespace ui;

  // アイチE��スロチE��3つを描画
  for (int i = 0; i < 3; ++i) {
    RenderItemSlot(operation_panel_.item_slots[i]);
  }

  // ===== ソーチEI�E�裁E��スロチE��と所持一覧の間！E====
  // 余白調整: ソートと所持アイチE��一覧を少し下げめE
  const float sort_bar_y_relative =
      operation_panel_.passive_slots[0].position.y +
      operation_panel_.passive_slots[0].height + 30.0f;
  const float sort_bar_y = operation_panel_.y + sort_bar_y_relative;
  const float sort_bar_h = 44.0f;
  const float sort_x = operation_panel_.x + 20.0f;
  const float sort_w = operation_panel_.width - 40.0f;
  systemAPI_->Render().DrawTextDefault("ソート", sort_x, sort_bar_y - 26.0f,
                                       18.0f, OverlayColors::TEXT_GOLD);
  systemAPI_->Render().DrawRectangle(sort_x, sort_bar_y, sort_w, sort_bar_h,
                                     OverlayColors::ORANGE_PANEL_BG_DARK);
  systemAPI_->Render().DrawRectangleLines(sort_x, sort_bar_y, sort_w,
                                          sort_bar_h, 2.0f,
                                          OverlayColors::BORDER_DEFAULT);

  auto sortKeyLabel = [](OperationPanel::ItemSortKey k) -> const char * {
    switch (k) {
    case OperationPanel::ItemSortKey::Name:
      return "名前";
    case OperationPanel::ItemSortKey::OwnedCount:
      return "所持数";
    case OperationPanel::ItemSortKey::Attack:
      return "ATK";
    case OperationPanel::ItemSortKey::Defense:
      return "DEF";
    case OperationPanel::ItemSortKey::Hp:
      return "HP";
    default:
      return "SORT";
    }
  };

  const float btn_h = sort_bar_h - 8.0f;
  const float sort_btn_y = sort_bar_y + 4.0f;
  const float btn_gap = 8.0f;
  const float toggle_w = 90.0f;
  const float btn_w = (sort_w - toggle_w - btn_gap * 6.0f) / 5.0f;

  OperationPanel::ItemSortKey keys[5] = {
      OperationPanel::ItemSortKey::Name,
      OperationPanel::ItemSortKey::OwnedCount,
      OperationPanel::ItemSortKey::Attack, OperationPanel::ItemSortKey::Defense,
      OperationPanel::ItemSortKey::Hp};

  for (int i = 0; i < 5; ++i) {
    const float x = sort_x + btn_gap + i * (btn_w + btn_gap);
    const bool active = (operation_panel_.item_sort_key == keys[i]);
    systemAPI_->Render().DrawRectangle(x, sort_btn_y, btn_w, btn_h,
                                       active ? OverlayColors::CARD_BG_SELECTED
                                              : OverlayColors::CARD_BG_NORMAL);
    systemAPI_->Render().DrawRectangleLines(
        x, sort_btn_y, btn_w, btn_h, active ? 3.0f : 2.0f,
        active ? OverlayColors::BORDER_GOLD : OverlayColors::BORDER_DEFAULT);
    Vector2 ts =
        systemAPI_->Render().MeasureTextDefault(sortKeyLabel(keys[i]), 18.0f);
    systemAPI_->Render().DrawTextDefault(
        sortKeyLabel(keys[i]), x + (btn_w - ts.x) / 2.0f,
        sort_btn_y + (btn_h - ts.y) / 2.0f, 18.0f, OverlayColors::TEXT_PRIMARY);
  }

  // 昁E��トグル
  const float toggle_x = sort_x + sort_w - toggle_w - btn_gap;
  const bool asc = operation_panel_.item_sort_ascending;
  systemAPI_->Render().DrawRectangle(toggle_x, sort_btn_y, toggle_w, btn_h,
                                     OverlayColors::CARD_BG_NORMAL);
  systemAPI_->Render().DrawRectangleLines(toggle_x, sort_btn_y, toggle_w, btn_h,
                                          2.0f, OverlayColors::BORDER_DEFAULT);
  systemAPI_->Render().DrawTextDefault(asc ? "↑昇順" : "↓降順",
                                       toggle_x + 12.0f, sort_btn_y + 10.0f,
                                       18.0f, OverlayColors::TEXT_SECONDARY);

  // ===== インベントリ�E�E列、スクロール可能�E�E====
  const float list_title_y = sort_bar_y + sort_bar_h + 10.0f;
  const float list_y = list_title_y + 26.0f;
  const float footer_h = 70.0f;
  const float list_height = (operation_panel_.y + operation_panel_.height) -
                            list_y - footer_h - 10.0f;

  const float item_h = 80.0f;
  const float gap_x = 10.0f;
  const float list_x = operation_panel_.x + 20.0f;
  const float list_w = operation_panel_.width - 40.0f;
  const float col_w = (list_w - gap_x) / 2.0f;

  systemAPI_->Render().DrawTextDefault("所持アイテム一覧（ドラッグで装備）",
                                       list_x, list_title_y, 20.0f,
                                       OverlayColors::TEXT_GOLD);

  // 枠
  systemAPI_->Render().DrawRectangle(list_x, list_y, list_w, list_height,
                                     OverlayColors::ORANGE_PANEL_BG_DARK);
  systemAPI_->Render().DrawRectangleLines(list_x, list_y, list_w, list_height,
                                          2.0f, OverlayColors::BORDER_DEFAULT);

  // 裁E��中�E�同名�E全キャラ�E�E
  std::unordered_set<std::string> equippedNames;
  if (sharedContext_) {
    equippedNames = BuildEquippedEquipmentNames(*sharedContext_);
  }

  // ソート（描画前に反映�E�E
  const auto getOwned = [&](const std::string &id) -> int {
    if (!sharedContext_ || !sharedContext_->gameplayDataAPI)
      return 0;
    return sharedContext_->gameplayDataAPI->GetOwnedEquipmentCount(id);
  };
  const bool activeKeyIsNumeric =
      (operation_panel_.item_sort_key != OperationPanel::ItemSortKey::Name);
  // 初期想宁E 数値系は降頁E��名前�E昁E��E
  bool ascOrder = operation_panel_.item_sort_ascending;
  if (operation_panel_.item_sort_key != OperationPanel::ItemSortKey::Name &&
      operation_panel_.item_sort_ascending) {
    // 数値系はユーザーが�E頁E��刁E��替えたときだけ�E頁E���E期�E降頁E��したぁE�Eでここではそ�Eまま
    (void)activeKeyIsNumeric;
  }
  std::stable_sort(
      operation_panel_.available_items.begin(),
      operation_panel_.available_items.end(),
      [&](const entities::Equipment *a, const entities::Equipment *b) {
        if (!a || !b)
          return false;
        auto cmpStrAsc = [&](const std::string &lhs, const std::string &rhs) {
          return lhs < rhs;
        };
        auto cmpInt = [&](int lhs, int rhs) {
          return ascOrder ? (lhs < rhs) : (lhs > rhs);
        };

        switch (operation_panel_.item_sort_key) {
        case OperationPanel::ItemSortKey::OwnedCount:
          if (getOwned(a->id) != getOwned(b->id))
            return cmpInt(getOwned(a->id), getOwned(b->id));
          break;
        case OperationPanel::ItemSortKey::Attack:
          if (static_cast<int>(a->attack_bonus) !=
              static_cast<int>(b->attack_bonus))
            return cmpInt(static_cast<int>(a->attack_bonus),
                          static_cast<int>(b->attack_bonus));
          break;
        case OperationPanel::ItemSortKey::Defense:
          if (static_cast<int>(a->defense_bonus) !=
              static_cast<int>(b->defense_bonus))
            return cmpInt(static_cast<int>(a->defense_bonus),
                          static_cast<int>(b->defense_bonus));
          break;
        case OperationPanel::ItemSortKey::Hp:
          if (static_cast<int>(a->hp_bonus) != static_cast<int>(b->hp_bonus))
            return cmpInt(static_cast<int>(a->hp_bonus),
                          static_cast<int>(b->hp_bonus));
          break;
        case OperationPanel::ItemSortKey::Name:
        default:
          if (a->name != b->name)
            return ascOrder ? cmpStrAsc(a->name, b->name)
                            : cmpStrAsc(b->name, a->name);
          break;
        }
        // tie-breaker
        if (a->name != b->name)
          return a->name < b->name;
        return a->id < b->id;
      });

  const int total = static_cast<int>(operation_panel_.available_items.size());
  const int totalRows = (total + 1) / 2;
  const int visibleRows =
      std::max(1, static_cast<int>(std::floor(list_height / item_h)));
  operation_panel_.item_scroll_offset =
      std::max(0, std::min(operation_panel_.item_scroll_offset,
                           std::max(0, totalRows - visibleRows)));

  for (int i = 0; i < total; ++i) {
    const int row = i / 2;
    const int col = i % 2;
    const float x = list_x + col * (col_w + gap_x);
    const float y =
        list_y + (row - operation_panel_.item_scroll_offset) * item_h;

    if (y < list_y || y >= list_y + list_height - item_h) {
      continue;
    }

    const auto *item = operation_panel_.available_items[i];
    if (!item)
      continue;

    const bool isEquippedByName =
        (!equippedNames.empty() &&
         equippedNames.find(item->name) != equippedNames.end());
    const Color bg = isEquippedByName ? OverlayColors::CARD_BG_SELECTED
                                      : OverlayColors::PANEL_BG_ORANGE;
    const Color border = isEquippedByName ? OverlayColors::BORDER_GOLD
                                          : OverlayColors::BORDER_DEFAULT;

    // 背景
    systemAPI_->Render().DrawRectangle(x, y, col_w, item_h - 6.0f, bg);
    systemAPI_->Render().DrawRectangleLines(x, y, col_w, item_h - 6.0f, 2.0f,
                                            border);

    const float icon_size = 24.0f;
    const float text_x = x + 10.0f + icon_size + 8.0f;

    if (!item->icon_path.empty()) {
      void *texturePtr = systemAPI_->Resource().GetTexture(item->icon_path);
      if (texturePtr) {
        Texture2D *texture = static_cast<Texture2D *>(texturePtr);
        if (texture && texture->id != 0) {
          Rectangle src{0.0f, 0.0f, static_cast<float>(texture->width),
                        static_cast<float>(texture->height)};
          Rectangle dst{x + 10.0f, y + 10.0f, icon_size, icon_size};
          systemAPI_->Render().DrawTexturePro(*texture, src, dst, {0.0f, 0.0f},
                                              0.0f, WHITE);
        }
      }
    }

    // 名前
    systemAPI_->Render().DrawTextDefault(item->name, text_x, y + 12.0f, 18.0f,
                                         WHITE);
    // 所持数
    if (sharedContext_ && sharedContext_->gameplayDataAPI) {
      const int owned =
          sharedContext_->gameplayDataAPI->GetOwnedEquipmentCount(item->id);
      systemAPI_->Render().DrawTextDefault(TextFormat("x%d", owned),
                                           x + col_w - 52.0f, y + 12.0f, 16.0f,
                                           OverlayColors::TEXT_SECONDARY);
    }
    // 裁E��中ラベル
    if (isEquippedByName) {
      systemAPI_->Render().DrawTextDefault("裁E��中", x + col_w - 90.0f,
                                           y + 34.0f, 14.0f,
                                           OverlayColors::TEXT_GOLD);
    }

    // スチE�Eタス
    std::string stats;
    if (item->hp_bonus != 0)
      stats += "HP" + std::string(item->hp_bonus > 0 ? "+" : "") +
               std::to_string(static_cast<int>(item->hp_bonus)) + " ";
    if (item->attack_bonus != 0)
      stats += "ATK" + std::string(item->attack_bonus > 0 ? "+" : "") +
               std::to_string(static_cast<int>(item->attack_bonus)) + " ";
    if (item->defense_bonus != 0)
      stats += "DEF" + std::string(item->defense_bonus > 0 ? "+" : "") +
               std::to_string(static_cast<int>(item->defense_bonus));
    systemAPI_->Render().DrawTextDefault(stats, text_x, y + 44.0f, 14.0f,
                                         OverlayColors::TEXT_SUCCESS);
  }

  // ドラチE��中のプレビュー描画
  if (is_item_dragging_ && dragging_item_) {
    Rectangle rec{item_drag_pos_.x - 140.0f, item_drag_pos_.y - 30.0f, 280.0f,
                  60.0f};
    Color bg = OverlayColors::SLOT_ORANGE_SELECTED;
    bg.a = 200;
    systemAPI_->Render().DrawRectangle(rec.x, rec.y, rec.width, rec.height, bg);
    systemAPI_->Render().DrawRectangleLines(rec.x, rec.y, rec.width, rec.height,
                                            2.0f, OverlayColors::BORDER_GOLD);
    systemAPI_->Render().DrawTextDefault(dragging_item_->name, rec.x + 10.0f,
                                         rec.y + 10.0f, 16.0f, WHITE);
  }

  // フッター: すべて外す
  const float btn_y = operation_panel_.y + operation_panel_.height - footer_h;
  Rectangle removeAllRect{operation_panel_.x + 20.0f, btn_y + 8.0f,
                          operation_panel_.width - 40.0f, 44.0f};
  auto mouse_pos =
      ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};
  const bool hover = (mouse_pos.x >= removeAllRect.x &&
                      mouse_pos.x < removeAllRect.x + removeAllRect.width &&
                      mouse_pos.y >= removeAllRect.y &&
                      mouse_pos.y < removeAllRect.y + removeAllRect.height);
  UIEffects::DrawModernButton(
      systemAPI_, removeAllRect.x, removeAllRect.y, removeAllRect.width,
      removeAllRect.height, OverlayColors::BUTTON_SECONDARY_DARK,
      OverlayColors::BUTTON_SECONDARY_BRIGHT, hover, false);
  systemAPI_->Render().DrawTextDefault("すべて外す", removeAllRect.x + 18.0f,
                                       removeAllRect.y + 10.0f, 22.0f,
                                       OverlayColors::TEXT_DARK);
}

void CharacterEnhancementOverlay::RenderPassivePopup(SharedContext &ctx) {
  using namespace ui;

  // 半透�E背景
  systemAPI_->Render().DrawRectangle(0, 0, 1920, 1080,
                                     OverlayColors::OVERLAY_BG);

  // ポップアチE�Eボックス
  const float popup_width = 400.0f;
  const float popup_height = 300.0f;
  const float popup_x = (1920.0f - popup_width) / 2.0f;
  const float popup_y = (1080.0f - popup_height) / 2.0f;

  systemAPI_->Render().DrawRectangle(popup_x, popup_y, popup_width,
                                     popup_height,
                                     OverlayColors::PANEL_BG_ORANGE);
  systemAPI_->Render().DrawRectangleLines(popup_x, popup_y, popup_width,
                                          popup_height, 3.0f,
                                          OverlayColors::BORDER_GOLD);

  // タイトル
  systemAPI_->Render().DrawTextDefault("パッシブ設定", popup_x + 20.0f,
                                       popup_y + 20.0f, 24.0f,
                                       OverlayColors::TEXT_GOLD);

  // 選択肢
  const float option_height = 50.0f;
  const float option_y_start = popup_y + 70.0f;
  auto mouse_pos =
      ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};

  const auto &slot =
      operation_panel_.passive_slots[operation_panel_.popup_slot_id];
  bool is_empty = (slot.assigned_passive == nullptr);

  std::vector<PopupMenuItem> options;
  if (is_empty) {
    options.push_back({"ランダム付与", OverlayColors::TEXT_SUCCESS, 0, false});
    options.push_back({"キャンセル", OverlayColors::TEXT_SECONDARY, 3, false});
  } else {
    options.push_back({"強化(Lv+" + std::to_string(1) + ")",
                       OverlayColors::TEXT_GOLD, 0, false});
    options.push_back(
        {"ランダム変更", OverlayColors::STATUS_NEUTRAL, 1, false});
    options.push_back({"削除", OverlayColors::TEXT_ERROR, 2, false});
    options.push_back({"キャンセル", OverlayColors::TEXT_SECONDARY, 3, false});
  }

  for (size_t i = 0; i < options.size(); ++i) {
    float option_y = option_y_start + i * (option_height + 10.0f);
    bool hover =
        (mouse_pos.x >= popup_x + 20.0f &&
         mouse_pos.x < popup_x + popup_width - 20.0f &&
         mouse_pos.y >= option_y && mouse_pos.y < option_y + option_height);

    Color bg_color = hover ? OverlayColors::PANEL_BG_ORANGE_LIGHT
                           : OverlayColors::ORANGE_PANEL_BG_DARK;

    systemAPI_->Render().DrawRectangle(popup_x + 20.0f, option_y,
                                       popup_width - 40.0f, option_height,
                                       bg_color);
    systemAPI_->Render().DrawRectangleLines(popup_x + 20.0f, option_y,
                                            popup_width - 40.0f, option_height,
                                            2.0f, options[i].color);

    Vector2 text_size =
        systemAPI_->Render().MeasureTextDefault(options[i].label, 20.0f);
    systemAPI_->Render().DrawTextDefault(
        options[i].label, popup_x + (popup_width - text_size.x) / 2.0f,
        option_y + (option_height - text_size.y) / 2.0f, 20.0f, WHITE);
  }
}

void CharacterEnhancementOverlay::RenderItemPopup(SharedContext &ctx) {
  using namespace ui;

  // 半透�E背景
  systemAPI_->Render().DrawRectangle(0, 0, 1920, 1080,
                                     OverlayColors::OVERLAY_BG);

  const float popup_width = 420.0f;
  const float popup_height = 260.0f;
  const float popup_x = (1920.0f - popup_width) / 2.0f;
  const float popup_y = (1080.0f - popup_height) / 2.0f;

  systemAPI_->Render().DrawRectangle(popup_x, popup_y, popup_width,
                                     popup_height,
                                     OverlayColors::PANEL_BG_ORANGE);
  systemAPI_->Render().DrawRectangleLines(popup_x, popup_y, popup_width,
                                          popup_height, 3.0f,
                                          OverlayColors::BORDER_GOLD);

  systemAPI_->Render().DrawTextDefault("装備スロット設定", popup_x + 20.0f,
                                       popup_y + 20.0f, 24.0f,
                                       OverlayColors::TEXT_GOLD);

  const int slotId = operation_panel_.popup_item_slot_id;
  if (slotId < 0 || slotId >= 3) {
    return;
  }
  const auto &slot = operation_panel_.item_slots[slotId];
  const bool is_empty = (slot.assigned_item == nullptr);

  const float option_height = 50.0f;
  const float option_y_start = popup_y + 70.0f;
  auto mouse_pos =
      ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};

  std::vector<PopupMenuItem> options;
  options.push_back(
      {"このスロットに装備する", OverlayColors::TEXT_GOLD, 0, false});
  if (!is_empty) {
    options.push_back({"外す", OverlayColors::TEXT_ERROR, 1, false});
  }
  options.push_back({"キャンセル", OverlayColors::TEXT_SECONDARY, 2, false});

  for (size_t i = 0; i < options.size(); ++i) {
    float option_y =
        option_y_start + static_cast<float>(i) * (option_height + 10.0f);
    bool hover =
        (mouse_pos.x >= popup_x + 20.0f &&
         mouse_pos.x < popup_x + popup_width - 20.0f &&
         mouse_pos.y >= option_y && mouse_pos.y < option_y + option_height);

    Color bg_color = hover ? OverlayColors::PANEL_BG_ORANGE_LIGHT
                           : OverlayColors::ORANGE_PANEL_BG_DARK;
    systemAPI_->Render().DrawRectangle(popup_x + 20.0f, option_y,
                                       popup_width - 40.0f, option_height,
                                       bg_color);
    systemAPI_->Render().DrawRectangleLines(popup_x + 20.0f, option_y,
                                            popup_width - 40.0f, option_height,
                                            2.0f, options[i].color);

    Vector2 text_size =
        systemAPI_->Render().MeasureTextDefault(options[i].label, 20.0f);
    systemAPI_->Render().DrawTextDefault(
        options[i].label, popup_x + (popup_width - text_size.x) / 2.0f,
        option_y + (option_height - text_size.y) / 2.0f, 20.0f, WHITE);
  }
}

void CharacterEnhancementOverlay::RenderPassiveSlot(
    SharedContext &ctx, const OperationPanel::PassiveSlot &slot) {
  using namespace ui;

  // スロチE��座標を操作パネル基準で計箁E
  const float abs_x = operation_panel_.x + slot.position.x;
  const float abs_y = operation_panel_.y + slot.position.y;

  Color bg_color = slot.is_hovered ? OverlayColors::SLOT_ORANGE_SELECTED
                                   : OverlayColors::SLOT_ORANGE_EMPTY;

  systemAPI_->Render().DrawRectangle(abs_x, abs_y, slot.width, slot.height,
                                     bg_color);
  systemAPI_->Render().DrawRectangleLines(abs_x, abs_y, slot.width, slot.height,
                                          2.0f, OverlayColors::BORDER_GOLD);

  if (slot.assigned_passive) {
    // パッシブ名
    systemAPI_->Render().DrawTextDefault(slot.assigned_passive->name,
                                         abs_x + 10.0f, abs_y + 20.0f, 18.0f,
                                         WHITE);

    // レベル
    std::string level_text = "Lv." + std::to_string(slot.level);
    systemAPI_->Render().DrawTextDefault(level_text, abs_x + 10.0f,
                                         abs_y + 50.0f, 16.0f,
                                         OverlayColors::TEXT_GOLD);

    // 効果値
    std::string value_text =
        "効极E +" + std::to_string(static_cast<int>(
                        slot.assigned_passive->value * slot.level));
    systemAPI_->Render().DrawTextDefault(value_text, abs_x + 10.0f,
                                         abs_y + 75.0f, 14.0f,
                                         OverlayColors::TEXT_SECONDARY);
  } else {
    // 空スロチE��
    Vector2 plus_size = systemAPI_->Render().MeasureTextDefault("+", 40.0f);
    systemAPI_->Render().DrawTextDefault(
        "+", abs_x + (slot.width - plus_size.x) / 2.0f, abs_y + 40.0f, 40.0f,
        OverlayColors::TEXT_SECONDARY);

    Vector2 empty_size =
        systemAPI_->Render().MeasureTextDefault("空きスロット", 16.0f);
    systemAPI_->Render().DrawTextDefault(
        "空きスロット", abs_x + (slot.width - empty_size.x) / 2.0f,
        abs_y + 100.0f, 16.0f, OverlayColors::TEXT_SECONDARY);
  }

  const float btn_h = 28.0f;
  const float btn_padding = 10.0f;
  const float btn_gap = 6.0f;
  const float btn_y = abs_y + slot.height - btn_h - 10.0f;
  auto mouse_pos =
      ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};
  const int ownedGold =
      (ctx.gameplayDataAPI ? ctx.gameplayDataAPI->GetGold() : 0);

  if (slot.assigned_passive == nullptr) {
    const bool canAssign = !operation_panel_.available_passives.empty();
    Rectangle assignRect{abs_x + btn_padding, btn_y,
                         slot.width - btn_padding * 2.0f, btn_h};
    const bool hoverAssign = (mouse_pos.x >= assignRect.x &&
                              mouse_pos.x < assignRect.x + assignRect.width &&
                              mouse_pos.y >= assignRect.y &&
                              mouse_pos.y < assignRect.y + assignRect.height);
    UIEffects::DrawModernButton(
        systemAPI_, assignRect.x, assignRect.y, assignRect.width,
        assignRect.height, OverlayColors::BUTTON_PRIMARY_DARK,
        OverlayColors::BUTTON_PRIMARY_BRIGHT, hoverAssign, !canAssign);
    const Color assignText =
        (canAssign ? OverlayColors::TEXT_DARK : OverlayColors::TEXT_PRIMARY);
    systemAPI_->Render().DrawTextDefault(
        "付与", assignRect.x + 16.0f, assignRect.y + 6.0f, 16.0f, assignText);
  } else {
    const float btn_w =
        (slot.width - btn_padding * 2.0f - btn_gap * 2.0f) / 3.0f;
    Rectangle upgradeRect{abs_x + btn_padding, btn_y, btn_w, btn_h};
    Rectangle changeRect{abs_x + btn_padding + (btn_w + btn_gap), btn_y, btn_w,
                         btn_h};
    Rectangle removeRect{abs_x + btn_padding + (btn_w + btn_gap) * 2.0f, btn_y,
                         btn_w, btn_h};

    const bool canUpgrade = (slot.level < 3);
    const bool canChange = (ownedGold >= 25);

    const bool hoverUpgrade =
        (mouse_pos.x >= upgradeRect.x &&
         mouse_pos.x < upgradeRect.x + upgradeRect.width &&
         mouse_pos.y >= upgradeRect.y &&
         mouse_pos.y < upgradeRect.y + upgradeRect.height);
    const bool hoverChange = (mouse_pos.x >= changeRect.x &&
                              mouse_pos.x < changeRect.x + changeRect.width &&
                              mouse_pos.y >= changeRect.y &&
                              mouse_pos.y < changeRect.y + changeRect.height);
    const bool hoverRemove = (mouse_pos.x >= removeRect.x &&
                              mouse_pos.x < removeRect.x + removeRect.width &&
                              mouse_pos.y >= removeRect.y &&
                              mouse_pos.y < removeRect.y + removeRect.height);

    UIEffects::DrawModernButton(
        systemAPI_, upgradeRect.x, upgradeRect.y, upgradeRect.width,
        upgradeRect.height, OverlayColors::BUTTON_PRIMARY_DARK,
        OverlayColors::BUTTON_PRIMARY_BRIGHT, hoverUpgrade, !canUpgrade);
    UIEffects::DrawModernButton(
        systemAPI_, changeRect.x, changeRect.y, changeRect.width,
        changeRect.height, OverlayColors::BUTTON_PRIMARY_DARK,
        OverlayColors::BUTTON_PRIMARY_BRIGHT, hoverChange, !canChange);
    UIEffects::DrawModernButton(
        systemAPI_, removeRect.x, removeRect.y, removeRect.width,
        removeRect.height, OverlayColors::BUTTON_SECONDARY_DARK,
        OverlayColors::BUTTON_SECONDARY_BRIGHT, hoverRemove, false);

    const Color upgradeText =
        (canUpgrade ? OverlayColors::TEXT_DARK : OverlayColors::TEXT_PRIMARY);
    const Color changeText =
        (canChange ? OverlayColors::TEXT_DARK : OverlayColors::TEXT_PRIMARY);
    systemAPI_->Render().DrawTextDefault("強化", upgradeRect.x + 10.0f,
                                         upgradeRect.y + 6.0f, 16.0f,
                                         upgradeText);
    systemAPI_->Render().DrawTextDefault(
        "変更", changeRect.x + 10.0f, changeRect.y + 6.0f, 16.0f, changeText);
    systemAPI_->Render().DrawTextDefault("削除", removeRect.x + 10.0f,
                                         removeRect.y + 6.0f, 16.0f,
                                         OverlayColors::TEXT_DARK);
  }
}

void CharacterEnhancementOverlay::RenderItemSlot(
    const OperationPanel::ItemSlot &slot) {
  using namespace ui;

  // スロチE��座標を操作パネル基準で計箁E
  const float abs_x = operation_panel_.x + slot.position.x;
  const float abs_y = operation_panel_.y + slot.position.y;

  const bool is_selected =
      (operation_panel_.selected_item_slot_id == slot.slot_id);
  Color bg_color = (slot.is_hovered || is_selected)
                       ? OverlayColors::SLOT_ORANGE_SELECTED
                       : OverlayColors::SLOT_ORANGE_EMPTY;

  systemAPI_->Render().DrawRectangle(abs_x, abs_y, slot.width, slot.height,
                                     bg_color);
  systemAPI_->Render().DrawRectangleLines(abs_x, abs_y, slot.width, slot.height,
                                          2.0f, OverlayColors::BORDER_GOLD);

  if (slot.assigned_item) {
    // アイチE��吁E
    systemAPI_->Render().DrawTextDefault(
        slot.assigned_item->name, abs_x + 10.0f, abs_y + 20.0f, 18.0f, WHITE);

    // スチE�Eタスボ�Eナス
    std::string bonus_text = "";
    if (slot.assigned_item->hp_bonus > 0) {
      bonus_text +=
          "HP+" +
          std::to_string(static_cast<int>(slot.assigned_item->hp_bonus)) + " ";
    }
    if (slot.assigned_item->attack_bonus > 0) {
      bonus_text +=
          "ATK+" +
          std::to_string(static_cast<int>(slot.assigned_item->attack_bonus)) +
          " ";
    }
    if (slot.assigned_item->defense_bonus > 0) {
      bonus_text +=
          "DEF+" +
          std::to_string(static_cast<int>(slot.assigned_item->defense_bonus));
    }

    systemAPI_->Render().DrawTextDefault(bonus_text, abs_x + 10.0f,
                                         abs_y + 50.0f, 14.0f,
                                         OverlayColors::TEXT_SUCCESS);

    // 解除ボタン�E�右上�E×！E
    const float btnSize = 22.0f;
    Rectangle xRect{abs_x + slot.width - btnSize - 6.0f, abs_y + 6.0f, btnSize,
                    btnSize};
    systemAPI_->Render().DrawRectangle(xRect.x, xRect.y, xRect.width,
                                       xRect.height,
                                       OverlayColors::BUTTON_SECONDARY_BRIGHT);
    systemAPI_->Render().DrawRectangleLines(xRect.x, xRect.y, xRect.width,
                                            xRect.height, 2.0f,
                                            OverlayColors::BORDER_DEFAULT);
    // フォント欠け対策で「�E」を使用
    systemAPI_->Render().DrawTextDefault("ー", xRect.x + 6.0f, xRect.y + 2.0f,
                                         20.0f, OverlayColors::TEXT_DARK);
  } else {
    // 空スロチE��
    Vector2 empty_size =
        systemAPI_->Render().MeasureTextDefault("空きスロット", 20.0f);
    systemAPI_->Render().DrawTextDefault(
        "空きスロット", abs_x + (slot.width - empty_size.x) / 2.0f,
        abs_y + 70.0f, 20.0f, OverlayColors::TEXT_SECONDARY);
  }
}

void CharacterEnhancementOverlay::RenderTabButton(float x, float y, float width,
                                                  float height,
                                                  const char *label,
                                                  bool is_active) {
  using namespace ui;

  // タブ�Eタンの背景色を統一�E�選択時は明るぁE��景、E��選択時は通常背景�E�E
  Color bg_color = is_active ? OverlayColors::CARD_BG_SELECTED
                             : OverlayColors::CARD_BG_NORMAL;
  // 斁E���E白系で統一�E�黒�E見づらいので避ける�E�E
  Color text_color =
      is_active ? OverlayColors::TEXT_PRIMARY : OverlayColors::TEXT_SECONDARY;
  Color border_color =
      is_active ? OverlayColors::BORDER_GOLD : OverlayColors::BORDER_DEFAULT;
  float border_width = is_active ? 3.0f : 2.0f;

  systemAPI_->Render().DrawRectangle(x, y, width, height, bg_color);
  systemAPI_->Render().DrawRectangleLines(x, y, width, height, border_width,
                                          border_color);

  // 選択中のタブ�E上部にアクセントラインを追加
  if (is_active) {
    systemAPI_->Render().DrawLine(x, y, x + width, y, 3.0f,
                                  OverlayColors::ACCENT_GOLD);
  }

  Vector2 text_size = systemAPI_->Render().MeasureTextDefault(label, 26.0f);
  systemAPI_->Render().DrawTextDefault(label, x + (width - text_size.x) / 2.0f,
                                       y + (height - text_size.y) / 2.0f, 26.0f,
                                       text_color);
}

// ========== イベント�E琁E��ソチE�� ==========

void CharacterEnhancementOverlay::ProcessMouseInput(SharedContext &ctx) {
  if (ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) {
    auto mouse_pos =
        ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};

    // ソートUIクリック処理
    const float sort_x = unit_info_panel_.x + 10.0f;
    const float sort_y = unit_info_panel_.y + 40.0f;
    const float sort_w = unit_info_panel_.width - 20.0f;
    const float sort_bar_h = 32.0f;
    const float sort_bar_y = sort_y;
    float btn_h = sort_bar_h - 6.0f;
    float sort_btn_y = sort_bar_y + 3.0f;
    float btn_gap = 6.0f;
    float toggle_w = 70.0f;
    float btn_w = (sort_w - toggle_w - btn_gap * 6.0f) / 5.0f;

    if (mouse_pos.y >= sort_btn_y && mouse_pos.y < sort_btn_y + btn_h &&
        mouse_pos.x >= sort_x && mouse_pos.x < sort_x + sort_w) {
      // ソートキーボタン
      SortKey keys[5] = {SortKey::Name, SortKey::Rarity, SortKey::Cost,
                         SortKey::Level, SortKey::Owned};
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
          LoadCharacterList(ctx); // ソートと未所持非表示を再適用
          return;
        }
      }
      // 昇順/降順トグル
      float toggle_x = sort_x + sort_w - toggle_w - btn_gap;
      if (mouse_pos.x >= toggle_x && mouse_pos.x < toggle_x + toggle_w) {
        sortAscending_ = !sortAscending_;
        LoadCharacterList(ctx); // ソートを再適用
        return;
      }
    }

    // ポップアップは廃止

    // ユニット一覧クリック
    const float list_top = sort_bar_y + sort_bar_h + 10.0f;
    // 描画と同じリスト高さを使用
    const float list_height =
        unit_info_panel_.height - (list_top - unit_info_panel_.y) - 10.0f;
    for (int i = 0; i < static_cast<int>(unit_info_panel_.entries.size());
         ++i) {
      float item_y = list_top + (i - unit_info_panel_.scroll_offset) *
                                    unit_info_panel_.item_height;

      if (item_y < list_top || item_y >= list_top + list_height) {
        continue;
      }

      if (mouse_pos.x >= unit_info_panel_.x &&
          mouse_pos.x < unit_info_panel_.x + unit_info_panel_.width &&
          mouse_pos.y >= item_y &&
          mouse_pos.y < item_y + unit_info_panel_.item_height) {
        OnUnitListItemClick(ctx, i);
        return;
      }
    }

    // タブクリチE��
    const float tab_width = operation_panel_.width / 2.0f;
    const float tab_height = 50.0f;
    const float tab_y = operation_panel_.y + 10.0f;

    if (mouse_pos.y >= tab_y && mouse_pos.y < tab_y + tab_height) {
      if (mouse_pos.x >= operation_panel_.x &&
          mouse_pos.x < operation_panel_.x + tab_width) {
        OnTabClick(OperationPanel::TabType::Enhancement);
        return;
      } else if (mouse_pos.x >= operation_panel_.x + tab_width &&
                 mouse_pos.x < operation_panel_.x + operation_panel_.width) {
        OnTabClick(OperationPanel::TabType::Equipment);
        return;
      }
    }

    // レベルアチE�E/ダウン�E�スチE�Eタスパネル下部�E�E
    {
      const float button_h = 44.0f;
      const float row_gap = 8.0f;
      const float button_w =
          (status_panel_.width - status_panel_.padding * 2.0f - 10.0f) / 2.0f;
      const float bx = status_panel_.x + status_panel_.padding;
      const float button_y_top = status_panel_.y + status_panel_.height -
                                 (button_h * 3.0f + row_gap * 2.0f + 16.0f);
      const float button_y_mid = button_y_top + button_h + row_gap;
      const float button_y_bottom = button_y_mid + button_h + row_gap;
      Rectangle downRect{bx, button_y_top, button_w, button_h};
      Rectangle upRect{bx + button_w + 10.0f, button_y_top, button_w, button_h};
      Rectangle down5Rect{bx, button_y_mid, button_w, button_h};
      Rectangle up5Rect{bx + button_w + 10.0f, button_y_mid, button_w,
                        button_h};
      Rectangle downMaxRect{bx, button_y_bottom, button_w, button_h};
      Rectangle upMaxRect{bx + button_w + 10.0f, button_y_bottom, button_w,
                          button_h};
      if (mouse_pos.x >= downRect.x &&
          mouse_pos.x < downRect.x + downRect.width &&
          mouse_pos.y >= downRect.y &&
          mouse_pos.y < downRect.y + downRect.height) {
        OnLevelDownClick(ctx);
        return;
      }
      if (mouse_pos.x >= upRect.x && mouse_pos.x < upRect.x + upRect.width &&
          mouse_pos.y >= upRect.y && mouse_pos.y < upRect.y + upRect.height) {
        OnLevelUpClick(ctx);
        return;
      }
      if (mouse_pos.x >= down5Rect.x &&
          mouse_pos.x < down5Rect.x + down5Rect.width &&
          mouse_pos.y >= down5Rect.y &&
          mouse_pos.y < down5Rect.y + down5Rect.height) {
        OnLevelDownBatchClick(ctx, 5);
        return;
      }
      if (mouse_pos.x >= up5Rect.x && mouse_pos.x < up5Rect.x + up5Rect.width &&
          mouse_pos.y >= up5Rect.y &&
          mouse_pos.y < up5Rect.y + up5Rect.height) {
        OnLevelUpBatchClick(ctx, 5);
        return;
      }
      if (mouse_pos.x >= downMaxRect.x &&
          mouse_pos.x < downMaxRect.x + downMaxRect.width &&
          mouse_pos.y >= downMaxRect.y &&
          mouse_pos.y < downMaxRect.y + downMaxRect.height) {
        OnLevelDownMaxClick(ctx);
        return;
      }
      if (mouse_pos.x >= upMaxRect.x &&
          mouse_pos.x < upMaxRect.x + upMaxRect.width &&
          mouse_pos.y >= upMaxRect.y &&
          mouse_pos.y < upMaxRect.y + upMaxRect.height) {
        OnLevelUpMaxClick(ctx);
        return;
      }
    }

    // パッシブスロチE��クリチE���E�強化タブ時�E�E
    if (operation_panel_.active_tab == OperationPanel::TabType::Enhancement) {
      auto commitPassiveChange = [&](bool changed) {
        if (!changed) {
          return;
        }
        has_unsaved_changes_ = true;
        PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
        ns.level = editing_character_state_.level;
        if (!TryCommitEditingState(ctx, ns)) {
          editing_character_state_ = saved_character_state_;
          ApplyStateToUI(ctx, saved_character_state_);
          UpdateStatusPanel(ctx);
        }
      };

      for (int i = 0; i < 3; ++i) {
        const auto &slot = operation_panel_.passive_slots[i];
        const float slot_abs_x = operation_panel_.x + slot.position.x;
        const float slot_abs_y = operation_panel_.y + slot.position.y;
        const float btn_h = 28.0f;
        const float btn_padding = 10.0f;
        const float btn_gap = 6.0f;
        const float btn_y = slot_abs_y + slot.height - btn_h - 10.0f;

        if (slot.assigned_passive == nullptr) {
          Rectangle assignRect{slot_abs_x + btn_padding, btn_y,
                               slot.width - btn_padding * 2.0f, btn_h};
          if (mouse_pos.x >= assignRect.x &&
              mouse_pos.x < assignRect.x + assignRect.width &&
              mouse_pos.y >= assignRect.y &&
              mouse_pos.y < assignRect.y + assignRect.height) {
            commitPassiveChange(AssignRandomPassive(i));
            return;
          }
        } else {
          const float btn_w =
              (slot.width - btn_padding * 2.0f - btn_gap * 2.0f) / 3.0f;
          Rectangle upgradeRect{slot_abs_x + btn_padding, btn_y, btn_w, btn_h};
          Rectangle changeRect{slot_abs_x + btn_padding + (btn_w + btn_gap),
                               btn_y, btn_w, btn_h};
          Rectangle removeRect{slot_abs_x + btn_padding +
                                   (btn_w + btn_gap) * 2.0f,
                               btn_y, btn_w, btn_h};
          if (mouse_pos.x >= upgradeRect.x &&
              mouse_pos.x < upgradeRect.x + upgradeRect.width &&
              mouse_pos.y >= upgradeRect.y &&
              mouse_pos.y < upgradeRect.y + upgradeRect.height) {
            commitPassiveChange(UpgradePassive(i));
            return;
          }
          if (mouse_pos.x >= changeRect.x &&
              mouse_pos.x < changeRect.x + changeRect.width &&
              mouse_pos.y >= changeRect.y &&
              mouse_pos.y < changeRect.y + changeRect.height) {
            commitPassiveChange(ReplacePassive(ctx, i));
            return;
          }
          if (mouse_pos.x >= removeRect.x &&
              mouse_pos.x < removeRect.x + removeRect.width &&
              mouse_pos.y >= removeRect.y &&
              mouse_pos.y < removeRect.y + removeRect.height) {
            commitPassiveChange(RemovePassive(i));
            return;
          }
        }
      }

      // Reset / Reroll ボタン
      const float buttons_y = operation_panel_.y +
                              operation_panel_.passive_slots[0].position.y +
                              operation_panel_.passive_slots[0].height + 18.0f;
      const float buttons_h = 44.0f;
      const float buttons_w = (operation_panel_.width - 40.0f - 10.0f) / 2.0f;
      const float bx = operation_panel_.x + 20.0f;
      Rectangle resetRect{bx, buttons_y, buttons_w, buttons_h};
      Rectangle rerollRect{bx + buttons_w + 10.0f, buttons_y, buttons_w,
                           buttons_h};
      if (mouse_pos.x >= resetRect.x &&
          mouse_pos.x < resetRect.x + resetRect.width &&
          mouse_pos.y >= resetRect.y &&
          mouse_pos.y < resetRect.y + resetRect.height) {
        ResetAllPassives(ctx);
        return;
      }
      if (mouse_pos.x >= rerollRect.x &&
          mouse_pos.x < rerollRect.x + rerollRect.width &&
          mouse_pos.y >= rerollRect.y &&
          mouse_pos.y < rerollRect.y + rerollRect.height) {
        const int ownedGold =
            (ctx.gameplayDataAPI ? ctx.gameplayDataAPI->GetGold() : 0);
        const bool canReroll = (ownedGold >= 50);
        if (canReroll) {
          RerollAllPassives(ctx);
        }
        return;
      }
    }

    // アイチE��スロチE��クリチE���E�裁E��タブ時�E�E
    if (operation_panel_.active_tab == OperationPanel::TabType::Equipment) {
      for (int i = 0; i < 3; ++i) {
        const auto &slot = operation_panel_.item_slots[i];
        const float slot_abs_x = operation_panel_.x + slot.position.x;
        const float slot_abs_y = operation_panel_.y + slot.position.y;

        if (mouse_pos.x >= slot_abs_x &&
            mouse_pos.x < slot_abs_x + slot.width &&
            mouse_pos.y >= slot_abs_y &&
            mouse_pos.y < slot_abs_y + slot.height) {
          // 右上×解除
          if (slot.assigned_item) {
            const float btnSize = 22.0f;
            Rectangle xRect{slot_abs_x + slot.width - btnSize - 6.0f,
                            slot_abs_y + 6.0f, btnSize, btnSize};
            if (mouse_pos.x >= xRect.x && mouse_pos.x < xRect.x + xRect.width &&
                mouse_pos.y >= xRect.y &&
                mouse_pos.y < xRect.y + xRect.height) {
              operation_panel_.item_slots[i].assigned_item = nullptr;
              PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
              ns.level = editing_character_state_.level;
              if (!TryCommitEditingState(ctx, ns)) {
                editing_character_state_ = saved_character_state_;
                ApplyStateToUI(ctx, saved_character_state_);
                UpdateStatusPanel(ctx);
              }
              return;
            }
          }

          // 選択�Eみ
          OnItemSlotClick(i);
          return;
        }
      }

      // フッター: すべて外す
      {
        const float footer_h = 70.0f;
        const float btn_y =
            operation_panel_.y + operation_panel_.height - footer_h;
        Rectangle removeAllRect{operation_panel_.x + 20.0f, btn_y + 8.0f,
                                operation_panel_.width - 40.0f, 44.0f};
        if (mouse_pos.x >= removeAllRect.x &&
            mouse_pos.x < removeAllRect.x + removeAllRect.width &&
            mouse_pos.y >= removeAllRect.y &&
            mouse_pos.y < removeAllRect.y + removeAllRect.height) {
          RemoveAllEquipment(ctx);
          return;
        }
      }

      // ソーチEIクリチE��
      {
        const float sort_bar_y_relative =
            operation_panel_.passive_slots[0].position.y +
            operation_panel_.passive_slots[0].height + 30.0f;
        const float sort_bar_y = operation_panel_.y + sort_bar_y_relative;
        const float sort_bar_h = 44.0f;
        const float sort_x = operation_panel_.x + 20.0f;
        const float sort_w = operation_panel_.width - 40.0f;

        const float btn_h = sort_bar_h - 8.0f;
        const float btn_y = sort_bar_y + 4.0f;
        const float btn_gap = 8.0f;
        const float toggle_w = 90.0f;
        const float btn_w = (sort_w - toggle_w - btn_gap * 6.0f) / 5.0f;

        OperationPanel::ItemSortKey keys[5] = {
            OperationPanel::ItemSortKey::Name,
            OperationPanel::ItemSortKey::OwnedCount,
            OperationPanel::ItemSortKey::Attack,
            OperationPanel::ItemSortKey::Defense,
            OperationPanel::ItemSortKey::Hp};

        // 5つのキー
        for (int i = 0; i < 5; ++i) {
          const float x = sort_x + btn_gap + i * (btn_w + btn_gap);
          Rectangle r{x, btn_y, btn_w, btn_h};
          if (mouse_pos.x >= r.x && mouse_pos.x < r.x + r.width &&
              mouse_pos.y >= r.y && mouse_pos.y < r.y + r.height) {
            if (operation_panel_.item_sort_key == keys[i]) {
              operation_panel_.item_sort_ascending =
                  !operation_panel_.item_sort_ascending;
            } else {
              operation_panel_.item_sort_key = keys[i];
              // 名前は昁E��E��数値系は降頁E��初期とする
              operation_panel_.item_sort_ascending =
                  (keys[i] == OperationPanel::ItemSortKey::Name);
            }
            operation_panel_.item_scroll_offset = 0;
            return;
          }
        }

        // 昁E��トグル
        Rectangle toggle{sort_x + sort_w - toggle_w - btn_gap, btn_y, toggle_w,
                         btn_h};
        if (mouse_pos.x >= toggle.x && mouse_pos.x < toggle.x + toggle.width &&
            mouse_pos.y >= toggle.y && mouse_pos.y < toggle.y + toggle.height) {
          operation_panel_.item_sort_ascending =
              !operation_panel_.item_sort_ascending;
          operation_panel_.item_scroll_offset = 0;
          return;
        }
      }

      // アイチE��一覧�E�E列！E ドラチE��候補選抁E
      {
        const float sort_bar_y_relative =
            operation_panel_.passive_slots[0].position.y +
            operation_panel_.passive_slots[0].height + 30.0f;
        const float sort_bar_h = 44.0f;
        const float list_y =
            (operation_panel_.y + sort_bar_y_relative) + sort_bar_h + 36.0f;
        const float footer_h = 70.0f;
        const float list_h = (operation_panel_.y + operation_panel_.height) -
                             list_y - footer_h - 10.0f;
        const float list_x = operation_panel_.x + 20.0f;
        const float list_w = operation_panel_.width - 40.0f;
        const float item_h = 80.0f;
        const float gap_x = 10.0f;
        const float col_w = (list_w - gap_x) / 2.0f;

        if (mouse_pos.x >= list_x && mouse_pos.x < list_x + list_w &&
            mouse_pos.y >= list_y && mouse_pos.y < list_y + list_h) {
          const int total =
              static_cast<int>(operation_panel_.available_items.size());
          const int totalRows = (total + 1) / 2;
          const int visibleRows =
              std::max(1, static_cast<int>(std::floor(list_h / item_h)));
          operation_panel_.item_scroll_offset =
              std::max(0, std::min(operation_panel_.item_scroll_offset,
                                   std::max(0, totalRows - visibleRows)));

          const float relY = mouse_pos.y - list_y;
          const int row = static_cast<int>(std::floor(relY / item_h)) +
                          operation_panel_.item_scroll_offset;
          const float relX = mouse_pos.x - list_x;
          int col = -1;
          if (relX < col_w)
            col = 0;
          else if (relX >= col_w + gap_x)
            col = 1;
          else
            col = -1; // gap

          if (col >= 0) {
            const int idx = row * 2 + col;
            if (idx >= 0 && idx < total) {
              dragging_item_index_ = idx;
              dragging_item_ = operation_panel_.available_items[idx];
              if (dragging_item_) {
                item_drag_started_ = true;
                is_item_dragging_ = false;
                item_drag_start_pos_ = mouse_pos;
                item_drag_pos_ = mouse_pos;
                return;
              }
            }
          }
        }
      }
    }

    // 決宁E取消�Eタンは廁E���E�即時セーブ！E
  }

  // ドラチE��更新
  if (operation_panel_.active_tab == OperationPanel::TabType::Equipment) {
    auto mouse_pos =
        ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};
    if (item_drag_started_ && dragging_item_) {
      // ドラチE��開始判宁E
      if (!is_item_dragging_ && ctx.inputAPI &&
          ctx.inputAPI->IsLeftClickDown()) {
        const float dx = mouse_pos.x - item_drag_start_pos_.x;
        const float dy = mouse_pos.y - item_drag_start_pos_.y;
        if (std::sqrt(dx * dx + dy * dy) > 3.0f) {
          is_item_dragging_ = true;
        }
      }

      if (is_item_dragging_) {
        item_drag_pos_ = mouse_pos;
      }

      // ドロチE�E
      if (ctx.inputAPI && ctx.inputAPI->IsLeftClickReleased()) {
        if (is_item_dragging_) {
          for (int i = 0; i < 3; ++i) {
            const auto &slot = operation_panel_.item_slots[i];
            const float slot_abs_x = operation_panel_.x + slot.position.x;
            const float slot_abs_y = operation_panel_.y + slot.position.y;
            if (mouse_pos.x >= slot_abs_x &&
                mouse_pos.x < slot_abs_x + slot.width &&
                mouse_pos.y >= slot_abs_y &&
                mouse_pos.y < slot_abs_y + slot.height) {
              operation_panel_.item_slots[i].assigned_item = dragging_item_;
              PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
              ns.level = editing_character_state_.level;
              if (!TryCommitEditingState(ctx, ns)) {
                editing_character_state_ = saved_character_state_;
                ApplyStateToUI(ctx, saved_character_state_);
                UpdateStatusPanel(ctx);
              }
              break;
            }
          }
        }

        // 終亁E
        item_drag_started_ = false;
        is_item_dragging_ = false;
        dragging_item_index_ = -1;
        dragging_item_ = nullptr;
      }
    }
  }

  // スクロール処琁E
  float wheel = ctx.inputAPI ? ctx.inputAPI->GetMouseWheelMove() : 0.0f;
  if (wheel != 0.0f) {
    auto mouse_pos =
        ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};

    // ユニット一覧スクロール
    const float list_top = unit_info_panel_.y + 40.0f + 32.0f + 10.0f;
    const float list_height =
        unit_info_panel_.height - (list_top - unit_info_panel_.y) - 10.0f;
    if (mouse_pos.x >= unit_info_panel_.x &&
        mouse_pos.x < unit_info_panel_.x + unit_info_panel_.width &&
        mouse_pos.y >= list_top && mouse_pos.y < list_top + list_height) {
      unit_info_panel_.scroll_offset -= static_cast<int>(wheel);
      unit_info_panel_.scroll_offset = std::max(
          0, std::min(unit_info_panel_.scroll_offset,
                      static_cast<int>(unit_info_panel_.entries.size()) - 5));
    }

    // アイチE��一覧スクロール�E�裁E��タブ時�E�E
    if (operation_panel_.active_tab == OperationPanel::TabType::Equipment) {
      const float sort_bar_y_relative =
          operation_panel_.passive_slots[0].position.y +
          operation_panel_.passive_slots[0].height + 30.0f;
      const float sort_bar_h = 44.0f;
      const float list_y =
          (operation_panel_.y + sort_bar_y_relative) + sort_bar_h + 36.0f;
      const float footer_h = 70.0f;
      const float list_h = (operation_panel_.y + operation_panel_.height) -
                           list_y - footer_h - 10.0f;
      const float list_x = operation_panel_.x + 20.0f;
      const float list_w = operation_panel_.width - 40.0f;
      if (mouse_pos.x >= list_x && mouse_pos.x < list_x + list_w &&
          mouse_pos.y >= list_y && mouse_pos.y < list_y + list_h) {
        const int total =
            static_cast<int>(operation_panel_.available_items.size());
        const int totalRows = (total + 1) / 2;
        const float item_h = 80.0f;
        const int visibleRows =
            std::max(1, static_cast<int>(std::floor(list_h / item_h)));
        const int maxOffset = std::max(0, totalRows - visibleRows);

        operation_panel_.item_scroll_offset -= static_cast<int>(wheel);
        operation_panel_.item_scroll_offset = std::max(
            0, std::min(operation_panel_.item_scroll_offset, maxOffset));
      }
    }

    // パッシブ一覧スクロール�E�パチE��ブスキルタブ時�E�E
    if (operation_panel_.active_tab == OperationPanel::TabType::Enhancement) {
      const float buttons_y = operation_panel_.y +
                              operation_panel_.passive_slots[0].position.y +
                              operation_panel_.passive_slots[0].height + 18.0f;
      const float list_y = buttons_y + 44.0f + 14.0f;
      const float list_x = operation_panel_.x + 20.0f;
      const float list_w = operation_panel_.width - 40.0f;
      const float list_h =
          operation_panel_.y + operation_panel_.height - list_y - 20.0f;

      if (mouse_pos.x >= list_x && mouse_pos.x < list_x + list_w &&
          mouse_pos.y >= list_y && mouse_pos.y < list_y + list_h) {
        operation_panel_.passive_scroll_offset -= static_cast<int>(wheel);
        const int maxOffset = std::max(
            0,
            static_cast<int>(operation_panel_.available_passives.size()) - 3);
        operation_panel_.passive_scroll_offset = std::max(
            0, std::min(operation_panel_.passive_scroll_offset, maxOffset));
      }
    }
  }
}

void CharacterEnhancementOverlay::UpdateHoverStates(Vec2 mouse_pos) {
  // パッシブスロチE��のホバー状慁E
  for (int i = 0; i < 3; ++i) {
    auto &slot = operation_panel_.passive_slots[i];
    const float slot_abs_x = operation_panel_.x + slot.position.x;
    const float slot_abs_y = operation_panel_.y + slot.position.y;

    slot.is_hovered =
        (mouse_pos.x >= slot_abs_x && mouse_pos.x < slot_abs_x + slot.width &&
         mouse_pos.y >= slot_abs_y && mouse_pos.y < slot_abs_y + slot.height);
  }

  // アイチE��スロチE��のホバー状慁E
  for (int i = 0; i < 3; ++i) {
    auto &slot = operation_panel_.item_slots[i];
    const float slot_abs_x = operation_panel_.x + slot.position.x;
    const float slot_abs_y = operation_panel_.y + slot.position.y;

    slot.is_hovered =
        (mouse_pos.x >= slot_abs_x && mouse_pos.x < slot_abs_x + slot.width &&
         mouse_pos.y >= slot_abs_y && mouse_pos.y < slot_abs_y + slot.height);
  }
}

void CharacterEnhancementOverlay::ProcessKeyboardInput(SharedContext &ctx) {
  // ESCキーで閉じめE
  if (ctx.inputAPI && ctx.inputAPI->IsEscapePressed()) { // KEY_ESCAPE
    if (operation_panel_.show_passive_popup) {
      operation_panel_.show_passive_popup = false;
    } else {
      requestClose_ = true;
    }
  }
}

// ========== イベントハンドラー ==========

void CharacterEnhancementOverlay::OnUnitListItemClick(SharedContext &ctx,
                                                      int index) {
  if (index < 0 || index >= static_cast<int>(unit_info_panel_.entries.size())) {
    return;
  }

  unit_info_panel_.selected_index = index;
  SelectCharacter(ctx, unit_info_panel_.entries[index]);
}

void CharacterEnhancementOverlay::OnTabClick(OperationPanel::TabType tab) {
  operation_panel_.active_tab = tab;
  if (tab != OperationPanel::TabType::Equipment) {
    operation_panel_.show_item_popup = false;
    operation_panel_.popup_item_slot_id = -1;
    operation_panel_.selected_item_slot_id = -1;
  }
  LOG_INFO("CharacterEnhancementOverlay: Tab switched to {}",
           tab == OperationPanel::TabType::Enhancement ? "Enhancement"
                                                       : "Equipment");
}

void CharacterEnhancementOverlay::OnPassiveSlotClick(int slot_id) {
  if (slot_id < 0 || slot_id >= 3) {
    return;
  }

  // パッシブ操作はスロット内ボタンに一本化
  LOG_INFO("CharacterEnhancementOverlay: Passive slot {} clicked (no popup)",
           slot_id);
}

void CharacterEnhancementOverlay::OnPassivePopupOption(SharedContext &ctx,
                                                       int option) {
  const int slot_id = operation_panel_.popup_slot_id;
  bool changed = false;

  switch (option) {
  case 0: // ランダム付丁E強匁E
    if (operation_panel_.passive_slots[slot_id].assigned_passive == nullptr) {
      changed = AssignRandomPassive(slot_id);
    } else {
      changed = UpgradePassive(slot_id);
    }
    break;
  case 1: // ランダム変更
    changed = ReplacePassive(ctx, slot_id);
    break;
  case 2: // 削除
    changed = RemovePassive(slot_id);
    break;
  case 3: // キャンセル
    break;
  }

  operation_panel_.show_passive_popup = false;
  has_unsaved_changes_ = (changed && option != 3);
  if (has_unsaved_changes_) {
    PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
    ns.level = editing_character_state_.level;
    if (!TryCommitEditingState(ctx, ns)) {
      editing_character_state_ = saved_character_state_;
      ApplyStateToUI(ctx, saved_character_state_);
      UpdateStatusPanel(ctx);
    }
  }
}

void CharacterEnhancementOverlay::OnItemSlotClick(int slot_id) {
  // ロックされたキャラは編集できない
  const auto *character = GetSelectedCharacter();
  if (character && sharedContext_ && sharedContext_->gameplayDataAPI) {
    const auto st =
        sharedContext_->gameplayDataAPI->GetCharacterState(character->id);
    if (!st.unlocked) {
      return; // ロックされている場合は操作を無効化
    }
  }
  if (slot_id < 0 || slot_id >= 3) {
    return;
  }

  operation_panel_.selected_item_slot_id = slot_id;
  LOG_INFO("CharacterEnhancementOverlay: Item slot {} clicked", slot_id);
}

void CharacterEnhancementOverlay::OnItemListClick(SharedContext &ctx,
                                                  int index) {
  if (index < 0 ||
      index >= static_cast<int>(operation_panel_.available_items.size())) {
    return;
  }

  int targetSlot = operation_panel_.selected_item_slot_id;
  if (targetSlot < 0 || targetSlot >= 3) {
    // 空きスロチE��優允E
    for (int i = 0; i < 3; ++i) {
      if (operation_panel_.item_slots[i].assigned_item == nullptr) {
        targetSlot = i;
        break;
      }
    }
    // それでも無ければ 0 を置き換ぁE
    if (targetSlot < 0 || targetSlot >= 3) {
      targetSlot = 0;
    }
  }

  operation_panel_.item_slots[targetSlot].assigned_item =
      operation_panel_.available_items[index];
  has_unsaved_changes_ = true;
  PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
  ns.level = editing_character_state_.level;
  if (!TryCommitEditingState(ctx, ns)) {
    editing_character_state_ = saved_character_state_;
    ApplyStateToUI(ctx, saved_character_state_);
    UpdateStatusPanel(ctx);
  }
  LOG_INFO("CharacterEnhancementOverlay: Item equipped to slot {}", targetSlot);
}

void CharacterEnhancementOverlay::OnItemPopupOption(SharedContext &ctx,
                                                    int option) {
  const int slot_id = operation_panel_.popup_item_slot_id;
  if (slot_id < 0 || slot_id >= 3) {
    operation_panel_.show_item_popup = false;
    return;
  }

  switch (option) {
  case 0: // こ�EスロチE��に裁E��する�E�選択！E
    operation_panel_.selected_item_slot_id = slot_id;
    break;
  case 1: // 外す
    operation_panel_.item_slots[slot_id].assigned_item = nullptr;
    has_unsaved_changes_ = true;
    {
      PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
      ns.level = editing_character_state_.level;
      if (!TryCommitEditingState(ctx, ns)) {
        editing_character_state_ = saved_character_state_;
        ApplyStateToUI(ctx, saved_character_state_);
        UpdateStatusPanel(ctx);
      }
    }
    break;
  case 2: // キャンセル
  default:
    break;
  }

  operation_panel_.show_item_popup = false;
}

void CharacterEnhancementOverlay::OnLevelUpClick(SharedContext &ctx) {
  const auto *character = GetSelectedCharacter();
  if (!character)
    return;
  if (!ctx.gameplayDataAPI) {
    return;
  }
  // ロックされたキャラは編集できない
  const auto st = ctx.gameplayDataAPI->GetCharacterState(character->id);
  if (!st.unlocked) {
    return; // ロックされている場合は操作を無効化
  }
  if (!ctx.gameplayDataAPI) {
    return;
  }

  const int curLevel = std::max(1, editing_character_state_.level);
  const int nextLevel = std::min(10, curLevel + 1);
  if (nextLevel == curLevel) {
    return;
  }

  // cost = 100 * 現在Lv�E�Ev1ↁE=100, Lv2ↁE=200...�E�E
  const int costGold = 200 * curLevel;
  const int ownedGold = ctx.gameplayDataAPI->GetGold();
  if (ownedGold < costGold) {
    LOG_INFO("CharacterEnhancementOverlay: LevelUp blocked (not enough gold): "
             "owned={} cost={}",
             ownedGold, costGold);
    return;
  }

  PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
  ns.level = nextLevel;
  if (!TryCommitEditingState(ctx, ns)) {
    editing_character_state_ = saved_character_state_;
    ApplyStateToUI(ctx, saved_character_state_);
    UpdateStatusPanel(ctx);
    return;
  }

  // 消費してセーチE
  ctx.gameplayDataAPI->AddGold(-costGold);
  ctx.gameplayDataAPI->Save();
}

void CharacterEnhancementOverlay::OnLevelUpBatchClick(SharedContext &ctx,
                                                      int levels) {
  const auto *character = GetSelectedCharacter();
  if (!character)
    return;
  if (!ctx.gameplayDataAPI) {
    return;
  }
  // ロックされたキャラは編集できない
  const auto st = ctx.gameplayDataAPI->GetCharacterState(character->id);
  if (!st.unlocked) {
    return; // ロックされている場合は操作を無効化
  }
  if (levels <= 0) {
    return;
  }

  const int curLevel = std::max(1, editing_character_state_.level);
  int targetLevel = curLevel;
  int remainingGold = ctx.gameplayDataAPI->GetGold();
  int totalCost = 0;

  for (int i = 0; i < levels; ++i) {
    if (targetLevel >= 10) {
      break;
    }
    const int cost = 200 * targetLevel;
    if (remainingGold < cost) {
      break;
    }
    remainingGold -= cost;
    totalCost += cost;
    targetLevel++;
  }

  if (targetLevel == curLevel) {
    return;
  }

  PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
  ns.level = targetLevel;
  if (!TryCommitEditingState(ctx, ns)) {
    editing_character_state_ = saved_character_state_;
    ApplyStateToUI(ctx, saved_character_state_);
    UpdateStatusPanel(ctx);
    return;
  }

  ctx.gameplayDataAPI->AddGold(-totalCost);
  ctx.gameplayDataAPI->Save();
}

void CharacterEnhancementOverlay::OnLevelUpMaxClick(SharedContext &ctx) {
  OnLevelUpBatchClick(ctx, 10);
}

void CharacterEnhancementOverlay::OnLevelDownClick(SharedContext &ctx) {
  const auto *character = GetSelectedCharacter();
  if (!character)
    return;
  if (!ctx.gameplayDataAPI) {
    return;
  }
  // ロックされたキャラは編集できない
  const auto st = ctx.gameplayDataAPI->GetCharacterState(character->id);
  if (!st.unlocked) {
    return; // ロックされている場合は操作を無効化
  }

  const int curLevel = std::max(1, editing_character_state_.level);
  const int nextLevel = std::max(1, curLevel - 1);
  if (nextLevel == curLevel) {
    return;
  }

  // Lvダウン時�E「そのLvに上がる�Eに忁E��だったGold」�E80%を返却
  // 侁E Lv3ↁE は Lv2ↁE の cost=200 の80% => 160
  const int levelUpCostForThisLevel = 200 * nextLevel;
  const int refundGold = static_cast<int>(
      std::round(static_cast<float>(levelUpCostForThisLevel) * 0.8f));

  PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
  ns.level = nextLevel;
  if (!TryCommitEditingState(ctx, ns)) {
    editing_character_state_ = saved_character_state_;
    ApplyStateToUI(ctx, saved_character_state_);
    UpdateStatusPanel(ctx);
    return;
  }

  ctx.gameplayDataAPI->AddGold(refundGold);
  ctx.gameplayDataAPI->Save();
}

void CharacterEnhancementOverlay::OnLevelDownBatchClick(SharedContext &ctx,
                                                        int levels) {
  const auto *character = GetSelectedCharacter();
  if (!character)
    return;
  if (!ctx.gameplayDataAPI) {
    return;
  }
  // ロックされたキャラは編集できない
  const auto st = ctx.gameplayDataAPI->GetCharacterState(character->id);
  if (!st.unlocked) {
    return; // ロックされている場合は操作を無効化
  }
  if (levels <= 0) {
    return;
  }

  const int curLevel = std::max(1, editing_character_state_.level);
  int targetLevel = std::max(1, curLevel - levels);
  if (targetLevel == curLevel) {
    return;
  }

  int refundGold = 0;
  for (int lvl = curLevel; lvl > targetLevel; --lvl) {
    const int levelUpCostForThisLevel = 200 * (lvl - 1);
    refundGold += static_cast<int>(
        std::round(static_cast<float>(levelUpCostForThisLevel) * 0.8f));
  }

  PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
  ns.level = targetLevel;
  if (!TryCommitEditingState(ctx, ns)) {
    editing_character_state_ = saved_character_state_;
    ApplyStateToUI(ctx, saved_character_state_);
    UpdateStatusPanel(ctx);
    return;
  }

  ctx.gameplayDataAPI->AddGold(refundGold);
  ctx.gameplayDataAPI->Save();
}

void CharacterEnhancementOverlay::OnLevelDownMaxClick(SharedContext &ctx) {
  OnLevelDownBatchClick(ctx, 10);
}

bool CharacterEnhancementOverlay::ResetAllPassives(SharedContext &ctx) {
  bool changed = false;
  for (int i = 0; i < 3; ++i) {
    if (operation_panel_.passive_slots[i].assigned_passive != nullptr ||
        operation_panel_.passive_slots[i].level != 1) {
      changed = true;
    }
    operation_panel_.passive_slots[i].assigned_passive = nullptr;
    operation_panel_.passive_slots[i].level = 1;
  }
  if (!changed) {
    return false;
  }
  PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
  ns.level = editing_character_state_.level;
  if (!TryCommitEditingState(ctx, ns)) {
    editing_character_state_ = saved_character_state_;
    ApplyStateToUI(ctx, saved_character_state_);
    UpdateStatusPanel(ctx);
    return false;
  }
  return true;
}

bool CharacterEnhancementOverlay::RerollAllPassives(SharedContext &ctx) {
  if (!ctx.gameplayDataAPI) {
    return false;
  }
  const int ownedGold = ctx.gameplayDataAPI->GetGold();
  if (ownedGold < 50) {
    LOG_INFO("CharacterEnhancementOverlay: RerollAllPassives blocked (not "
             "enough gold): owned={} cost=50",
             ownedGold);
    return false;
  }
  if (operation_panel_.available_passives.empty()) {
    LOG_WARN("CharacterEnhancementOverlay: No available passives for reroll");
    return false;
  }

  // 3枠を�E抽選�E�重褁E��容�E�E
  for (int i = 0; i < 3; ++i) {
    AssignRandomPassive(i);
    operation_panel_.passive_slots[i].level = 1;
  }
  PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
  ns.level = editing_character_state_.level;
  if (!TryCommitEditingState(ctx, ns)) {
    editing_character_state_ = saved_character_state_;
    ApplyStateToUI(ctx, saved_character_state_);
    UpdateStatusPanel(ctx);
    return false;
  }
  ctx.gameplayDataAPI->AddGold(-50);
  ctx.gameplayDataAPI->Save();
  return true;
}

void CharacterEnhancementOverlay::RemoveAllEquipment(SharedContext &ctx) {
  for (int i = 0; i < 3; ++i) {
    operation_panel_.item_slots[i].assigned_item = nullptr;
  }
  PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
  ns.level = editing_character_state_.level;
  if (!TryCommitEditingState(ctx, ns)) {
    editing_character_state_ = saved_character_state_;
    ApplyStateToUI(ctx, saved_character_state_);
    UpdateStatusPanel(ctx);
  }
}

// ========== パッシブ操作メソチE�� ==========

bool CharacterEnhancementOverlay::AssignRandomPassive(int slot_id) {
  if (operation_panel_.available_passives.empty()) {
    LOG_WARN("CharacterEnhancementOverlay: No available passives");
    return false;
  }

  std::uniform_int_distribution<int> dist(
      0, static_cast<int>(operation_panel_.available_passives.size()) - 1);
  int random_index = dist(rng_);

  operation_panel_.passive_slots[slot_id].assigned_passive =
      operation_panel_.available_passives[random_index];
  operation_panel_.passive_slots[slot_id].level = 1;

  LOG_INFO("CharacterEnhancementOverlay: Random passive assigned to slot {}",
           slot_id);
  return true;
}

bool CharacterEnhancementOverlay::UpgradePassive(int slot_id) {
  auto &slot = operation_panel_.passive_slots[slot_id];
  if (!slot.assigned_passive) {
    return false;
  }

  if (slot.level < 3) {
    slot.level++;
    LOG_INFO("CharacterEnhancementOverlay: Passive upgraded to level {}",
             slot.level);
    return true;
  } else {
    LOG_WARN("CharacterEnhancementOverlay: Passive already at max level");
    return false;
  }
}

bool CharacterEnhancementOverlay::ReplacePassive(SharedContext &ctx,
                                                 int slot_id) {
  if (!ctx.gameplayDataAPI) {
    return false;
  }
  const int ownedGold = ctx.gameplayDataAPI->GetGold();
  if (ownedGold < 25) {
    LOG_INFO("CharacterEnhancementOverlay: ReplacePassive blocked (not enough "
             "gold): owned={} cost=25",
             ownedGold);
    return false;
  }
  if (operation_panel_.available_passives.empty()) {
    LOG_WARN("CharacterEnhancementOverlay: No available passives for replace");
    return false;
  }

  const bool removed = RemovePassive(slot_id);
  const bool assigned = AssignRandomPassive(slot_id);
  if (removed || assigned) {
    ctx.gameplayDataAPI->AddGold(-25);
    ctx.gameplayDataAPI->Save();
    LOG_INFO("CharacterEnhancementOverlay: Passive replaced at slot {}",
             slot_id);
    return true;
  }
  return false;
}

bool CharacterEnhancementOverlay::RemovePassive(int slot_id) {
  bool changed = false;
  if (operation_panel_.passive_slots[slot_id].assigned_passive != nullptr ||
      operation_panel_.passive_slots[slot_id].level != 1) {
    changed = true;
  }
  operation_panel_.passive_slots[slot_id].assigned_passive = nullptr;
  operation_panel_.passive_slots[slot_id].level = 1;
  if (changed) {
    LOG_INFO("CharacterEnhancementOverlay: Passive removed from slot {}",
             slot_id);
  }
  return changed;
}

// ========== ユーチE��リチE��メソチE�� ==========

const entities::Character *
CharacterEnhancementOverlay::GetSelectedCharacter() const {
  return unit_info_panel_.selected_character;
}

int CharacterEnhancementOverlay::GetPassiveSlotAtPosition(
    Vector2 position) const {
  for (int i = 0; i < 3; ++i) {
    const auto &slot = operation_panel_.passive_slots[i];
    if (position.x >= slot.position.x &&
        position.x < slot.position.x + slot.width &&
        position.y >= slot.position.y &&
        position.y < slot.position.y + slot.height) {
      return i;
    }
  }
  return -1;
}

int CharacterEnhancementOverlay::GetItemSlotAtPosition(Vector2 position) const {
  for (int i = 0; i < 3; ++i) {
    const auto &slot = operation_panel_.item_slots[i];
    if (position.x >= slot.position.x &&
        position.x < slot.position.x + slot.width &&
        position.y >= slot.position.y &&
        position.y < slot.position.y + slot.height) {
      return i;
    }
  }
  return -1;
}

} // namespace core
} // namespace game
