#pragma once

#include "Data/Graphics/IFrameProvider.h"
#include "Data/Graphics/FrameRef.h"
#include <raylib.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>
#include <vector>

namespace Game::Graphics {

/**
 * @brief 開発用分割スプライトプロバイダー
 * 
 * 状態アニメーションごとに分割されたPNGファイルを管理
 * 各状態は個別のJSONファイルで設定を管理
 */
class SeparatedSpriteProvider : public Shared::Data::Graphics::IFrameProvider {
public:
    /**
     * @brief コンストラクタ
     * @param devAnimationJsonPath 開発用アニメーション設定JSONのパス
     */
    SeparatedSpriteProvider(const std::string& devAnimationJsonPath);
    
    ~SeparatedSpriteProvider();

    // ===== IFrameProvider インターフェース =====
    bool HasClip(const std::string& clipName) const override;
    int GetFrameCount(const std::string& clipName) const override;
    Shared::Data::Graphics::FrameRef GetFrame(const std::string& clipName, int frameIndex) const override;
    float GetClipFps(const std::string& clipName) const override;
    bool IsLooping(const std::string& clipName) const override;

private:
    // ===== 構造体定義 =====
    
    /// 1フレームの情報
    struct FrameData {
        Texture2D texture{};          // テクスチャ（個別PNG）
        float durationSec = 0.083f;   // 表示時間（秒）
        bool loaded = false;          // ロード済みフラグ
    };
    
    /// 1クリップ（アニメーション）の情報
    struct ClipData {
        std::string name;              // クリップ名
        std::vector<FrameData> frames; // フレーム列
        float defaultFps = 12.0f;      // デフォルトFPS
        bool loop = true;              // ループフラグ
        float footOffsetY = 0.0f;      // 足元オフセット
    };

    // ===== メンバ変数 =====
    
    std::string spriteBasePath_;                          // スプライトのベースパス
    std::unordered_map<std::string, ClipData> clips_;     // クリップ集（clipName -> ClipData）
    float defaultFootOffsetY_ = 0.0f;                     // デフォルト足元オフセット

    // ===== ヘルパーメソッド =====
    
    /// 開発用アニメーション設定JSONをロード
    void LoadDevAnimationConfig(const std::string& jsonPath);
    
    /// 個別のクリップ設定JSONをロード
    void LoadClipConfig(const std::string& clipName, const std::string& configPath);
    
    /// テクスチャをロード（遅延ロード対応）
    bool LoadFrameTexture(FrameData& frameData, const std::string& filePath);
    
    /// FrameRefを生成
    Shared::Data::Graphics::FrameRef CreateFrameRef(
        const ClipData& clip,
        int frameIndex
    ) const;
    
    /// 足元ベースのoriginを計算
    Vector2 GetFootOrigin(const Texture2D& texture, float footOffsetY) const;
};

} // namespace Game::Graphics

