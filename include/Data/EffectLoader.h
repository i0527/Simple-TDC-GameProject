/**
 * @file EffectLoader.h
 * @brief エフェクト定義JSONローダー
 * 
 * JSONからパーティクル、視覚エフェクト、画面エフェクト定義を読み込む。
 */
#pragma once

#include "Data/EffectDef.h"
#include "Core/FileUtils.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <optional>
#include <filesystem>

namespace Data {

/**
 * @brief エフェクト定義ローダー
 */
class EffectLoader {
public:
    /**
     * @brief パーティクルエフェクト定義を読み込む
     */
    static std::optional<ParticleEffectDef> LoadParticleEffect(const std::string& path) {
        auto content = Core::FileUtils::ReadUtf8File(path);
        if (!content) {
            std::cerr << "EffectLoader: Failed to read file: " << path << std::endl;
            return std::nullopt;
        }
        
        try {
            nlohmann::json j = nlohmann::json::parse(*content);
            return ParseParticleEffect(j);
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "EffectLoader: JSON parse error in " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        } catch (const std::exception& e) {
            std::cerr << "EffectLoader: Error loading " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        }
    }
    
    /**
     * @brief スプライトエフェクト定義を読み込む
     */
    static std::optional<SpriteEffectDef> LoadSpriteEffect(const std::string& path) {
        auto content = Core::FileUtils::ReadUtf8File(path);
        if (!content) {
            return std::nullopt;
        }
        
        try {
            nlohmann::json j = nlohmann::json::parse(*content);
            return ParseSpriteEffect(j);
        } catch (const std::exception& e) {
            std::cerr << "EffectLoader: Error loading " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        }
    }
    
    /**
     * @brief 画面エフェクト定義を読み込む
     */
    static std::optional<ScreenEffectDef> LoadScreenEffect(const std::string& path) {
        auto content = Core::FileUtils::ReadUtf8File(path);
        if (!content) {
            return std::nullopt;
        }
        
        try {
            nlohmann::json j = nlohmann::json::parse(*content);
            return ParseScreenEffect(j);
        } catch (const std::exception& e) {
            std::cerr << "EffectLoader: Error loading " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        }
    }
    
    /**
     * @brief 複合エフェクト定義を読み込む
     */
    static std::optional<CompositeEffectDef> LoadCompositeEffect(const std::string& path) {
        auto content = Core::FileUtils::ReadUtf8File(path);
        if (!content) {
            return std::nullopt;
        }
        
        try {
            nlohmann::json j = nlohmann::json::parse(*content);
            return ParseCompositeEffect(j);
        } catch (const std::exception& e) {
            std::cerr << "EffectLoader: Error loading " << path << ": " << e.what() << std::endl;
            return std::nullopt;
        }
    }

private:
    // ===== ヘルパー =====
    
    static FloatRange ParseFloatRange(const nlohmann::json& j) {
        if (j.is_number()) {
            return FloatRange::Single(j.get<float>());
        } else if (j.is_array() && j.size() >= 2) {
            return FloatRange::Range(j[0].get<float>(), j[1].get<float>());
        } else if (j.is_object()) {
            return FloatRange::Range(
                j.value("min", 0.0f),
                j.value("max", 0.0f)
            );
        }
        return FloatRange::Single(0.0f);
    }
    
    static Vec2Range ParseVec2Range(const nlohmann::json& j) {
        if (j.is_object()) {
            if (j.contains("x") && j.contains("y")) {
                // 単一値
                float x = j["x"].is_array() 
                    ? j["x"][0].get<float>() 
                    : j["x"].get<float>();
                float y = j["y"].is_array() 
                    ? j["y"][0].get<float>() 
                    : j["y"].get<float>();
                    
                float maxX = j["x"].is_array() && j["x"].size() > 1
                    ? j["x"][1].get<float>() : x;
                float maxY = j["y"].is_array() && j["y"].size() > 1
                    ? j["y"][1].get<float>() : y;
                    
                return Vec2Range{x, maxX, y, maxY};
            } else {
                return Vec2Range{
                    j.value("minX", 0.0f),
                    j.value("maxX", 0.0f),
                    j.value("minY", 0.0f),
                    j.value("maxY", 0.0f)
                };
            }
        } else if (j.is_array() && j.size() >= 2) {
            float x = j[0].get<float>();
            float y = j[1].get<float>();
            return Vec2Range::Single(x, y);
        }
        return Vec2Range::Single(0, 0);
    }
    
    static ColorDef ParseColor(const nlohmann::json& j) {
        if (j.is_string()) {
            // Hex形式: "#RRGGBB" or "#RRGGBBAA"
            std::string hex = j.get<std::string>();
            if (hex.size() > 0 && hex[0] == '#') {
                hex = hex.substr(1);
            }
            unsigned int val = std::stoul(hex, nullptr, 16);
            
            if (hex.size() == 6) {
                return ColorDef{
                    ((val >> 16) & 0xFF) / 255.0f,
                    ((val >> 8) & 0xFF) / 255.0f,
                    (val & 0xFF) / 255.0f,
                    1.0f
                };
            } else if (hex.size() == 8) {
                return ColorDef{
                    ((val >> 24) & 0xFF) / 255.0f,
                    ((val >> 16) & 0xFF) / 255.0f,
                    ((val >> 8) & 0xFF) / 255.0f,
                    (val & 0xFF) / 255.0f
                };
            }
        } else if (j.is_array()) {
            ColorDef c;
            if (j.size() >= 1) c.r = j[0].get<float>();
            if (j.size() >= 2) c.g = j[1].get<float>();
            if (j.size() >= 3) c.b = j[2].get<float>();
            if (j.size() >= 4) c.a = j[3].get<float>();
            return c;
        } else if (j.is_object()) {
            return ColorDef{
                j.value("r", 1.0f),
                j.value("g", 1.0f),
                j.value("b", 1.0f),
                j.value("a", 1.0f)
            };
        }
        return ColorDef::White();
    }
    
    static ColorRange ParseColorRange(const nlohmann::json& j) {
        ColorRange range;
        
        if (j.is_object() && j.contains("start")) {
            range.start = ParseColor(j["start"]);
            range.end = j.contains("end") ? ParseColor(j["end"]) : range.start;
            range.isGradient = j.value("gradient", true);
        } else {
            range.start = ParseColor(j);
            range.end = range.start;
        }
        
        return range;
    }
    
    // ===== エミッター形状パース =====
    
    static EmitterShapeDef ParseEmitterShape(const nlohmann::json& j) {
        EmitterShapeDef shape;
        
        std::string shapeStr = j.value("type", "point");
        shape.shape = StringToEmitterShape(shapeStr);
        
        shape.width = j.value("width", 0.0f);
        shape.height = j.value("height", 0.0f);
        shape.radius = j.value("radius", 0.0f);
        shape.innerRadius = j.value("innerRadius", 0.0f);
        shape.angle = j.value("angle", 360.0f);
        shape.rotation = j.value("rotation", 0.0f);
        shape.edgeOnly = j.value("edgeOnly", false);
        
        return shape;
    }
    
    // ===== パーティクルエミッターパース =====
    
    static ParticleEmitterDef ParseEmitter(const nlohmann::json& j) {
        ParticleEmitterDef emitter;
        
        emitter.id = j.value("id", "");
        emitter.name = j.value("name", emitter.id);
        
        // スプライト
        emitter.textureId = j.value("texture", "");
        emitter.spriteIndex = j.value("spriteIndex", 0);
        emitter.animated = j.value("animated", false);
        emitter.frameCount = j.value("frameCount", 1);
        emitter.frameRate = j.value("frameRate", 10.0f);
        
        // エミッター設定
        if (j.contains("shape")) {
            emitter.shape = ParseEmitterShape(j["shape"]);
        }
        
        std::string modeStr = j.value("emissionMode", "continuous");
        if (modeStr == "burst") {
            emitter.emissionMode = EmissionMode::Burst;
        } else if (modeStr == "distance") {
            emitter.emissionMode = EmissionMode::Distance;
        } else {
            emitter.emissionMode = EmissionMode::Continuous;
        }
        
        emitter.emissionRate = j.value("rate", 10.0f);
        
        // バースト
        if (j.contains("bursts")) {
            for (const auto& b : j["bursts"]) {
                BurstDef burst;
                burst.time = b.value("time", 0.0f);
                burst.minCount = b.value("min", 1);
                burst.maxCount = b.value("max", burst.minCount);
                burst.interval = b.value("interval", 0.0f);
                burst.cycles = b.value("cycles", 1);
                emitter.bursts.push_back(burst);
            }
        }
        
        // 初期値
        emitter.lifetime = j.contains("lifetime") 
            ? ParseFloatRange(j["lifetime"]) 
            : FloatRange::Range(1.0f, 2.0f);
        emitter.speed = j.contains("speed") 
            ? ParseFloatRange(j["speed"]) 
            : FloatRange::Range(50.0f, 100.0f);
        emitter.angle = j.contains("angle") 
            ? ParseFloatRange(j["angle"]) 
            : FloatRange::Range(0.0f, 360.0f);
        emitter.scale = j.contains("scale") 
            ? ParseFloatRange(j["scale"]) 
            : FloatRange::Single(1.0f);
        emitter.rotation = j.contains("rotation") 
            ? ParseFloatRange(j["rotation"]) 
            : FloatRange::Single(0.0f);
            
        if (j.contains("color")) {
            emitter.color = ParseColorRange(j["color"]);
        }
        
        // ライフタイム中の変化
        if (j.contains("overLifetime")) {
            const auto& ol = j["overLifetime"];
            auto& olt = emitter.overLifetime;
            
            olt.startScale = ol.value("startScale", 1.0f);
            olt.endScale = ol.value("endScale", 1.0f);
            olt.scaleEasing = StringToEaseType(ol.value("scaleEasing", "linear"));
            
            if (ol.contains("startColor")) {
                olt.startColor = ParseColor(ol["startColor"]);
            }
            if (ol.contains("endColor")) {
                olt.endColor = ParseColor(ol["endColor"]);
            }
            olt.colorEasing = StringToEaseType(ol.value("colorEasing", "linear"));
            
            olt.startAlpha = ol.value("startAlpha", 1.0f);
            olt.endAlpha = ol.value("endAlpha", 0.0f);
            olt.alphaEasing = StringToEaseType(ol.value("alphaEasing", "linear"));
            
            olt.rotationSpeed = ol.contains("rotationSpeed") 
                ? ParseFloatRange(ol["rotationSpeed"]) 
                : FloatRange::Single(0.0f);
        }
        
        // 物理
        if (j.contains("gravity")) {
            emitter.gravity = ParseVec2Range(j["gravity"]);
        }
        emitter.drag = j.value("drag", 0.0f);
        emitter.velocityDamping = j.value("velocityDamping", 1.0f);
        
        // レンダリング
        emitter.blendMode = StringToBlendMode(j.value("blendMode", "additive"));
        emitter.sortingOrder = j.value("sortingOrder", 0);
        emitter.worldSpace = j.value("worldSpace", true);
        
        // 制限
        emitter.maxParticles = j.value("maxParticles", 100);
        
        return emitter;
    }
    
    // ===== パーティクルエフェクトパース =====
    
    static ParticleEffectDef ParseParticleEffect(const nlohmann::json& j) {
        ParticleEffectDef def;
        
        def.id = j.value("id", "");
        def.name = j.value("name", def.id);
        
        // エミッター
        if (j.contains("emitters")) {
            for (const auto& e : j["emitters"]) {
                def.emitters.push_back(ParseEmitter(e));
            }
        }
        
        // 全体設定
        def.duration = j.value("duration", 1.0f);
        def.loop = j.value("loop", false);
        def.autoDestroy = j.value("autoDestroy", true);
        def.scale = j.value("scale", 1.0f);
        
        // サウンド
        def.startSoundId = j.value("startSound", "");
        def.endSoundId = j.value("endSound", "");
        
        // タグ
        if (j.contains("tags")) {
            for (const auto& tag : j["tags"]) {
                def.tags.push_back(tag.get<std::string>());
            }
        }
        
        return def;
    }
    
    // ===== スプライトエフェクトパース =====
    
    static SpriteEffectDef ParseSpriteEffect(const nlohmann::json& j) {
        SpriteEffectDef def;
        
        def.id = j.value("id", "");
        
        std::string typeStr = j.value("type", "flash");
        if (typeStr == "flash") def.type = SpriteEffectType::Flash;
        else if (typeStr == "colorTint") def.type = SpriteEffectType::ColorTint;
        else if (typeStr == "fadeIn") def.type = SpriteEffectType::FadeIn;
        else if (typeStr == "fadeOut") def.type = SpriteEffectType::FadeOut;
        else if (typeStr == "scale") def.type = SpriteEffectType::Scale;
        else if (typeStr == "shake") def.type = SpriteEffectType::Shake;
        else if (typeStr == "pulse") def.type = SpriteEffectType::Pulse;
        
        def.duration = j.value("duration", 0.2f);
        def.easing = StringToEaseType(j.value("easing", "linear"));
        
        if (j.contains("color")) {
            def.color = ParseColor(j["color"]);
        }
        def.intensity = j.value("intensity", 1.0f);
        
        def.startScale = j.value("startScale", 1.0f);
        def.endScale = j.value("endScale", 1.0f);
        
        def.shakeIntensity = j.value("shakeIntensity", 5.0f);
        def.shakeFrequency = j.value("shakeFrequency", 30.0f);
        
        def.pulseMin = j.value("pulseMin", 0.9f);
        def.pulseMax = j.value("pulseMax", 1.1f);
        def.pulseSpeed = j.value("pulseSpeed", 2.0f);
        
        def.loop = j.value("loop", false);
        
        return def;
    }
    
    // ===== 画面エフェクトパース =====
    
    static ScreenEffectDef ParseScreenEffect(const nlohmann::json& j) {
        ScreenEffectDef def;
        
        def.id = j.value("id", "");
        def.name = j.value("name", def.id);
        def.type = StringToScreenEffectType(j.value("type", "shake"));
        
        def.duration = j.value("duration", 0.5f);
        def.easing = StringToEaseType(j.value("easing", "easeOut"));
        
        // Shake
        def.shakeIntensity = j.value("shakeIntensity", 10.0f);
        def.shakeFrequency = j.value("shakeFrequency", 20.0f);
        def.shakeDecay = j.value("shakeDecay", true);
        
        // Flash
        if (j.contains("flashColor")) {
            def.flashColor = ParseColor(j["flashColor"]);
        }
        
        // Fade
        if (j.contains("fadeColor")) {
            def.fadeColor = ParseColor(j["fadeColor"]);
        }
        
        // Vignette
        def.vignetteIntensity = j.value("vignetteIntensity", 0.5f);
        def.vignetteSmoothness = j.value("vignetteSmoothness", 0.5f);
        
        // ColorGrading
        def.saturation = j.value("saturation", 1.0f);
        def.contrast = j.value("contrast", 1.0f);
        def.brightness = j.value("brightness", 1.0f);
        if (j.contains("colorTint")) {
            def.colorTint = ParseColor(j["colorTint"]);
        }
        
        // Zoom
        def.zoomAmount = j.value("zoomAmount", 1.2f);
        if (j.contains("zoomCenter")) {
            def.zoomCenter = ParseVec2Range(j["zoomCenter"]);
        }
        
        // Blur
        def.blurRadius = j.value("blurRadius", 5.0f);
        
        // Chromatic
        def.chromaticIntensity = j.value("chromaticIntensity", 0.02f);
        
        // SlowMotion
        def.timeScale = j.value("timeScale", 0.5f);
        
        return def;
    }
    
    // ===== 複合エフェクトパース =====
    
    static CompositeEffectDef ParseCompositeEffect(const nlohmann::json& j) {
        CompositeEffectDef def;
        
        def.id = j.value("id", "");
        def.name = j.value("name", def.id);
        
        // パーティクル
        if (j.contains("particles")) {
            for (const auto& p : j["particles"]) {
                EffectEntry entry;
                entry.effectId = p.value("effect", "");
                entry.startTime = p.value("time", 0.0f);
                if (p.contains("offset")) {
                    entry.offset = ParseVec2Range(p["offset"]);
                }
                entry.scale = p.value("scale", 1.0f);
                def.particles.push_back(entry);
            }
        }
        
        // スプライト
        if (j.contains("sprites")) {
            for (const auto& s : j["sprites"]) {
                EffectEntry entry;
                entry.effectId = s.value("effect", "");
                entry.startTime = s.value("time", 0.0f);
                if (s.contains("offset")) {
                    entry.offset = ParseVec2Range(s["offset"]);
                }
                entry.scale = s.value("scale", 1.0f);
                def.sprites.push_back(entry);
            }
        }
        
        // 画面エフェクト
        if (j.contains("screen")) {
            for (const auto& sc : j["screen"]) {
                EffectEntry entry;
                entry.effectId = sc.value("effect", "");
                entry.startTime = sc.value("time", 0.0f);
                def.screenEffects.push_back(entry);
            }
        }
        
        // サウンド
        if (j.contains("sounds")) {
            for (const auto& snd : j["sounds"]) {
                CompositeEffectDef::SoundEntry entry;
                entry.soundId = snd.value("sound", "");
                entry.startTime = snd.value("time", 0.0f);
                def.sounds.push_back(entry);
            }
        }
        
        // 全体設定
        def.duration = j.value("duration", 1.0f);
        def.loop = j.value("loop", false);
        
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
