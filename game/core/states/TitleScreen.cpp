#include "TitleScreen.hpp"
#include "../../utils/Log.h"
#include "../system/OverlayManager.hpp"
#include <raylib.h>

namespace game {
namespace core {

TitleScreen::TitleScreen()
    : systemAPI_(nullptr)
    , sharedContext_(nullptr)
    , overlayManager_(nullptr)
    , isInitialized_(false)
    , hasTransitionRequest_(false)
    , requestedNextState_(GameState::Title)
    , requestQuit_(false)
    , has_background_(false)
{
    title_text_ = "Cat Tower Defense";
    version_text_ = "v1.0";
}

TitleScreen::~TitleScreen() {
    if (isInitialized_) {
        LOG_WARN("TitleScreen not properly shutdown");
    }
}

bool TitleScreen::Initialize(BaseSystemAPI* systemAPI, SharedContext* ctx) {
    if (isInitialized_) {
        LOG_ERROR("TitleScreen already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("TitleScreen: systemAPI is null");
        return false;
    }

    if (!ctx) {
        LOG_ERROR("TitleScreen: SharedContext is null");
        return false;
    }

    systemAPI_ = systemAPI;
    sharedContext_ = ctx;
    overlayManager_ = ctx->overlayManager;
    
    if (!overlayManager_) {
        LOG_ERROR("TitleScreen: OverlayManager is null");
        return false;
    }

    isInitialized_ = true;
    hasTransitionRequest_ = false;
    requestQuit_ = false;

    // ========== UI要素初期化 ==========
    
    // 画面中央X座標
    const float centerX = 1920.0f / 2.0f;  // 960.0f
    
    // ゲーム開始ボタン（中央下に配置、FHD座標）
    start_button_.width = 450.0f;
    start_button_.height = 90.0f;
    start_button_.x = centerX - start_button_.width / 2.0f;  // 中央揃え
    start_button_.y = 780.0f;
    start_button_.label = "ゲーム開始";
    
    // メニュー項目（フッターメニュー、統一されたボタンサイズで配置）
    const float menuButtonWidth = 280.0f;   // 統一されたボタン幅
    const float menuButtonHeight = 60.0f;  // 統一されたボタン高さ
    const float menuSpacing = 320.0f;      // メニュー項目間の間隔（ボタン中心間の距離）
    
    // ライセンスボタン（中央から左）
    menu_items_[LICENSE].width = menuButtonWidth;
    menu_items_[LICENSE].height = menuButtonHeight;
    menu_items_[LICENSE].x = (centerX - menuSpacing) - menuButtonWidth / 2.0f;  // ボタン中心から左端を計算
    menu_items_[LICENSE].y = 975.0f;
    menu_items_[LICENSE].label = "ライセンス";
    
    // 設定ボタン（中央）
    menu_items_[SETTINGS].width = menuButtonWidth;
    menu_items_[SETTINGS].height = menuButtonHeight;
    menu_items_[SETTINGS].x = centerX - menuButtonWidth / 2.0f;  // 中央揃え
    menu_items_[SETTINGS].y = 975.0f;
    menu_items_[SETTINGS].label = "設定";
    
    // ゲーム終了ボタン（中央から右）
    menu_items_[QUIT].width = menuButtonWidth;
    menu_items_[QUIT].height = menuButtonHeight;
    menu_items_[QUIT].x = (centerX + menuSpacing) - menuButtonWidth / 2.0f;  // ボタン中心から左端を計算
    menu_items_[QUIT].y = 975.0f;
    menu_items_[QUIT].label = "ゲーム終了";
    
    // 背景読み込み試行
    if (!LoadBackgroundImage()) {
        LOG_WARN("Background image not found. Using fallback gradient.");
        has_background_ = false;
    }
    
    LOG_INFO("TitleScreen initialized");
    return true;
}

void TitleScreen::Update(float deltaTime) {
    if (!isInitialized_) {
        LOG_ERROR("TitleScreen not initialized");
        return;
    }

    // ========== ホバー状態更新 ==========
    UpdateHoverStates();
    
    // ========== マウスクリック処理 ==========
    if (systemAPI_->IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = systemAPI_->GetMousePosition();
        
        // ゲーム開始ボタン判定
        if (start_button_.IsPointInside(mouse.x, mouse.y)) {
            OnStartButtonClick();
            systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
            return;
        }
        
        // メニュー項目判定（統一されたボタンサイズで判定）
        for (int i = 0; i < 3; ++i) {
            if (menu_items_[i].IsPointInside(mouse.x, mouse.y)) {
                OnMenuItemClick(i);
                systemAPI_->ConsumeMouseButton(MOUSE_LEFT_BUTTON);
                return;
            }
        }
    }
}

void TitleScreen::Render() {
    if (!isInitialized_) {
        return;
    }

    // 背景描画
    RenderBackground();
    
    // タイトルテキスト
    RenderTitle();
    
    // ゲーム開始ボタン
    RenderStartButton();
    
    // フッターメニュー
    RenderFooterMenu();
    
    // バージョン情報
    RenderVersionInfo();
}

void TitleScreen::Shutdown() {
    if (!isInitialized_) {
        LOG_WARN("TitleScreen not initialized");
        return;
    }

    if (has_background_) {
        UnloadTexture(background_image_);
        has_background_ = false;
    }

    isInitialized_ = false;
    systemAPI_ = nullptr;
    sharedContext_ = nullptr;
    overlayManager_ = nullptr;
    hasTransitionRequest_ = false;
    LOG_INFO("TitleScreen shutdown");
}

bool TitleScreen::RequestTransition(GameState& nextState) {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false; // リクエストをリセット
        return true;
    }
    return false;
}

bool TitleScreen::RequestQuit() {
    if (requestQuit_) {
        requestQuit_ = false; // リクエストをリセット
        return true;
    }
    return false;
}

// ========== 描画ヘルパー ==========

void TitleScreen::RenderBackground() {
    if (has_background_) {
        // 画像背景描画
        Rectangle source = {0.0f, 0.0f, static_cast<float>(background_image_.width), 
                           static_cast<float>(background_image_.height)};
        Rectangle dest = {0.0f, 0.0f, 1920.0f, 1080.0f};
        Vector2 origin = {0.0f, 0.0f};
        systemAPI_->DrawTexturePro(
            background_image_,
            source,
            dest,
            origin,
            0.0f,
            WHITE
        );
    } else {
        // グラデーション背景
        DrawGradientBackground();
    }
}

void TitleScreen::DrawGradientBackground() {
    // グラデーション背景（垂直グラデーション）
    Color top_color = {10, 20, 40, 255};      // 暗青
    Color bottom_color = {20, 40, 80, 255};   // 薄青
    
    systemAPI_->DrawRectangleGradientV(
        0, 0,
        1920, 1080,
        top_color,
        bottom_color
    );
}

void TitleScreen::RenderTitle() {
    // タイトルテキスト描画（FHD座標: 960, 270）
    Vector2 titleSize = systemAPI_->MeasureTextDefault(title_text_.c_str(), TITLE_FONT_SIZE, 1.0f);
    float titleX = 960.0f - titleSize.x / 2.0f;
    float titleY = 270.0f;
    
    systemAPI_->DrawTextDefault(
        title_text_.c_str(),
        titleX, titleY,
        TITLE_FONT_SIZE,
        WHITE
    );
}

void TitleScreen::RenderStartButton() {
    // ボタン背景
    Color button_color = start_button_.is_hovered ? 
        Color{255, 180, 60, 255} :    // ホバー時: 明るい
        Color{240, 160, 40, 255};     // 通常: オレンジ
    
    systemAPI_->DrawRectangle(
        static_cast<int>(start_button_.x),
        static_cast<int>(start_button_.y),
        static_cast<int>(start_button_.width),
        static_cast<int>(start_button_.height),
        button_color
    );
    
    // ボタン枠線
    systemAPI_->DrawRectangleLines(
        static_cast<int>(start_button_.x),
        static_cast<int>(start_button_.y),
        static_cast<int>(start_button_.width),
        static_cast<int>(start_button_.height),
        2.0f,
        Color{200, 170, 100, 255}  // ゴールド枠
    );
    
    // ボタンテキスト
    Vector2 textSize = systemAPI_->MeasureTextDefault(start_button_.label.c_str(), BUTTON_FONT_SIZE, 1.0f);
    float textX = start_button_.x + (start_button_.width - textSize.x) / 2.0f;
    float textY = start_button_.y + (start_button_.height - BUTTON_FONT_SIZE) / 2.0f;
    
    systemAPI_->DrawTextDefault(
        start_button_.label.c_str(),
        textX, textY,
        BUTTON_FONT_SIZE,
        WHITE
    );
}

void TitleScreen::RenderFooterMenu() {
    for (int i = 0; i < 3; ++i) {
        // ボタン背景（ゲーム開始ボタンより控えめな色）
        Color button_color = menu_items_[i].is_hovered ? 
            Color{100, 120, 180, 200} :    // ホバー時: 明るい青（半透明）
            Color{60, 80, 120, 150};      // 通常: 暗い青（半透明）
        
        systemAPI_->DrawRectangle(
            static_cast<int>(menu_items_[i].x),
            static_cast<int>(menu_items_[i].y),
            static_cast<int>(menu_items_[i].width),
            static_cast<int>(menu_items_[i].height),
            button_color
        );
        
        // ボタン枠線
        Color border_color = menu_items_[i].is_hovered ? 
            Color{150, 170, 220, 255} :  // ホバー時: 明るい枠
            Color{100, 120, 160, 255};   // 通常: 暗い枠
        systemAPI_->DrawRectangleLines(
            static_cast<int>(menu_items_[i].x),
            static_cast<int>(menu_items_[i].y),
            static_cast<int>(menu_items_[i].width),
            static_cast<int>(menu_items_[i].height),
            2.0f,
            border_color
        );
        
        // ボタンテキスト（中央揃え）
        Color text_color = menu_items_[i].is_hovered ? 
            Color{255, 255, 255, 255} :  // ホバー時: 白
            Color{200, 200, 220, 255};   // 通常: 薄い白
        
        Vector2 textSize = systemAPI_->MeasureTextDefault(menu_items_[i].label.c_str(), MENU_FONT_SIZE, 1.0f);
        float textX = menu_items_[i].x + (menu_items_[i].width - textSize.x) / 2.0f;
        float textY = menu_items_[i].y + (menu_items_[i].height - MENU_FONT_SIZE) / 2.0f;
        
        systemAPI_->DrawTextDefault(
            menu_items_[i].label.c_str(),
            textX, textY,
            MENU_FONT_SIZE,
            text_color
        );
    }
}

void TitleScreen::RenderVersionInfo() {
    // バージョン情報（FHD座標: 1830, 15）
    Vector2 versionSize = systemAPI_->MeasureTextDefault(version_text_.c_str(), VERSION_FONT_SIZE, 1.0f);
    float versionX = 1920.0f - versionSize.x - 10.0f;
    float versionY = 15.0f;
    
    systemAPI_->DrawTextDefault(
        version_text_.c_str(),
        versionX, versionY,
        VERSION_FONT_SIZE,
        Color{180, 180, 180, 255}
    );
}

// ========== イベント処理 ==========

void TitleScreen::OnStartButtonClick() {
    LOG_INFO("Start button clicked");
    
    hasTransitionRequest_ = true;
    requestedNextState_ = GameState::Home;
}

void TitleScreen::OnMenuItemClick(int item_index) {
    switch (item_index) {
        case LICENSE:
            LOG_INFO("License button clicked");
            if (overlayManager_) {
                overlayManager_->PushOverlay(OverlayState::License, systemAPI_);
            }
            break;
        case SETTINGS:
            LOG_INFO("Settings button clicked");
            if (overlayManager_) {
                overlayManager_->PushOverlay(OverlayState::Settings, systemAPI_);
            }
            break;
        case QUIT:
            LOG_INFO("Quit button clicked");
            requestQuit_ = true;
            break;
        default:
            break;
    }
}

void TitleScreen::UpdateHoverStates() {
    Vector2 mouse = systemAPI_->GetMousePosition();
    
    // ボタンホバー
    start_button_.is_hovered = start_button_.IsPointInside(mouse.x, mouse.y);
    
    // メニュー項目ホバー（統一されたボタンサイズで判定）
    for (int i = 0; i < 3; ++i) {
        menu_items_[i].is_hovered = menu_items_[i].IsPointInside(mouse.x, mouse.y);
    }
}

// ========== ユーティリティ ==========

bool TitleScreen::LoadBackgroundImage() {
    const char* bg_path = "assets/images/title_bg.png";
    
    // ファイル存在確認（RaylibのFileExistsを使用）
    if (FileExists(bg_path)) {
        background_image_ = LoadTexture(bg_path);
        has_background_ = true;
        LOG_INFO("Background image loaded: {}", bg_path);
        return true;
    }
    
    LOG_WARN("Background image not found: {}", bg_path);
    return false;
}

} // namespace core
} // namespace game
