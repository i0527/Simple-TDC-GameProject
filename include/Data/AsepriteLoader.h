/**
 * @file AsepriteLoader.h
 * @brief Aseprite出力JSON/PNGローダー
 * 
 * AsepriteからExportされたJSONとPNGファイルを読み込み、
 * SpriteAnimationDefに変換する。
 * 
 * Aseprite Export設定:
 * - Array形式を推奨
 * - Frame Tags を含める
 * - Meta情報を含める
 */
#pragma once

#include "Data/AnimationDef.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <optional>
#include <codecvt>
#include <locale>

namespace Data {

/**
 * @brief Asepriteフレーム情報
 */
struct AsepriteFrame {
    std::string filename;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    int duration = 100;  // ミリ秒
    bool rotated = false;
    bool trimmed = false;
    
    // ソースサイズ（トリミング前）
    int sourceW = 0;
    int sourceH = 0;
    
    // スプライトソースのオフセット
    int spriteSourceX = 0;
    int spriteSourceY = 0;
};

/**
 * @brief Asepriteタグ（アニメーションクリップ）
 */
struct AsepriteTag {
    std::string name;
    int from = 0;
    int to = 0;
    std::string direction = "forward";  // forward, reverse, pingpong
};

/**
 * @brief Aseprite JSONローダー
 */
class AsepriteLoader {
public:
    /**
     * @brief Aseprite JSONファイルからSpriteAnimationDefを読み込む
     * @param jsonPath Aseprite出力JSONファイルパス
     * @param textureId テクスチャID（リソース管理用）
     */
    static std::optional<SpriteAnimationDef> LoadFromFile(
        const std::string& jsonPath, 
        const std::string& textureId = ""
    ) {
        std::ifstream file(jsonPath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "AsepriteLoader: Failed to open file: " << jsonPath << std::endl;
            return std::nullopt;
        }
        
        try {
            // UTF-8 BOMをスキップ
            char bom[3];
            file.read(bom, 3);
            if (!(bom[0] == '\xEF' && bom[1] == '\xBB' && bom[2] == '\xBF')) {
                file.seekg(0);  // BOMがなければ先頭に戻る
            }
            
            nlohmann::json j;
            file >> j;
            
            std::string actualTextureId = textureId;
            if (actualTextureId.empty()) {
                // JSONパスからテクスチャIDを生成
                size_t lastSlash = jsonPath.find_last_of("/\\");
                size_t lastDot = jsonPath.find_last_of('.');
                if (lastDot != std::string::npos) {
                    actualTextureId = jsonPath.substr(
                        lastSlash != std::string::npos ? lastSlash + 1 : 0,
                        lastDot - (lastSlash != std::string::npos ? lastSlash + 1 : 0)
                    );
                }
            }
            
            return ParseAsepriteJson(j, actualTextureId);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "AsepriteLoader: JSON parse error in " << jsonPath << ": " << e.what() << std::endl;
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "AsepriteLoader: Error loading " << jsonPath << ": " << e.what() << std::endl;
            return std::nullopt;
        }
    }

private:
    static SpriteAnimationDef ParseAsepriteJson(const nlohmann::json& j, const std::string& textureId) {
        SpriteAnimationDef anim;
        
        anim.id = textureId;
        anim.name = textureId;
        
        // フレーム情報をパース
        std::vector<AsepriteFrame> frames;
        
        if (j.contains("frames")) {
            const auto& framesJson = j["frames"];
            
            // Array形式
            if (framesJson.is_array()) {
                for (const auto& frameJson : framesJson) {
                    frames.push_back(ParseFrame(frameJson));
                }
            }
            // Hash形式
            else if (framesJson.is_object()) {
                for (auto& [key, frameJson] : framesJson.items()) {
                    AsepriteFrame frame = ParseFrame(frameJson);
                    frame.filename = key;
                    frames.push_back(frame);
                }
            }
        }
        
        if (frames.empty()) {
            std::cerr << "AsepriteLoader: No frames found" << std::endl;
            return anim;
        }
        
        // スプライトシート情報を設定
        anim.spriteSheet.textureId = textureId;
        anim.spriteSheet.frameWidth = frames[0].w;
        anim.spriteSheet.frameHeight = frames[0].h;
        anim.spriteSheet.totalFrames = static_cast<int>(frames.size());
        
        // メタ情報からサイズを取得
        if (j.contains("meta")) {
            const auto& meta = j["meta"];
            if (meta.contains("size")) {
                int imageW = meta["size"].value("w", 0);
                int imageH = meta["size"].value("h", 0);
                
                if (frames[0].w > 0 && frames[0].h > 0) {
                    anim.spriteSheet.columns = imageW / frames[0].w;
                    anim.spriteSheet.rows = imageH / frames[0].h;
                }
            }
            
            // 画像ファイル名
            if (meta.contains("image")) {
                // PNGファイル名を記録（将来のテクスチャ読み込み用）
                anim.spriteSheet.textureId = meta["image"].get<std::string>();
            }
        }
        
        // タグ（アニメーションクリップ）をパース
        std::vector<AsepriteTag> tags;
        if (j.contains("meta") && j["meta"].contains("frameTags")) {
            for (const auto& tagJson : j["meta"]["frameTags"]) {
                tags.push_back(ParseTag(tagJson));
            }
        }
        
        // タグがない場合は全フレームを"default"クリップとして登録
        if (tags.empty()) {
            AsepriteTag defaultTag;
            defaultTag.name = "default";
            defaultTag.from = 0;
            defaultTag.to = static_cast<int>(frames.size()) - 1;
            tags.push_back(defaultTag);
        }
        
        // 各タグをAnimClipDefに変換
        for (const auto& tag : tags) {
            AnimClipDef clip;
            clip.id = tag.name;
            clip.name = tag.name;
            
            // ループモードを変換
            if (tag.direction == "pingpong") {
                clip.loopMode = AnimLoopMode::PingPong;
            } else if (tag.direction == "reverse") {
                clip.loopMode = AnimLoopMode::Loop;
                // reverseは後でフレーム順を逆にする
            } else {
                clip.loopMode = AnimLoopMode::Loop;
            }
            
            // フレームを追加
            int start = tag.from;
            int end = tag.to;
            
            if (tag.direction == "reverse") {
                for (int i = end; i >= start; --i) {
                    if (i < static_cast<int>(frames.size())) {
                        clip.frames.push_back(ConvertFrame(frames[i], i));
                    }
                }
            } else {
                for (int i = start; i <= end; ++i) {
                    if (i < static_cast<int>(frames.size())) {
                        clip.frames.push_back(ConvertFrame(frames[i], i));
                    }
                }
            }
            
            anim.clips[clip.id] = std::move(clip);
        }
        
        // デフォルトクリップを設定
        if (!tags.empty()) {
            anim.defaultClip = tags[0].name;
        }
        
        // ピボットを下端中央に設定（にゃんこ大戦争スタイル）
        anim.pivotX = 0.5f;
        anim.pivotY = 1.0f;
        
        return anim;
    }
    
    static AsepriteFrame ParseFrame(const nlohmann::json& j) {
        AsepriteFrame frame;
        
        if (j.contains("filename")) {
            frame.filename = j["filename"].get<std::string>();
        }
        
        if (j.contains("frame")) {
            const auto& f = j["frame"];
            frame.x = f.value("x", 0);
            frame.y = f.value("y", 0);
            frame.w = f.value("w", 0);
            frame.h = f.value("h", 0);
        }
        
        frame.duration = j.value("duration", 100);
        frame.rotated = j.value("rotated", false);
        frame.trimmed = j.value("trimmed", false);
        
        if (j.contains("sourceSize")) {
            frame.sourceW = j["sourceSize"].value("w", frame.w);
            frame.sourceH = j["sourceSize"].value("h", frame.h);
        }
        
        if (j.contains("spriteSourceSize")) {
            frame.spriteSourceX = j["spriteSourceSize"].value("x", 0);
            frame.spriteSourceY = j["spriteSourceSize"].value("y", 0);
        }
        
        return frame;
    }
    
    static AsepriteTag ParseTag(const nlohmann::json& j) {
        AsepriteTag tag;
        
        tag.name = j.value("name", "unnamed");
        tag.from = j.value("from", 0);
        tag.to = j.value("to", 0);
        tag.direction = j.value("direction", "forward");
        
        return tag;
    }
    
    static SpriteFrameDef ConvertFrame(const AsepriteFrame& aseFrame, int index) {
        SpriteFrameDef frame;
        
        frame.spriteIndex = index;
        frame.duration = aseFrame.duration / 1000.0f;  // ミリ秒→秒
        
        // トリミングされている場合のオフセット
        if (aseFrame.trimmed) {
            frame.offsetX = static_cast<float>(aseFrame.spriteSourceX);
            frame.offsetY = static_cast<float>(aseFrame.spriteSourceY);
        }
        
        return frame;
    }
};

/**
 * @brief UTF-8ファイル読み書きユーティリティ
 */
class UnicodeFileUtils {
public:
    /**
     * @brief UTF-8ファイルを文字列として読み込む
     */
    static std::optional<std::string> ReadUtf8File(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            return std::nullopt;
        }
        
        // ファイルサイズを取得
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // UTF-8 BOMをスキップ
        char bom[3];
        file.read(bom, 3);
        if (bom[0] == '\xEF' && bom[1] == '\xBB' && bom[2] == '\xBF') {
            size -= 3;
        } else {
            file.seekg(0);
        }
        
        // 内容を読み込む
        std::string content(size, '\0');
        file.read(&content[0], size);
        
        return content;
    }
    
    /**
     * @brief UTF-8ファイルに文字列を書き込む（BOMなし）
     */
    static bool WriteUtf8File(const std::string& path, const std::string& content, bool withBom = false) {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // BOMを書き込む（オプション）
        if (withBom) {
            file.write("\xEF\xBB\xBF", 3);
        }
        
        file.write(content.c_str(), content.size());
        return true;
    }
    
    /**
     * @brief JSONをUTF-8ファイルに整形して保存
     */
    static bool SaveJsonUtf8(const std::string& path, const nlohmann::json& j, int indent = 4) {
        try {
            std::string content = j.dump(indent, ' ', false, nlohmann::json::error_handler_t::replace);
            return WriteUtf8File(path, content, false);
        } catch (const std::exception& e) {
            std::cerr << "UnicodeFileUtils: Error saving JSON: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief UTF-8 JSONファイルを読み込む
     */
    static std::optional<nlohmann::json> LoadJsonUtf8(const std::string& path) {
        auto content = ReadUtf8File(path);
        if (!content) {
            return std::nullopt;
        }
        
        try {
            return nlohmann::json::parse(*content);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "UnicodeFileUtils: JSON parse error: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
};

} // namespace Data
