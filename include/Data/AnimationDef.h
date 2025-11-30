/**
 * @file AnimationDef.h
 * @brief スプライトアニメーション定義構造体
 * 
 * Phase 4C: データ駆動アニメーションシステム
 * JSONからアニメーション定義を読み込み可能
 */
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace Data {

/**
 * @brief アニメーションのループモード
 */
enum class AnimLoopMode {
    Once,           // 1回再生して停止
    Loop,           // 無限ループ
    PingPong,       // 往復ループ（1→2→3→2→1→...）
    LoopCount       // 指定回数ループ
};

/**
 * @brief フレームイベントタイプ
 */
enum class FrameEventType {
    Sound,          // 効果音再生
    Particle,       // パーティクル発生
    Callback,       // コールバック呼び出し
    DamageWindow,   // ダメージ判定開始/終了
    SpawnProjectile // 投射物生成
};

/**
 * @brief フレームイベント定義
 * 特定フレームで発生するイベント
 */
struct FrameEventDef {
    int frame = 0;                      // 発生フレーム
    FrameEventType type = FrameEventType::Callback;
    std::string eventName;              // イベント識別子
    std::string soundId;                // 効果音ID（Sound時）
    std::string particleId;             // パーティクルID（Particle時）
    float offsetX = 0.0f;               // X オフセット
    float offsetY = 0.0f;               // Y オフセット
    std::unordered_map<std::string, float> params;  // 追加パラメータ
};

/**
 * @brief 単一フレーム定義
 */
struct SpriteFrameDef {
    int spriteIndex = 0;                // スプライトシート内インデックス
    float duration = 0.1f;              // このフレームの表示時間（秒）
    
    // フレーム別オフセット（オプション）
    std::optional<float> offsetX;
    std::optional<float> offsetY;
    
    // フレーム別スケール（オプション）
    std::optional<float> scaleX;
    std::optional<float> scaleY;
    
    // フレーム別回転（オプション）
    std::optional<float> rotation;
    
    // フレーム別透明度（オプション）
    std::optional<float> alpha;
};

/**
 * @brief アニメーションクリップ定義
 * 1つのアニメーション（idle, walk, attack等）
 */
struct AnimClipDef {
    std::string id;                     // クリップID（例: "idle", "walk"）
    std::string name;                   // 表示名
    
    // フレームデータ
    std::vector<SpriteFrameDef> frames;
    
    // ループ設定
    AnimLoopMode loopMode = AnimLoopMode::Loop;
    int loopCount = 1;                  // LoopCountモード時のループ回数
    
    // 全体設定
    float speed = 1.0f;                 // 再生速度倍率
    float defaultDuration = 0.1f;       // フレームのデフォルト表示時間
    
    // イベント
    std::vector<FrameEventDef> events;
    
    // 遷移設定
    std::string nextClip;               // 終了後に自動遷移するクリップID
    bool canInterrupt = true;           // 途中で他のアニメに割り込み可能か
    
    /**
     * @brief アニメーションの総再生時間を計算
     */
    float GetTotalDuration() const {
        float total = 0.0f;
        for (const auto& frame : frames) {
            total += frame.duration;
        }
        return total / speed;
    }
    
    /**
     * @brief フレーム数を取得
     */
    size_t GetFrameCount() const {
        return frames.size();
    }
};

/**
 * @brief スプライトシート定義
 */
struct SpriteSheetDef {
    std::string textureId;              // テクスチャID
    int frameWidth = 64;                // 1フレームの幅
    int frameHeight = 64;               // 1フレームの高さ
    int columns = 1;                    // 列数
    int rows = 1;                       // 行数
    int totalFrames = 1;                // 総フレーム数
    int paddingX = 0;                   // フレーム間X余白
    int paddingY = 0;                   // フレーム間Y余白
    int offsetX = 0;                    // シート開始Xオフセット
    int offsetY = 0;                    // シート開始Yオフセット
};

/**
 * @brief スプライトアニメーション定義（キャラクター等1体分）
 * 複数のアニメーションクリップを持つ
 */
struct SpriteAnimationDef {
    std::string id;                     // アニメーション定義ID
    std::string name;                   // 表示名
    
    // スプライトシート
    SpriteSheetDef spriteSheet;
    
    // アニメーションクリップ一覧
    std::unordered_map<std::string, AnimClipDef> clips;
    
    // デフォルト設定
    std::string defaultClip = "idle";   // デフォルトのクリップID
    float pivotX = 0.5f;                // ピボットX（0-1、0.5=中心）
    float pivotY = 1.0f;                // ピボットY（1.0=下端）
    
    // 共通オフセット
    float globalOffsetX = 0.0f;
    float globalOffsetY = 0.0f;
    
    /**
     * @brief クリップを取得
     */
    const AnimClipDef* GetClip(const std::string& clipId) const {
        auto it = clips.find(clipId);
        return it != clips.end() ? &it->second : nullptr;
    }
    
    /**
     * @brief クリップが存在するか
     */
    bool HasClip(const std::string& clipId) const {
        return clips.count(clipId) > 0;
    }
};

/**
 * @brief 文字列からAnimLoopModeに変換
 */
inline AnimLoopMode ParseLoopMode(const std::string& str) {
    if (str == "once") return AnimLoopMode::Once;
    if (str == "loop") return AnimLoopMode::Loop;
    if (str == "pingpong" || str == "ping_pong") return AnimLoopMode::PingPong;
    if (str == "loop_count" || str == "loopCount") return AnimLoopMode::LoopCount;
    return AnimLoopMode::Loop;
}

/**
 * @brief 文字列からFrameEventTypeに変換
 */
inline FrameEventType ParseFrameEventType(const std::string& str) {
    if (str == "sound") return FrameEventType::Sound;
    if (str == "particle") return FrameEventType::Particle;
    if (str == "callback") return FrameEventType::Callback;
    if (str == "damage_window" || str == "damageWindow") return FrameEventType::DamageWindow;
    if (str == "spawn_projectile" || str == "spawnProjectile") return FrameEventType::SpawnProjectile;
    return FrameEventType::Callback;
}

} // namespace Data
