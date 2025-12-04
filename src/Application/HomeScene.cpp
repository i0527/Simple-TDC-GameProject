/**
 * @file HomeScene.cpp
 * @brief ホームシーン実装
 * 
 * ゲームモード選択を行うホーム画面。
 * TD/Roguelikeを選択できるUIを提供。
 */

#include "Core/Platform.h"
#include "Application/HomeScene.h"
#include <iostream>

namespace Application {

HomeScene::HomeScene()
    : selectedIndex_(0)
    , initialized_(false)
{
    // メニュー項目を初期化
    AddMenuItem("Tower Defense", GameMode::TD, true);
    AddMenuItem("Roguelike RPG", GameMode::Roguelike, true);
    AddMenuItem("Exit", GameMode::Menu, true);  // Exitは特殊扱い
}

void HomeScene::Initialize(Core::World& world, Core::GameContext& context) {
    if (initialized_) {
        std::cerr << "HomeScene: Already initialized\n";
        return;
    }
    
    selectedIndex_ = 0;
    nextScene_.clear();
    
    initialized_ = true;
    std::cout << "HomeScene initialized\n";
}

void HomeScene::Update(Core::World& world, Core::GameContext& context, float deltaTime) {
    if (!initialized_) return;
    
    ProcessInput();
}

void HomeScene::Render(Core::World& world, Core::GameContext& context) {
    if (!initialized_) return;
    
    DrawUI();
}

void HomeScene::Shutdown(Core::World& world, Core::GameContext& context) {
    if (!initialized_) return;
    
    selectedIndex_ = 0;
    nextScene_.clear();
    initialized_ = false;
    
    std::cout << "HomeScene shutdown\n";
}

void HomeScene::AddMenuItem(const std::string& label, GameMode mode, bool enabled) {
    menuItems_.emplace_back(label, mode, enabled);
}

void HomeScene::ProcessInput() {
    // 上下キーで選択移動
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        selectedIndex_ = (selectedIndex_ - 1 + static_cast<int>(menuItems_.size())) % menuItems_.size();
        // 無効な項目をスキップ
        while (!menuItems_[selectedIndex_].enabled && selectedIndex_ > 0) {
            selectedIndex_ = (selectedIndex_ - 1 + static_cast<int>(menuItems_.size())) % menuItems_.size();
        }
    }
    
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(menuItems_.size());
        // 無効な項目をスキップ
        while (!menuItems_[selectedIndex_].enabled) {
            selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(menuItems_.size());
        }
    }
    
    // Enter/Spaceで決定
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        ExecuteSelection();
    }
    
    // ESCで終了（将来、確認ダイアログを表示）
    if (IsKeyPressed(KEY_ESCAPE)) {
        nextScene_ = "Exit";
    }
}

void HomeScene::ExecuteSelection() {
    if (selectedIndex_ < 0 || selectedIndex_ >= static_cast<int>(menuItems_.size())) {
        return;
    }
    
    const auto& item = menuItems_[selectedIndex_];
    
    if (!item.enabled) {
        return;
    }
    
    // Exit処理
    if (item.label == "Exit") {
        nextScene_ = "Exit";
        return;
    }
    
    // ゲームモード選択（コールバックでUnifiedGameに通知）
    // UnifiedGameがGameModeに応じて適切な処理を行う
    if (gameModeCallback_) {
        gameModeCallback_(item.gameMode);
    }
    
    // シーン遷移は行わない（GameMode切り替えのみ）
    // 将来的にTDGameScene、RoguelikeGameSceneを実装したら、ここで遷移を設定
    // 現時点では、UnifiedGameがGameModeに応じてシステムを切り替える
}

void HomeScene::DrawUI() {
    // 背景（FHD座標: 1920x1080）
    ClearBackground(Color{30, 30, 40, 255});
    
    // タイトル
    const char* titleText = "Simple TD Game";
    int titleWidth = MeasureText(titleText, TITLE_FONT_SIZE);
    DrawText(titleText, CENTER_X - titleWidth / 2, 150, TITLE_FONT_SIZE, WHITE);
    
    // サブタイトル
    const char* subtitleText = "Select Game Mode";
    int subtitleWidth = MeasureText(subtitleText, 36);
    DrawText(subtitleText, CENTER_X - subtitleWidth / 2, 250, 36, LIGHTGRAY);
    
    // メニュー項目を描画
    int startY = CENTER_Y - (static_cast<int>(menuItems_.size()) * (MENU_ITEM_HEIGHT + MENU_ITEM_SPACING)) / 2;
    
    for (size_t i = 0; i < menuItems_.size(); ++i) {
        const auto& item = menuItems_[i];
        int y = startY + static_cast<int>(i) * (MENU_ITEM_HEIGHT + MENU_ITEM_SPACING);
        
        // 選択中かどうか
        bool isSelected = (static_cast<int>(i) == selectedIndex_);
        
        // 背景色
        Color bgColor = isSelected ? Color{100, 100, 150, 255} : Color{50, 50, 60, 255};
        if (!item.enabled) {
            bgColor = Color{30, 30, 30, 255};
        }
        
        // 背景矩形
        int itemWidth = MeasureText(item.label.c_str(), MENU_FONT_SIZE) + 60;
        Rectangle bgRect = {
            static_cast<float>(CENTER_X - itemWidth / 2),
            static_cast<float>(y),
            static_cast<float>(itemWidth),
            static_cast<float>(MENU_ITEM_HEIGHT)
        };
        DrawRectangleRounded(bgRect, 0.3f, 8, bgColor);
        
        // テキスト色
        Color textColor = item.enabled ? (isSelected ? WHITE : LIGHTGRAY) : DARKGRAY;
        
        // テキスト描画
        int textWidth = MeasureText(item.label.c_str(), MENU_FONT_SIZE);
        DrawText(item.label.c_str(), 
                CENTER_X - textWidth / 2, 
                y + (MENU_ITEM_HEIGHT - MENU_FONT_SIZE) / 2,
                MENU_FONT_SIZE, 
                textColor);
        
        // 選択カーソル
        if (isSelected && item.enabled) {
            DrawText(">", CENTER_X - itemWidth / 2 - 30, y + (MENU_ITEM_HEIGHT - MENU_FONT_SIZE) / 2,
                    MENU_FONT_SIZE, YELLOW);
            DrawText("<", CENTER_X + itemWidth / 2 + 10, y + (MENU_ITEM_HEIGHT - MENU_FONT_SIZE) / 2,
                    MENU_FONT_SIZE, YELLOW);
        }
    }
    
    // ヘルプテキスト
    const char* helpText = "[Arrow Keys] Navigate  [Enter/Space] Select  [ESC] Exit";
    int helpWidth = MeasureText(helpText, 24);
    DrawText(helpText, CENTER_X - helpWidth / 2, 1080 - 80, 24, GRAY);
}

} // namespace Application

