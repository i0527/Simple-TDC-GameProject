/**
 * @file UIDefinitions.h
 * @brief UI要素のJSON定義構造体
 * 
 * Phase 4B: データ駆動UIシステム
 * JSONからUI配置を読み込み、動的にUI構築が可能
 */
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include "Core/Platform.h"

namespace Data {

/**
 * @brief アンカーポイント（配置基準点）
 */
enum class UIAnchor {
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    Center,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

/**
 * @brief UI要素タイプ
 */
enum class UIElementType {
    Panel,      // 背景パネル
    Text,       // テキスト
    Image,      // 画像
    Button,     // ボタン
    ProgressBar,// プログレスバー
    Slot,       // スロット（クリック可能領域）
    Container   // 子要素を持つコンテナ
};

/**
 * @brief 色定義（RGBA 0-255）
 */
struct UIColor {
    unsigned char r = 255;
    unsigned char g = 255;
    unsigned char b = 255;
    unsigned char a = 255;
    
    Color ToRaylib() const {
        return Color{r, g, b, a};
    }
    
    static UIColor FromRaylib(Color c) {
        return UIColor{c.r, c.g, c.b, c.a};
    }
};

/**
 * @brief UI要素定義
 */
struct UIElementDef {
    std::string id;                     // 要素ID（イベント識別用）
    UIElementType type = UIElementType::Panel;
    
    // 位置・サイズ（FHD座標）
    float x = 0.0f;
    float y = 0.0f;
    float width = 100.0f;
    float height = 50.0f;
    
    // アンカー
    UIAnchor anchor = UIAnchor::TopLeft;
    UIAnchor pivot = UIAnchor::TopLeft;   // 自身の基準点
    
    // 見た目
    UIColor backgroundColor{50, 50, 60, 255};
    UIColor borderColor{80, 80, 100, 255};
    float borderWidth = 0.0f;
    float cornerRadius = 0.0f;
    float opacity = 1.0f;
    
    // テキスト（type=Text, Buttonの場合）
    std::string text;
    std::string fontId;                 // フォントID（将来用）
    int fontSize = 16;
    UIColor textColor{255, 255, 255, 255};
    UIAnchor textAlign = UIAnchor::Center;
    
    // 画像（type=Imageの場合）
    std::string imageId;
    
    // プログレスバー（type=ProgressBarの場合）
    UIColor fillColor{100, 200, 100, 255};
    std::string bindValue;              // バインド先（例: "player.hp"）
    bool vertical = false;
    
    // インタラクション
    bool interactive = false;           // クリック可能か
    std::string onClick;                // クリック時イベント名
    std::string onHover;                // ホバー時イベント名
    
    // ホバー時スタイル
    std::optional<UIColor> hoverBackgroundColor;
    std::optional<UIColor> hoverBorderColor;
    
    // 状態
    bool visible = true;
    bool enabled = true;
    
    // 子要素
    std::vector<UIElementDef> children;
    
    // 繰り返し表示（スロットリストなど）
    int repeatCount = 0;                // 0=繰り返しなし
    float repeatSpacingX = 0.0f;
    float repeatSpacingY = 0.0f;
    std::string repeatBindArray;        // バインド先配列
};

/**
 * @brief UIレイアウト定義（1画面分のUI）
 */
struct UILayoutDef {
    std::string id;
    std::string name;
    
    // ベース解像度（通常FHD）
    int baseWidth = 1920;
    int baseHeight = 1080;
    
    // ルート要素
    std::vector<UIElementDef> elements;
    
    // 条件付き表示
    std::unordered_map<std::string, bool> conditions;
};

/**
 * @brief 文字列からUIAnchorに変換
 * 単純形式（left, center, right）もサポート
 */
inline UIAnchor ParseAnchor(const std::string& str) {
    // 完全形式
    if (str == "top_left" || str == "topLeft") return UIAnchor::TopLeft;
    if (str == "top_center" || str == "topCenter") return UIAnchor::TopCenter;
    if (str == "top_right" || str == "topRight") return UIAnchor::TopRight;
    if (str == "middle_left" || str == "middleLeft") return UIAnchor::MiddleLeft;
    if (str == "center") return UIAnchor::Center;
    if (str == "middle_right" || str == "middleRight") return UIAnchor::MiddleRight;
    if (str == "bottom_left" || str == "bottomLeft") return UIAnchor::BottomLeft;
    if (str == "bottom_center" || str == "bottomCenter") return UIAnchor::BottomCenter;
    if (str == "bottom_right" || str == "bottomRight") return UIAnchor::BottomRight;
    
    // 単純形式（テキスト配置用）
    if (str == "left") return UIAnchor::MiddleLeft;
    if (str == "right") return UIAnchor::MiddleRight;
    if (str == "top") return UIAnchor::TopCenter;
    if (str == "bottom") return UIAnchor::BottomCenter;
    
    return UIAnchor::TopLeft;
}

/**
 * @brief 文字列からUIElementTypeに変換
 */
inline UIElementType ParseElementType(const std::string& str) {
    if (str == "panel") return UIElementType::Panel;
    if (str == "text") return UIElementType::Text;
    if (str == "image") return UIElementType::Image;
    if (str == "button") return UIElementType::Button;
    if (str == "progressBar" || str == "progress_bar") return UIElementType::ProgressBar;
    if (str == "slot") return UIElementType::Slot;
    if (str == "container") return UIElementType::Container;
    return UIElementType::Panel;
}

/**
 * @brief アンカーから画面上の座標オフセットを計算
 */
inline void GetAnchorOffset(UIAnchor anchor, float screenWidth, float screenHeight, float& outX, float& outY) {
    switch (anchor) {
        case UIAnchor::TopLeft:     outX = 0; outY = 0; break;
        case UIAnchor::TopCenter:   outX = screenWidth / 2; outY = 0; break;
        case UIAnchor::TopRight:    outX = screenWidth; outY = 0; break;
        case UIAnchor::MiddleLeft:  outX = 0; outY = screenHeight / 2; break;
        case UIAnchor::Center:      outX = screenWidth / 2; outY = screenHeight / 2; break;
        case UIAnchor::MiddleRight: outX = screenWidth; outY = screenHeight / 2; break;
        case UIAnchor::BottomLeft:  outX = 0; outY = screenHeight; break;
        case UIAnchor::BottomCenter:outX = screenWidth / 2; outY = screenHeight; break;
        case UIAnchor::BottomRight: outX = screenWidth; outY = screenHeight; break;
    }
}

/**
 * @brief ピボットから要素内の基準点オフセットを計算
 */
inline void GetPivotOffset(UIAnchor pivot, float width, float height, float& outX, float& outY) {
    switch (pivot) {
        case UIAnchor::TopLeft:     outX = 0; outY = 0; break;
        case UIAnchor::TopCenter:   outX = width / 2; outY = 0; break;
        case UIAnchor::TopRight:    outX = width; outY = 0; break;
        case UIAnchor::MiddleLeft:  outX = 0; outY = height / 2; break;
        case UIAnchor::Center:      outX = width / 2; outY = height / 2; break;
        case UIAnchor::MiddleRight: outX = width; outY = height / 2; break;
        case UIAnchor::BottomLeft:  outX = 0; outY = height; break;
        case UIAnchor::BottomCenter:outX = width / 2; outY = height; break;
        case UIAnchor::BottomRight: outX = width; outY = height; break;
    }
}

} // namespace Data
