/**
 * @file CommonTypes.h
 * @brief 共通データ型定義
 * 
 * 全ての定義で共通して使用される基本型を定義
 */
#pragma once

namespace Data {

/**
 * @brief 2D矩形（ヒットボックス、エフェクトエリア等）
 */
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

/**
 * @brief 2Dベクトル
 */
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

/**
 * @brief 2Dサイズ
 */
struct Size {
    float width = 0.0f;
    float height = 0.0f;
};

} // namespace Data

