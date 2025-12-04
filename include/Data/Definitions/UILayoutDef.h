/**
 * @file UILayoutDef.h
 * @brief UIレイアウト定義構造体
 * 
 * データ駆動UIシステム用のUIレイアウト定義
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

} // namespace Data

