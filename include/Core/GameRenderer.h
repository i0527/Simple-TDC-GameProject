/**
 * @file GameRenderer.h
 * @brief FHD固定レンダリングシステム
 * 
 * Phase 4A: 内部解像度をFHD(1920x1080)に固定し、
 * RenderTextureに描画後スケーリングして表示。
 * エディターとゲームで座標系を統一するための基盤。
 */
#pragma once

#include "Core/Platform.h"
#include <cmath>

namespace Core {

/**
 * @brief FHD固定レンダリングクラス
 * 
 * 使用例:
 * @code
 * GameRenderer renderer;
 * renderer.Initialize(windowWidth, windowHeight);
 * 
 * // ゲームループ内
 * renderer.BeginRender();
 * // FHD座標で描画（1920x1080空間）
 * DrawRectangle(100, 100, 200, 50, RED);
 * renderer.EndRender();
 * 
 * // マウス座標変換
 * Vector2 worldPos = renderer.ScreenToWorld(GetMousePosition());
 * @endcode
 */
class GameRenderer {
public:
    // 内部レンダリング解像度（FHD固定）
    static constexpr int RENDER_WIDTH = 1920;
    static constexpr int RENDER_HEIGHT = 1080;
    
    GameRenderer() = default;
    ~GameRenderer() {
        if (isInitialized_) {
            Shutdown();
        }
    }
    
    // コピー禁止
    GameRenderer(const GameRenderer&) = delete;
    GameRenderer& operator=(const GameRenderer&) = delete;
    
    /**
     * @brief 初期化
     * @param windowWidth ウィンドウ幅
     * @param windowHeight ウィンドウ高さ
     */
    void Initialize(int windowWidth, int windowHeight) {
        windowWidth_ = windowWidth;
        windowHeight_ = windowHeight;
        
        // RenderTexture作成（FHD解像度）
        renderTarget_ = LoadRenderTexture(RENDER_WIDTH, RENDER_HEIGHT);
        
        // スケール計算
        UpdateScale();
        
        isInitialized_ = true;
    }
    
    /**
     * @brief シャットダウン
     */
    void Shutdown() {
        if (isInitialized_) {
            UnloadRenderTexture(renderTarget_);
            isInitialized_ = false;
        }
    }
    
    /**
     * @brief ウィンドウサイズ変更時の更新
     */
    void OnWindowResize(int newWidth, int newHeight) {
        windowWidth_ = newWidth;
        windowHeight_ = newHeight;
        UpdateScale();
    }
    
    /**
     * @brief レンダリング開始
     * 
     * この後の描画はすべてFHD座標系（1920x1080）で行う
     */
    void BeginRender() {
        BeginTextureMode(renderTarget_);
        ClearBackground(backgroundColor_);
    }
    
    /**
     * @brief レンダリング終了
     * 
     * RenderTextureをスケーリングして画面に表示
     */
    void EndRender() {
        EndTextureMode();
        
        // 画面に描画
        BeginDrawing();
        ClearBackground(letterboxColor_);
        
        // RenderTextureを反転して描画（RenderTextureはY反転されるため）
        Rectangle srcRect = {
            0.0f, 
            0.0f, 
            static_cast<float>(RENDER_WIDTH), 
            -static_cast<float>(RENDER_HEIGHT)  // Y反転
        };
        
        Rectangle destRect = {
            offsetX_,
            offsetY_,
            static_cast<float>(RENDER_WIDTH) * scale_,
            static_cast<float>(RENDER_HEIGHT) * scale_
        };
        
        DrawTexturePro(
            renderTarget_.texture,
            srcRect,
            destRect,
            Vector2{0, 0},
            0.0f,
            WHITE
        );
        
        EndDrawing();
    }
    
    /**
     * @brief スクリーン座標をFHDワールド座標に変換
     * @param screenPos スクリーン座標（ウィンドウ座標）
     * @return FHD空間での座標
     */
    Vector2 ScreenToWorld(Vector2 screenPos) const {
        Vector2 world;
        world.x = (screenPos.x - offsetX_) / scale_;
        world.y = (screenPos.y - offsetY_) / scale_;
        return world;
    }
    
    /**
     * @brief FHDワールド座標をスクリーン座標に変換
     * @param worldPos FHD空間での座標
     * @return スクリーン座標（ウィンドウ座標）
     */
    Vector2 WorldToScreen(Vector2 worldPos) const {
        Vector2 screen;
        screen.x = worldPos.x * scale_ + offsetX_;
        screen.y = worldPos.y * scale_ + offsetY_;
        return screen;
    }
    
    /**
     * @brief マウスのFHD座標を取得
     */
    Vector2 GetMouseWorldPosition() const {
        return ScreenToWorld(GetMousePosition());
    }
    
    /**
     * @brief FHD座標がレンダリング領域内か判定
     */
    bool IsWorldPositionValid(Vector2 worldPos) const {
        return worldPos.x >= 0 && worldPos.x < RENDER_WIDTH &&
               worldPos.y >= 0 && worldPos.y < RENDER_HEIGHT;
    }
    
    /**
     * @brief 背景色を設定
     */
    void SetBackgroundColor(Color color) { backgroundColor_ = color; }
    
    /**
     * @brief レターボックス色を設定
     */
    void SetLetterboxColor(Color color) { letterboxColor_ = color; }
    
    // アクセサ
    int GetRenderWidth() const { return RENDER_WIDTH; }
    int GetRenderHeight() const { return RENDER_HEIGHT; }
    int GetWindowWidth() const { return windowWidth_; }
    int GetWindowHeight() const { return windowHeight_; }
    float GetScale() const { return scale_; }
    float GetOffsetX() const { return offsetX_; }
    float GetOffsetY() const { return offsetY_; }
    bool IsInitialized() const { return isInitialized_; }
    
    /**
     * @brief RenderTextureを直接取得（高度な用途向け）
     */
    const RenderTexture2D& GetRenderTarget() const { return renderTarget_; }

private:
    void UpdateScale() {
        // アスペクト比を維持してスケール計算
        float scaleX = static_cast<float>(windowWidth_) / RENDER_WIDTH;
        float scaleY = static_cast<float>(windowHeight_) / RENDER_HEIGHT;
        
        // 小さい方のスケールを採用（レターボックス）
        scale_ = std::min(scaleX, scaleY);
        
        // センタリングオフセット計算
        float scaledWidth = RENDER_WIDTH * scale_;
        float scaledHeight = RENDER_HEIGHT * scale_;
        
        offsetX_ = (windowWidth_ - scaledWidth) / 2.0f;
        offsetY_ = (windowHeight_ - scaledHeight) / 2.0f;
    }
    
    RenderTexture2D renderTarget_{};
    
    int windowWidth_ = 1280;
    int windowHeight_ = 720;
    
    float scale_ = 1.0f;
    float offsetX_ = 0.0f;
    float offsetY_ = 0.0f;
    
    Color backgroundColor_ = RAYWHITE;
    Color letterboxColor_ = BLACK;
    
    bool isInitialized_ = false;
};

/**
 * @brief FHD座標定数
 * 
 * よく使う座標やサイズを定義
 */
namespace FHD {
    // 基本解像度（GameRendererの定数をエクスポート）
    constexpr int RENDER_WIDTH = GameRenderer::RENDER_WIDTH;
    constexpr int RENDER_HEIGHT = GameRenderer::RENDER_HEIGHT;
    constexpr int WIDTH = RENDER_WIDTH;
    constexpr int HEIGHT = RENDER_HEIGHT;
    
    constexpr float CENTER_X = WIDTH / 2.0f;
    constexpr float CENTER_Y = HEIGHT / 2.0f;
    
    // バトルフィールド領域
    constexpr float BATTLEFIELD_LEFT = 90.0f;
    constexpr float BATTLEFIELD_RIGHT = WIDTH - 90.0f;
    constexpr float BATTLEFIELD_WIDTH = BATTLEFIELD_RIGHT - BATTLEFIELD_LEFT;
    
    // 拠点位置
    constexpr float BASE_LEFT_X = 20.0f;      // 敵拠点（左）
    constexpr float BASE_RIGHT_X = WIDTH - 80.0f;  // プレイヤー拠点（右）
    constexpr float BASE_WIDTH = 60.0f;
    constexpr float BASE_HEIGHT = 220.0f;
    
    // UI領域定義
    namespace UI {
        // 上部バー
        constexpr float TOP_BAR_X = 80.0f;
        constexpr float TOP_BAR_Y = 5.0f;
        constexpr float TOP_BAR_WIDTH = 800.0f;
        constexpr float TOP_BAR_HEIGHT = 70.0f;
        
        // デッキスロット（下部）
        constexpr float DECK_PANEL_Y = HEIGHT - 100.0f;
        constexpr float DECK_SLOT_Y = HEIGHT - 90.0f;
        constexpr float DECK_SLOT_WIDTH = 120.0f;
        constexpr float DECK_SLOT_HEIGHT = 80.0f;
        constexpr float DECK_SLOT_SPACING = 130.0f;
        
        // 互換性のため
        constexpr float DECK_Y = DECK_SLOT_Y;
        
        // 拠点（UIでの参照用）
        constexpr float PLAYER_BASE_X = BASE_RIGHT_X;
        constexpr float ENEMY_BASE_X = BASE_LEFT_X;
        
        // レーンエリア
        constexpr float LANE_LEFT = BATTLEFIELD_LEFT;
        constexpr float LANE_RIGHT = BATTLEFIELD_RIGHT;
        constexpr float LANE_WIDTH = BATTLEFIELD_WIDTH;
    }
}

} // namespace Core
