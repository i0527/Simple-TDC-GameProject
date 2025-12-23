#pragma once

#include "Data/Graphics/IFrameProvider.h"
#include "Data/Graphics/FrameRef.h"
#include "Data/Graphics/AnimClip.h"
#include <raylib.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>

namespace Game::Graphics {

/**
 * @brief Aseprite JSON Atlas Provider
 *
 * AsepriteのJSON Array形式をサポートし、
 * Packedテクスチャ（trim/offset補正）に対応
 */
class AsepriteJsonAtlasProvider : public Shared::Data::Graphics::IFrameProvider {
public:
    /**
     * @brief Aseprite JSON Atlas Provider コンストラクタ
     * @param texture Packedテクスチャ
     * @param atlasJson Aseprite JSON Array形式のデータ
     */
    AsepriteJsonAtlasProvider(Texture2D texture, const nlohmann::json& atlasJson);
    ~AsepriteJsonAtlasProvider();

    // ===== IFrameProvider インターフェース =====

    bool HasClip(const std::string& clipName) const override;
    int GetFrameCount(const std::string& clipName) const override;
    Shared::Data::Graphics::FrameRef GetFrame(const std::string& clipName, int frameIndex) const override;
    float GetClipFps(const std::string& clipName) const override;
    bool IsLooping(const std::string& clipName) const override;

private:
    // ===== Aseprite JSON 構造体 =====

    /// Asepriteの1フレーム情報
    struct AsepriteFrame {
        std::string filename;
        Rectangle frame;           // テクスチャ内位置・サイズ {x, y, w, h}
        bool trimmed = false;
        Rectangle spriteSourceSize; // 元サイズ内位置・サイズ {x, y, w, h}
        Vector2 sourceSize;        // 元サイズ {w, h}
        int duration = 100;        // 表示時間（ミリ秒）
    };

    /// Asepriteのアニメーションタグ
    struct AsepriteFrameTag {
        std::string name;
        int from = 0;              // 開始フレームインデックス
        int to = 0;                // 終了フレームインデックス
        std::string direction = "forward"; // "forward", "reverse", "pingpong"
    };

    // ===== メンバ変数 =====

    Texture2D texture_;           // Packedテクスチャ
    Shared::Data::Graphics::SpriteSet spriteSet_;         // 生成されたクリップ集
    float footOffsetY_ = 0.0f;    // 足元オフセット（オプション）

    // フレームデータ（JSONからパース）
    std::vector<AsepriteFrame> frames_;
    std::vector<AsepriteFrameTag> frameTags_;

    // ===== ヘルパーメソッド =====

    /// JSONからフレームデータをパース
    void ParseFrames(const nlohmann::json& framesJson);

    /// JSONからフレームタグをパース
    void ParseFrameTags(const nlohmann::json& metaJson);

    /// フレームタグからAnimClipを生成
    void BuildClipsFromTags();

    /// 個別のAsepriteFrameからFrameRefを生成
    Shared::Data::Graphics::FrameRef CreateFrameRef(const AsepriteFrame& aseFrame) const;

    /// trim offsetを計算（spriteSourceSizeからの補正）
    Vector2 GetTrimOffset(const AsepriteFrame& frame) const;

    /// 足元ベースのoriginを計算
    Vector2 GetFootOrigin(const AsepriteFrame& frame) const;

    /// ミリ秒を秒に変換
    static float MillisecondsToSeconds(int ms) { return static_cast<float>(ms) / 1000.0f; }
};

} // namespace Game::Graphics
