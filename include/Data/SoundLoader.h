/**
 * @file SoundLoader.h
 * @brief サウンド定義JSONローダー
 * 
 * JSONからサウンド定義を読み込む。
 */
#pragma once

#include "Data/SoundDef.h"
#include "Core/FileUtils.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <optional>
#include <filesystem>

namespace Data {

/**
 * @brief サウンド定義ローダー
 */
class SoundLoader {
public:
    /**
     * @brief サウンド定義をJSONファイルから読み込む
     */
    static std::optional<SoundDef> LoadSound(const std::string& path) {
        auto content = Core::FileUtils::ReadUtf8File(path);
        if (!content) {
            std::cerr << "SoundLoader: Failed to read file: " << path << std::endl;
            return std::nullopt;
        }
        
        try {
            nlohmann::json j = nlohmann::json::parse(*content);
            return ParseSound(j);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "SoundLoader: JSON parse error in " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "SoundLoader: Error loading " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        }
    }
    
    /**
     * @brief BGM定義をJSONファイルから読み込む
     */
    static std::optional<MusicDef> LoadMusic(const std::string& path) {
        auto content = Core::FileUtils::ReadUtf8File(path);
        if (!content) {
            std::cerr << "SoundLoader: Failed to read file: " << path << std::endl;
            return std::nullopt;
        }
        
        try {
            nlohmann::json j = nlohmann::json::parse(*content);
            return ParseMusic(j);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "SoundLoader: JSON parse error in " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "SoundLoader: Error loading " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        }
    }
    
    /**
     * @brief サウンドバンクをJSONファイルから読み込む
     */
    static std::optional<SoundBankDef> LoadSoundBank(const std::string& path) {
        auto content = Core::FileUtils::ReadUtf8File(path);
        if (!content) {
            std::cerr << "SoundLoader: Failed to read file: " << path << std::endl;
            return std::nullopt;
        }
        
        try {
            nlohmann::json j = nlohmann::json::parse(*content);
            return ParseSoundBank(j);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "SoundLoader: JSON parse error in " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "SoundLoader: Error loading " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        }
    }
    
    /**
     * @brief ディレクトリ内の全サウンドファイルを読み込む
     */
    static std::vector<SoundDef> LoadSoundsFromDirectory(const std::string& dirPath) {
        std::vector<SoundDef> sounds;
        
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(dirPath, ec)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".json" || ext == ".sound") {
                    if (auto sound = LoadSound(entry.path().string())) {
                        sounds.push_back(std::move(*sound));
                    }
                }
            }
        }
        
        return sounds;
    }
    
private:
    static SoundDef ParseSound(const nlohmann::json& j) {
        SoundDef def;
        
        def.id = j.value("id", "");
        def.name = j.value("name", def.id);
        def.type = StringToSoundType(j.value("type", "sfx"));
        def.priority = StringToSoundPriority(j.value("priority", "normal"));
        
        // バリエーション
        if (j.contains("variations")) {
            for (const auto& v : j["variations"]) {
                SoundVariation var;
                var.filePath = v.value("file", "");
                var.weight = v.value("weight", 1.0f);
                var.pitchOffset = v.value("pitchOffset", 0.0f);
                var.volumeOffset = v.value("volumeOffset", 0.0f);
                def.variations.push_back(var);
            }
        } else if (j.contains("file")) {
            // 単一ファイル
            SoundVariation var;
            var.filePath = j["file"].get<std::string>();
            def.variations.push_back(var);
        }
        
        // 再生設定
        def.volume = j.value("volume", 1.0f);
        def.pitch = j.value("pitch", 1.0f);
        def.pitchVariation = j.value("pitchVariation", 0.0f);
        def.volumeVariation = j.value("volumeVariation", 0.0f);
        def.loop = j.value("loop", false);
        
        // 3Dサウンド
        def.is3D = j.value("is3D", false);
        if (j.contains("spatial")) {
            const auto& spatial = j["spatial"];
            def.minDistance = spatial.value("minDistance", 1.0f);
            def.maxDistance = spatial.value("maxDistance", 100.0f);
            def.rolloffFactor = spatial.value("rolloff", 1.0f);
        }
        
        // 再生制限
        def.maxInstances = j.value("maxInstances", 4);
        def.cooldown = j.value("cooldown", 0.0f);
        def.stopOldest = j.value("stopOldest", true);
        
        // フェード
        def.fadeInTime = j.value("fadeIn", 0.0f);
        def.fadeOutTime = j.value("fadeOut", 0.0f);
        def.fadeType = StringToFadeType(j.value("fadeType", "linear"));
        
        // グループ・タグ
        def.group = j.value("group", "");
        if (j.contains("tags")) {
            for (const auto& tag : j["tags"]) {
                def.tags.push_back(tag.get<std::string>());
            }
        }
        
        return def;
    }
    
    static MusicDef ParseMusic(const nlohmann::json& j) {
        MusicDef def;
        
        def.id = j.value("id", "");
        def.name = j.value("name", def.id);
        def.filePath = j.value("file", "");
        
        // 基本設定
        def.volume = j.value("volume", 0.8f);
        def.bpm = j.value("bpm", 120.0f);
        def.beatsPerBar = j.value("beatsPerBar", 4);
        
        // ループ設定
        if (j.contains("loop")) {
            const auto& loop = j["loop"];
            def.loop.enabled = loop.value("enabled", true);
            def.loop.loopStart = loop.value("start", 0.0f);
            def.loop.loopEnd = loop.value("end", 0.0f);
            def.loop.loopCount = loop.value("count", -1);
        }
        
        // イントロ/アウトロ
        def.introFilePath = j.value("intro", "");
        def.outroFilePath = j.value("outro", "");
        
        // トランジション
        def.crossfadeDuration = j.value("crossfade", 2.0f);
        def.crossfadeType = StringToFadeType(j.value("crossfadeType", "easeInOut"));
        
        // レイヤー
        if (j.contains("layers")) {
            for (const auto& layerJson : j["layers"]) {
                MusicLayer layer;
                layer.id = layerJson.value("id", "");
                layer.filePath = layerJson.value("file", "");
                layer.volume = layerJson.value("volume", 1.0f);
                layer.enabled = layerJson.value("enabled", true);
                layer.condition = layerJson.value("condition", "");
                def.layers.push_back(layer);
            }
        }
        
        // グループ・タグ
        def.group = j.value("group", "music");
        if (j.contains("tags")) {
            for (const auto& tag : j["tags"]) {
                def.tags.push_back(tag.get<std::string>());
            }
        }
        
        return def;
    }
    
    static SoundBankDef ParseSoundBank(const nlohmann::json& j) {
        SoundBankDef def;
        
        def.id = j.value("id", "");
        def.name = j.value("name", def.id);
        
        // サウンドID
        if (j.contains("sounds")) {
            for (const auto& id : j["sounds"]) {
                def.soundIds.push_back(id.get<std::string>());
            }
        }
        
        // ミュージックID
        if (j.contains("music")) {
            for (const auto& id : j["music"]) {
                def.musicIds.push_back(id.get<std::string>());
            }
        }
        
        // イベント
        if (j.contains("events")) {
            for (auto& [eventId, eventJson] : j["events"].items()) {
                SoundEvent event;
                event.id = eventId;
                event.name = eventJson.value("name", eventId);
                
                // キュー
                if (eventJson.contains("cues")) {
                    for (const auto& cueJson : eventJson["cues"]) {
                        SoundCue cue;
                        cue.id = cueJson.value("id", "");
                        cue.soundId = cueJson.value("sound", "");
                        cue.delay = cueJson.value("delay", 0.0f);
                        cue.probability = cueJson.value("probability", 1.0f);
                        cue.condition = cueJson.value("condition", "");
                        event.cues.push_back(cue);
                    }
                }
                
                // 再生モード
                std::string modeStr = eventJson.value("playMode", "all");
                if (modeStr == "random") {
                    event.playMode = SoundEvent::PlayMode::Random;
                } else if (modeStr == "sequence") {
                    event.playMode = SoundEvent::PlayMode::Sequence;
                } else {
                    event.playMode = SoundEvent::PlayMode::All;
                }
                
                event.cooldown = eventJson.value("cooldown", 0.0f);
                
                def.events[eventId] = event;
            }
        }
        
        // バンク設定
        def.preload = j.value("preload", false);
        def.persistent = j.value("persistent", false);
        
        // タグ
        if (j.contains("tags")) {
            for (const auto& tag : j["tags"]) {
                def.tags.push_back(tag.get<std::string>());
            }
        }
        
        return def;
    }
};

} // namespace Data
