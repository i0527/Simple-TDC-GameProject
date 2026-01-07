#include "CodexOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../entities/Character.hpp"
#include "../../entities/CharacterManager.hpp"
#include <raylib.h>
#include <algorithm>
#include <regex>

namespace game {
namespace core {

// ========== コンストラクタ・デストラクタ ==========

CodexOverlay::CodexOverlay()
    : systemAPI_(nullptr)
    , isInitialized_(false)
    , requestClose_(false)
    , hasTransitionRequest_(false)
    , requestedNextState_(GameState::Title)
{
}

// ========== IOverlay 実装 ==========

bool CodexOverlay::Initialize(BaseSystemAPI* systemAPI) {
    if (isInitialized_) {
        LOG_ERROR("CodexOverlay already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("CodexOverlay: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    requestClose_ = false;
    hasTransitionRequest_ = false;
    
    list_panel_.selected_index = -1;
    list_panel_.scroll_offset = 0;
    character_viewport_.has_error = false;
    character_viewport_.error_message.clear();

    isInitialized_ = true;
    LOG_INFO("CodexOverlay initialized");
    return true;
}

void CodexOverlay::Update(SharedContext& ctx, float deltaTime) {
    if (!isInitialized_) {
        return;
    }

    // CharacterManagerからデータを取得（初回のみ）
    if (list_panel_.entries.empty() && ctx.characterManager) {
        const auto& masters = ctx.characterManager->GetAllMasters();
        for (const auto& [id, ch] : masters) {
            list_panel_.entries.push_back(&ch);
        }
        SortCharactersById();
        
        if (!list_panel_.entries.empty()) {
            list_panel_.selected_index = 0;
        }
        
        LOG_INFO("CodexOverlay: Loaded {} characters", list_panel_.entries.size());
    }

    // ESCキーで閉じる
    if (systemAPI_->IsKeyPressed(256)) {  // ESC key
        requestClose_ = true;
    }

    // アニメーション更新
    if (!character_viewport_.has_error) {
        character_viewport_.animation_timer += deltaTime;
        if (character_viewport_.animation_timer >= character_viewport_.animation_speed) {
            character_viewport_.animation_frame++;
            character_viewport_.animation_timer = 0.0f;
        }
    }

    // マウスイベント処理
    // コンテンツ領域のオフセット（ヘッダーとタブバーの間）
    const float contentOffsetX = 20.0f;
    const float contentOffsetY = 90.0f;
    
    if (systemAPI_->IsMouseButtonPressed(0)) {  // Left mouse button
        Vector2 mouse_pos = systemAPI_->GetMousePosition();
        
        // コンテンツ領域内の相対座標に変換
        float relative_x = mouse_pos.x - contentOffsetX;
        float relative_y = mouse_pos.y - contentOffsetY;
        
        // コンテンツ領域外は無視
        if (relative_x < 0.0f || relative_x >= 1880.0f || 
            relative_y < 0.0f || relative_y >= 900.0f) {
            return;
        }
        
        // リスト項目クリック判定
        for (int i = 0; i < static_cast<int>(list_panel_.entries.size()); ++i) {
            float item_y = list_panel_.y + (i - list_panel_.scroll_offset) * list_panel_.item_height;
            
            // ビュー外判定
            if (item_y < list_panel_.y || item_y >= list_panel_.y + list_panel_.height) {
                continue;
            }
            
            if (relative_x >= list_panel_.x && 
                relative_x < list_panel_.x + list_panel_.width &&
                relative_y >= item_y && 
                relative_y < item_y + list_panel_.item_height) {
                OnListItemClick(i);
                break;
            }
        }
    }
    
    // スクロール処理
    float wheel_delta = systemAPI_->GetMouseWheelMove();
    if (wheel_delta != 0.0f) {
        OnListScroll(static_cast<int>(wheel_delta));
    }
}

void CodexOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) {
        return;
    }

    // コンテンツ領域のオフセット（ヘッダーとタブバーの間）
    const float contentOffsetX = 20.0f;
    const float contentOffsetY = 90.0f;
    
    // 描画前に座標をオフセット
    float saved_list_x = list_panel_.x;
    float saved_list_y = list_panel_.y;
    float saved_viewport_x = character_viewport_.x;
    float saved_viewport_y = character_viewport_.y;
    float saved_status_x = status_panel_.x;
    float saved_status_y = status_panel_.y;
    float saved_info_x = info_panel_.x;
    float saved_info_y = info_panel_.y;
    
    list_panel_.x += contentOffsetX;
    list_panel_.y += contentOffsetY;
    character_viewport_.x += contentOffsetX;
    character_viewport_.y += contentOffsetY;
    status_panel_.x += contentOffsetX;
    status_panel_.y += contentOffsetY;
    info_panel_.x += contentOffsetX;
    info_panel_.y += contentOffsetY;

    RenderListPanel();
    RenderCharacterViewport();
    RenderStatusPanel();
    RenderInfoPanel();
    
    // 座標を元に戻す
    list_panel_.x = saved_list_x;
    list_panel_.y = saved_list_y;
    character_viewport_.x = saved_viewport_x;
    character_viewport_.y = saved_viewport_y;
    status_panel_.x = saved_status_x;
    status_panel_.y = saved_status_y;
    info_panel_.x = saved_info_x;
    info_panel_.y = saved_info_y;
}

void CodexOverlay::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    list_panel_.entries.clear();
    isInitialized_ = false;
    systemAPI_ = nullptr;
    LOG_INFO("CodexOverlay shutdown");
}

// ========== 描画ヘルパー ==========

void CodexOverlay::RenderListPanel() {
    // 背景
    systemAPI_->DrawRectangle(
        list_panel_.x, list_panel_.y,
        list_panel_.width, list_panel_.height,
        {80, 60, 50, 255}
    );
    
    // 枠線
    systemAPI_->DrawRectangleLines(
        list_panel_.x, list_panel_.y,
        list_panel_.width, list_panel_.height,
        2.0f,
        {200, 170, 100, 255}
    );
    
    // リスト項目描画
    for (int i = 0; i < static_cast<int>(list_panel_.entries.size()); ++i) {
        float item_y = list_panel_.y + (i - list_panel_.scroll_offset) * list_panel_.item_height;
        
        // ビュー外判定
        if (item_y < list_panel_.y || item_y >= list_panel_.y + list_panel_.height) {
            continue;
        }
        
        const entities::Character* entry = list_panel_.entries[i];
        if (!entry) continue;
        
        // 選択状態の背景
        if (i == list_panel_.selected_index) {
            systemAPI_->DrawRectangle(
                list_panel_.x, item_y,
                list_panel_.width, list_panel_.item_height,
                {200, 170, 100, 180}
            );
        }
        
        // テキスト: ID + 名前
        std::string label = entry->id + " " + entry->name;
        
        Color text_color = entry->is_discovered ? WHITE : GRAY;
        systemAPI_->DrawTextDefault(
            label,
            list_panel_.x + 10.0f, item_y + 10.0f,
            16.0f, text_color
        );
    }
}

void CodexOverlay::RenderCharacterViewport() {
    // 背景
    systemAPI_->DrawRectangle(
        character_viewport_.x, character_viewport_.y,
        character_viewport_.width, character_viewport_.height,
        {160, 130, 100, 255}
    );
    
    // 枠線
    systemAPI_->DrawRectangleLines(
        character_viewport_.x, character_viewport_.y,
        character_viewport_.width, character_viewport_.height,
        2.0f,
        {200, 170, 100, 255}
    );
    
    // エラー表示またはキャラクター表示
    if (character_viewport_.has_error) {
        systemAPI_->DrawTextDefault(
            "エラー: " + character_viewport_.error_message,
            character_viewport_.x + 20.0f,
            character_viewport_.y + character_viewport_.height / 2.0f - 20.0f,
            20.0f, RED
        );
    } else {
        const entities::Character* selected = GetSelectedCharacter();
        if (selected) {
            // テクスチャ読み込み試行（エラーハンドリング付き）
            try {
                if (!selected->icon_path.empty()) {
                    void* texture = systemAPI_->GetTexture(selected->icon_path);
                    if (texture) {
                        // テクスチャ描画（将来的に実装）
                        // 今はテキスト表示
                        systemAPI_->DrawTextDefault(
                            selected->name + " Sprite",
                            character_viewport_.x + character_viewport_.width / 2.0f - 100.0f,
                            character_viewport_.y + character_viewport_.height / 2.0f - 20.0f,
                            20.0f, LIGHTGRAY
                        );
                    } else {
                        character_viewport_.has_error = true;
                        character_viewport_.error_message = "テクスチャが見つかりません";
                    }
                } else {
                    systemAPI_->DrawTextDefault(
                        selected->name,
                        character_viewport_.x + character_viewport_.width / 2.0f - 50.0f,
                        character_viewport_.y + character_viewport_.height / 2.0f - 20.0f,
                        24.0f, LIGHTGRAY
                    );
                }
            } catch (const std::exception& e) {
                character_viewport_.has_error = true;
                character_viewport_.error_message = "リソース読み込みエラー";
                LOG_WARN("CodexOverlay: Resource load error: {}", e.what());
            }
        }
    }
}

void CodexOverlay::RenderStatusPanel() {
    const entities::Character* selected = GetSelectedCharacter();
    if (!selected) return;
    
    // 背景
    systemAPI_->DrawRectangle(
        status_panel_.x, status_panel_.y,
        status_panel_.width, status_panel_.height,
        {140, 110, 80, 255}
    );
    
    // 枠線
    systemAPI_->DrawRectangleLines(
        status_panel_.x, status_panel_.y,
        status_panel_.width, status_panel_.height,
        2.0f,
        {200, 170, 100, 255}
    );
    
    // ステータス表示
    float x = status_panel_.x + status_panel_.padding;
    float y = status_panel_.y + status_panel_.padding;
    
    // AttackType, EffectTypeを文字列に変換する関数
    auto attackTypeToString = [](entities::AttackType type) -> std::string {
        switch (type) {
            case entities::AttackType::Single: return "単体";
            case entities::AttackType::Range: return "範囲";
            case entities::AttackType::Line: return "直線";
            default: return "不明";
        }
    };
    
    auto effectTypeToString = [](entities::EffectType type) -> std::string {
        switch (type) {
            case entities::EffectType::Normal: return "通常";
            case entities::EffectType::Fire: return "炎";
            case entities::EffectType::Ice: return "氷";
            case entities::EffectType::Lightning: return "雷";
            case entities::EffectType::Heal: return "回復";
            default: return "不明";
        }
    };
    
    // 表示項目をシンプルに（主要なステータスのみ）
    std::vector<std::pair<std::string, std::string>> stats = {
        {"HP", std::to_string(selected->hp)},
        {"POW", std::to_string(selected->attack)},
        {"VIT", std::to_string(selected->defense)},
        {"SPEED", std::to_string(static_cast<int>(selected->move_speed))},
        {"SPAN", std::to_string(static_cast<int>(selected->attack_span * 10)) + " (x0.1s)"},
        {"COST", std::to_string(selected->cost)},
        {"STATUS", selected->is_discovered ? "Discovered" : "Hidden"}
    };
    
    for (size_t i = 0; i < stats.size(); ++i) {
        const auto& stat = stats[i];
        float line_y = y + static_cast<float>(i) * status_panel_.line_height;
        
        // ラベル
        systemAPI_->DrawTextDefault(
            stat.first,
            x, line_y,
            static_cast<float>(status_panel_.font_size), LIGHTGRAY
        );
        
        // 値（右寄せ）
        Vector2 text_size = systemAPI_->MeasureTextDefault(stat.second, static_cast<float>(status_panel_.font_size));
        systemAPI_->DrawTextDefault(
            stat.second,
            x + status_panel_.width - status_panel_.padding * 2.0f - text_size.x,
            line_y,
            static_cast<float>(status_panel_.font_size), WHITE
        );
    }
}

void CodexOverlay::RenderInfoPanel() {
    const entities::Character* selected = GetSelectedCharacter();
    if (!selected) return;
    
    // 背景
    systemAPI_->DrawRectangle(
        info_panel_.x, info_panel_.y,
        info_panel_.width, info_panel_.height,
        {140, 110, 80, 255}
    );
    
    // 枠線
    systemAPI_->DrawRectangleLines(
        info_panel_.x, info_panel_.y,
        info_panel_.width, info_panel_.height,
        2.0f,
        {200, 170, 100, 255}
    );
    
    float x = info_panel_.x + info_panel_.padding;
    float y = info_panel_.y + info_panel_.padding;
    
    // 説明文
    if (!selected->description.empty()) {
        // テキストを折り返して表示（簡易版）
        std::string desc = selected->description;
        float max_width = info_panel_.width - info_panel_.padding * 2.0f;
        float current_y = y;
        
        // 文字列を適切な長さで分割（簡易実装）
        size_t pos = 0;
        while (pos < desc.length()) {
            size_t next_pos = pos + 30;  // 簡易的に30文字ごとに分割
            if (next_pos > desc.length()) {
                next_pos = desc.length();
            }
            
            std::string line = desc.substr(pos, next_pos - pos);
            systemAPI_->DrawTextDefault(
                line,
                x, current_y,
                static_cast<float>(info_panel_.font_size), LIGHTGRAY
            );
            current_y += info_panel_.line_height;
            pos = next_pos;
        }
    } else {
        systemAPI_->DrawTextDefault(
            "説明文がありません",
            x, y,
            static_cast<float>(info_panel_.font_size), GRAY
        );
    }
}

// ========== イベント処理 ==========

void CodexOverlay::OnListItemClick(int index) {
    if (index >= 0 && index < static_cast<int>(list_panel_.entries.size())) {
        list_panel_.selected_index = index;
        character_viewport_.animation_timer = 0.0f;
        character_viewport_.animation_frame = 0;
        character_viewport_.has_error = false;
        character_viewport_.error_message.clear();
        
        const entities::Character* ch = list_panel_.entries[index];
        if (ch) {
            LOG_INFO("CodexOverlay: Selected: {} (Cost: {})", ch->name, ch->cost);
        }
    }
}

void CodexOverlay::OnListScroll(int delta) {
    int max_scroll = static_cast<int>(list_panel_.entries.size()) - 
                    static_cast<int>(list_panel_.height / list_panel_.item_height);
    
    if (max_scroll < 0) max_scroll = 0;
    
    list_panel_.scroll_offset = std::max(0, std::min(max_scroll,
                                list_panel_.scroll_offset - delta));
}

// ========== ユーティリティ ==========

const entities::Character* CodexOverlay::GetSelectedCharacter() const {
    if (list_panel_.selected_index >= 0 && 
        list_panel_.selected_index < static_cast<int>(list_panel_.entries.size())) {
        return list_panel_.entries[list_panel_.selected_index];
    }
    return nullptr;
}

int CodexOverlay::ExtractIdNumber(const std::string& id) {
    // IDから数値部分を抽出（例: "cat_001" -> 1, "dog_001" -> 1）
    std::regex pattern(R"(_(\d+)$)");
    std::smatch match;
    if (std::regex_search(id, match, pattern)) {
        try {
            return std::stoi(match[1].str());
        } catch (...) {
            return 9999;  // パース失敗時は最後に
        }
    }
    return 9999;
}

void CodexOverlay::SortCharactersById() {
    std::sort(list_panel_.entries.begin(), list_panel_.entries.end(),
        [](const entities::Character* a, const entities::Character* b) {
            if (!a || !b) return false;
            int num_a = ExtractIdNumber(a->id);
            int num_b = ExtractIdNumber(b->id);
            if (num_a != num_b) {
                return num_a < num_b;
            }
            return a->id < b->id;
        });
}

bool CodexOverlay::RequestClose() const {
    if (requestClose_) {
        requestClose_ = false;
        return true;
    }
    return false;
}

bool CodexOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}

} // namespace core
} // namespace game