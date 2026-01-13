#include "TabBarManager.hpp"
#include "../../../api/BaseSystemAPI.hpp"
#include "../../../../utils/Log.h"
#include "../../../ui/OverlayColors.hpp"
#include <raylib.h>

namespace game {
namespace core {
namespace states {
namespace overlays {
namespace home {

TabBarManager::TabBarManager()
    : current_tab_(HomeTab::StageSelect)
    , hovered_tab_index_(-1)
{
}

TabBarManager::~TabBarManager() {
}

bool TabBarManager::Initialize() {
    LayoutTabs();
    return true;
}

void TabBarManager::LayoutTabs() {
    // 論理座標: 1920x1080 (FHD)
    // タブバー: y=990, height=90
    // 7つのタブを等幅配置
    
    const float TAB_Y = 990.0f;
    const float TAB_HEIGHT = 90.0f;
    const float TAB_WIDTH = 1920.0f / static_cast<float>(static_cast<int>(HomeTab::COUNT));
    
    tabs_.clear();
    
    // タブの順番: ステージ、編成、ユニット、強化、ガチャ、図鑑、設定
    std::vector<std::pair<HomeTab, std::string>> tab_configs = {
        {HomeTab::StageSelect, "ステージ"},
        {HomeTab::Formation, "編成"},
        {HomeTab::Unit, "ユニット(new)"},
        {HomeTab::Enhancement, "強化"},
        {HomeTab::Gacha, "ガチャ"},
        {HomeTab::Codex, "図鑑"},
        {HomeTab::Settings, "設定"}
    };
    
    for (int i = 0; i < static_cast<int>(tab_configs.size()); ++i) {
        TabButton btn;
        btn.tab_id = tab_configs[i].first;
        btn.label = tab_configs[i].second;
        btn.x = i * TAB_WIDTH;
        btn.y = TAB_Y;
        btn.width = TAB_WIDTH;
        btn.height = TAB_HEIGHT;
        btn.is_selected = (i == 0);  // 初期: StageSelect
        
        tabs_.push_back(btn);
    }
    
    current_tab_ = HomeTab::StageSelect;
}

void TabBarManager::Update(float deltaTime) {
    // タブボタン状態更新（ホバー等）
    // マウス位置を取得してホバー状態を更新
}

void TabBarManager::Render(BaseSystemAPI* systemAPI) {
    if (!systemAPI) {
        return;
    }
    
    // タブバー背景
    systemAPI->DrawRectangle(0, 990, 1920, 90, ui::OverlayColors::PANEL_BG);
    
    // 各タブボタンを描画
    for (auto& tab : tabs_) {
        tab.is_selected = (tab.tab_id == current_tab_);
        
        // タブボタンの背景色
        Color bgColor;
        if (tab.is_selected) {
            bgColor = ui::OverlayColors::CARD_BG_SELECTED;  // 選択時: 明るい背景
        } else if (hovered_tab_index_ >= 0 && 
                   static_cast<int>(tab.tab_id) == hovered_tab_index_) {
            bgColor = ui::OverlayColors::CARD_BG_NORMAL;  // ホバー時: 通常背景
        } else {
            bgColor = ui::OverlayColors::SLOT_ORANGE_EMPTY;  // 通常時: ダーク背景
        }
        
        // タブボタン背景
        systemAPI->DrawRectangle(tab.x, tab.y, tab.width, tab.height, bgColor);
        
        // タブボタンの枠線
        Color borderColor = tab.is_selected ? ui::OverlayColors::BORDER_GOLD : ui::OverlayColors::BORDER_DEFAULT;
        float borderWidth = tab.is_selected ? 3.0f : 2.0f;  // 選択時は太めの枠線
        systemAPI->DrawRectangleLines(tab.x, tab.y, tab.width, tab.height, borderWidth, borderColor);
        
        // 選択中のタブの上部にアクセントラインを追加
        if (tab.is_selected) {
            systemAPI->DrawLine(tab.x, tab.y, tab.x + tab.width, tab.y, 3.0f, ui::OverlayColors::ACCENT_GOLD);
        }
        
        // テキスト描画（中央揃え）
        float textX = tab.x + tab.width / 2.0f;
        float textY = tab.y + tab.height / 2.0f;
        
        Color textColor = tab.is_selected ? ui::OverlayColors::TEXT_PRIMARY : ui::OverlayColors::TEXT_SECONDARY;
        
        // テキストサイズ計算
        float fontSize = 28.0f;
        Vector2 textSize = systemAPI->MeasureTextDefault(tab.label, fontSize, 1.0f);
        
        // 中央揃えで描画
        systemAPI->DrawTextDefault(
            tab.label,
            textX - textSize.x / 2.0f,
            textY - textSize.y / 2.0f,
            fontSize,
            textColor
        );
    }
}

bool TabBarManager::OnMouseClick(float x, float y) {
    for (int i = 0; i < static_cast<int>(tabs_.size()); ++i) {
        const auto& tab = tabs_[i];
        if (x >= tab.x && x < tab.x + tab.width &&
            y >= tab.y && y < tab.y + tab.height) {
            SelectTab(tab.tab_id);
            return true;
        }
    }
    return false;
}

bool TabBarManager::OnMouseHover(float x, float y) {
    int new_hovered = -1;
    for (int i = 0; i < static_cast<int>(tabs_.size()); ++i) {
        const auto& tab = tabs_[i];
        if (x >= tab.x && x < tab.x + tab.width &&
            y >= tab.y && y < tab.y + tab.height) {
            new_hovered = i;
            break;
        }
    }
    
    bool changed = (new_hovered != hovered_tab_index_);
    hovered_tab_index_ = new_hovered;
    return changed;
}

void TabBarManager::SelectTab(HomeTab tab) {
    if (current_tab_ == tab) return;
    
    current_tab_ = tab;
    if (on_tab_changed_) {
        on_tab_changed_(tab);
    }
}

const TabButton& TabBarManager::GetTabButton(HomeTab tab) const {
    for (const auto& btn : tabs_) {
        if (btn.tab_id == tab) {
            return btn;
        }
    }
    // Should not reach here
    static TabButton dummy;
    return dummy;
}

} // namespace home
} // namespace overlays
} // namespace states
} // namespace core
} // namespace game
