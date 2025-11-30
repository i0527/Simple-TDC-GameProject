/**
 * @file AnimationLoader.h
 * @brief アニメーション定義JSONローダー
 * 
 * Phase 4C: データ駆動アニメーションシステム
 */
#pragma once

#include "Data/AnimationDef.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <optional>

namespace Data {

/**
 * @brief アニメーション定義ローダー
 */
class AnimationLoader {
public:
    /**
     * @brief JSONファイルからSpriteAnimationDefを読み込む
     */
    static std::optional<SpriteAnimationDef> LoadFromFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "AnimationLoader: Failed to open file: " << path << std::endl;
            return std::nullopt;
        }
        
        try {
            nlohmann::json j;
            file >> j;
            return ParseAnimation(j);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "AnimationLoader: JSON parse error in " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "AnimationLoader: Error loading " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        }
    }
    
    /**
     * @brief JSON文字列からSpriteAnimationDefを読み込む
     */
    static std::optional<SpriteAnimationDef> LoadFromString(const std::string& jsonStr) {
        try {
            nlohmann::json j = nlohmann::json::parse(jsonStr);
            return ParseAnimation(j);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "AnimationLoader: JSON parse error: " << e.what() << std::endl;
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "AnimationLoader: Error: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

private:
    static SpriteAnimationDef ParseAnimation(const nlohmann::json& j) {
        SpriteAnimationDef anim;
        
        anim.id = j.value("id", "unnamed");
        anim.name = j.value("name", anim.id);
        anim.defaultClip = j.value("defaultClip", "idle");
        anim.pivotX = j.value("pivotX", 0.5f);
        anim.pivotY = j.value("pivotY", 1.0f);
        anim.globalOffsetX = j.value("globalOffsetX", 0.0f);
        anim.globalOffsetY = j.value("globalOffsetY", 0.0f);
        
        // スプライトシート
        if (j.contains("spriteSheet")) {
            anim.spriteSheet = ParseSpriteSheet(j["spriteSheet"]);
        }
        
        // クリップ一覧
        if (j.contains("clips") && j["clips"].is_object()) {
            for (auto& [clipId, clipJson] : j["clips"].items()) {
                AnimClipDef clip = ParseClip(clipJson);
                clip.id = clipId;
                anim.clips[clipId] = std::move(clip);
            }
        }
        
        // 配列形式のクリップ
        if (j.contains("clips") && j["clips"].is_array()) {
            for (const auto& clipJson : j["clips"]) {
                AnimClipDef clip = ParseClip(clipJson);
                if (!clip.id.empty()) {
                    anim.clips[clip.id] = std::move(clip);
                }
            }
        }
        
        return anim;
    }
    
    static SpriteSheetDef ParseSpriteSheet(const nlohmann::json& j) {
        SpriteSheetDef sheet;
        
        sheet.textureId = j.value("textureId", "");
        sheet.frameWidth = j.value("frameWidth", 64);
        sheet.frameHeight = j.value("frameHeight", 64);
        sheet.columns = j.value("columns", 1);
        sheet.rows = j.value("rows", 1);
        sheet.totalFrames = j.value("totalFrames", sheet.columns * sheet.rows);
        sheet.paddingX = j.value("paddingX", 0);
        sheet.paddingY = j.value("paddingY", 0);
        sheet.offsetX = j.value("offsetX", 0);
        sheet.offsetY = j.value("offsetY", 0);
        
        return sheet;
    }
    
    static AnimClipDef ParseClip(const nlohmann::json& j) {
        AnimClipDef clip;
        
        clip.id = j.value("id", "");
        clip.name = j.value("name", clip.id);
        clip.loopMode = ParseLoopMode(j.value("loopMode", "loop"));
        clip.loopCount = j.value("loopCount", 1);
        clip.speed = j.value("speed", 1.0f);
        clip.defaultDuration = j.value("defaultDuration", 0.1f);
        clip.nextClip = j.value("nextClip", "");
        clip.canInterrupt = j.value("canInterrupt", true);
        
        // フレーム定義
        if (j.contains("frames") && j["frames"].is_array()) {
            for (const auto& frameJson : j["frames"]) {
                clip.frames.push_back(ParseFrame(frameJson, clip.defaultDuration));
            }
        }
        
        // 簡易フレーム指定（フレームインデックスの配列）
        if (j.contains("frameIndices") && j["frameIndices"].is_array()) {
            for (const auto& idx : j["frameIndices"]) {
                SpriteFrameDef frame;
                frame.spriteIndex = idx.get<int>();
                frame.duration = clip.defaultDuration;
                clip.frames.push_back(frame);
            }
        }
        
        // 範囲指定（startFrame〜endFrame）
        if (j.contains("startFrame") && j.contains("endFrame")) {
            int start = j["startFrame"].get<int>();
            int end = j["endFrame"].get<int>();
            for (int i = start; i <= end; ++i) {
                SpriteFrameDef frame;
                frame.spriteIndex = i;
                frame.duration = clip.defaultDuration;
                clip.frames.push_back(frame);
            }
        }
        
        // イベント
        if (j.contains("events") && j["events"].is_array()) {
            for (const auto& eventJson : j["events"]) {
                clip.events.push_back(ParseFrameEvent(eventJson));
            }
        }
        
        return clip;
    }
    
    static SpriteFrameDef ParseFrame(const nlohmann::json& j, float defaultDuration) {
        SpriteFrameDef frame;
        
        // 単純な数値の場合はインデックスのみ
        if (j.is_number()) {
            frame.spriteIndex = j.get<int>();
            frame.duration = defaultDuration;
            return frame;
        }
        
        frame.spriteIndex = j.value("index", j.value("spriteIndex", 0));
        frame.duration = j.value("duration", defaultDuration);
        
        if (j.contains("offsetX")) frame.offsetX = j["offsetX"].get<float>();
        if (j.contains("offsetY")) frame.offsetY = j["offsetY"].get<float>();
        if (j.contains("scaleX")) frame.scaleX = j["scaleX"].get<float>();
        if (j.contains("scaleY")) frame.scaleY = j["scaleY"].get<float>();
        if (j.contains("rotation")) frame.rotation = j["rotation"].get<float>();
        if (j.contains("alpha")) frame.alpha = j["alpha"].get<float>();
        
        return frame;
    }
    
    static FrameEventDef ParseFrameEvent(const nlohmann::json& j) {
        FrameEventDef event;
        
        event.frame = j.value("frame", 0);
        event.type = ParseFrameEventType(j.value("type", "callback"));
        event.eventName = j.value("eventName", j.value("name", ""));
        event.soundId = j.value("soundId", "");
        event.particleId = j.value("particleId", "");
        event.offsetX = j.value("offsetX", 0.0f);
        event.offsetY = j.value("offsetY", 0.0f);
        
        // 追加パラメータ
        if (j.contains("params") && j["params"].is_object()) {
            for (auto& [key, value] : j["params"].items()) {
                if (value.is_number()) {
                    event.params[key] = value.get<float>();
                }
            }
        }
        
        return event;
    }
};

} // namespace Data
