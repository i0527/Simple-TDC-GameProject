#include "CharacterEnhancementOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../entities/CharacterManager.hpp"
#include "../../entities/ItemPassiveManager.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UIEffects.hpp"
#include <algorithm>
#include <random>

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
    unit_info_panel_.icon_size = 200.0f;
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
        SelectCharacter(unit_info_panel_.entries[0]);
    }
    
    LOG_INFO("CharacterEnhancementOverlay: Loaded {} characters", unit_info_panel_.entries.size());
}

void CharacterEnhancementOverlay::SelectCharacter(const entities::Character* character) {
    if (!character) {
        return;
    }

    unit_info_panel_.selected_character = character;
    
    // ステータスパネルを更新
    UpdateStatusPanel();
    
    // パッシブスロットに既存のパッシブを割り当て
    for (int i = 0; i < 3; ++i) {
        if (i < static_cast<int>(character->passive_skills.size())) {
            operation_panel_.passive_slots[i].assigned_passive = &character->passive_skills[i];
            operation_panel_.passive_slots[i].level = 1;  // TODO: レベル情報があれば取得
        } else {
            operation_panel_.passive_slots[i].assigned_passive = nullptr;
            operation_panel_.passive_slots[i].level = 1;
        }
    }
    
    // アイテムスロットに既存の装備を割り当て
    for (int i = 0; i < 3; ++i) {
        if (i < static_cast<int>(character->equipment.size())) {
            operation_panel_.item_slots[i].assigned_item = &character->equipment[i];
        } else {
            operation_panel_.item_slots[i].assigned_item = nullptr;
        }
    }
    
    LOG_INFO("CharacterEnhancementOverlay: Selected character: {}", character->id);
}

void CharacterEnhancementOverlay::UpdateStatusPanel() {
    const auto* character = GetSelectedCharacter();
    if (!character) {
        return;
    }

    // 基本値設定
    status_panel_.hp.base = character->hp;
    status_panel_.attack.base = character->attack;
    status_panel_.defense.base = character->defense;
    status_panel_.speed.base = static_cast<int>(character->move_speed);
    status_panel_.range.base = static_cast<int>(character->attack_size.x);

    // ボーナス値のリセット
    status_panel_.hp.bonus = 0;
    status_panel_.attack.bonus = 0;
    status_panel_.defense.bonus = 0;
    status_panel_.speed.bonus = 0;
    status_panel_.range.bonus = 0;

    // アイテムスロットからのボーナス計算
    for (int i = 0; i < 3; ++i) {
        if (operation_panel_.item_slots[i].assigned_item) {
            const auto* item = operation_panel_.item_slots[i].assigned_item;
            status_panel_.hp.bonus += static_cast<int>(item->hp_bonus);
            status_panel_.attack.bonus += static_cast<int>(item->attack_bonus);
            status_panel_.defense.bonus += static_cast<int>(item->defense_bonus);
        }
    }

    // パッシブスロットからのボーナス計算 (仮実装: パッシブ内容に応じて分岐が必要だが、現在はvalueを加算)
    for (int i = 0; i < 3; ++i) {
        if (operation_panel_.passive_slots[i].assigned_passive) {
            const auto* passive = operation_panel_.passive_slots[i].assigned_passive;
            int level = operation_panel_.passive_slots[i].level;
            float effect = passive->value * level;

            // TODO: IDや名前で効果対象を判別
            if (passive->name.find("攻撃") != std::string::npos || passive->name.find("ATK") != std::string::npos) {
                status_panel_.attack.bonus += static_cast<int>(effect);
            } else if (passive->name.find("防御") != std::string::npos || passive->name.find("DEF") != std::string::npos) {
                status_panel_.defense.bonus += static_cast<int>(effect);
            } else if (passive->name.find("体力") != std::string::npos || passive->name.find("HP") != std::string::npos) {
                status_panel_.hp.bonus += static_cast<int>(effect);
            } else if (passive->name.find("速度") != std::string::npos || passive->name.find("SPD") != std::string::npos) {
                status_panel_.speed.bonus += static_cast<int>(effect);
            }
        }
    }
}

void CharacterEnhancementOverlay::FilterAvailablePassives(SharedContext& ctx) {
    operation_panel_.available_passives.clear();
    
    if (!ctx.itemPassiveManager) {
        return;
    }

    operation_panel_.available_passives = ctx.itemPassiveManager->GetAllPassiveSkills();
    
    LOG_INFO("CharacterEnhancementOverlay: Loaded {} available passives", 
             operation_panel_.available_passives.size());
}

void CharacterEnhancementOverlay::FilterAvailableItems(SharedContext& ctx) {
    operation_panel_.available_items.clear();
    
    if (!ctx.itemPassiveManager) {
        return;
    }

    operation_panel_.available_items = ctx.itemPassiveManager->GetAllEquipment();
    
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
    
    // 上部: ユニット一覧（スクロール可能）
    const float list_top = unit_info_panel_.y + 45.0f;
    const float list_height = 300.0f;
    const float detail_y = list_top + list_height + 20.0f;
    
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
    
    // 下部: 選択ユニット詳細
    if (unit_info_panel_.selected_character) {
        const auto* ch = unit_info_panel_.selected_character;
        
        // セパレーター
        systemAPI_->DrawRectangle(
            unit_info_panel_.x + 10.0f, detail_y - 10.0f,
            unit_info_panel_.width - 20.0f, 2.0f,
            OverlayColors::BORDER_GOLD
        );
        
        // アイコン（200×200px）
        const float icon_x = unit_info_panel_.x + (unit_info_panel_.width - unit_info_panel_.icon_size) / 2.0f;
        const float icon_y = detail_y + 10.0f;
        
        systemAPI_->DrawRectangle(
            icon_x, icon_y,
            unit_info_panel_.icon_size, unit_info_panel_.icon_size,
            OverlayColors::ORANGE_PANEL_BG_DARK
        );
        systemAPI_->DrawRectangleLines(
            icon_x, icon_y,
            unit_info_panel_.icon_size, unit_info_panel_.icon_size,
            2.0f,
            OverlayColors::BORDER_GOLD
        );
        
        // プレースホルダーテキスト
        Vector2 ph_size = systemAPI_->MeasureTextDefault("画像", 20.0f);
        systemAPI_->DrawTextDefault(
            "画像",
            icon_x + (unit_info_panel_.icon_size - ph_size.x) / 2.0f,
            icon_y + (unit_info_panel_.icon_size - ph_size.y) / 2.0f,
            20.0f, OverlayColors::TEXT_SECONDARY
        );
        
        // 名前（24px、金色）
        const float name_y = icon_y + unit_info_panel_.icon_size + 15.0f;
        Vector2 name_size = systemAPI_->MeasureTextDefault(ch->name, 24.0f);
        systemAPI_->DrawTextDefault(
            ch->name,
            unit_info_panel_.x + (unit_info_panel_.width - name_size.x) / 2.0f, name_y,
            24.0f, OverlayColors::TEXT_GOLD
        );
        
        // レベル（20px、白）
        std::string level_text = "Lv: " + std::to_string(1);  // TODO: レベル情報
        const float level_y = name_y + 35.0f;
        Vector2 level_size = systemAPI_->MeasureTextDefault(level_text, 20.0f);
        systemAPI_->DrawTextDefault(
            level_text,
            unit_info_panel_.x + (unit_info_panel_.width - level_size.x) / 2.0f, level_y,
            20.0f, WHITE
        );
        
        // スキル名（16px、グレー）
        if (!ch->passive_skills.empty()) {
            std::string skill_text = "スキル: " + ch->passive_skills[0].name;
            const float skill_y = level_y + 30.0f;
            Vector2 skill_size = systemAPI_->MeasureTextDefault(skill_text, 16.0f);
            systemAPI_->DrawTextDefault(
                skill_text,
                unit_info_panel_.x + (unit_info_panel_.width - skill_size.x) / 2.0f, skill_y,
                16.0f, OverlayColors::TEXT_SECONDARY
            );
        }
        
        // ステータス概要（簡易表示）
        const float stats_y = level_y + 60.0f;
        std::string stats_text = TextFormat("HP:%d  ATK:%d  DEF:%d", ch->hp, ch->attack, ch->defense);
        Vector2 stats_size = systemAPI_->MeasureTextDefault(stats_text, 16.0f);
        systemAPI_->DrawTextDefault(
            stats_text,
            unit_info_panel_.x + (unit_info_panel_.width - stats_size.x) / 2.0f, stats_y,
            16.0f, OverlayColors::TEXT_SECONDARY
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

    renderStatLine("HP (体力)", status_panel_.hp, 0);
    renderStatLine("ATK (攻撃)", status_panel_.attack, 1);
    renderStatLine("DEF (防御)", status_panel_.defense, 2);
    renderStatLine("SPD (速度)", status_panel_.speed, 3);
    renderStatLine("RNG (射程)", status_panel_.range, 4);
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
    
    RenderTabButton(operation_panel_.x, tab_y, tab_width, tab_height, "強化", 
                    operation_panel_.active_tab == OperationPanel::TabType::Enhancement);
    RenderTabButton(operation_panel_.x + tab_width, tab_y, tab_width, tab_height, "装備",
                    operation_panel_.active_tab == OperationPanel::TabType::Equipment);
    
    // タブ内容
    if (operation_panel_.active_tab == OperationPanel::TabType::Enhancement) {
        RenderEnhancementTab();
    } else {
        RenderEquipmentTab();
    }
    
    // 決定/取消ボタン（下部固定）
    const float button_width = (operation_panel_.width - 30.0f) / 2.0f;
    const float button_height = 50.0f;
    const float button_y = operation_panel_.y + operation_panel_.height - button_height - 10.0f;
    const float button_gap = 10.0f;
    
    Vector2 mouse_pos = systemAPI_->GetMousePosition();
    
    // 決定ボタン
    const float confirm_x = operation_panel_.x + 10.0f;
    bool confirm_hover = (mouse_pos.x >= confirm_x && mouse_pos.x < confirm_x + button_width &&
                         mouse_pos.y >= button_y && mouse_pos.y < button_y + button_height);
    
    UIEffects::DrawModernButton(systemAPI_, confirm_x, button_y, button_width, button_height,
                                   OverlayColors::BUTTON_PRIMARY_DARK, OverlayColors::BUTTON_PRIMARY_BRIGHT,
                                   confirm_hover, false);
    
    Vector2 confirm_text_size = systemAPI_->MeasureTextDefault("決定", 28.0f);
    // 明るい緑背景なので暗いテキストを使用
    systemAPI_->DrawTextDefault("決定", 
                                confirm_x + (button_width - confirm_text_size.x) / 2.0f,
                                button_y + (button_height - confirm_text_size.y) / 2.0f,
                                28.0f, OverlayColors::TEXT_DARK);
    
    // 取消ボタン
    const float cancel_x = confirm_x + button_width + button_gap;
    bool cancel_hover = (mouse_pos.x >= cancel_x && mouse_pos.x < cancel_x + button_width &&
                        mouse_pos.y >= button_y && mouse_pos.y < button_y + button_height);
    
    UIEffects::DrawModernButton(systemAPI_, cancel_x, button_y, button_width, button_height,
                                   OverlayColors::BUTTON_SECONDARY_DARK, OverlayColors::BUTTON_SECONDARY_BRIGHT,
                                   cancel_hover, false);
    
    Vector2 cancel_text_size = systemAPI_->MeasureTextDefault("取消", 28.0f);
    // 明るい青背景なので暗いテキストを使用
    systemAPI_->DrawTextDefault("取消",
                                cancel_x + (button_width - cancel_text_size.x) / 2.0f,
                                button_y + (button_height - cancel_text_size.y) / 2.0f,
                                28.0f, OverlayColors::TEXT_DARK);
}

void CharacterEnhancementOverlay::RenderEnhancementTab() {
    // パッシブスロット3つを描画
    for (int i = 0; i < 3; ++i) {
        RenderPassiveSlot(operation_panel_.passive_slots[i]);
    }
}

void CharacterEnhancementOverlay::RenderEquipmentTab() {
    using namespace ui;

    // アイテムスロット3つを描画
    for (int i = 0; i < 3; ++i) {
        RenderItemSlot(operation_panel_.item_slots[i]);
    }
    
    // アイテム一覧（スクロール可能）
    const float list_y_relative = operation_panel_.passive_slots[0].position.y + operation_panel_.passive_slots[0].height + 30.0f;
    const float list_y = operation_panel_.y + list_y_relative;
    const float list_height = operation_panel_.height - list_y_relative - 80.0f;
    const float item_height = 80.0f;
    
    systemAPI_->DrawTextDefault("所持アイテム一覧", 
                                operation_panel_.x + 20.0f, list_y - 25.0f,
                                20.0f, OverlayColors::TEXT_GOLD);
    
    for (int i = 0; i < static_cast<int>(operation_panel_.available_items.size()); ++i) {
        float item_y = list_y + (i - operation_panel_.item_scroll_offset) * item_height;
        
        if (item_y < list_y || item_y >= list_y + list_height - item_height) {
            continue;
        }
        
        const auto* item = operation_panel_.available_items[i];
        if (!item) continue;
        
        // アイテム背景
        systemAPI_->DrawRectangle(
            operation_panel_.x + 20.0f, item_y,
            operation_panel_.width - 40.0f, item_height - 5.0f,
            OverlayColors::ORANGE_PANEL_BG_DARK
        );
        systemAPI_->DrawRectangleLines(
            operation_panel_.x + 20.0f, item_y,
            operation_panel_.width - 40.0f, item_height - 5.0f,
            1.0f,
            OverlayColors::BORDER_DEFAULT
        );
        
        // アイコン
        const float icon_size = 60.0f;
        systemAPI_->DrawRectangle(
            operation_panel_.x + 30.0f, item_y + 10.0f,
            icon_size, icon_size,
            OverlayColors::ORANGE_PANEL_BG
        );
        
        // 名前とステータス
        systemAPI_->DrawTextDefault(item->name,
                                    operation_panel_.x + 100.0f, item_y + 15.0f,
                                    18.0f, WHITE);
        
        std::string stats = "";
        if (item->hp_bonus > 0) stats += "HP+" + std::to_string(static_cast<int>(item->hp_bonus)) + " ";
        if (item->attack_bonus > 0) stats += "ATK+" + std::to_string(static_cast<int>(item->attack_bonus)) + " ";
        if (item->defense_bonus > 0) stats += "DEF+" + std::to_string(static_cast<int>(item->defense_bonus));
        
        systemAPI_->DrawTextDefault(stats,
                                    operation_panel_.x + 100.0f, item_y + 40.0f,
                                    14.0f, OverlayColors::TEXT_SUCCESS);
    }
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

    Color bg_color = slot.is_hovered ? OverlayColors::SLOT_ORANGE_SELECTED : OverlayColors::SLOT_ORANGE_EMPTY;
    
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
                        OnPassivePopupOption(i == 0 ? 0 : 3);
                    } else {
                        OnPassivePopupOption(i);
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
                OnUnitListItemClick(i);
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
        }
        
        // アイテムスロットクリック（装備タブ時）
        if (operation_panel_.active_tab == OperationPanel::TabType::Equipment) {
            for (int i = 0; i < 3; ++i) {
                const auto& slot = operation_panel_.item_slots[i];
                const float slot_abs_x = operation_panel_.x + slot.position.x;
                const float slot_abs_y = operation_panel_.y + slot.position.y;
                
                if (mouse_pos.x >= slot_abs_x && mouse_pos.x < slot_abs_x + slot.width &&
                    mouse_pos.y >= slot_abs_y && mouse_pos.y < slot_abs_y + slot.height) {
                    OnItemSlotClick(i);
                    return;
                }
            }
            
            // アイテム一覧クリック
            const float list_y_relative = operation_panel_.passive_slots[0].position.y + operation_panel_.passive_slots[0].height + 30.0f;
            const float list_y = operation_panel_.y + list_y_relative;
            const float list_height = operation_panel_.height - list_y_relative - 80.0f;
            const float item_height = 80.0f;
            
            for (int i = 0; i < static_cast<int>(operation_panel_.available_items.size()); ++i) {
                float item_y = list_y + (i - operation_panel_.item_scroll_offset) * item_height;
                
                if (item_y < list_y || item_y >= list_y + list_height - item_height) {
                    continue;
                }
                
                if (mouse_pos.x >= operation_panel_.x + 20.0f && mouse_pos.x < operation_panel_.x + operation_panel_.width - 20.0f &&
                    mouse_pos.y >= item_y && mouse_pos.y < item_y + item_height - 5.0f) {
                    OnItemListClick(i);
                    return;
                }
            }
        }
        
        // 決定/取消ボタンクリック
        const float button_width = (operation_panel_.width - 30.0f) / 2.0f;
        const float button_height = 50.0f;
        const float button_y = operation_panel_.y + operation_panel_.height - button_height - 10.0f;
        const float button_gap = 10.0f;
        const float confirm_x = operation_panel_.x + 10.0f;
        const float cancel_x = confirm_x + button_width + button_gap;
        
        if (mouse_pos.y >= button_y && mouse_pos.y < button_y + button_height) {
            if (mouse_pos.x >= confirm_x && mouse_pos.x < confirm_x + button_width) {
                OnConfirmButtonClick();
                return;
            } else if (mouse_pos.x >= cancel_x && mouse_pos.x < cancel_x + button_width) {
                OnCancelButtonClick();
                return;
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
            
            if (mouse_pos.x >= operation_panel_.x && mouse_pos.x < operation_panel_.x + operation_panel_.width &&
                mouse_pos.y >= list_y) {
                operation_panel_.item_scroll_offset -= static_cast<int>(wheel);
                operation_panel_.item_scroll_offset = std::max(0, std::min(operation_panel_.item_scroll_offset,
                                                                            static_cast<int>(operation_panel_.available_items.size()) - 3));
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

void CharacterEnhancementOverlay::OnUnitListItemClick(int index) {
    if (index < 0 || index >= static_cast<int>(unit_info_panel_.entries.size())) {
        return;
    }
    
    unit_info_panel_.selected_index = index;
    SelectCharacter(unit_info_panel_.entries[index]);
}

void CharacterEnhancementOverlay::OnTabClick(OperationPanel::TabType tab) {
    operation_panel_.active_tab = tab;
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

void CharacterEnhancementOverlay::OnPassivePopupOption(int option) {
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
}

void CharacterEnhancementOverlay::OnItemSlotClick(int slot_id) {
    if (slot_id < 0 || slot_id >= 3) {
        return;
    }
    
    LOG_INFO("CharacterEnhancementOverlay: Item slot {} clicked", slot_id);
    // TODO: アイテム選択ダイアログ or ドロップ処理
}

void CharacterEnhancementOverlay::OnItemListClick(int index) {
    if (index < 0 || index >= static_cast<int>(operation_panel_.available_items.size())) {
        return;
    }
    
    // 空きスロットを探して装備
    for (int i = 0; i < 3; ++i) {
        if (operation_panel_.item_slots[i].assigned_item == nullptr) {
            operation_panel_.item_slots[i].assigned_item = operation_panel_.available_items[index];
            has_unsaved_changes_ = true;
            LOG_INFO("CharacterEnhancementOverlay: Item equipped to slot {}", i);
            return;
        }
    }
    
    LOG_WARN("CharacterEnhancementOverlay: No empty item slot");
}

void CharacterEnhancementOverlay::OnConfirmButtonClick() {
    LOG_INFO("CharacterEnhancementOverlay: Confirm button clicked");
    // TODO: 変更を保存
    has_unsaved_changes_ = false;
    requestClose_ = true;
}

void CharacterEnhancementOverlay::OnCancelButtonClick() {
    LOG_INFO("CharacterEnhancementOverlay: Cancel button clicked");
    requestClose_ = true;
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
