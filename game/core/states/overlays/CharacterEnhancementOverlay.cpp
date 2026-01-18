#include "CharacterEnhancementOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../entities/CharacterManager.hpp"
#include "../../entities/ItemPassiveManager.hpp"
#include "../../entities/CharacterStatCalculator.hpp"
#include "../../system/PlayerDataManager.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UIEffects.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <unordered_map>
#include <unordered_set>

namespace game {
namespace core {

CharacterEnhancementOverlay::CharacterEnhancementOverlay()
    : systemAPI_(nullptr)
    , isInitialized_(false)
    , requestClose_(false)
    , hasTransitionRequest_(false)
    , requestedNextState_(GameState::Title)
    , has_unsaved_changes_(false)
    , rng_(std::random_device{}())
{
}

bool CharacterEnhancementOverlay::Initialize(BaseSystemAPI* systemAPI) {
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

    // パネル初期化
    InitializePanels();

    isInitialized_ = true;
    LOG_INFO("CharacterEnhancementOverlay initialized");
    return true;
}

void CharacterEnhancementOverlay::Update(SharedContext& ctx, float deltaTime) {
    if (!isInitialized_) {
        return;
    }

    // キャラクターリストの読み込み（初回のみ）
    if (unit_info_panel_.entries.empty() && ctx.characterManager) {
        LoadCharacterList(ctx);
    }
    
    // パッシブ・アイテムリストの読み込み（初回のみ）
    if (operation_panel_.available_passives.empty() && ctx.itemPassiveManager) {
        FilterAvailablePassives(ctx);
    }
    if (operation_panel_.available_items.empty() && ctx.itemPassiveManager) {
        FilterAvailableItems(ctx);
    }

    // マウス入力処理
    ProcessMouseInput(ctx);

    // キーボード入力処理
    ProcessKeyboardInput(ctx);

    // ホバー状態更新
    Vector2 mouse_pos = systemAPI_->GetMousePosition();
    UpdateHoverStates(mouse_pos);
}

void CharacterEnhancementOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) {
        return;
    }

    using namespace ui;

    // 各パネルの描画（InitializePanelsで設定された座標をそのまま使用）
    RenderUnitInfoPanel();
    RenderStatusPanel();
    RenderOperationPanel();

    // ポップアップは最後に描画（最前面）
    if (operation_panel_.show_passive_popup) {
        RenderPassivePopup();
    }
    if (operation_panel_.show_item_popup) {
        RenderItemPopup();
    }
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
    LOG_INFO("CharacterEnhancementOverlay shutdown");
}

bool CharacterEnhancementOverlay::RequestClose() const {
    return requestClose_;
}

bool CharacterEnhancementOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        return true;
    }
    return false;
}

// ========== 初期化メソッド ==========

void CharacterEnhancementOverlay::InitializePanels() {
    // 画面レイアウト定数
    const float SCREEN_WIDTH = 1920.0f;
    const float SCREEN_HEIGHT = 1080.0f;
    const float HEADER_HEIGHT = 90.0f;     // ヘッダー（Gold/Gems/Tickets表示）
    const float TAB_BAR_HEIGHT = 90.0f;    // タブバー（下部、y=990～1080）
    const float CONTENT_START_Y = HEADER_HEIGHT;  // コンテンツ開始位置 y=90
    const float CONTENT_END_Y = SCREEN_HEIGHT - TAB_BAR_HEIGHT;  // コンテンツ終了位置 y=990
    
    // マージンとギャップ
    const float MARGIN = 5.0f;              // 画面端からのマージン
    const float PANEL_GAP = 10.0f;          // パネル間のギャップ
    
    // 利用可能な描画領域を計算（y=90～990の900px範囲内）
    const float available_width = SCREEN_WIDTH - (MARGIN * 2);
    const float available_height = CONTENT_END_Y - CONTENT_START_Y - (MARGIN * 2);
    
    // パネル幅の配分（比率: ユニット:ステータス:操作 = 1:1.3:2）
    const float total_ratio = 1.0f + 1.3f + 2.0f;  // 4.3
    const float unit_width = (available_width - PANEL_GAP * 2) * (1.0f / total_ratio);
    const float status_width = (available_width - PANEL_GAP * 2) * (1.3f / total_ratio);
    const float operation_width = (available_width - PANEL_GAP * 2) * (2.0f / total_ratio);
    
    // ユニット情報パネル（左）- コンテンツ領域内に配置
    unit_info_panel_.x = MARGIN;
    unit_info_panel_.y = CONTENT_START_Y + MARGIN;
    unit_info_panel_.width = unit_width;
    unit_info_panel_.height = available_height;
    unit_info_panel_.selected_index = -1;
    unit_info_panel_.scroll_offset = 0;
    unit_info_panel_.item_height = 60.0f;
    unit_info_panel_.selected_character = nullptr;
    
    // ステータスパネル（中央）- コンテンツ領域内に配置
    status_panel_.x = unit_info_panel_.x + unit_info_panel_.width + PANEL_GAP;
    status_panel_.y = CONTENT_START_Y + MARGIN;
    status_panel_.width = status_width;
    status_panel_.height = available_height;
    status_panel_.padding = 30.0f;
    status_panel_.line_height = 45.0f;
    status_panel_.font_size = 24;
    
    // 操作パネル（右）- コンテンツ領域内に配置
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
    
    // パッシブスロット（横並び）- 操作パネルの相対位置で配置
    const float slot_margin = 15.0f;  // スロット左右のマージン
    const float slot_spacing = 15.0f;  // スロット間の間隔
    const float slot_width = (operation_panel_.width - slot_margin * 2 - slot_spacing * 2) / 3.0f;
    const float slot_height = 180.0f;
    const float slot_start_x = slot_margin;  // パネル内の相対位置
    const float slot_y = 80.0f;  // パネル内の相対位置（タブの下）
    
    for (int i = 0; i < 3; ++i) {
        operation_panel_.passive_slots[i].slot_id = i;
        operation_panel_.passive_slots[i].assigned_passive = nullptr;
        operation_panel_.passive_slots[i].position.x = slot_start_x + i * (slot_width + slot_spacing);
        operation_panel_.passive_slots[i].position.y = slot_y;
        operation_panel_.passive_slots[i].width = slot_width;
        operation_panel_.passive_slots[i].height = slot_height;
        operation_panel_.passive_slots[i].is_hovered = false;
        operation_panel_.passive_slots[i].level = 1;
    }
    
    // アイテムスロット（横並び）- パッシブスロットと同じ配置
    for (int i = 0; i < 3; ++i) {
        operation_panel_.item_slots[i].slot_id = i;
        operation_panel_.item_slots[i].assigned_item = nullptr;
        operation_panel_.item_slots[i].position.x = slot_start_x + i * (slot_width + slot_spacing);
        operation_panel_.item_slots[i].position.y = slot_y;
        operation_panel_.item_slots[i].width = slot_width;
        operation_panel_.item_slots[i].height = slot_height;
        operation_panel_.item_slots[i].is_hovered = false;
    }
    
    LOG_INFO("CharacterEnhancementOverlay: Panel layout calculated");
    LOG_INFO("  Unit panel: x={:.1f} y={:.1f} w={:.1f} h={:.1f}", 
             unit_info_panel_.x, unit_info_panel_.y, unit_info_panel_.width, unit_info_panel_.height);
    LOG_INFO("  Status panel: x={:.1f} y={:.1f} w={:.1f} h={:.1f}", 
             status_panel_.x, status_panel_.y, status_panel_.width, status_panel_.height);
    LOG_INFO("  Operation panel: x={:.1f} y={:.1f} w={:.1f} h={:.1f}", 
             operation_panel_.x, operation_panel_.y, operation_panel_.width, operation_panel_.height);
    LOG_INFO("  Slot size: {:.1f}x{:.1f}", slot_width, slot_height);
}

void CharacterEnhancementOverlay::LoadCharacterList(SharedContext& ctx) {
    if (!ctx.characterManager) {
        return;
    }

    unit_info_panel_.entries.clear();
    const auto& masters = ctx.characterManager->GetAllMasters();
    for (const auto& [id, ch] : masters) {
        unit_info_panel_.entries.push_back(&ch);
    }
    
    // IDでソート
    std::sort(unit_info_panel_.entries.begin(), unit_info_panel_.entries.end(),
              [](const entities::Character* a, const entities::Character* b) {
                  if (!a || !b) return false;
                  auto extractNumber = [](const std::string& id) -> int {
                      size_t pos = id.find_last_of('_');
                      if (pos != std::string::npos) {
                          try {
                              return std::stoi(id.substr(pos + 1));
                          } catch (...) {
                              return 0;
                          }
                      }
                      return 0;
                  };
                  return extractNumber(a->id) < extractNumber(b->id);
              });
    
    if (!unit_info_panel_.entries.empty()) {
        unit_info_panel_.selected_index = 0;
        SelectCharacter(ctx, unit_info_panel_.entries[0]);
    }
    
    LOG_INFO("CharacterEnhancementOverlay: Loaded {} characters", unit_info_panel_.entries.size());
}

void CharacterEnhancementOverlay::SelectCharacter(SharedContext& ctx, const entities::Character* character) {
    if (!character) {
        return;
    }

    unit_info_panel_.selected_character = character;

    // セーブからロードアウトを復元（なければ空）
    PlayerDataManager::CharacterState st;
    if (ctx.playerDataManager) {
        st = ctx.playerDataManager->GetCharacterState(character->id);
    }
    saved_character_state_ = st;
    editing_character_state_ = st;
    editing_character_id_ = character->id;

    ApplyStateToUI(ctx, editing_character_state_);

    // 候補リストを現在キャラに合わせて更新（装備中/付与中が所持0でも表示されるように）
    FilterAvailablePassives(ctx);
    FilterAvailableItems(ctx);

    // ステータスパネルを更新（ロードアウト反映後）
    UpdateStatusPanel(ctx);

    has_unsaved_changes_ = false;
    
    LOG_INFO("CharacterEnhancementOverlay: Selected character: {}", character->id);
}

void CharacterEnhancementOverlay::UpdateStatusPanel(SharedContext& ctx) {
    const auto* character = GetSelectedCharacter();
    if (!character) {
        return;
    }

    // 共通計算（UI/戦闘で一致させる）
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

    // ItemPassiveManager が無い場合はベース値のみ
    if (!ctx.itemPassiveManager) {
        status_panel_.hp.base = character->hp;
        status_panel_.attack.base = character->attack;
        status_panel_.defense.base = character->defense;
        status_panel_.speed.base = static_cast<int>(character->move_speed);
        status_panel_.range.base = static_cast<int>(character->attack_size.x);
        return;
    }

    const auto editingState = BuildCurrentEditingState();
    const auto calc = entities::CharacterStatCalculator::Calculate(*character, editingState, *ctx.itemPassiveManager);

    status_panel_.hp.base = calc.hp.base;
    status_panel_.hp.bonus = calc.hp.bonus;
    status_panel_.attack.base = calc.attack.base;
    status_panel_.attack.bonus = calc.attack.bonus;
    status_panel_.defense.base = calc.defense.base;
    status_panel_.defense.bonus = calc.defense.bonus;

    status_panel_.speed.base = static_cast<int>(std::round(calc.moveSpeed.base));
    status_panel_.speed.bonus = static_cast<int>(std::round(calc.moveSpeed.final - calc.moveSpeed.base));
    status_panel_.range.base = static_cast<int>(std::round(calc.range.base));
    status_panel_.range.bonus = static_cast<int>(std::round(calc.range.final - calc.range.base));

    status_panel_.attack_span = calc.attackSpan.final;
    status_panel_.level = std::max(1, std::min(50, editingState.level));
}

PlayerDataManager::CharacterState CharacterEnhancementOverlay::BuildCurrentEditingState() const {
    PlayerDataManager::CharacterState st;
    st.unlocked = editing_character_state_.unlocked;
    st.level = editing_character_state_.level;

    for (int i = 0; i < 3; ++i) {
        const auto* p = operation_panel_.passive_slots[i].assigned_passive;
        st.passives[i].id = p ? p->id : "";
        st.passives[i].level = std::max(1, operation_panel_.passive_slots[i].level);

        const auto* e = operation_panel_.item_slots[i].assigned_item;
        st.equipment[i] = e ? e->id : "";
    }
    return st;
}

void CharacterEnhancementOverlay::ApplyStateToUI(SharedContext& ctx, const PlayerDataManager::CharacterState& state) {
    (void)ctx;
    // パッシブ
    for (int i = 0; i < 3; ++i) {
        operation_panel_.passive_slots[i].assigned_passive = nullptr;
        operation_panel_.passive_slots[i].level = 1;

        if (ctx.itemPassiveManager) {
            const std::string pid = state.passives[i].id;
            if (!pid.empty()) {
                operation_panel_.passive_slots[i].assigned_passive = ctx.itemPassiveManager->GetPassiveSkill(pid);
                operation_panel_.passive_slots[i].level = std::max(1, state.passives[i].level);
            }
        }
    }

    // 装備
    for (int i = 0; i < 3; ++i) {
        operation_panel_.item_slots[i].assigned_item = nullptr;
        if (ctx.itemPassiveManager) {
            const std::string eid = state.equipment[i];
            if (!eid.empty()) {
                operation_panel_.item_slots[i].assigned_item = ctx.itemPassiveManager->GetEquipment(eid);
            }
        }
    }
}

bool CharacterEnhancementOverlay::TryCommitEditingState(SharedContext& ctx, const PlayerDataManager::CharacterState& newState) {
    const auto* character = GetSelectedCharacter();
    if (!character) return false;
    if (!ctx.playerDataManager) {
        // セーブ無しの場合でも編集状態だけ更新
        editing_character_state_ = newState;
        UpdateStatusPanel(ctx);
        return true;
    }

    const auto oldState = saved_character_state_;

    // ===== inventory整合（このキャラの付け替え差分のみ）=====
    std::unordered_map<std::string, int> oldEqCount;
    std::unordered_map<std::string, int> newEqCount;
    std::unordered_map<std::string, int> oldPsCount;
    std::unordered_map<std::string, int> newPsCount;

    for (int i = 0; i < 3; ++i) {
        if (!oldState.equipment[i].empty()) oldEqCount[oldState.equipment[i]]++;
        if (!newState.equipment[i].empty()) newEqCount[newState.equipment[i]]++;
        if (!oldState.passives[i].id.empty()) oldPsCount[oldState.passives[i].id]++;
        if (!newState.passives[i].id.empty()) newPsCount[newState.passives[i].id]++;
    }

    // 装備チェック
    for (const auto& [id, required] : newEqCount) {
        const int available = ctx.playerDataManager->GetOwnedEquipmentCount(id) + (oldEqCount.count(id) ? oldEqCount[id] : 0);
        if (required > available) {
            LOG_WARN("CharacterEnhancementOverlay: Not enough equipment '{}' required={} available={}", id, required, available);
            return false;
        }
    }
    // パッシブチェック
    for (const auto& [id, required] : newPsCount) {
        const int available = ctx.playerDataManager->GetOwnedPassiveCount(id) + (oldPsCount.count(id) ? oldPsCount[id] : 0);
        if (required > available) {
            LOG_WARN("CharacterEnhancementOverlay: Not enough passive '{}' required={} available={}", id, required, available);
            return false;
        }
    }

    // 装備の残数更新（このキャラ分だけ戻して→消費）
    for (const auto& [id, cnt] : oldEqCount) {
        ctx.playerDataManager->SetOwnedEquipmentCount(id, ctx.playerDataManager->GetOwnedEquipmentCount(id) + cnt);
    }
    for (const auto& [id, cnt] : newEqCount) {
        ctx.playerDataManager->SetOwnedEquipmentCount(id, ctx.playerDataManager->GetOwnedEquipmentCount(id) - cnt);
    }
    // パッシブの残数更新
    for (const auto& [id, cnt] : oldPsCount) {
        ctx.playerDataManager->SetOwnedPassiveCount(id, ctx.playerDataManager->GetOwnedPassiveCount(id) + cnt);
    }
    for (const auto& [id, cnt] : newPsCount) {
        ctx.playerDataManager->SetOwnedPassiveCount(id, ctx.playerDataManager->GetOwnedPassiveCount(id) - cnt);
    }

    ctx.playerDataManager->SetCharacterState(character->id, newState);
    ctx.playerDataManager->Save();

    saved_character_state_ = newState;
    editing_character_state_ = newState;

    // 候補リストを更新（装備中/付与中をpin）
    FilterAvailablePassives(ctx);
    FilterAvailableItems(ctx);
    UpdateStatusPanel(ctx);
    has_unsaved_changes_ = false;
    return true;
}

void CharacterEnhancementOverlay::FilterAvailablePassives(SharedContext& ctx) {
    operation_panel_.available_passives.clear();
    
    if (!ctx.itemPassiveManager) {
        return;
    }

    // 現在付与中のものは所持0でも候補に残す
    std::unordered_set<std::string> pinned;
    for (int i = 0; i < 3; ++i) {
        if (operation_panel_.passive_slots[i].assigned_passive) {
            pinned.insert(operation_panel_.passive_slots[i].assigned_passive->id);
        }
    }

    const auto all = ctx.itemPassiveManager->GetAllPassiveSkills();
    operation_panel_.available_passives.reserve(all.size());
    for (const auto* p : all) {
        if (!p) continue;
        if (pinned.find(p->id) != pinned.end()) {
            operation_panel_.available_passives.push_back(p);
            continue;
        }
        if (!ctx.playerDataManager) {
            operation_panel_.available_passives.push_back(p);
            continue;
        }
        if (ctx.playerDataManager->GetOwnedPassiveCount(p->id) > 0) {
            operation_panel_.available_passives.push_back(p);
        }
    }
    
    LOG_INFO("CharacterEnhancementOverlay: Loaded {} available passives", 
             operation_panel_.available_passives.size());
}

void CharacterEnhancementOverlay::FilterAvailableItems(SharedContext& ctx) {
    operation_panel_.available_items.clear();
    
    if (!ctx.itemPassiveManager) {
        return;
    }

    std::unordered_set<std::string> pinned;
    for (int i = 0; i < 3; ++i) {
        if (operation_panel_.item_slots[i].assigned_item) {
            pinned.insert(operation_panel_.item_slots[i].assigned_item->id);
        }
    }

    const auto all = ctx.itemPassiveManager->GetAllEquipment();
    operation_panel_.available_items.reserve(all.size());
    for (const auto* e : all) {
        if (!e) continue;
        if (pinned.find(e->id) != pinned.end()) {
            operation_panel_.available_items.push_back(e);
            continue;
        }
        if (!ctx.playerDataManager) {
            operation_panel_.available_items.push_back(e);
            continue;
        }
        if (ctx.playerDataManager->GetOwnedEquipmentCount(e->id) > 0) {
            operation_panel_.available_items.push_back(e);
        }
    }
    
    LOG_INFO("CharacterEnhancementOverlay: Loaded {} available items", 
             operation_panel_.available_items.size());
}

// ========== 描画メソッド ==========

void CharacterEnhancementOverlay::RenderUnitInfoPanel() {
    using namespace ui;

    // 背景
    systemAPI_->DrawRectangle(
        unit_info_panel_.x, unit_info_panel_.y,
        unit_info_panel_.width, unit_info_panel_.height,
        OverlayColors::PANEL_BG_ORANGE
    );
    
    // ゴールドボーダー
    systemAPI_->DrawRectangleLines(
        unit_info_panel_.x, unit_info_panel_.y,
        unit_info_panel_.width, unit_info_panel_.height,
        2.0f,
        OverlayColors::BORDER_GOLD
    );

    // タイトル
    systemAPI_->DrawTextDefault("ユニット選択", 
                                unit_info_panel_.x + 10.0f, unit_info_panel_.y + 10.0f,
                                24.0f, OverlayColors::TEXT_GOLD);
    
    // ユニット一覧（スクロール可能、全高さ使用）
    const float list_top = unit_info_panel_.y + 45.0f;
    const float list_height = unit_info_panel_.height - 55.0f;
    
    for (int i = 0; i < static_cast<int>(unit_info_panel_.entries.size()); ++i) {
        float item_y = list_top + (i - unit_info_panel_.scroll_offset) * unit_info_panel_.item_height;
        
        if (item_y < list_top || item_y >= list_top + list_height) {
            continue;
        }
        
        const auto* entry = unit_info_panel_.entries[i];
        if (!entry) continue;
        
        bool is_selected = (i == unit_info_panel_.selected_index);
        
        // 選択状態の背景
        if (is_selected) {
            systemAPI_->DrawRectangle(
                unit_info_panel_.x, item_y,
                unit_info_panel_.width, unit_info_panel_.item_height,
                OverlayColors::PANEL_BG_ORANGE_LIGHT
            );
        }
        
        // 名前
        Color text_color = is_selected ? WHITE : OverlayColors::TEXT_SECONDARY;
        systemAPI_->DrawTextDefault(
            entry->name,
            unit_info_panel_.x + 15.0f, item_y + 15.0f,
            20.0f, text_color
        );
    }
}

void CharacterEnhancementOverlay::RenderStatusPanel() {
    using namespace ui;

    // 背景
    systemAPI_->DrawRectangle(
        status_panel_.x, status_panel_.y,
        status_panel_.width, status_panel_.height,
        OverlayColors::ORANGE_PANEL_BG_DARK
    );
    
    // ゴールドボーダー
    systemAPI_->DrawRectangleLines(
        status_panel_.x, status_panel_.y,
        status_panel_.width, status_panel_.height,
        2.0f,
        OverlayColors::BORDER_GOLD
    );

    // タイトル
    systemAPI_->DrawTextDefault("ステータス", 
                                status_panel_.x + status_panel_.padding, status_panel_.y + 20.0f,
                                24.0f, OverlayColors::TEXT_GOLD);
    
    float start_y = status_panel_.y + 70.0f;
    float x = status_panel_.x + status_panel_.padding;
    float width = status_panel_.width - status_panel_.padding * 2.0f;

    auto renderStatLine = [&](const std::string& label, const StatusPanel::StatValue& stat, int index) {
        float line_y = start_y + index * status_panel_.line_height;
        
        // ラベル
        systemAPI_->DrawTextDefault(label, x, line_y, (float)status_panel_.font_size, OverlayColors::TEXT_SECONDARY);
        
        // 合計値 (強化済み)
        int total = stat.base + stat.bonus;
        std::string total_str = std::to_string(total);
        Vector2 total_size = systemAPI_->MeasureTextDefault(total_str, (float)status_panel_.font_size);
        
        systemAPI_->DrawTextDefault(total_str, x + width - total_size.x, line_y, (float)status_panel_.font_size, ui::OverlayColors::TEXT_PRIMARY);
        
        // 強化・装備内容 (+-)
        if (stat.bonus != 0) {
            std::string bonus_str = (stat.bonus > 0 ? "+" : "") + std::to_string(stat.bonus);
            Color bonus_color = stat.bonus > 0 ? OverlayColors::TEXT_SUCCESS : OverlayColors::TEXT_ERROR;
            Vector2 bonus_size = systemAPI_->MeasureTextDefault(bonus_str, (float)status_panel_.font_size * 0.8f);
            
            systemAPI_->DrawTextDefault(bonus_str, x + width - total_size.x - bonus_size.x - 10.0f, line_y + 2.0f, 
                                        (float)status_panel_.font_size * 0.8f, bonus_color);
        }
    };

    // 編成相当の情報量へ拡張（上部テキスト）
    auto renderKV = [&](const std::string& label, const std::string& value, int index) {
        float line_y = start_y + index * status_panel_.line_height;
        systemAPI_->DrawTextDefault(label, x, line_y, (float)status_panel_.font_size, OverlayColors::TEXT_SECONDARY);
        Vector2 vs = systemAPI_->MeasureTextDefault(value, (float)status_panel_.font_size);
        systemAPI_->DrawTextDefault(value, x + width - vs.x, line_y, (float)status_panel_.font_size, OverlayColors::TEXT_PRIMARY);
    };

    int row = 0;
    renderKV("Level", std::to_string(std::max(1, status_panel_.level)), row++);

    std::string rarityStr;
    const int rarity = std::max(1, status_panel_.rarity);
    for (int i = 0; i < rarity; ++i) rarityStr += "★";
    if (!status_panel_.rarity_name.empty()) {
        rarityStr += " (" + status_panel_.rarity_name + ")";
    }
    renderKV("Rarity", rarityStr.empty() ? "★" : rarityStr, row++);

    renderStatLine("HP (体力)", status_panel_.hp, row++);
    renderStatLine("ATK (攻撃)", status_panel_.attack, row++);
    renderStatLine("DEF (防御)", status_panel_.defense, row++);
    renderStatLine("SPD (速度)", status_panel_.speed, row++);
    renderStatLine("RNG (射程)", status_panel_.range, row++);
    renderKV("Interval", TextFormat("%.3fs", status_panel_.attack_span), row++);
    renderKV("Cost", std::to_string(status_panel_.cost), row++);

    auto attackTypeToString = [](entities::AttackType type) -> const char* {
        if (type == entities::AttackType::Single) return "単体";
        if (type == entities::AttackType::Range) return "範囲";
        if (type == entities::AttackType::Line) return "直線";
        return "不明";
    };
    auto effectTypeToString = [](entities::EffectType type) -> const char* {
        if (type == entities::EffectType::Normal) return "通常";
        if (type == entities::EffectType::Fire) return "炎";
        if (type == entities::EffectType::Ice) return "氷";
        if (type == entities::EffectType::Lightning) return "雷";
        if (type == entities::EffectType::Heal) return "回復";
        return "不明";
    };
    renderKV("Type", attackTypeToString(status_panel_.attack_type), row++);
    renderKV("Element", effectTypeToString(status_panel_.effect_type), row++);

    // ===== Lv操作 UI（下部）=====
    const float button_y = status_panel_.y + status_panel_.height - 70.0f;
    const float button_h = 48.0f;
    const float button_w = (status_panel_.width - status_panel_.padding * 2.0f - 10.0f) / 2.0f;
    const float bx = status_panel_.x + status_panel_.padding;
    Vector2 mouse = systemAPI_->GetMousePosition();

    Rectangle downRect{bx, button_y, button_w, button_h};
    Rectangle upRect{bx + button_w + 10.0f, button_y, button_w, button_h};
    bool hoverDown = (mouse.x >= downRect.x && mouse.x < downRect.x + downRect.width &&
                      mouse.y >= downRect.y && mouse.y < downRect.y + downRect.height);
    bool hoverUp = (mouse.x >= upRect.x && mouse.x < upRect.x + upRect.width &&
                    mouse.y >= upRect.y && mouse.y < upRect.y + upRect.height);

    UIEffects::DrawModernButton(systemAPI_, downRect.x, downRect.y, downRect.width, downRect.height,
                                OverlayColors::BUTTON_SECONDARY_DARK, OverlayColors::BUTTON_SECONDARY_BRIGHT,
                                hoverDown, false);
    UIEffects::DrawModernButton(systemAPI_, upRect.x, upRect.y, upRect.width, upRect.height,
                                OverlayColors::BUTTON_PRIMARY_DARK, OverlayColors::BUTTON_PRIMARY_BRIGHT,
                                hoverUp, false);

    systemAPI_->DrawTextDefault("Lv -",
                                downRect.x + 18.0f, downRect.y + 10.0f,
                                26.0f, OverlayColors::TEXT_DARK);
    systemAPI_->DrawTextDefault("Lv +",
                                upRect.x + 18.0f, upRect.y + 10.0f,
                                26.0f, OverlayColors::TEXT_DARK);
}

void CharacterEnhancementOverlay::RenderOperationPanel() {
    using namespace ui;

    // 背景
    systemAPI_->DrawRectangle(
        operation_panel_.x, operation_panel_.y,
        operation_panel_.width, operation_panel_.height,
        OverlayColors::PANEL_BG_ORANGE
    );
    
    // ゴールドボーダー
    systemAPI_->DrawRectangleLines(
        operation_panel_.x, operation_panel_.y,
        operation_panel_.width, operation_panel_.height,
        2.0f,
        OverlayColors::BORDER_GOLD
    );
    
    // タブボタン
    const float tab_width = operation_panel_.width / 2.0f;
    const float tab_height = 50.0f;
    const float tab_y = operation_panel_.y + 10.0f;
    
    RenderTabButton(operation_panel_.x, tab_y, tab_width, tab_height, "パッシブスキル", 
                    operation_panel_.active_tab == OperationPanel::TabType::Enhancement);
    RenderTabButton(operation_panel_.x + tab_width, tab_y, tab_width, tab_height, "装備",
                    operation_panel_.active_tab == OperationPanel::TabType::Equipment);
    
    // タブ内容
    if (operation_panel_.active_tab == OperationPanel::TabType::Enhancement) {
        RenderEnhancementTab();
    } else {
        RenderEquipmentTab();
    }
    
    // 決定/取消ボタンは廃止（即時セーブ）
}

void CharacterEnhancementOverlay::RenderEnhancementTab() {
    using namespace ui;

    // パッシブスロット3つを描画
    for (int i = 0; i < 3; ++i) {
        RenderPassiveSlot(operation_panel_.passive_slots[i]);
    }

    // ===== Reset / Reroll ボタン =====
    const float buttons_y = operation_panel_.y + operation_panel_.passive_slots[0].position.y + operation_panel_.passive_slots[0].height + 18.0f;
    const float buttons_h = 44.0f;
    const float buttons_w = (operation_panel_.width - 40.0f - 10.0f) / 2.0f;
    const float bx = operation_panel_.x + 20.0f;
    Rectangle resetRect{bx, buttons_y, buttons_w, buttons_h};
    Rectangle rerollRect{bx + buttons_w + 10.0f, buttons_y, buttons_w, buttons_h};

    Vector2 mouse_pos = systemAPI_->GetMousePosition();
    const bool hoverReset = (mouse_pos.x >= resetRect.x && mouse_pos.x < resetRect.x + resetRect.width &&
                             mouse_pos.y >= resetRect.y && mouse_pos.y < resetRect.y + resetRect.height);
    const bool hoverReroll = (mouse_pos.x >= rerollRect.x && mouse_pos.x < rerollRect.x + rerollRect.width &&
                              mouse_pos.y >= rerollRect.y && mouse_pos.y < rerollRect.y + rerollRect.height);

    UIEffects::DrawModernButton(systemAPI_, resetRect.x, resetRect.y, resetRect.width, resetRect.height,
                                OverlayColors::BUTTON_SECONDARY_DARK, OverlayColors::BUTTON_SECONDARY_BRIGHT,
                                hoverReset, false);
    UIEffects::DrawModernButton(systemAPI_, rerollRect.x, rerollRect.y, rerollRect.width, rerollRect.height,
                                OverlayColors::BUTTON_PRIMARY_DARK, OverlayColors::BUTTON_PRIMARY_BRIGHT,
                                hoverReroll, false);

    systemAPI_->DrawTextDefault("全リセット",
                                resetRect.x + 16.0f, resetRect.y + 10.0f,
                                22.0f, OverlayColors::TEXT_DARK);
    systemAPI_->DrawTextDefault("リロール",
                                rerollRect.x + 18.0f, rerollRect.y + 10.0f,
                                22.0f, OverlayColors::TEXT_DARK);

    // ===== 下部: パッシブ一覧＋効果表（スクロール）=====
    const float list_y = buttons_y + buttons_h + 14.0f;
    const float list_x = operation_panel_.x + 20.0f;
    const float list_w = operation_panel_.width - 40.0f;
    const float list_h = operation_panel_.y + operation_panel_.height - list_y - 20.0f;
    const float row_h = 58.0f;

    systemAPI_->DrawTextDefault("パッシブ一覧 / Lv別効果 (Lv1-3)",
                                list_x, list_y - 26.0f, 18.0f, OverlayColors::TEXT_GOLD);

    // 背景枠
    systemAPI_->DrawRectangle(list_x, list_y, list_w, list_h, OverlayColors::ORANGE_PANEL_BG_DARK);
    systemAPI_->DrawRectangleLines(list_x, list_y, list_w, list_h, 2.0f, OverlayColors::BORDER_DEFAULT);

    auto targetStatToShort = [](entities::PassiveTargetStat st) -> const char* {
        switch (st) {
        case entities::PassiveTargetStat::Attack: return "ATK";
        case entities::PassiveTargetStat::Defense: return "DEF";
        case entities::PassiveTargetStat::Hp: return "HP";
        case entities::PassiveTargetStat::MoveSpeed: return "SPD";
        case entities::PassiveTargetStat::Range: return "RNG";
        case entities::PassiveTargetStat::AttackSpeed: return "AS";
        default: return "-";
        }
    };

    auto formatLvEffect = [&](const entities::PassiveSkill& p, int lv) -> std::string {
        const float v = p.value * static_cast<float>(lv);
        const char* stat = targetStatToShort(p.target_stat);
        if (p.effect_type == entities::PassiveEffectType::Percentage) {
            return TextFormat("%s %+d%%", stat, static_cast<int>(std::round(v * 100.0f)));
        }
        if (p.target_stat == entities::PassiveTargetStat::AttackSpeed) {
            // flat は攻撃間隔短縮（秒）
            return TextFormat("%s -%.2fs", stat, v);
        }
        return TextFormat("%s %+d", stat, static_cast<int>(std::round(v)));
    };

    // ヘッダー行
    systemAPI_->DrawTextDefault("Name", list_x + 10.0f, list_y + 8.0f, 16.0f, OverlayColors::TEXT_SECONDARY);
    systemAPI_->DrawTextDefault("R", list_x + list_w * 0.45f, list_y + 8.0f, 16.0f, OverlayColors::TEXT_SECONDARY);
    systemAPI_->DrawTextDefault("Lv1", list_x + list_w * 0.52f, list_y + 8.0f, 16.0f, OverlayColors::TEXT_SECONDARY);
    systemAPI_->DrawTextDefault("Lv2", list_x + list_w * 0.70f, list_y + 8.0f, 16.0f, OverlayColors::TEXT_SECONDARY);
    systemAPI_->DrawTextDefault("Lv3", list_x + list_w * 0.86f, list_y + 8.0f, 16.0f, OverlayColors::TEXT_SECONDARY);

    // 一覧（オフセットはヘッダーの1行分）
    const float content_y0 = list_y + 30.0f;
    const float content_h = list_h - 36.0f;

    for (int i = 0; i < static_cast<int>(operation_panel_.available_passives.size()); ++i) {
        const float y = content_y0 + (i - operation_panel_.passive_scroll_offset) * row_h;
        if (y < content_y0 || y >= content_y0 + content_h - row_h) {
            continue;
        }
        const auto* p = operation_panel_.available_passives[i];
        if (!p) continue;

        // 行背景
        systemAPI_->DrawRectangle(list_x + 6.0f, y, list_w - 12.0f, row_h - 6.0f, OverlayColors::PANEL_BG_ORANGE);
        systemAPI_->DrawRectangleLines(list_x + 6.0f, y, list_w - 12.0f, row_h - 6.0f, 1.0f, OverlayColors::BORDER_DEFAULT);

        // 名前
        systemAPI_->DrawTextDefault(p->name, list_x + 12.0f, y + 8.0f, 16.0f, WHITE);
        // レア
        std::string stars;
        const int passive_rarity = std::max(1, p->rarity);
        for (int s = 0; s < passive_rarity; ++s) stars += "★";
        systemAPI_->DrawTextDefault(stars, list_x + list_w * 0.45f, y + 10.0f, 14.0f, OverlayColors::TEXT_GOLD);

        // 効果
        systemAPI_->DrawTextDefault(formatLvEffect(*p, 1), list_x + list_w * 0.52f, y + 10.0f, 14.0f, OverlayColors::TEXT_SECONDARY);
        systemAPI_->DrawTextDefault(formatLvEffect(*p, 2), list_x + list_w * 0.70f, y + 10.0f, 14.0f, OverlayColors::TEXT_SECONDARY);
        systemAPI_->DrawTextDefault(formatLvEffect(*p, 3), list_x + list_w * 0.86f, y + 10.0f, 14.0f, OverlayColors::TEXT_SECONDARY);
    }
}

void CharacterEnhancementOverlay::RenderEquipmentTab() {
    using namespace ui;

    // アイテムスロット3つを描画
    for (int i = 0; i < 3; ++i) {
        RenderItemSlot(operation_panel_.item_slots[i]);
    }
    
    // インベントリ（2列、スクロール可能）
    const float list_y_relative = operation_panel_.passive_slots[0].position.y + operation_panel_.passive_slots[0].height + 30.0f;
    const float list_y = operation_panel_.y + list_y_relative;
    const float footer_h = 60.0f;
    const float list_height = operation_panel_.height - list_y_relative - footer_h - 10.0f;

    const float item_h = 80.0f;
    const float gap_x = 10.0f;
    const float list_x = operation_panel_.x + 20.0f;
    const float list_w = operation_panel_.width - 40.0f;
    const float col_w = (list_w - gap_x) / 2.0f;

    systemAPI_->DrawTextDefault("所持アイテム一覧（ドラッグ＆ドロップ）",
                                list_x, list_y - 25.0f,
                                20.0f, OverlayColors::TEXT_GOLD);

    // 枠
    systemAPI_->DrawRectangle(list_x, list_y, list_w, list_height, OverlayColors::ORANGE_PANEL_BG_DARK);
    systemAPI_->DrawRectangleLines(list_x, list_y, list_w, list_height, 2.0f, OverlayColors::BORDER_DEFAULT);

    const int total = static_cast<int>(operation_panel_.available_items.size());
    const int totalRows = (total + 1) / 2;
    const int visibleRows = std::max(1, static_cast<int>(std::floor(list_height / item_h)));
    operation_panel_.item_scroll_offset = std::max(0, std::min(operation_panel_.item_scroll_offset, std::max(0, totalRows - visibleRows)));

    for (int i = 0; i < total; ++i) {
        const int row = i / 2;
        const int col = i % 2;
        const float x = list_x + col * (col_w + gap_x);
        const float y = list_y + (row - operation_panel_.item_scroll_offset) * item_h;

        if (y < list_y || y >= list_y + list_height - item_h) {
            continue;
        }

        const auto* item = operation_panel_.available_items[i];
        if (!item) continue;

        // 背景
        systemAPI_->DrawRectangle(x, y, col_w, item_h - 6.0f, OverlayColors::PANEL_BG_ORANGE);
        systemAPI_->DrawRectangleLines(x, y, col_w, item_h - 6.0f, 1.0f, OverlayColors::BORDER_DEFAULT);

        // 名前
        systemAPI_->DrawTextDefault(item->name, x + 10.0f, y + 12.0f, 18.0f, WHITE);

        // ステータス
        std::string stats;
        if (item->hp_bonus != 0) stats += "HP" + std::string(item->hp_bonus > 0 ? "+" : "") + std::to_string(static_cast<int>(item->hp_bonus)) + " ";
        if (item->attack_bonus != 0) stats += "ATK" + std::string(item->attack_bonus > 0 ? "+" : "") + std::to_string(static_cast<int>(item->attack_bonus)) + " ";
        if (item->defense_bonus != 0) stats += "DEF" + std::string(item->defense_bonus > 0 ? "+" : "") + std::to_string(static_cast<int>(item->defense_bonus));
        systemAPI_->DrawTextDefault(stats, x + 10.0f, y + 44.0f, 14.0f, OverlayColors::TEXT_SUCCESS);
    }

    // ドラッグ中のプレビュー描画
    if (is_item_dragging_ && dragging_item_) {
        Rectangle rec{item_drag_pos_.x - 140.0f, item_drag_pos_.y - 30.0f, 280.0f, 60.0f};
        Color bg = OverlayColors::SLOT_ORANGE_SELECTED;
        bg.a = 200;
        systemAPI_->DrawRectangle(rec.x, rec.y, rec.width, rec.height, bg);
        systemAPI_->DrawRectangleLines(rec.x, rec.y, rec.width, rec.height, 2.0f, OverlayColors::BORDER_GOLD);
        systemAPI_->DrawTextDefault(dragging_item_->name, rec.x + 10.0f, rec.y + 10.0f, 16.0f, WHITE);
    }

    // フッター: すべて外す
    const float btn_y = operation_panel_.y + operation_panel_.height - footer_h;
    Rectangle removeAllRect{operation_panel_.x + 20.0f, btn_y + 8.0f, operation_panel_.width - 40.0f, 44.0f};
    Vector2 mouse_pos = systemAPI_->GetMousePosition();
    const bool hover = (mouse_pos.x >= removeAllRect.x && mouse_pos.x < removeAllRect.x + removeAllRect.width &&
                        mouse_pos.y >= removeAllRect.y && mouse_pos.y < removeAllRect.y + removeAllRect.height);
    UIEffects::DrawModernButton(systemAPI_, removeAllRect.x, removeAllRect.y, removeAllRect.width, removeAllRect.height,
                                OverlayColors::BUTTON_SECONDARY_DARK, OverlayColors::BUTTON_SECONDARY_BRIGHT,
                                hover, false);
    systemAPI_->DrawTextDefault("すべて外す", removeAllRect.x + 18.0f, removeAllRect.y + 10.0f, 22.0f, OverlayColors::TEXT_DARK);
}

void CharacterEnhancementOverlay::RenderPassivePopup() {
    using namespace ui;

    // 半透明背景
    systemAPI_->DrawRectangle(0, 0, 1920, 1080, OverlayColors::OVERLAY_BG);
    
    // ポップアップボックス
    const float popup_width = 400.0f;
    const float popup_height = 300.0f;
    const float popup_x = (1920.0f - popup_width) / 2.0f;
    const float popup_y = (1080.0f - popup_height) / 2.0f;
    
    systemAPI_->DrawRectangle(popup_x, popup_y, popup_width, popup_height, OverlayColors::PANEL_BG_ORANGE);
    systemAPI_->DrawRectangleLines(popup_x, popup_y, popup_width, popup_height, 3.0f, OverlayColors::BORDER_GOLD);
    
    // タイトル
    systemAPI_->DrawTextDefault("パッシブ設定",
                                popup_x + 20.0f, popup_y + 20.0f,
                                24.0f, OverlayColors::TEXT_GOLD);
    
    // 選択肢
    const float option_height = 50.0f;
    const float option_y_start = popup_y + 70.0f;
    Vector2 mouse_pos = systemAPI_->GetMousePosition();
    
    const auto& slot = operation_panel_.passive_slots[operation_panel_.popup_slot_id];
    bool is_empty = (slot.assigned_passive == nullptr);
    
    std::vector<PopupMenuItem> options;
    if (is_empty) {
        options.push_back({"ランダム付与", OverlayColors::TEXT_SUCCESS, 0, false});
        options.push_back({"キャンセル", OverlayColors::TEXT_SECONDARY, 3, false});
    } else {
        options.push_back({"強化 (Lv+" + std::to_string(1) + ")", OverlayColors::TEXT_GOLD, 0, false});
        options.push_back({"ランダム変更", OverlayColors::STATUS_NEUTRAL, 1, false});
        options.push_back({"削除", OverlayColors::TEXT_ERROR, 2, false});
        options.push_back({"キャンセル", OverlayColors::TEXT_SECONDARY, 3, false});
    }
    
    for (size_t i = 0; i < options.size(); ++i) {
        float option_y = option_y_start + i * (option_height + 10.0f);
        bool hover = (mouse_pos.x >= popup_x + 20.0f && mouse_pos.x < popup_x + popup_width - 20.0f &&
                     mouse_pos.y >= option_y && mouse_pos.y < option_y + option_height);
        
        Color bg_color = hover ? OverlayColors::PANEL_BG_ORANGE_LIGHT : OverlayColors::ORANGE_PANEL_BG_DARK;
        
        systemAPI_->DrawRectangle(popup_x + 20.0f, option_y, popup_width - 40.0f, option_height, bg_color);
        systemAPI_->DrawRectangleLines(popup_x + 20.0f, option_y, popup_width - 40.0f, option_height, 2.0f, options[i].color);
        
        Vector2 text_size = systemAPI_->MeasureTextDefault(options[i].label, 20.0f);
        systemAPI_->DrawTextDefault(options[i].label,
                                    popup_x + (popup_width - text_size.x) / 2.0f,
                                    option_y + (option_height - text_size.y) / 2.0f,
                                    20.0f, WHITE);
    }
}

void CharacterEnhancementOverlay::RenderItemPopup() {
    using namespace ui;

    // 半透明背景
    systemAPI_->DrawRectangle(0, 0, 1920, 1080, OverlayColors::OVERLAY_BG);

    const float popup_width = 420.0f;
    const float popup_height = 260.0f;
    const float popup_x = (1920.0f - popup_width) / 2.0f;
    const float popup_y = (1080.0f - popup_height) / 2.0f;

    systemAPI_->DrawRectangle(popup_x, popup_y, popup_width, popup_height, OverlayColors::PANEL_BG_ORANGE);
    systemAPI_->DrawRectangleLines(popup_x, popup_y, popup_width, popup_height, 3.0f, OverlayColors::BORDER_GOLD);

    systemAPI_->DrawTextDefault("装備スロット設定",
                                popup_x + 20.0f, popup_y + 20.0f,
                                24.0f, OverlayColors::TEXT_GOLD);

    const int slotId = operation_panel_.popup_item_slot_id;
    if (slotId < 0 || slotId >= 3) {
        return;
    }
    const auto& slot = operation_panel_.item_slots[slotId];
    const bool is_empty = (slot.assigned_item == nullptr);

    const float option_height = 50.0f;
    const float option_y_start = popup_y + 70.0f;
    Vector2 mouse_pos = systemAPI_->GetMousePosition();

    std::vector<PopupMenuItem> options;
    options.push_back({"このスロットに装備する", OverlayColors::TEXT_GOLD, 0, false});
    if (!is_empty) {
        options.push_back({"外す", OverlayColors::TEXT_ERROR, 1, false});
    }
    options.push_back({"キャンセル", OverlayColors::TEXT_SECONDARY, 2, false});

    for (size_t i = 0; i < options.size(); ++i) {
        float option_y = option_y_start + static_cast<float>(i) * (option_height + 10.0f);
        bool hover = (mouse_pos.x >= popup_x + 20.0f && mouse_pos.x < popup_x + popup_width - 20.0f &&
                      mouse_pos.y >= option_y && mouse_pos.y < option_y + option_height);

        Color bg_color = hover ? OverlayColors::PANEL_BG_ORANGE_LIGHT : OverlayColors::ORANGE_PANEL_BG_DARK;
        systemAPI_->DrawRectangle(popup_x + 20.0f, option_y, popup_width - 40.0f, option_height, bg_color);
        systemAPI_->DrawRectangleLines(popup_x + 20.0f, option_y, popup_width - 40.0f, option_height, 2.0f, options[i].color);

        Vector2 text_size = systemAPI_->MeasureTextDefault(options[i].label, 20.0f);
        systemAPI_->DrawTextDefault(options[i].label,
                                    popup_x + (popup_width - text_size.x) / 2.0f,
                                    option_y + (option_height - text_size.y) / 2.0f,
                                    20.0f, WHITE);
    }
}

void CharacterEnhancementOverlay::RenderPassiveSlot(const OperationPanel::PassiveSlot& slot) {
    using namespace ui;

    // スロット座標を操作パネル基準で計算
    const float abs_x = operation_panel_.x + slot.position.x;
    const float abs_y = operation_panel_.y + slot.position.y;

    Color bg_color = slot.is_hovered ? OverlayColors::SLOT_ORANGE_SELECTED : OverlayColors::SLOT_ORANGE_EMPTY;
    
    systemAPI_->DrawRectangle(abs_x, abs_y, slot.width, slot.height, bg_color);
    systemAPI_->DrawRectangleLines(abs_x, abs_y, slot.width, slot.height, 2.0f, OverlayColors::BORDER_GOLD);
    
    if (slot.assigned_passive) {
        // パッシブ名
        systemAPI_->DrawTextDefault(slot.assigned_passive->name,
                                    abs_x + 10.0f, abs_y + 20.0f,
                                    18.0f, WHITE);
        
        // レベル
        std::string level_text = "Lv." + std::to_string(slot.level);
        systemAPI_->DrawTextDefault(level_text,
                                    abs_x + 10.0f, abs_y + 50.0f,
                                    16.0f, OverlayColors::TEXT_GOLD);
        
        // 効果値
        std::string value_text = "効果: +" + std::to_string(static_cast<int>(slot.assigned_passive->value * slot.level));
        systemAPI_->DrawTextDefault(value_text,
                                    abs_x + 10.0f, abs_y + 75.0f,
                                    14.0f, OverlayColors::TEXT_SECONDARY);
    } else {
        // 空スロット
        Vector2 plus_size = systemAPI_->MeasureTextDefault("+", 40.0f);
        systemAPI_->DrawTextDefault("+",
                                    abs_x + (slot.width - plus_size.x) / 2.0f, abs_y + 40.0f,
                                    40.0f, OverlayColors::TEXT_SECONDARY);
        
        Vector2 empty_size = systemAPI_->MeasureTextDefault("空きスロット", 16.0f);
        systemAPI_->DrawTextDefault("空きスロット",
                                    abs_x + (slot.width - empty_size.x) / 2.0f, abs_y + 100.0f,
                                    16.0f, OverlayColors::TEXT_SECONDARY);
    }
}

void CharacterEnhancementOverlay::RenderItemSlot(const OperationPanel::ItemSlot& slot) {
    using namespace ui;

    // スロット座標を操作パネル基準で計算
    const float abs_x = operation_panel_.x + slot.position.x;
    const float abs_y = operation_panel_.y + slot.position.y;

    const bool is_selected = (operation_panel_.selected_item_slot_id == slot.slot_id);
    Color bg_color = (slot.is_hovered || is_selected) ? OverlayColors::SLOT_ORANGE_SELECTED : OverlayColors::SLOT_ORANGE_EMPTY;
    
    systemAPI_->DrawRectangle(abs_x, abs_y, slot.width, slot.height, bg_color);
    systemAPI_->DrawRectangleLines(abs_x, abs_y, slot.width, slot.height, 2.0f, OverlayColors::BORDER_GOLD);
    
    if (slot.assigned_item) {
        // アイテム名
        systemAPI_->DrawTextDefault(slot.assigned_item->name,
                                    abs_x + 10.0f, abs_y + 20.0f,
                                    18.0f, WHITE);
        
        // ステータスボーナス
        std::string bonus_text = "";
        if (slot.assigned_item->hp_bonus > 0) {
            bonus_text += "HP+" + std::to_string(static_cast<int>(slot.assigned_item->hp_bonus)) + " ";
        }
        if (slot.assigned_item->attack_bonus > 0) {
            bonus_text += "ATK+" + std::to_string(static_cast<int>(slot.assigned_item->attack_bonus)) + " ";
        }
        if (slot.assigned_item->defense_bonus > 0) {
            bonus_text += "DEF+" + std::to_string(static_cast<int>(slot.assigned_item->defense_bonus));
        }
        
        systemAPI_->DrawTextDefault(bonus_text,
                                    abs_x + 10.0f, abs_y + 50.0f,
                                    14.0f, OverlayColors::TEXT_SUCCESS);

        // 解除ボタン（右上の×）
        const float btnSize = 22.0f;
        Rectangle xRect{abs_x + slot.width - btnSize - 6.0f, abs_y + 6.0f, btnSize, btnSize};
        systemAPI_->DrawRectangle(xRect.x, xRect.y, xRect.width, xRect.height, OverlayColors::BUTTON_SECONDARY_BRIGHT);
        systemAPI_->DrawRectangleLines(xRect.x, xRect.y, xRect.width, xRect.height, 2.0f, OverlayColors::BORDER_DEFAULT);
        systemAPI_->DrawTextDefault("×", xRect.x + 6.0f, xRect.y + 2.0f, 20.0f, OverlayColors::TEXT_DARK);
    } else {
        // 空スロット
        Vector2 empty_size = systemAPI_->MeasureTextDefault("空きスロット", 20.0f);
        systemAPI_->DrawTextDefault("空きスロット",
                                    abs_x + (slot.width - empty_size.x) / 2.0f, abs_y + 70.0f,
                                    20.0f, OverlayColors::TEXT_SECONDARY);
    }
}

void CharacterEnhancementOverlay::RenderTabButton(float x, float y, float width, float height, const char* label, bool is_active) {
    using namespace ui;

    // タブボタンの背景色を統一（選択時は明るい背景、非選択時は通常背景）
    Color bg_color = is_active ? OverlayColors::CARD_BG_SELECTED : OverlayColors::CARD_BG_NORMAL;
    // 明るい背景(CARD_BG_SELECTED)の場合は暗いテキストを使用
    Color text_color = is_active ? OverlayColors::TEXT_DARK : OverlayColors::TEXT_SECONDARY;
    Color border_color = is_active ? OverlayColors::BORDER_GOLD : OverlayColors::BORDER_DEFAULT;
    float border_width = is_active ? 3.0f : 2.0f;
    
    systemAPI_->DrawRectangle(x, y, width, height, bg_color);
    systemAPI_->DrawRectangleLines(x, y, width, height, border_width, border_color);
    
    // 選択中のタブの上部にアクセントラインを追加
    if (is_active) {
        systemAPI_->DrawLine(x, y, x + width, y, 3.0f, OverlayColors::ACCENT_GOLD);
    }
    
    Vector2 text_size = systemAPI_->MeasureTextDefault(label, 26.0f);
    systemAPI_->DrawTextDefault(label,
                                x + (width - text_size.x) / 2.0f,
                                y + (height - text_size.y) / 2.0f,
                                26.0f, text_color);
}

// ========== イベント処理メソッド ==========

void CharacterEnhancementOverlay::ProcessMouseInput(SharedContext& ctx) {
    if (systemAPI_->IsMouseButtonPressed(0)) {
        Vector2 mouse_pos = systemAPI_->GetMousePosition();
        
        // ポップアップが表示中の場合、ポップアップの処理が優先
        if (operation_panel_.show_passive_popup) {
            const float popup_width = 400.0f;
            const float popup_height = 300.0f;
            const float popup_x = (1920.0f - popup_width) / 2.0f;
            const float popup_y = (1080.0f - popup_height) / 2.0f;
            const float option_height = 50.0f;
            const float option_y_start = popup_y + 70.0f;
            
            const auto& slot = operation_panel_.passive_slots[operation_panel_.popup_slot_id];
            bool is_empty = (slot.assigned_passive == nullptr);
            int num_options = is_empty ? 2 : 4;
            
            for (int i = 0; i < num_options; ++i) {
                float option_y = option_y_start + i * (option_height + 10.0f);
                if (mouse_pos.x >= popup_x + 20.0f && mouse_pos.x < popup_x + popup_width - 20.0f &&
                    mouse_pos.y >= option_y && mouse_pos.y < option_y + option_height) {
                    if (is_empty) {
                        OnPassivePopupOption(ctx, i == 0 ? 0 : 3);
                    } else {
                        OnPassivePopupOption(ctx, i);
                    }
                    return;
                }
            }
            
            if (mouse_pos.x < popup_x || mouse_pos.x >= popup_x + popup_width ||
                mouse_pos.y < popup_y || mouse_pos.y >= popup_y + popup_height) {
                operation_panel_.show_passive_popup = false;
            }
            return;
        }

        if (operation_panel_.show_item_popup) {
            const float popup_width = 420.0f;
            const float popup_height = 260.0f;
            const float popup_x = (1920.0f - popup_width) / 2.0f;
            const float popup_y = (1080.0f - popup_height) / 2.0f;
            const float option_height = 50.0f;
            const float option_y_start = popup_y + 70.0f;

            const int slotId = operation_panel_.popup_item_slot_id;
            bool is_empty = true;
            if (slotId >= 0 && slotId < 3) {
                is_empty = (operation_panel_.item_slots[slotId].assigned_item == nullptr);
            }
            const int num_options = is_empty ? 2 : 3;

            for (int i = 0; i < num_options; ++i) {
                float option_y = option_y_start + i * (option_height + 10.0f);
                if (mouse_pos.x >= popup_x + 20.0f && mouse_pos.x < popup_x + popup_width - 20.0f &&
                    mouse_pos.y >= option_y && mouse_pos.y < option_y + option_height) {
                    // empty: [equip_here, cancel] => option_id: 0,2
                    // non-empty: [equip_here, remove, cancel] => 0,1,2
                    if (is_empty) {
                        OnItemPopupOption(ctx, i == 0 ? 0 : 2);
                    } else {
                        OnItemPopupOption(ctx, i);
                    }
                    return;
                }
            }

            if (mouse_pos.x < popup_x || mouse_pos.x >= popup_x + popup_width ||
                mouse_pos.y < popup_y || mouse_pos.y >= popup_y + popup_height) {
                operation_panel_.show_item_popup = false;
            }
            return;
        }

        // ユニット一覧クリック
        const float list_top = unit_info_panel_.y + 45.0f;
        const float list_height = 300.0f;
        for (int i = 0; i < static_cast<int>(unit_info_panel_.entries.size()); ++i) {
            float item_y = list_top + (i - unit_info_panel_.scroll_offset) * unit_info_panel_.item_height;
            
            if (item_y < list_top || item_y >= list_top + list_height) {
                continue;
            }
            
            if (mouse_pos.x >= unit_info_panel_.x && mouse_pos.x < unit_info_panel_.x + unit_info_panel_.width &&
                mouse_pos.y >= item_y && mouse_pos.y < item_y + unit_info_panel_.item_height) {
                OnUnitListItemClick(ctx, i);
                return;
            }
        }
        
        // タブクリック
        const float tab_width = operation_panel_.width / 2.0f;
        const float tab_height = 50.0f;
        const float tab_y = operation_panel_.y + 10.0f;
        
        if (mouse_pos.y >= tab_y && mouse_pos.y < tab_y + tab_height) {
            if (mouse_pos.x >= operation_panel_.x && mouse_pos.x < operation_panel_.x + tab_width) {
                OnTabClick(OperationPanel::TabType::Enhancement);
                return;
            } else if (mouse_pos.x >= operation_panel_.x + tab_width && mouse_pos.x < operation_panel_.x + operation_panel_.width) {
                OnTabClick(OperationPanel::TabType::Equipment);
                return;
            }
        }
        
        // レベルアップ/ダウン（ステータスパネル下部）
        {
            const float button_y = status_panel_.y + status_panel_.height - 70.0f;
            const float button_h = 48.0f;
            const float button_w = (status_panel_.width - status_panel_.padding * 2.0f - 10.0f) / 2.0f;
            const float bx = status_panel_.x + status_panel_.padding;
            Rectangle downRect{bx, button_y, button_w, button_h};
            Rectangle upRect{bx + button_w + 10.0f, button_y, button_w, button_h};
            if (mouse_pos.x >= downRect.x && mouse_pos.x < downRect.x + downRect.width &&
                mouse_pos.y >= downRect.y && mouse_pos.y < downRect.y + downRect.height) {
                OnLevelDownClick(ctx);
                return;
            }
            if (mouse_pos.x >= upRect.x && mouse_pos.x < upRect.x + upRect.width &&
                mouse_pos.y >= upRect.y && mouse_pos.y < upRect.y + upRect.height) {
                OnLevelUpClick(ctx);
                return;
            }
        }

        // パッシブスロットクリック（強化タブ時）
        if (operation_panel_.active_tab == OperationPanel::TabType::Enhancement) {
            for (int i = 0; i < 3; ++i) {
                const auto& slot = operation_panel_.passive_slots[i];
                const float slot_abs_x = operation_panel_.x + slot.position.x;
                const float slot_abs_y = operation_panel_.y + slot.position.y;
                
                if (mouse_pos.x >= slot_abs_x && mouse_pos.x < slot_abs_x + slot.width &&
                    mouse_pos.y >= slot_abs_y && mouse_pos.y < slot_abs_y + slot.height) {
                    OnPassiveSlotClick(i);
                    return;
                }
            }

            // Reset / Reroll ボタン
            const float buttons_y = operation_panel_.y + operation_panel_.passive_slots[0].position.y + operation_panel_.passive_slots[0].height + 18.0f;
            const float buttons_h = 44.0f;
            const float buttons_w = (operation_panel_.width - 40.0f - 10.0f) / 2.0f;
            const float bx = operation_panel_.x + 20.0f;
            Rectangle resetRect{bx, buttons_y, buttons_w, buttons_h};
            Rectangle rerollRect{bx + buttons_w + 10.0f, buttons_y, buttons_w, buttons_h};
            if (mouse_pos.x >= resetRect.x && mouse_pos.x < resetRect.x + resetRect.width &&
                mouse_pos.y >= resetRect.y && mouse_pos.y < resetRect.y + resetRect.height) {
                ResetAllPassives(ctx);
                return;
            }
            if (mouse_pos.x >= rerollRect.x && mouse_pos.x < rerollRect.x + rerollRect.width &&
                mouse_pos.y >= rerollRect.y && mouse_pos.y < rerollRect.y + rerollRect.height) {
                RerollAllPassives(ctx);
                return;
            }
        }
        
        // アイテムスロットクリック（装備タブ時）
        if (operation_panel_.active_tab == OperationPanel::TabType::Equipment) {
            for (int i = 0; i < 3; ++i) {
                const auto& slot = operation_panel_.item_slots[i];
                const float slot_abs_x = operation_panel_.x + slot.position.x;
                const float slot_abs_y = operation_panel_.y + slot.position.y;
                
                if (mouse_pos.x >= slot_abs_x && mouse_pos.x < slot_abs_x + slot.width &&
                    mouse_pos.y >= slot_abs_y && mouse_pos.y < slot_abs_y + slot.height) {
                    // 右上×解除
                    if (slot.assigned_item) {
                        const float btnSize = 22.0f;
                        Rectangle xRect{slot_abs_x + slot.width - btnSize - 6.0f, slot_abs_y + 6.0f, btnSize, btnSize};
                        if (mouse_pos.x >= xRect.x && mouse_pos.x < xRect.x + xRect.width &&
                            mouse_pos.y >= xRect.y && mouse_pos.y < xRect.y + xRect.height) {
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

                    // 選択のみ
                    OnItemSlotClick(i);
                    return;
                }
            }

            // フッター: すべて外す
            {
                const float list_y_relative = operation_panel_.passive_slots[0].position.y + operation_panel_.passive_slots[0].height + 30.0f;
                const float footer_h = 60.0f;
                const float btn_y = operation_panel_.y + operation_panel_.height - footer_h;
                Rectangle removeAllRect{operation_panel_.x + 20.0f, btn_y + 8.0f, operation_panel_.width - 40.0f, 44.0f};
                if (mouse_pos.x >= removeAllRect.x && mouse_pos.x < removeAllRect.x + removeAllRect.width &&
                    mouse_pos.y >= removeAllRect.y && mouse_pos.y < removeAllRect.y + removeAllRect.height) {
                    (void)list_y_relative;
                    RemoveAllEquipment(ctx);
                    return;
                }
            }

            // アイテム一覧（2列）: ドラッグ候補選択
            {
                const float list_y_relative = operation_panel_.passive_slots[0].position.y + operation_panel_.passive_slots[0].height + 30.0f;
                const float list_y = operation_panel_.y + list_y_relative;
                const float footer_h = 60.0f;
                const float list_h = operation_panel_.height - list_y_relative - footer_h - 10.0f;
                const float list_x = operation_panel_.x + 20.0f;
                const float list_w = operation_panel_.width - 40.0f;
                const float item_h = 80.0f;
                const float gap_x = 10.0f;
                const float col_w = (list_w - gap_x) / 2.0f;

                if (mouse_pos.x >= list_x && mouse_pos.x < list_x + list_w &&
                    mouse_pos.y >= list_y && mouse_pos.y < list_y + list_h) {
                    const int total = static_cast<int>(operation_panel_.available_items.size());
                    const int totalRows = (total + 1) / 2;
                    const int visibleRows = std::max(1, static_cast<int>(std::floor(list_h / item_h)));
                    operation_panel_.item_scroll_offset = std::max(0, std::min(operation_panel_.item_scroll_offset, std::max(0, totalRows - visibleRows)));

                    const float relY = mouse_pos.y - list_y;
                    const int row = static_cast<int>(std::floor(relY / item_h)) + operation_panel_.item_scroll_offset;
                    const float relX = mouse_pos.x - list_x;
                    int col = -1;
                    if (relX < col_w) col = 0;
                    else if (relX >= col_w + gap_x) col = 1;
                    else col = -1; // gap

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
        
        // 決定/取消ボタンは廃止（即時セーブ）
    }

    // ドラッグ更新
    if (operation_panel_.active_tab == OperationPanel::TabType::Equipment) {
        Vector2 mouse_pos = systemAPI_->GetMousePosition();
        if (item_drag_started_ && dragging_item_) {
            // ドラッグ開始判定
            if (!is_item_dragging_ && systemAPI_->IsMouseButtonDown(0)) {
                const float dx = mouse_pos.x - item_drag_start_pos_.x;
                const float dy = mouse_pos.y - item_drag_start_pos_.y;
                if (std::sqrt(dx * dx + dy * dy) > 3.0f) {
                    is_item_dragging_ = true;
                }
            }

            if (is_item_dragging_) {
                item_drag_pos_ = mouse_pos;
            }

            // ドロップ
            if (systemAPI_->IsMouseButtonReleased(0)) {
                if (is_item_dragging_) {
                    for (int i = 0; i < 3; ++i) {
                        const auto& slot = operation_panel_.item_slots[i];
                        const float slot_abs_x = operation_panel_.x + slot.position.x;
                        const float slot_abs_y = operation_panel_.y + slot.position.y;
                        if (mouse_pos.x >= slot_abs_x && mouse_pos.x < slot_abs_x + slot.width &&
                            mouse_pos.y >= slot_abs_y && mouse_pos.y < slot_abs_y + slot.height) {
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

                // 終了
                item_drag_started_ = false;
                is_item_dragging_ = false;
                dragging_item_index_ = -1;
                dragging_item_ = nullptr;
            }
        }
    }
    
    // スクロール処理
    float wheel = systemAPI_->GetMouseWheelMove();
    if (wheel != 0.0f) {
        Vector2 mouse_pos = systemAPI_->GetMousePosition();
        
        // ユニット一覧スクロール
        const float list_top = unit_info_panel_.y + 45.0f;
        if (mouse_pos.x >= unit_info_panel_.x && mouse_pos.x < unit_info_panel_.x + unit_info_panel_.width &&
            mouse_pos.y >= list_top && mouse_pos.y < list_top + 300.0f) {
            unit_info_panel_.scroll_offset -= static_cast<int>(wheel);
            unit_info_panel_.scroll_offset = std::max(0, std::min(unit_info_panel_.scroll_offset, 
                                                                   static_cast<int>(unit_info_panel_.entries.size()) - 5));
        }
        
        // アイテム一覧スクロール（装備タブ時）
        if (operation_panel_.active_tab == OperationPanel::TabType::Equipment) {
            const float list_y_relative = operation_panel_.passive_slots[0].position.y + operation_panel_.passive_slots[0].height + 30.0f;
            const float list_y = operation_panel_.y + list_y_relative;
            const float footer_h = 60.0f;
            const float list_h = operation_panel_.height - list_y_relative - footer_h - 10.0f;
            const float list_x = operation_panel_.x + 20.0f;
            const float list_w = operation_panel_.width - 40.0f;
            if (mouse_pos.x >= list_x && mouse_pos.x < list_x + list_w &&
                mouse_pos.y >= list_y && mouse_pos.y < list_y + list_h) {
                const int total = static_cast<int>(operation_panel_.available_items.size());
                const int totalRows = (total + 1) / 2;
                const float item_h = 80.0f;
                const int visibleRows = std::max(1, static_cast<int>(std::floor(list_h / item_h)));
                const int maxOffset = std::max(0, totalRows - visibleRows);

                operation_panel_.item_scroll_offset -= static_cast<int>(wheel);
                operation_panel_.item_scroll_offset = std::max(0, std::min(operation_panel_.item_scroll_offset, maxOffset));
            }
        }

        // パッシブ一覧スクロール（パッシブスキルタブ時）
        if (operation_panel_.active_tab == OperationPanel::TabType::Enhancement) {
            const float buttons_y = operation_panel_.y + operation_panel_.passive_slots[0].position.y + operation_panel_.passive_slots[0].height + 18.0f;
            const float list_y = buttons_y + 44.0f + 14.0f;
            const float list_x = operation_panel_.x + 20.0f;
            const float list_w = operation_panel_.width - 40.0f;
            const float list_h = operation_panel_.y + operation_panel_.height - list_y - 20.0f;

            if (mouse_pos.x >= list_x && mouse_pos.x < list_x + list_w &&
                mouse_pos.y >= list_y && mouse_pos.y < list_y + list_h) {
                operation_panel_.passive_scroll_offset -= static_cast<int>(wheel);
                const int maxOffset = std::max(0, static_cast<int>(operation_panel_.available_passives.size()) - 3);
                operation_panel_.passive_scroll_offset = std::max(0, std::min(operation_panel_.passive_scroll_offset, maxOffset));
            }
        }
    }
}

void CharacterEnhancementOverlay::UpdateHoverStates(Vector2 mouse_pos) {
    // パッシブスロットのホバー状態
    for (int i = 0; i < 3; ++i) {
        auto& slot = operation_panel_.passive_slots[i];
        const float slot_abs_x = operation_panel_.x + slot.position.x;
        const float slot_abs_y = operation_panel_.y + slot.position.y;
        
        slot.is_hovered = (
            mouse_pos.x >= slot_abs_x &&
            mouse_pos.x < slot_abs_x + slot.width &&
            mouse_pos.y >= slot_abs_y &&
            mouse_pos.y < slot_abs_y + slot.height
        );
    }
    
    // アイテムスロットのホバー状態
    for (int i = 0; i < 3; ++i) {
        auto& slot = operation_panel_.item_slots[i];
        const float slot_abs_x = operation_panel_.x + slot.position.x;
        const float slot_abs_y = operation_panel_.y + slot.position.y;
        
        slot.is_hovered = (
            mouse_pos.x >= slot_abs_x &&
            mouse_pos.x < slot_abs_x + slot.width &&
            mouse_pos.y >= slot_abs_y &&
            mouse_pos.y < slot_abs_y + slot.height
        );
    }
}

void CharacterEnhancementOverlay::ProcessKeyboardInput(SharedContext& ctx) {
    // ESCキーで閉じる
    if (systemAPI_->IsKeyPressed(256)) {  // KEY_ESCAPE
        if (operation_panel_.show_passive_popup) {
            operation_panel_.show_passive_popup = false;
        } else {
            requestClose_ = true;
        }
    }
}

// ========== イベントハンドラー ==========

void CharacterEnhancementOverlay::OnUnitListItemClick(SharedContext& ctx, int index) {
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
             tab == OperationPanel::TabType::Enhancement ? "Enhancement" : "Equipment");
}

void CharacterEnhancementOverlay::OnPassiveSlotClick(int slot_id) {
    if (slot_id < 0 || slot_id >= 3) {
        return;
    }
    
    // パッシブ一覧が空の場合は警告のみ（Update()で自動ロードされる）
    if (operation_panel_.available_passives.empty()) {
        LOG_WARN("CharacterEnhancementOverlay: Available passives not loaded yet");
    }
    
    operation_panel_.show_passive_popup = true;
    operation_panel_.popup_slot_id = slot_id;
    operation_panel_.popup_position = operation_panel_.passive_slots[slot_id].position;
    
    LOG_INFO("CharacterEnhancementOverlay: Passive slot {} clicked", slot_id);
}

void CharacterEnhancementOverlay::OnPassivePopupOption(SharedContext& ctx, int option) {
    const int slot_id = operation_panel_.popup_slot_id;
    
    switch (option) {
        case 0:  // ランダム付与/強化
            if (operation_panel_.passive_slots[slot_id].assigned_passive == nullptr) {
                AssignRandomPassive(slot_id);
            } else {
                UpgradePassive(slot_id);
            }
            break;
        case 1:  // ランダム変更
            ReplacePassive(slot_id);
            break;
        case 2:  // 削除
            RemovePassive(slot_id);
            break;
        case 3:  // キャンセル
            break;
    }
    
    operation_panel_.show_passive_popup = false;
    has_unsaved_changes_ = (option != 3);
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
    if (slot_id < 0 || slot_id >= 3) {
        return;
    }
    
    operation_panel_.selected_item_slot_id = slot_id;
    operation_panel_.show_item_popup = true;
    operation_panel_.popup_item_slot_id = slot_id;
    LOG_INFO("CharacterEnhancementOverlay: Item slot {} clicked", slot_id);
}

void CharacterEnhancementOverlay::OnItemListClick(SharedContext& ctx, int index) {
    if (index < 0 || index >= static_cast<int>(operation_panel_.available_items.size())) {
        return;
    }
    
    int targetSlot = operation_panel_.selected_item_slot_id;
    if (targetSlot < 0 || targetSlot >= 3) {
        // 空きスロット優先
        for (int i = 0; i < 3; ++i) {
            if (operation_panel_.item_slots[i].assigned_item == nullptr) {
                targetSlot = i;
                break;
            }
        }
        // それでも無ければ 0 を置き換え
        if (targetSlot < 0 || targetSlot >= 3) {
            targetSlot = 0;
        }
    }

    operation_panel_.item_slots[targetSlot].assigned_item = operation_panel_.available_items[index];
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

void CharacterEnhancementOverlay::OnItemPopupOption(SharedContext& ctx, int option) {
    const int slot_id = operation_panel_.popup_item_slot_id;
    if (slot_id < 0 || slot_id >= 3) {
        operation_panel_.show_item_popup = false;
        return;
    }

    switch (option) {
    case 0: // このスロットに装備する（選択）
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

void CharacterEnhancementOverlay::OnLevelUpClick(SharedContext& ctx) {
    const auto* character = GetSelectedCharacter();
    if (!character) return;
    PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
    ns.level = std::min(50, std::max(1, editing_character_state_.level) + 1);
    if (!TryCommitEditingState(ctx, ns)) {
        editing_character_state_ = saved_character_state_;
        ApplyStateToUI(ctx, saved_character_state_);
        UpdateStatusPanel(ctx);
        return;
    }
}

void CharacterEnhancementOverlay::OnLevelDownClick(SharedContext& ctx) {
    const auto* character = GetSelectedCharacter();
    if (!character) return;
    PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
    ns.level = std::max(1, std::max(1, editing_character_state_.level) - 1);
    if (!TryCommitEditingState(ctx, ns)) {
        editing_character_state_ = saved_character_state_;
        ApplyStateToUI(ctx, saved_character_state_);
        UpdateStatusPanel(ctx);
        return;
    }
}

void CharacterEnhancementOverlay::ResetAllPassives(SharedContext& ctx) {
    for (int i = 0; i < 3; ++i) {
        operation_panel_.passive_slots[i].assigned_passive = nullptr;
        operation_panel_.passive_slots[i].level = 1;
    }
    PlayerDataManager::CharacterState ns = BuildCurrentEditingState();
    ns.level = editing_character_state_.level;
    if (!TryCommitEditingState(ctx, ns)) {
        editing_character_state_ = saved_character_state_;
        ApplyStateToUI(ctx, saved_character_state_);
        UpdateStatusPanel(ctx);
    }
}

void CharacterEnhancementOverlay::RerollAllPassives(SharedContext& ctx) {
    // 3枠を再抽選（重複許容）
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
    }
}

void CharacterEnhancementOverlay::RemoveAllEquipment(SharedContext& ctx) {
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

// ========== パッシブ操作メソッド ==========

void CharacterEnhancementOverlay::AssignRandomPassive(int slot_id) {
    if (operation_panel_.available_passives.empty()) {
        LOG_WARN("CharacterEnhancementOverlay: No available passives");
        return;
    }
    
    std::uniform_int_distribution<int> dist(0, static_cast<int>(operation_panel_.available_passives.size()) - 1);
    int random_index = dist(rng_);
    
    operation_panel_.passive_slots[slot_id].assigned_passive = operation_panel_.available_passives[random_index];
    operation_panel_.passive_slots[slot_id].level = 1;
    
    LOG_INFO("CharacterEnhancementOverlay: Random passive assigned to slot {}", slot_id);
}

void CharacterEnhancementOverlay::UpgradePassive(int slot_id) {
    auto& slot = operation_panel_.passive_slots[slot_id];
    if (!slot.assigned_passive) {
        return;
    }
    
    if (slot.level < 3) {
        slot.level++;
        LOG_INFO("CharacterEnhancementOverlay: Passive upgraded to level {}", slot.level);
    } else {
        LOG_WARN("CharacterEnhancementOverlay: Passive already at max level");
    }
}

void CharacterEnhancementOverlay::ReplacePassive(int slot_id) {
    RemovePassive(slot_id);
    AssignRandomPassive(slot_id);
    LOG_INFO("CharacterEnhancementOverlay: Passive replaced at slot {}", slot_id);
}

void CharacterEnhancementOverlay::RemovePassive(int slot_id) {
    operation_panel_.passive_slots[slot_id].assigned_passive = nullptr;
    operation_panel_.passive_slots[slot_id].level = 1;
    LOG_INFO("CharacterEnhancementOverlay: Passive removed from slot {}", slot_id);
}

// ========== ユーティリティメソッド ==========

const entities::Character* CharacterEnhancementOverlay::GetSelectedCharacter() const {
    return unit_info_panel_.selected_character;
}

int CharacterEnhancementOverlay::GetPassiveSlotAtPosition(Vector2 position) const {
    for (int i = 0; i < 3; ++i) {
        const auto& slot = operation_panel_.passive_slots[i];
        if (position.x >= slot.position.x && position.x < slot.position.x + slot.width &&
            position.y >= slot.position.y && position.y < slot.position.y + slot.height) {
            return i;
        }
    }
    return -1;
}

int CharacterEnhancementOverlay::GetItemSlotAtPosition(Vector2 position) const {
    for (int i = 0; i < 3; ++i) {
        const auto& slot = operation_panel_.item_slots[i];
        if (position.x >= slot.position.x && position.x < slot.position.x + slot.width &&
            position.y >= slot.position.y && position.y < slot.position.y + slot.height) {
            return i;
        }
    }
    return -1;
}

} // namespace core
} // namespace game
