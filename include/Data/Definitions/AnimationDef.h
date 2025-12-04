/**
 * @file AnimationDef.h
 * @brief アニメーション定義構造体
 * 
 * キャラクターアニメーションを定義する構造体
 */
#pragma once

#include <string>
#include <vector>

namespace Data {

/**
 * @brief アニメーションフレーム定義
 */
struct FrameDef {
    int index = 0;           // スプライトシート内のフレーム番号
    float duration = 0.1f;   // フレーム表示時間（秒）
    std::string tag;         // オプショナルタグ（"attack_hit" 等）
};

/**
 * @brief アニメーション定義
 */
struct AnimationDef {
    std::string id;             // 一意のID
    std::string name;
    std::vector<FrameDef> frames;
    bool loop = true;
    std::string nextAnimation;  // ループしない場合の次アニメーション
};

} // namespace Data

