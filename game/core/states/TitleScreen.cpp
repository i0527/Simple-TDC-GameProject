#include "TitleScreen.hpp"
#include "../../utils/Log.h"
#include "../api/SceneOverlayControlAPI.hpp"
#include "../api/InputSystemAPI.hpp"
#include "../ui/OverlayColors.hpp"
#include "../config/RenderPrimitives.hpp"
#include "../ui/UiAssetKeys.hpp"

namespace game {
namespace core {

TitleScreen::TitleScreen()
    : systemAPI_(nullptr)
    , inputAPI_(nullptr)
    , sharedContext_(nullptr)
    , sceneOverlayAPI_(nullptr)
    , isInitialized_(false)
    , hasTransitionRequest_(false)
    , requestedNextState_(GameState::Title)
    , requestQuit_(false)
    , background_texture_(nullptr)
    , has_background_(false)
{
    title_text_ = "tower of defense\n(´・ω・｀)";
    version_text_ = "v1.0";
}

TitleScreen::~TitleScreen() {
    if (isInitialized_) {
        LOG_WARN("TitleScreen not properly shutdown");
    }
}

bool TitleScreen::Initialize(BaseSystemAPI* systemAPI) {
    LOG_INFO("TitleScreen::Initialize() called");
    
    if (isInitialized_) {
        LOG_ERROR("TitleScreen already initialized");
        return false;
    }

    if (!systemAPI) {
        LOG_ERROR("TitleScreen: systemAPI is null");
        return false;
    }

    if (!sharedContext_) {
        LOG_ERROR("TitleScreen: SharedContext is null");
        return false;
    }

    LOG_INFO("TitleScreen: Setting member variables...");
    systemAPI_ = systemAPI;
    if (!sceneOverlayAPI_) {
        LOG_ERROR("TitleScreen: SceneOverlayControlAPI is null");
        return false;
    }

    LOG_INFO("TitleScreen: Setting flags...");
    isInitialized_ = true;
    hasTransitionRequest_ = false;
    requestQuit_ = false;

    // ========== UI要素初期匁E==========
    LOG_INFO("TitleScreen: Initializing UI elements...");
    
    // 画面中央X座樁E
    const float centerX = 1920.0f / 2.0f;  // 960.0f
    
    // ゲーム開始�Eタン�E�中央下に配置、FHD座標！E
    start_button_.width = 450.0f;
    start_button_.height = 90.0f;
    start_button_.x = centerX - start_button_.width / 2.0f;  // 中央揁E��
    start_button_.y = 780.0f;
    start_button_.label = "ゲーム開始";
    
    // メニュー頁E���E�フチE��ーメニュー、統一された�Eタンサイズで配置�E�E
    const float menuButtonWidth = 280.0f;   // 統一された�Eタン幁E
    const float menuButtonHeight = 60.0f;  // 統一された�Eタン高さ
    const float menuSpacing = 320.0f;      // メニュー頁E��間�E間隔�E��Eタン中忁E��の距離�E�E
    
    // ライセンスボタン�E�中央から左�E�E
    menu_items_[LICENSE].width = menuButtonWidth;
    menu_items_[LICENSE].height = menuButtonHeight;
    menu_items_[LICENSE].x = (centerX - menuSpacing) - menuButtonWidth / 2.0f;  // ボタン中忁E��ら左端を計箁E
    menu_items_[LICENSE].y = 975.0f;
    menu_items_[LICENSE].label = "ライセンス";
    
    // 設定�Eタン�E�中央�E�E
    menu_items_[SETTINGS].width = menuButtonWidth;
    menu_items_[SETTINGS].height = menuButtonHeight;
    menu_items_[SETTINGS].x = centerX - menuButtonWidth / 2.0f;  // 中央揁E��
    menu_items_[SETTINGS].y = 975.0f;
    menu_items_[SETTINGS].label = "設定";
    
    // ゲーム終亁E�Eタン�E�中央から右�E�E
    menu_items_[QUIT].width = menuButtonWidth;
    menu_items_[QUIT].height = menuButtonHeight;
    menu_items_[QUIT].x = (centerX + menuSpacing) - menuButtonWidth / 2.0f;  // ボタン中忁E��ら左端を計箁E
    menu_items_[QUIT].y = 975.0f;
    menu_items_[QUIT].label = "ゲーム終了";
    
    LOG_INFO("TitleScreen: UI elements initialized, loading background...");
    
    // 背景読み込み試衁E
    if (!LoadBackgroundImage()) {
        LOG_WARN("Background image not found. Using fallback gradient.");
        has_background_ = false;
    }
    
    LOG_INFO("TitleScreen initialized successfully");
    return true;
}

void TitleScreen::Update(float deltaTime) {
    if (!isInitialized_) {
        LOG_ERROR("TitleScreen not initialized");
        return;
    }

    // オーバ�Eレイ�E�設宁Eライセンス等）が開いてぁE��間�E、背面のTitle画面が�E力を拾わなぁE��ぁE��する
    // これにより「設定選択中にゲーム開始などを押せる」問題を防ぁE
    if (sceneOverlayAPI_ && sceneOverlayAPI_->HasActiveOverlay()) {
        start_button_.is_hovered = false;
        for (int i = 0; i < 3; ++i) {
            menu_items_[i].is_hovered = false;
        }
        return;
    }

    // ========== ホバー状態更新 ==========
    UpdateHoverStates();
    
    // ========== マウスクリチE��処琁E==========
    if (inputAPI_ && inputAPI_->IsLeftClickPressed()) {
        auto mouse = inputAPI_->GetMousePosition();
        
        // ゲーム開始�Eタン判宁E
        if (start_button_.IsPointInside(mouse.x, mouse.y)) {
            OnStartButtonClick();
            inputAPI_->ConsumeLeftClick();
            return;
        }
        
        // メニュー頁E��判定（統一された�Eタンサイズで判定！E
        for (int i = 0; i < 3; ++i) {
            if (menu_items_[i].IsPointInside(mouse.x, mouse.y)) {
                OnMenuItemClick(i);
                inputAPI_->ConsumeLeftClick();
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
    
    // タイトルチE��スチE
    RenderTitle();
    
    // ゲーム開始�Eタン
    RenderStartButton();
    
    // フッターメニュー
    RenderFooterMenu();
    
    // バ�Eジョン惁E��
    RenderVersionInfo();
}

void TitleScreen::Shutdown() {
    if (!isInitialized_) {
        LOG_WARN("TitleScreen not initialized");
        return;
    }

    background_texture_ = nullptr;
    has_background_ = false;

    isInitialized_ = false;
    systemAPI_ = nullptr;
    sharedContext_ = nullptr;
    sceneOverlayAPI_ = nullptr;
    hasTransitionRequest_ = false;
    LOG_INFO("TitleScreen shutdown");
}

bool TitleScreen::RequestTransition(GameState& nextState) {
    if (hasTransitionRequest_) {
        nextState = requestedNextState_;
        hasTransitionRequest_ = false; // リクエストをリセチE��
        return true;
    }
    return false;
}

bool TitleScreen::RequestQuit() {
    if (requestQuit_) {
        requestQuit_ = false; // リクエストをリセチE��
        return true;
    }
    return false;
}

// ========== 描画ヘルパ�E ==========

void TitleScreen::RenderBackground() {
    if (has_background_ && background_texture_ && background_texture_->id != 0) {
        // 画像背景描画
        Rect source = {0.0f, 0.0f,
                       static_cast<float>(background_texture_->width),
                       static_cast<float>(background_texture_->height)};
        Rect dest = {0.0f, 0.0f, 1920.0f, 1080.0f};
        Vec2 origin = {0.0f, 0.0f};
        systemAPI_->Render().DrawTexturePro(
            *background_texture_,
            source,
            dest,
            origin,
            0.0f,
            ToCoreColor(WHITE)
        );
    } else {
        // グラチE�Eション背景
        DrawGradientBackground();
    }
}

void TitleScreen::DrawGradientBackground() {
    // GitHubダークチE�EマグラチE�Eション背景�E�垂直グラチE�Eション�E�E
    ColorRGBA top_color = ToCoreColor(ui::OverlayColors::PANEL_BG_GITHUB);      // 控えめ紺黁E
    ColorRGBA bottom_color = ToCoreColor(ui::OverlayColors::MAIN_BG);           // 完璧な黁E
    
    systemAPI_->Render().DrawRectangleGradientV(
        0, 0,
        1920, 1080,
        top_color,
        bottom_color
    );
}

void TitleScreen::RenderTitle() {
    // タイトルチE��スト描画�E�EHD座樁E 960, 270�E�E
    Vec2 titleSize = systemAPI_->Render().MeasureTextDefaultCore(
        title_text_.c_str(), TITLE_FONT_SIZE, 1.0f);
    float titleX = 960.0f - titleSize.x / 2.0f;
    float titleY = 270.0f;
    
    systemAPI_->Render().DrawTextDefault(
        title_text_.c_str(),
        titleX, titleY,
        TITLE_FONT_SIZE,
        ToCoreColor(ui::OverlayColors::TEXT_MAIN_GITHUB)
    );
}

void TitleScreen::RenderStartButton() {
    const char* textureKey = start_button_.is_hovered
        ? ui::UiAssetKeys::ButtonPrimaryHover
        : ui::UiAssetKeys::ButtonPrimaryNormal;
    Rect dest = {start_button_.x, start_button_.y, start_button_.width, start_button_.height};
    systemAPI_->Render().DrawUiNineSlice(textureKey, dest, 8, 8, 8, 8, ToCoreColor(WHITE));
    
    // ボタンチE��スチE
    Vec2 textSize = systemAPI_->Render().MeasureTextDefaultCore(
        start_button_.label.c_str(), BUTTON_FONT_SIZE, 1.0f);
    float textX = start_button_.x + (start_button_.width - textSize.x) / 2.0f;
    float textY = start_button_.y + (start_button_.height - BUTTON_FONT_SIZE) / 2.0f;
    
    systemAPI_->Render().DrawTextDefault(
        start_button_.label.c_str(),
        textX, textY,
        BUTTON_FONT_SIZE,
        ToCoreColor(systemAPI_->Render().GetReadableTextColor(textureKey))
    );
}

void TitleScreen::RenderFooterMenu() {
    for (int i = 0; i < 3; ++i) {
        const char* textureKey = menu_items_[i].is_hovered
            ? ui::UiAssetKeys::ButtonSecondaryHover
            : ui::UiAssetKeys::ButtonSecondaryNormal;
        Rect dest = {menu_items_[i].x, menu_items_[i].y, menu_items_[i].width, menu_items_[i].height};
        systemAPI_->Render().DrawUiNineSlice(textureKey, dest, 8, 8, 8, 8, ToCoreColor(WHITE));
        
        // ボタンチE��スト（中央揁E���E�E
        ColorRGBA text_color =
            ToCoreColor(systemAPI_->Render().GetReadableTextColor(textureKey));
        
        Vec2 textSize = systemAPI_->Render().MeasureTextDefaultCore(
            menu_items_[i].label.c_str(), MENU_FONT_SIZE, 1.0f);
        float textX = menu_items_[i].x + (menu_items_[i].width - textSize.x) / 2.0f;
        float textY = menu_items_[i].y + (menu_items_[i].height - MENU_FONT_SIZE) / 2.0f;
        
        systemAPI_->Render().DrawTextDefault(
            menu_items_[i].label.c_str(),
            textX, textY,
            MENU_FONT_SIZE,
            text_color
        );
    }
}

void TitleScreen::RenderVersionInfo() {
    // バ�Eジョン惁E���E�EHD座樁E 1830, 15�E�E
    Vec2 versionSize = systemAPI_->Render().MeasureTextDefaultCore(
        version_text_.c_str(), VERSION_FONT_SIZE, 1.0f);
    float versionX = 1920.0f - versionSize.x - 10.0f;
    float versionY = 15.0f;
    
    systemAPI_->Render().DrawTextDefault(
        version_text_.c_str(),
        versionX, versionY,
        VERSION_FONT_SIZE,
        ColorRGBA{180, 180, 180, 255}
    );
}

// ========== イベント�E琁E==========

void TitleScreen::OnStartButtonClick() {
    LOG_INFO("Start button clicked");
    
    hasTransitionRequest_ = true;
    requestedNextState_ = GameState::Home;
}

void TitleScreen::OnMenuItemClick(int item_index) {
    switch (item_index) {
        case LICENSE:
            LOG_INFO("License button clicked");
            if (sceneOverlayAPI_) {
                sceneOverlayAPI_->PushOverlay(OverlayState::License);
            }
            break;
        case SETTINGS:
            LOG_INFO("Settings button clicked");
            if (sceneOverlayAPI_) {
                sceneOverlayAPI_->PushOverlay(OverlayState::Settings);
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
    auto mouse = inputAPI_ ? inputAPI_->GetMousePosition()
                           : Vec2{0.0f, 0.0f};
    
    // ボタンホバー
    start_button_.is_hovered = start_button_.IsPointInside(mouse.x, mouse.y);
    
    // メニュー頁E��ホバー�E�統一された�Eタンサイズで判定！E
    for (int i = 0; i < 3; ++i) {
        menu_items_[i].is_hovered = menu_items_[i].IsPointInside(mouse.x, mouse.y);
    }
}

// ========== ユーチE��リチE�� ==========

bool TitleScreen::LoadBackgroundImage() {
    const char* bg_path = "assets/images/title_bg.png";

    if (!systemAPI_) {
        LOG_ERROR("TitleScreen: systemAPI is null");
        background_texture_ = nullptr;
        has_background_ = false;
        return false;
    }

    if (systemAPI_->Resource().TextureExists(bg_path)) {
        background_texture_ = systemAPI_->Resource().GetTexturePtr(bg_path);
        if (background_texture_ && background_texture_->id != 0) {
            has_background_ = true;
            LOG_INFO("Background image loaded: {}", bg_path);
            return true;
        }
        LOG_WARN("Background image invalid: {}", bg_path);
    } else {
        LOG_WARN("Background image not found: {}", bg_path);
    }

    background_texture_ = nullptr;
    has_background_ = false;
    return false;
}

} // namespace core
} // namespace game
