#include "Scenes/TitleScene.h"
#include "SceneManager.h"
#include "InputManager.h"
#include <iostream>

namespace Scenes {

TitleScene::TitleScene()
    : elapsedTime_(0.0f)
    , fadeAlpha_(0.0f)
    , shouldTransition_(false) {
}

void TitleScene::Initialize(entt::registry& registry) {
    std::cout << "Title Scene Initialized" << std::endl;
    elapsedTime_ = 0.0f;
    fadeAlpha_ = 0.0f;
    shouldTransition_ = false;
}

void TitleScene::Update(entt::registry& registry, float deltaTime) {
    elapsedTime_ += deltaTime;
    
    // フェードイン（最初の2秒）
    if (elapsedTime_ < 2.0f) {
        fadeAlpha_ = elapsedTime_ / 2.0f;
        if (fadeAlpha_ > 1.0f) fadeAlpha_ = 1.0f;
    } else {
        fadeAlpha_ = 1.0f;
    }
    
    // 入力チェック（任意のキーまたはマウスボタン）
    Core::InputManager& inputManager = Core::InputManager::GetInstance();
    inputManager.Update();
    
    if (elapsedTime_ > 1.0f) {  // 1秒後から入力を受け付ける
        // キー入力チェック
        bool anyKeyPressed = false;
        for (int key = KEY_SPACE; key <= KEY_Z; key++) {
            if (inputManager.IsKeyPressed(key)) {
                anyKeyPressed = true;
                break;
            }
        }
        
        // マウスボタンチェック
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || 
            IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            anyKeyPressed = true;
        }
        
        if (anyKeyPressed || elapsedTime_ > 5.0f) {  // 5秒経過で自動遷移
            shouldTransition_ = true;
        }
    }
    
    // シーン遷移
    if (shouldTransition_) {
        Core::SceneManager::GetInstance().ChangeScene("home");
        shouldTransition_ = false;
    }
}

void TitleScene::Render(entt::registry& registry) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    // タイトルテキスト
    const char* titleText = "Simple TDC Game";
    const char* subtitleText = "Press any key to continue...";
    
    int titleFontSize = 60;
    int subtitleFontSize = 24;
    
    // タイトル位置（中央）
    Vector2 titleSize = MeasureTextEx(GetFontDefault(), titleText, titleFontSize, 2.0f);
    Vector2 titlePos = {
        (screenWidth - titleSize.x) / 2.0f,
        screenHeight / 2.0f - 50.0f
    };
    
    // サブタイトル位置
    Vector2 subtitleSize = MeasureTextEx(GetFontDefault(), subtitleText, subtitleFontSize, 1.0f);
    Vector2 subtitlePos = {
        (screenWidth - subtitleSize.x) / 2.0f,
        screenHeight / 2.0f + 50.0f
    };
    
    // フェード効果を適用
    Color titleColor = Fade(WHITE, fadeAlpha_);
    Color subtitleColor = Fade(GRAY, fadeAlpha_);
    
    // 点滅効果（サブタイトル）
    if (elapsedTime_ > 1.0f) {
        float blink = (sinf(elapsedTime_ * 3.0f) + 1.0f) / 2.0f;
        subtitleColor = Fade(GRAY, fadeAlpha_ * (0.5f + blink * 0.5f));
    }
    
    // 描画
    DrawTextEx(GetFontDefault(), titleText, titlePos, titleFontSize, 2.0f, titleColor);
    DrawTextEx(GetFontDefault(), subtitleText, subtitlePos, subtitleFontSize, 1.0f, subtitleColor);
}

void TitleScene::Shutdown(entt::registry& registry) {
    std::cout << "Title Scene Shutdown" << std::endl;
}

} // namespace Scenes

