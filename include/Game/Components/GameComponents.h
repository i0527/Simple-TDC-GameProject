/**
 * @file GameComponents.h
 * @brief Game層コンポーネント
 * 
 * 汎用的なゲーム機能用コンポーネント。
 * スプライト、アニメーション、入力など。
 */
#pragma once

#include "Core/Platform.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace Game::Components {

// ===== スプライト関連 =====

/**
 * @brief スプライトテクスチャ参照
 */
struct Sprite {
    std::string textureName;     // ResourceManagerのキー
    Rectangle sourceRect = {};   // ソース矩形（スプライトシート内）
    Color tint = WHITE;          // 色調
    bool flipX = false;          // X反転
    bool flipY = false;          // Y反転
};

/**
 * @brief スプライトシート情報
 */
struct SpriteSheet {
    std::string textureName;
    int frameWidth = 64;
    int frameHeight = 64;
    int framesPerRow = 8;
    int totalFrames = 8;
};

/**
 * @brief アニメーション状態
 */
struct Animation {
    std::string currentAnimation;  // 現在のアニメーション名
    int currentFrameIndex = 0;     // 現在のフレームインデックス
    float elapsedTime = 0.0f;      // 経過時間
    bool isPlaying = true;
    bool isLooping = true;
    float speedMultiplier = 1.0f;  // 再生速度倍率
};

/**
 * @brief アニメーション定義キャッシュ
 * CharacterDefからコピーして使用
 */
struct AnimationData {
    struct Frame {
        int index;
        float duration;
        std::string tag;
    };
    
    struct AnimInfo {
        std::vector<Frame> frames;
        bool loop;
        std::string nextAnimation;
    };
    
    std::unordered_map<std::string, AnimInfo> animations;
    std::string defaultAnimation;
};

// ===== フォールバック描画 =====

/**
 * @brief フォールバック描画設定
 * テクスチャがない場合に自動生成されるプレースホルダー描画用
 */
struct FallbackVisual {
    enum class Shape {
        Circle,
        Rectangle,
        Diamond,
        Triangle
    };
    
    Shape shape = Shape::Circle;
    Color primaryColor = WHITE;
    Color secondaryColor = GRAY;      // アウトラインや模様用
    float size = 32.0f;               // 基本サイズ
    bool showAnimationIndicator = true; // アニメーションフレーム表示
};

/**
 * @brief 生成済みプレースホルダーテクスチャ
 * 動的に生成されたテスト用テクスチャを保持
 */
struct GeneratedTexture {
    Texture2D texture = {};
    bool isValid = false;
    int frameWidth = 64;
    int frameHeight = 64;
    int totalFrames = 4;
};

// ===== 入力関連 =====

/**
 * @brief プレイヤー操作対象（矢印キー）
 */
struct PlayerControlled {};

/**
 * @brief プレイヤー操作対象（WASD）
 */
struct WASDControlled {};

// ===== UI関連 =====

/**
 * @brief クリック可能
 */
struct Clickable {
    Rectangle bounds;
    bool isHovered = false;
    bool isPressed = false;
    bool isEnabled = true;
};

/**
 * @brief ドラッグ可能
 */
struct Draggable {
    bool isDragging = false;
    float dragOffsetX = 0.0f;
    float dragOffsetY = 0.0f;
};

// ===== レンダリング順序 =====

/**
 * @brief 描画順序（ZOrder）
 */
struct RenderOrder {
    int layer = 0;      // レイヤー（背景=0, キャラ=10, UI=100）
    int orderInLayer = 0;  // レイヤー内順序
};

} // namespace Game::Components
