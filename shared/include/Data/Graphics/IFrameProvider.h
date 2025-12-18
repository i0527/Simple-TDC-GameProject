#ifndef GRAPHICS_IFRAMPROVIDER_H
#define GRAPHICS_IFRAMPROVIDER_H

#include "FrameRef.h"
#include <string>
#include <memory>

/**
 * @brief スプライトシート形式を抽象化するインターフェース
 * 
 * GridSheetProvider, AsepriteJsonAtlasProvider など
 * あらゆるスプライトシート形式がこのインターフェースを実装
 * 
 * これにより:
 * - RenderingSystem は形式に依存しない
 * - Provider 切り替え時にゲームロジックは変わらない
 * - 段階的な最適化（Grid → Packed → Unified Atlas）が容易
 */
class IFrameProvider {
public:
    virtual ~IFrameProvider() = default;
    
    /**
     * @brief 指定名称のクリップが存在するか
     * @param clipName クリップ名（"idle", "walk" など）
     * @return 存在する場合 true
     */
    virtual bool HasClip(const std::string& clipName) const = 0;
    
    /**
     * @brief 指定クリップのフレーム数を取得
     * @param clipName クリップ名
     * @return フレーム数（存在しない場合は 0）
     */
    virtual int GetFrameCount(const std::string& clipName) const = 0;
    
    /**
     * @brief 指定クリップの指定フレームを取得
     * 
     * DrawTexturePro で即座に描画可能な状態で返す
     * 
     * @param clipName クリップ名
     * @param frameIndex フレーム0ベースのインデックス
     * @return FrameRef（有効・無効は valid フィールドで判定）
     */
    virtual FrameRef GetFrame(const std::string& clipName, int frameIndex) const = 0;
    
    /**
     * @brief 指定クリップのデフォルト FPS を取得
     * @param clipName クリップ名
     * @return FPS値（存在しない場合は 12.0f 等のデフォルト）
     */
    virtual float GetClipFps(const std::string& clipName) const = 0;
    
    /**
     * @brief 指定クリップがループ再生するか
     * @param clipName クリップ名
     * @return ループ設定
     */
    virtual bool IsLooping(const std::string& clipName) const = 0;
};

// ===== スマートポインタ型定義 =====
using FrameProviderPtr = std::unique_ptr<IFrameProvider>;
using FrameProviderSharedPtr = std::shared_ptr<IFrameProvider>;

#endif // GRAPHICS_IFRAMPROVIDER_H
