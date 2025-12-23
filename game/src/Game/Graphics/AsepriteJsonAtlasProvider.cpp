#include "Game/Graphics/AsepriteJsonAtlasProvider.h"
#include <iostream>
#include <algorithm>

namespace Game::Graphics {

AsepriteJsonAtlasProvider::AsepriteJsonAtlasProvider(Texture2D texture, const nlohmann::json& atlasJson)
    : texture_(texture) {

    try {
        // frames配列をパース
        if (atlasJson.contains("frames")) {
            ParseFrames(atlasJson["frames"]);
        }

        // meta.frameTagsをパース
        if (atlasJson.contains("meta") && atlasJson["meta"].contains("frameTags")) {
            ParseFrameTags(atlasJson["meta"]);
        }

        // フレームタグからAnimClipを生成
        BuildClipsFromTags();

        // デバッグ情報
        std::cout << "[AsepriteJsonAtlasProvider] Loaded " << frames_.size() << " frames, "
                  << frameTags_.size() << " tags, " << spriteSet_.clips.size() << " clips" << std::endl;

    } catch (const nlohmann::json::exception& e) {
        std::cerr << "[AsepriteJsonAtlasProvider] JSON parse error: " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "[AsepriteJsonAtlasProvider] Error: " << e.what() << std::endl;
        throw;
    }
}

AsepriteJsonAtlasProvider::~AsepriteJsonAtlasProvider() {
    if (texture_.id != 0) {
        UnloadTexture(texture_);
    }
}

void AsepriteJsonAtlasProvider::ParseFrames(const nlohmann::json& framesJson) {
    // Aseprite JSON Array形式に対応
    if (framesJson.is_array()) {
        for (const auto& frameJson : framesJson) {
            AsepriteFrame frame;

            // filename
            if (frameJson.contains("filename")) {
                frame.filename = frameJson["filename"];
            }

            // frame: {x, y, w, h}
            if (frameJson.contains("frame")) {
                const auto& f = frameJson["frame"];
                frame.frame = {
                    static_cast<float>(f["x"]),
                    static_cast<float>(f["y"]),
                    static_cast<float>(f["w"]),
                    static_cast<float>(f["h"])
                };
            }

            // trimmed
            if (frameJson.contains("trimmed")) {
                frame.trimmed = frameJson["trimmed"];
            }

            // spriteSourceSize: {x, y, w, h}
            if (frameJson.contains("spriteSourceSize")) {
                const auto& sss = frameJson["spriteSourceSize"];
                frame.spriteSourceSize = {
                    static_cast<float>(sss["x"]),
                    static_cast<float>(sss["y"]),
                    static_cast<float>(sss["w"]),
                    static_cast<float>(sss["h"])
                };
            }

            // sourceSize: {w, h}
            if (frameJson.contains("sourceSize")) {
                const auto& ss = frameJson["sourceSize"];
                frame.sourceSize = {
                    static_cast<float>(ss["w"]),
                    static_cast<float>(ss["h"])
                };
            }

            // duration (ミリ秒)
            if (frameJson.contains("duration")) {
                frame.duration = frameJson["duration"];
            }

            frames_.push_back(frame);
        }
    } else {
        // 古い形式（オブジェクト）に対応する場合の処理
        // 今回はArray形式のみサポート
        throw std::runtime_error("Unsupported Aseprite JSON format: expected frames array");
    }
}

void AsepriteJsonAtlasProvider::ParseFrameTags(const nlohmann::json& metaJson) {
    if (!metaJson.contains("frameTags")) return;

    for (const auto& tagJson : metaJson["frameTags"]) {
        AsepriteFrameTag tag;

        if (tagJson.contains("name")) {
            tag.name = tagJson["name"];
        }

        if (tagJson.contains("from")) {
            tag.from = tagJson["from"];
        }

        if (tagJson.contains("to")) {
            tag.to = tagJson["to"];
        }

        if (tagJson.contains("direction")) {
            tag.direction = tagJson["direction"];
        }

        frameTags_.push_back(tag);
    }
}

void AsepriteJsonAtlasProvider::BuildClipsFromTags() {
    for (const auto& tag : frameTags_) {
        Shared::Data::Graphics::AnimClip clip;
        clip.name = tag.name;
        clip.loop = true;  // Asepriteタグは通常ループ
        clip.defaultFps = 12.0f;  // デフォルト値、後で調整可能

        // from-to の範囲からフレームを収集
        for (int i = tag.from; i <= tag.to; ++i) {
            if (i >= 0 && static_cast<size_t>(i) < frames_.size()) {
                Shared::Data::Graphics::FrameRef frameRef = CreateFrameRef(frames_[i]);
                if (frameRef.valid) {
                    clip.frames.push_back(frameRef);
                }
            }
        }

        // 有効なフレームがある場合のみ追加
        if (!clip.frames.empty()) {
            spriteSet_.clips[tag.name] = clip;
        }
    }
}

Shared::Data::Graphics::FrameRef AsepriteJsonAtlasProvider::CreateFrameRef(const AsepriteFrame& aseFrame) const {
    Shared::Data::Graphics::FrameRef ref;
    ref.texture = &texture_;
    ref.src = aseFrame.frame;
    ref.valid = true;

    // duration を秒に変換
    ref.durationSec = MillisecondsToSeconds(aseFrame.duration);

    // trim offset 計算
    ref.offset = GetTrimOffset(aseFrame);

    // 足元基準の origin 計算
    ref.origin = GetFootOrigin(aseFrame);

    return ref;
}

Vector2 AsepriteJsonAtlasProvider::GetTrimOffset(const AsepriteFrame& frame) const {
    if (!frame.trimmed) {
        return {0, 0};  // trimされていない場合は補正不要
    }

    // Asepriteのtrim: 余白を除去して左上を(0,0)に詰める
    // spriteSourceSize.x/y が元画像内でのオフセット
    // 描画時にこのオフセットを加算して位置補正
    return {
        frame.spriteSourceSize.x,
        frame.spriteSourceSize.y
    };
}

Vector2 AsepriteJsonAtlasProvider::GetFootOrigin(const AsepriteFrame& frame) const {
    // 足元基準: 中心X, 下端Y
    // trimされている場合は、補正後のサイズを使う
    float width = frame.trimmed ? frame.spriteSourceSize.width : frame.frame.width;
    float height = frame.trimmed ? frame.spriteSourceSize.height : frame.frame.height;

    return Shared::Data::Graphics::FrameRef::ComputeFootOrigin(width, height);
}

// ===== IFrameProvider インターフェース実装 =====

bool AsepriteJsonAtlasProvider::HasClip(const std::string& clipName) const {
    return spriteSet_.HasClip(clipName);
}

int AsepriteJsonAtlasProvider::GetFrameCount(const std::string& clipName) const {
    const auto* clip = spriteSet_.GetClip(clipName);
    return clip ? static_cast<int>(clip->GetFrameCount()) : 0;
}

Shared::Data::Graphics::FrameRef AsepriteJsonAtlasProvider::GetFrame(const std::string& clipName, int frameIndex) const {
    const auto* clip = spriteSet_.GetClip(clipName);
    if (!clip || !clip->IsValidIndex(frameIndex)) {
        return Shared::Data::Graphics::FrameRef{};  // 無効なFrameRefを返す
    }

    return clip->frames[frameIndex];
}

float AsepriteJsonAtlasProvider::GetClipFps(const std::string& clipName) const {
    const auto* clip = spriteSet_.GetClip(clipName);
    return clip ? clip->defaultFps : 0.0f;
}

bool AsepriteJsonAtlasProvider::IsLooping(const std::string& clipName) const {
    const auto* clip = spriteSet_.GetClip(clipName);
    return clip ? clip->loop : false;
}

} // namespace Game::Graphics
