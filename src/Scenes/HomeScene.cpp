#include "Scenes/HomeScene.h"
#include "SceneManager.h"
#include "InputManager.h"
#include <iostream>

namespace Scenes {

HomeScene::HomeScene()
    : selectedIndex_(0)
    , buttonAnimationTime_(0.0f) {
    
    // メニューオプションを�E期化
    menuOptions_.push_back({"TDゲーム本佁E, "td_game", {0, 0, 0, 0}});
    menuOptions_.push_back({"TDチE��トゲーム", "td_test", {0, 0, 0, 0}});
    menuOptions_.push_back({"Nethack風ゲーム", "nethack", {0, 0, 0, 0}});
}

void HomeScene::Initialize(entt::registry& registry) {
    std::cout << "Home Scene Initialized" << std::endl;
    selectedIndex_ = 0;
    buttonAnimationTime_ = 0.0f;
    
    // ボタンの位置とサイズを計箁E
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    float buttonWidth = 400.0f;
    float buttonHeight = 80.0f;
    float buttonSpacing = 20.0f;
    float totalHeight = (buttonHeight + buttonSpacing) * menuOptions_.size() - buttonSpacing;
    float startY = (screenHeight - totalHeight) / 2.0f;
    
    for (size_t i = 0; i < menuOptions_.size(); i++) {
        menuOptions_[i].buttonRect = {
            (screenWidth - buttonWidth) / 2.0f,
            startY + i * (buttonHeight + buttonSpacing),
            buttonWidth,
            buttonHeight
        };
    }
}

void HomeScene::Update(entt::registry& registry, float deltaTime) {
    buttonAnimationTime_ += deltaTime;
    
    Core::InputManager& inputManager = Core::InputManager::GetInstance();
    inputManager.Update();
    
    UpdateInput();
}

void HomeScene::UpdateInput() {
    Core::InputManager& inputManager = Core::InputManager::GetInstance();
    
    // キーボ�Eド操佁E
    if (inputManager.IsKeyPressed(KEY_UP) || inputManager.IsKeyPressed(KEY_W)) {
        selectedIndex_--;
        if (selectedIndex_ < 0) {
            selectedIndex_ = static_cast<int>(menuOptions_.size()) - 1;
        }
    }
    
    if (inputManager.IsKeyPressed(KEY_DOWN) || inputManager.IsKeyPressed(KEY_S)) {
        selectedIndex_++;
        if (selectedIndex_ >= static_cast<int>(menuOptions_.size())) {
            selectedIndex_ = 0;
        }
    }
    
    // Enterキーで決宁E
    if (inputManager.IsKeyPressed(KEY_ENTER) || inputManager.IsKeyPressed(KEY_SPACE)) {
        if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(menuOptions_.size())) {
            Core::SceneManager::GetInstance().ChangeScene(menuOptions_[selectedIndex_].sceneName);
        }
    }
    
    // ESCキーで終亁E
    if (inputManager.IsKeyPressed(KEY_ESCAPE)) {
        // ゲーム終亁E�EGameクラスで処琁E��れる
    }
    
    // マウス操佁E
    Vector2 mousePos = GetMousePosition();
    for (size_t i = 0; i < menuOptions_.size(); i++) {
        if (IsPointInRectangle(mousePos, menuOptions_[i].buttonRect)) {
            selectedIndex_ = static_cast<int>(i);
            
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Core::SceneManager::GetInstance().ChangeScene(menuOptions_[i].sceneName);
            }
        }
    }
}

void HomeScene::Render(entt::registry& registry) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    // 背景
    ClearBackground(Color{30, 30, 40, 255});
    
    // タイトル
    const char* titleText = "ゲーム選抁E;
    int titleFontSize = 48;
    Vector2 titleSize = MeasureTextEx(GetFontDefault(), titleText, titleFontSize, 2.0f);
    Vector2 titlePos = {
        (screenWidth - titleSize.x) / 2.0f,
        100.0f
    };
    DrawTextEx(GetFontDefault(), titleText, titlePos, titleFontSize, 2.0f, WHITE);
    
    // メニューボタンを描画
    RenderMenu();
    
    // 操作説昁E
    const char* helpText = "[↑�E]選抁E [Enter]決宁E [ESC]終亁E;
    int helpFontSize = 18;
    Vector2 helpSize = MeasureTextEx(GetFontDefault(), helpText, helpFontSize, 1.0f);
    Vector2 helpPos = {
        (screenWidth - helpSize.x) / 2.0f,
        screenHeight - 50.0f
    };
    DrawTextEx(GetFontDefault(), helpText, helpPos, helpFontSize, 1.0f, GRAY);
}

void HomeScene::RenderMenu() {
    for (size_t i = 0; i < menuOptions_.size(); i++) {
        const auto& option = menuOptions_[i];
        bool isSelected = (static_cast<int>(i) == selectedIndex_);
        
        // ボタン背景
        Color bgColor = isSelected ? Color{80, 100, 150, 255} : Color{50, 50, 70, 255};
        
        // 選択時のアニメーション効极E
        if (isSelected) {
            float pulse = (sinf(buttonAnimationTime_ * 4.0f) + 1.0f) / 2.0f;
            bgColor.r = static_cast<unsigned char>(80 + pulse * 30);
            bgColor.g = static_cast<unsigned char>(100 + pulse * 30);
            bgColor.b = static_cast<unsigned char>(150 + pulse * 30);
        }
        
        DrawRectangleRounded(option.buttonRect, 0.2f, 8, bgColor);
        DrawRectangleRoundedLines(option.buttonRect, 0.2f, 8, 2.0f, isSelected ? YELLOW : WHITE);
        
        // ボタンチE��スチE
        int fontSize = 28;
        Vector2 textSize = MeasureTextEx(GetFontDefault(), option.label.c_str(), fontSize, 1.0f);
        Vector2 textPos = {
            option.buttonRect.x + (option.buttonRect.width - textSize.x) / 2.0f,
            option.buttonRect.y + (option.buttonRect.height - textSize.y) / 2.0f
        };
        
        Color textColor = isSelected ? YELLOW : WHITE;
        DrawTextEx(GetFontDefault(), option.label.c_str(), textPos, fontSize, 1.0f, textColor);
        
        // 選択インジケーター
        if (isSelected) {
            const char* indicator = "▶";
            Vector2 indicatorPos = {
                option.buttonRect.x + 20.0f,
                option.buttonRect.y + (option.buttonRect.height - fontSize) / 2.0f
            };
            DrawTextEx(GetFontDefault(), indicator, indicatorPos, fontSize, 1.0f, YELLOW);
        }
    }
}

bool HomeScene::IsPointInRectangle(Vector2 point, Rectangle rect) {
    return point.x >= rect.x && point.x <= rect.x + rect.width &&
           point.y >= rect.y && point.y <= rect.y + rect.height;
}

void HomeScene::Shutdown(entt::registry& registry) {
    std::cout << "Home Scene Shutdown" << std::endl;
}

} // namespace Scenes

