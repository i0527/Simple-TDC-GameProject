#pragma once

#include <raylib.h>
#include <string>
#include <vector>

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

    // スケーリングコンポーネント - 描画時のスケール倍率
    struct Scale {
        float x;  // X方向スケール
        float y;  // Y方向スケール
    };

    // レンダリング可能コンポーネント - シンプルなデータ構造
    struct Renderable {
        Color color;
        float radius;
    };

    // プレイヤータグコンポーネント（矢印キー操作）
    struct Player {};

    // WASD プレイヤータグコンポーネント（WASD操作）
    struct WASDPlayer {};

    // テクスチャアトラステクスチャアトラスフレーム参照用コンポーネント
    // フレーム名とソース矩形を保持するだけのPOD
    struct SpriteFrame {
        std::string frameName; // ImageManagerに登録されたフレーム名
        Rectangle sourceRect;  // 元画像上の矩形
    };

    // スプライトアニメーション情報コンポーネント
    // アニメーション再生に必要なデータを保持（ロジックはシステムで実装）
    struct SpriteAnimation {
        std::string spriteName;        // スプライトシート名（e.g., "cupslime"）
        std::vector<std::string> frames; // フレーム名リスト（順序保証）
        size_t currentFrameIndex;      // 現在のフレームインデックス
        float elapsedTime;             // フレーム経過時間（秒）
        bool isPlaying;                // 再生中フラグ
        bool isLooping;                // ループフラグ
    };

    // テクスチャ参照コンポーネント
    // 描画時に使用するテクスチャを保持
    struct SpriteTexture {
        std::string textureName;  // テクスチャマネージャーのキー名
    };
}
