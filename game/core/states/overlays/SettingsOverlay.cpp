#include "SettingsOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include <raylib.h>

namespace game {
namespace core {

SettingsOverlay::SettingsOverlay()
    : systemAPI_(nullptr)
    , isInitialized_(false)
    , requestClose_(false)
    , hasTransitionRequest_(false)
    , requestedNextState_(GameState::Title)
{
}

bool SettingsOverlay::Initialize(BaseSystemAPI* systemAPI) {
    if (isInitialized_) {
        LOG_ERROR("SettingsOverlay already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("SettingsOverlay: systemAPI is null");
        return false;
    }

    systemAPI_ = systemAPI;
    requestClose_ = false;
    hasTransitionRequest_ = false;

    // PanelとButtonは使用しない（Raylibで直接描画するため）

    isInitialized_ = true;
    LOG_INFO("SettingsOverlay initialized");
    return true;
}

void SettingsOverlay::Update(SharedContext& ctx, float deltaTime) {
    if (!isInitialized_) {
        return;
    }

    // ESCキーで閉じる
    if (systemAPI_->IsKeyPressed(256)) {
        requestClose_ = true;
    }
    
    // マウスクリックで閉じるボタンをチェック
    if (systemAPI_->IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = systemAPI_->GetMousePosition();
        
        // 閉じるボタンの位置とサイズ
        const float windowX = 200.0f;
        const float windowY = 150.0f;
        const float windowWidth = 1520.0f;
        const float windowHeight = 780.0f;
        const float buttonWidth = 150.0f;
        const float buttonHeight = 40.0f;
        float buttonX = windowX + windowWidth - buttonWidth - 40.0f;
        float buttonY = windowY + windowHeight - buttonHeight - 30.0f;
        
        // ボタン内にマウスがあるか判定
        if (mouse.x >= buttonX && mouse.x <= buttonX + buttonWidth &&
            mouse.y >= buttonY && mouse.y <= buttonY + buttonHeight) {
            requestClose_ = true;
            systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
        }
    }
}

void SettingsOverlay::Render(SharedContext& ctx) {
    if (!isInitialized_) {
        return;
    }

    // ウィンドウの位置とサイズ
    const float windowX = 200.0f;
    const float windowY = 150.0f;
    const float windowWidth = 1520.0f;
    const float windowHeight = 780.0f;
    
    // ウィンドウ背景（半透明の暗い背景）
    systemAPI_->DrawRectangle(
        static_cast<int>(windowX),
        static_cast<int>(windowY),
        static_cast<int>(windowWidth),
        static_cast<int>(windowHeight),
        Color{30, 30, 40, 240}
    );
    
    // ウィンドウ枠線
    systemAPI_->DrawRectangleLines(
        static_cast<int>(windowX),
        static_cast<int>(windowY),
        static_cast<int>(windowWidth),
        static_cast<int>(windowHeight),
        2.0f,
        Color{100, 100, 120, 255}
    );
    
    // タイトル
    const char* titleText = "設定";
    float titleFontSize = 36.0f;
    Vector2 titleSize = systemAPI_->MeasureTextDefault(titleText, titleFontSize, 1.0f);
    float titleX = windowX + (windowWidth - titleSize.x) / 2.0f;
    float titleY = windowY + 20.0f;
    systemAPI_->DrawTextDefault(titleText, titleX, titleY, titleFontSize, WHITE);
    
    // 設定項目（プレースホルダー）
    const char* settingsText = "設定項目は今後実装予定です";
    float textFontSize = 24.0f;
    Vector2 textSize = systemAPI_->MeasureTextDefault(settingsText, textFontSize, 1.0f);
    float textX = windowX + (windowWidth - textSize.x) / 2.0f;
    float textY = windowY + windowHeight / 2.0f;
    systemAPI_->DrawTextDefault(settingsText, textX, textY, textFontSize, Color{180, 180, 180, 255});
    
    // 閉じるボタン
    const float buttonWidth = 150.0f;
    const float buttonHeight = 40.0f;
    float buttonX = windowX + windowWidth - buttonWidth - 40.0f;
    float buttonY = windowY + windowHeight - buttonHeight - 30.0f;
    
    // ボタン背景
    Color buttonColor = Color{80, 100, 150, 255};
    systemAPI_->DrawRectangle(
        static_cast<int>(buttonX),
        static_cast<int>(buttonY),
        static_cast<int>(buttonWidth),
        static_cast<int>(buttonHeight),
        buttonColor
    );
    
    // ボタン枠線
    systemAPI_->DrawRectangleLines(
        static_cast<int>(buttonX),
        static_cast<int>(buttonY),
        static_cast<int>(buttonWidth),
        static_cast<int>(buttonHeight),
        2.0f,
        Color{120, 140, 180, 255}
    );
    
    // ボタンテキスト
    const char* buttonText = "閉じる";
    float buttonFontSize = 24.0f;
    Vector2 buttonTextSize = systemAPI_->MeasureTextDefault(buttonText, buttonFontSize, 1.0f);
    float buttonTextX = buttonX + (buttonWidth - buttonTextSize.x) / 2.0f;
    float buttonTextY = buttonY + (buttonHeight - buttonFontSize) / 2.0f;
    systemAPI_->DrawTextDefault(buttonText, buttonTextX, buttonTextY, buttonFontSize, WHITE);
}

void SettingsOverlay::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    isInitialized_ = false;
    systemAPI_ = nullptr;
    LOG_INFO("SettingsOverlay shutdown");
}

bool SettingsOverlay::RequestClose() const {
    if (requestClose_) {
        requestClose_ = false;
        return true;
    }
    return false;
}

bool SettingsOverlay::RequestTransition(GameState& nextState) const {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false;
        return true;
    }
    return false;
}

} // namespace core
} // namespace game
