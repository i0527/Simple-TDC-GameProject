#ifndef GRAPHICS_ANIM_CLIP_H
#define GRAPHICS_ANIM_CLIP_H

#include "FrameRef.h"
#include <string>
#include <vector>
#include <unordered_map>

/**
 * @brief アニメーション1本分の定義
 * 
 * 複数のフレーム参照と再生設定をまとめたもの
 */
struct AnimClip {
    /// クリップ名（"idle", "walk", "attack" など）
    std::string name;
    
    /// フレーム参照のリスト
    std::vector<FrameRef> frames;
    
    /// ループ再生するか
    bool loop = true;
    
    /// デフォルト再生速度（FPS）
    float defaultFps = 12.0f;
    
    // ===== ヘルパー =====
    
    /// フレーム数を取得
    size_t GetFrameCount() const { return frames.size(); }
    
    /// 指定インデックスが有効範囲か
    bool IsValidIndex(size_t index) const { return index < frames.size(); }
    
    /// 全フレームの合計再生時間（秒）
    float GetTotalDuration() const {
        float total = 0.0f;
        for (const auto& frame : frames) {
            total += frame.durationSec;
        }
        return total;
    }
};

/**
 * @brief キャラクター1体分の全アニメーション
 * 
 * GridSheetProvider/AsepriteJsonAtlasProvider 等から
 * 実行時に生成される
 */
struct SpriteSet {
    /// アニメーションクリップ集（名前→定義）
    std::unordered_map<std::string, AnimClip> clips;
    
    /// デバッグ用識別名（"Warrior", "Slime" など）
    std::string debugName;
    
    // ===== ヘルパー =====
    
    /// クリップが存在するか
    bool HasClip(const std::string& name) const {
        return clips.find(name) != clips.end();
    }
    
    /// クリップを取得（存在しない場合は nullptr）
    const AnimClip* GetClip(const std::string& name) const {
        auto it = clips.find(name);
        return (it != clips.end()) ? &it->second : nullptr;
    }
};

#endif // GRAPHICS_ANIM_CLIP_H
