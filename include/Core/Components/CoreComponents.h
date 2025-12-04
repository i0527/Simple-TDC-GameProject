/**
 * @file CoreComponents.h
 * @brief Core層コンポーネント
 * 
 * 最も基本的なECSコンポーネント。
 * どのゲームジャンルでも使用可能な汎用コンポーネント。
 */
#pragma once

#include <string>
#include "Core/Platform.h"

namespace Core::Components {

/**
 * @brief 位置コンポーネント
 */
struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

/**
 * @brief 速度コンポーネント
 */
struct Velocity {
    float dx = 0.0f;
    float dy = 0.0f;
};

/**
 * @brief スケールコンポーネント
 */
struct Scale {
    float x = 1.0f;
    float y = 1.0f;
};

/**
 * @brief 回転コンポーネント
 */
struct Rotation {
    float angle = 0.0f;  // 度
};

/**
 * @brief 基本描画コンポーネント（デバッグ/プレースホルダー用）
 */
struct Renderable {
    Color color = WHITE;
    float radius = 10.0f;
};

/**
 * @brief タグ: 破棄予約
 */
struct MarkedForDestruction {};

/**
 * @brief エンティティの識別情報
 */
struct Identity {
    std::string id;        // 一意のID
    std::string type;      // エンティティタイプ ("unit", "projectile", etc.)
    std::string name;      // 表示名
};

/**
 * @brief ライフタイム（時間経過で消滅）
 */
struct Lifetime {
    float remaining = 1.0f;  // 残り時間（秒）
};

/**
 * @brief 親子関係
 */
struct Parent {
    entt::entity parent = entt::null;
};

struct Children {
    std::vector<entt::entity> children;
};

} // namespace Core::Components
