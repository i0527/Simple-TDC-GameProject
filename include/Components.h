#pragma once

#include <raylib.h>

namespace Components {
    // 位置コンポーネント - シンプルなデータ構造
    struct Position {
        float x;
        float y;
    };

    // 速度コンポーネント - シンプルなデータ構造
    struct Velocity {
        float dx;
        float dy;
    };

    // レンダリング可能コンポーネント - シンプルなデータ構造
    struct Renderable {
        Color color;
        float radius;
    };

    // プレイヤータグコンポーネント
    struct Player {};
}
