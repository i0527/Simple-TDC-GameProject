/**
 * @file EffectManager.h
 * @brief エフェクトマネージャー
 * 
 * Phase 5A: エフェクトシステム実装
 * - パーティクルエフェクト
 * - 画面エフェクト（シェイク、フラッシュ等）
 * - 複合エフェクト
 */
#pragma once

#include "Core/Platform.h"
#include "Data/EffectDef.h"
#include "Core/DefinitionRegistry.h"
#include <vector>
#include <memory>
#include <random>
#include <cmath>

namespace Core {

// ===== パーティクル =====

/**
 * @brief 個々のパーティクル
 */
struct Particle {
    float x = 0.0f, y = 0.0f;       // 位置
    float vx = 0.0f, vy = 0.0f;     // 速度
    float lifetime = 0.0f;          // 残り寿命
    float maxLifetime = 1.0f;       // 最大寿命
    float scale = 1.0f;             // スケール
    float rotation = 0.0f;          // 回転
    float rotationSpeed = 0.0f;     // 回転速度
    Color color = WHITE;            // 色
    int spriteIndex = 0;            // スプライトインデックス
    bool active = false;            // アクティブフラグ
};

/**
 * @brief パーティクルエミッターインスタンス
 */
struct ParticleEmitterInstance {
    const Data::ParticleEmitterDef* def = nullptr;
    std::vector<Particle> particles;
    
    float x = 0.0f, y = 0.0f;       // エミッター位置
    float elapsed = 0.0f;           // 経過時間
    float emissionAccumulator = 0.0f;  // 発生蓄積
    int burstIndex = 0;             // 次のバーストインデックス
    bool active = true;
    bool emitting = true;           // 新規発生中
};

/**
 * @brief パーティクルエフェクトインスタンス
 */
struct ParticleEffectInstance {
    std::string effectId;
    const Data::ParticleEffectDef* def = nullptr;
    std::vector<ParticleEmitterInstance> emitters;
    
    float x = 0.0f, y = 0.0f;
    float scale = 1.0f;
    float elapsed = 0.0f;
    bool active = true;
    bool loop = false;
};

// ===== 画面エフェクト =====

/**
 * @brief 画面エフェクトインスタンス
 */
struct ScreenEffectInstance {
    std::string effectId;
    const Data::ScreenEffectDef* def = nullptr;
    
    float elapsed = 0.0f;
    float duration = 0.0f;
    bool active = true;
    
    // Shake用
    float shakeOffsetX = 0.0f;
    float shakeOffsetY = 0.0f;
    
    // Flash用
    float flashAlpha = 0.0f;
    
    // SlowMotion用
    float timeScale = 1.0f;
};

/**
 * @brief エフェクトマネージャー
 */
class EffectManager {
public:
    EffectManager() : rng_(std::random_device{}()) {}
    
    ~EffectManager() = default;
    
    /**
     * @brief 初期化
     */
    void Initialize() {
        particleEffects_.reserve(32);
        screenEffects_.reserve(8);
        initialized_ = true;
        std::cout << "EffectManager: Initialized" << std::endl;
    }
    
    /**
     * @brief 更新
     */
    void Update(float dt) {
        if (!initialized_) return;
        
        // 画面エフェクト更新
        UpdateScreenEffects(dt);
        
        // パーティクルエフェクト更新
        UpdateParticleEffects(dt);
        
        // 非アクティブエフェクトのクリーンアップ
        CleanupInactiveEffects();
    }
    
    /**
     * @brief 描画（BeginDrawing後に呼び出す）
     */
    void Render() {
        if (!initialized_) return;
        
        // パーティクル描画
        RenderParticles();
        
        // 画面エフェクト描画
        RenderScreenEffects();
    }
    
    // ===== パーティクルエフェクト =====
    
    /**
     * @brief パーティクルエフェクトを再生
     */
    bool PlayParticleEffect(const std::string& effectId, float x, float y, float scale = 1.0f) {
        if (!registry_) return false;
        
        const Data::ParticleEffectDef* def = registry_->TryGetParticleEffect(effectId);
        if (!def) {
            std::cerr << "EffectManager: Particle effect not found: " << effectId << std::endl;
            return false;
        }
        
        ParticleEffectInstance inst;
        inst.effectId = effectId;
        inst.def = def;
        inst.x = x;
        inst.y = y;
        inst.scale = scale * def->scale;
        inst.loop = def->loop;
        
        // エミッターを初期化
        for (const auto& emitterDef : def->emitters) {
            ParticleEmitterInstance emitter;
            emitter.def = &emitterDef;
            emitter.x = x;
            emitter.y = y;
            emitter.particles.resize(emitterDef.maxParticles);
            inst.emitters.push_back(std::move(emitter));
        }
        
        particleEffects_.push_back(std::move(inst));
        return true;
    }
    
    /**
     * @brief パーティクルエフェクトを停止
     */
    void StopParticleEffect(const std::string& effectId) {
        for (auto& effect : particleEffects_) {
            if (effect.effectId == effectId) {
                for (auto& emitter : effect.emitters) {
                    emitter.emitting = false;
                }
            }
        }
    }
    
    // ===== 画面エフェクト =====
    
    /**
     * @brief 画面エフェクトを再生
     */
    bool PlayScreenEffect(const std::string& effectId) {
        if (!registry_) return false;
        
        const Data::ScreenEffectDef* def = registry_->TryGetScreenEffect(effectId);
        if (!def) {
            std::cerr << "EffectManager: Screen effect not found: " << effectId << std::endl;
            return false;
        }
        
        ScreenEffectInstance inst;
        inst.effectId = effectId;
        inst.def = def;
        inst.duration = def->duration;
        
        screenEffects_.push_back(inst);
        return true;
    }
    
    /**
     * @brief 画面シェイク
     */
    void ShakeScreen(float intensity, float duration) {
        ScreenEffectInstance inst;
        inst.duration = duration;
        inst.active = true;
        
        // 仮のdefを使わずに直接シェイクパラメータを設定
        shakeIntensity_ = intensity;
        shakeDuration_ = duration;
        shakeElapsed_ = 0.0f;
    }
    
    /**
     * @brief 画面フラッシュ
     */
    void FlashScreen(Color color, float duration) {
        flashColor_ = color;
        flashDuration_ = duration;
        flashElapsed_ = 0.0f;
    }
    
    /**
     * @brief スローモーション
     */
    void SetSlowMotion(float timeScale, float duration) {
        slowMotionScale_ = timeScale;
        slowMotionDuration_ = duration;
        slowMotionElapsed_ = 0.0f;
    }
    
    // ===== 状態取得 =====
    
    /**
     * @brief 画面シェイクオフセット取得
     */
    Vector2 GetShakeOffset() const {
        return { shakeOffsetX_, shakeOffsetY_ };
    }
    
    /**
     * @brief タイムスケール取得
     */
    float GetTimeScale() const {
        if (slowMotionDuration_ > 0.0f && slowMotionElapsed_ < slowMotionDuration_) {
            return slowMotionScale_;
        }
        return 1.0f;
    }
    
    /**
     * @brief 定義レジストリを設定
     */
    void SetRegistry(Data::DefinitionRegistry* registry) {
        registry_ = registry;
    }

private:
    bool initialized_ = false;
    Data::DefinitionRegistry* registry_ = nullptr;
    
    std::vector<ParticleEffectInstance> particleEffects_;
    std::vector<ScreenEffectInstance> screenEffects_;
    
    std::mt19937 rng_;
    
    // 簡易画面エフェクト
    float shakeIntensity_ = 0.0f;
    float shakeDuration_ = 0.0f;
    float shakeElapsed_ = 0.0f;
    float shakeOffsetX_ = 0.0f;
    float shakeOffsetY_ = 0.0f;
    
    Color flashColor_ = WHITE;
    float flashDuration_ = 0.0f;
    float flashElapsed_ = 0.0f;
    
    float slowMotionScale_ = 1.0f;
    float slowMotionDuration_ = 0.0f;
    float slowMotionElapsed_ = 0.0f;
    
    // ===== イージング関数 =====
    
    float ApplyEasing(float t, Data::EaseType type) {
        switch (type) {
            case Data::EaseType::Linear: return t;
            case Data::EaseType::EaseIn: return t * t;
            case Data::EaseType::EaseOut: return 1.0f - (1.0f - t) * (1.0f - t);
            case Data::EaseType::EaseInOut:
                return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
            case Data::EaseType::EaseOutElastic: {
                if (t == 0.0f || t == 1.0f) return t;
                return std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * (2.0f * 3.14159f / 3.0f)) + 1.0f;
            }
            case Data::EaseType::EaseOutBounce: {
                const float n1 = 7.5625f;
                const float d1 = 2.75f;
                if (t < 1.0f / d1) return n1 * t * t;
                if (t < 2.0f / d1) { t -= 1.5f / d1; return n1 * t * t + 0.75f; }
                if (t < 2.5f / d1) { t -= 2.25f / d1; return n1 * t * t + 0.9375f; }
                t -= 2.625f / d1; return n1 * t * t + 0.984375f;
            }
            default: return t;
        }
    }
    
    // ===== 更新処理 =====
    
    void UpdateScreenEffects(float dt) {
        // 画面シェイク
        if (shakeDuration_ > 0.0f) {
            shakeElapsed_ += dt;
            if (shakeElapsed_ < shakeDuration_) {
                float progress = shakeElapsed_ / shakeDuration_;
                float intensity = shakeIntensity_ * (1.0f - progress);  // 減衰
                std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
                shakeOffsetX_ = dist(rng_) * intensity;
                shakeOffsetY_ = dist(rng_) * intensity;
            } else {
                shakeOffsetX_ = 0.0f;
                shakeOffsetY_ = 0.0f;
                shakeDuration_ = 0.0f;
            }
        }
        
        // フラッシュ
        if (flashDuration_ > 0.0f) {
            flashElapsed_ += dt;
            if (flashElapsed_ >= flashDuration_) {
                flashDuration_ = 0.0f;
            }
        }
        
        // スローモーション
        if (slowMotionDuration_ > 0.0f) {
            slowMotionElapsed_ += dt;
            if (slowMotionElapsed_ >= slowMotionDuration_) {
                slowMotionDuration_ = 0.0f;
                slowMotionScale_ = 1.0f;
            }
        }
        
        // 定義ベースのエフェクト
        for (auto& effect : screenEffects_) {
            if (!effect.active || !effect.def) continue;
            
            effect.elapsed += dt;
            float progress = std::min(effect.elapsed / effect.duration, 1.0f);
            float eased = ApplyEasing(progress, effect.def->easing);
            
            switch (effect.def->type) {
                case Data::ScreenEffectType::Shake:
                    if (effect.elapsed < effect.duration) {
                        float intensity = effect.def->shakeIntensity;
                        if (effect.def->shakeDecay) {
                            intensity *= (1.0f - eased);
                        }
                        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
                        effect.shakeOffsetX = dist(rng_) * intensity;
                        effect.shakeOffsetY = dist(rng_) * intensity;
                    }
                    break;
                    
                case Data::ScreenEffectType::Flash:
                    effect.flashAlpha = 1.0f - eased;
                    break;
                    
                case Data::ScreenEffectType::SlowMotion:
                    effect.timeScale = effect.def->timeScale;
                    break;
                    
                default:
                    break;
            }
            
            if (effect.elapsed >= effect.duration) {
                effect.active = false;
            }
        }
    }
    
    void UpdateParticleEffects(float dt) {
        for (auto& effect : particleEffects_) {
            if (!effect.active) continue;
            
            effect.elapsed += dt;
            
            // 終了チェック
            if (!effect.loop && effect.def && effect.elapsed >= effect.def->duration) {
                // 全エミッターの発生を停止
                for (auto& emitter : effect.emitters) {
                    emitter.emitting = false;
                }
            }
            
            bool hasActiveParticles = false;
            
            for (auto& emitter : effect.emitters) {
                if (!emitter.def) continue;
                
                // パーティクル発生
                if (emitter.emitting) {
                    // 連続発生
                    if (emitter.def->emissionMode == Data::EmissionMode::Continuous) {
                        emitter.emissionAccumulator += emitter.def->emissionRate * dt;
                        while (emitter.emissionAccumulator >= 1.0f) {
                            SpawnParticle(emitter, effect.x, effect.y, effect.scale);
                            emitter.emissionAccumulator -= 1.0f;
                        }
                    }
                    
                    // バースト発生
                    for (size_t i = emitter.burstIndex; i < emitter.def->bursts.size(); ++i) {
                        const auto& burst = emitter.def->bursts[i];
                        if (emitter.elapsed >= burst.time) {
                            std::uniform_int_distribution<int> countDist(burst.minCount, burst.maxCount);
                            int count = countDist(rng_);
                            for (int j = 0; j < count; ++j) {
                                SpawnParticle(emitter, effect.x, effect.y, effect.scale);
                            }
                            emitter.burstIndex = i + 1;
                        }
                    }
                }
                
                // パーティクル更新
                for (auto& p : emitter.particles) {
                    if (!p.active) continue;
                    
                    hasActiveParticles = true;
                    
                    // 寿命減少
                    p.lifetime -= dt;
                    if (p.lifetime <= 0.0f) {
                        p.active = false;
                        continue;
                    }
                    
                    // 進捗計算
                    float lifeProgress = 1.0f - (p.lifetime / p.maxLifetime);
                    
                    // 移動
                    p.vx += emitter.def->shape.width * dt;  // 簡易重力
                    p.vy += 98.0f * dt;  // デフォルト重力
                    p.x += p.vx * dt;
                    p.y += p.vy * dt;
                    
                    // 回転
                    p.rotation += p.rotationSpeed * dt;
                    
                    // サイズ変化
                    const auto& over = emitter.def->overLifetime;
                    float scaleT = ApplyEasing(lifeProgress, over.scaleEasing);
                    p.scale = over.startScale + (over.endScale - over.startScale) * scaleT;
                    
                    // アルファ変化
                    float alphaT = ApplyEasing(lifeProgress, over.alphaEasing);
                    float alpha = over.startAlpha + (over.endAlpha - over.startAlpha) * alphaT;
                    p.color.a = static_cast<unsigned char>(alpha * 255);
                }
            }
            
            // 全パーティクルが非アクティブで発生も停止なら終了
            if (!hasActiveParticles) {
                bool stillEmitting = false;
                for (const auto& e : effect.emitters) {
                    if (e.emitting) {
                        stillEmitting = true;
                        break;
                    }
                }
                if (!stillEmitting) {
                    effect.active = false;
                }
            }
        }
    }
    
    void SpawnParticle(ParticleEmitterInstance& emitter, float effectX, float effectY, float effectScale) {
        const auto& def = *emitter.def;
        
        // 空きパーティクルを探す
        Particle* p = nullptr;
        for (auto& particle : emitter.particles) {
            if (!particle.active) {
                p = &particle;
                break;
            }
        }
        if (!p) return;  // 満杯
        
        // 初期化
        p->active = true;
        
        // 位置（エミッター形状に基づく）
        float spawnX = 0.0f, spawnY = 0.0f;
        switch (def.shape.shape) {
            case Data::EmitterShape::Point:
                break;
            case Data::EmitterShape::Circle: {
                std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
                std::uniform_real_distribution<float> radiusDist(0.0f, def.shape.radius);
                float a = angleDist(rng_);
                float r = def.shape.edgeOnly ? def.shape.radius : radiusDist(rng_);
                spawnX = std::cos(a) * r;
                spawnY = std::sin(a) * r;
                break;
            }
            case Data::EmitterShape::Rectangle: {
                std::uniform_real_distribution<float> xDist(-def.shape.width / 2, def.shape.width / 2);
                std::uniform_real_distribution<float> yDist(-def.shape.height / 2, def.shape.height / 2);
                spawnX = xDist(rng_);
                spawnY = yDist(rng_);
                break;
            }
            default:
                break;
        }
        
        p->x = effectX + spawnX * effectScale;
        p->y = effectY + spawnY * effectScale;
        
        // 寿命
        std::uniform_real_distribution<float> lifeDist(def.lifetime.min, def.lifetime.max);
        p->lifetime = lifeDist(rng_);
        p->maxLifetime = p->lifetime;
        
        // 速度
        std::uniform_real_distribution<float> speedDist(def.speed.min, def.speed.max);
        std::uniform_real_distribution<float> angleDist(def.angle.min, def.angle.max);
        float speed = speedDist(rng_) * effectScale;
        float angle = angleDist(rng_) * 3.14159f / 180.0f;
        p->vx = std::cos(angle) * speed;
        p->vy = std::sin(angle) * speed;
        
        // スケール
        std::uniform_real_distribution<float> scaleDist(def.scale.min, def.scale.max);
        p->scale = scaleDist(rng_) * effectScale;
        
        // 回転
        std::uniform_real_distribution<float> rotDist(def.rotation.min, def.rotation.max);
        p->rotation = rotDist(rng_);
        
        std::uniform_real_distribution<float> rotSpeedDist(
            def.overLifetime.rotationSpeed.min,
            def.overLifetime.rotationSpeed.max
        );
        p->rotationSpeed = rotSpeedDist(rng_);
        
        // 色
        p->color = {
            static_cast<unsigned char>(def.overLifetime.startColor.r * 255),
            static_cast<unsigned char>(def.overLifetime.startColor.g * 255),
            static_cast<unsigned char>(def.overLifetime.startColor.b * 255),
            static_cast<unsigned char>(def.overLifetime.startAlpha * 255)
        };
    }
    
    // ===== 描画処理 =====
    
    void RenderParticles() {
        for (const auto& effect : particleEffects_) {
            if (!effect.active) continue;
            
            for (const auto& emitter : effect.emitters) {
                if (!emitter.def) continue;
                
                // ブレンドモード設定
                switch (emitter.def->blendMode) {
                    case Data::BlendMode::Additive:
                        BeginBlendMode(BLEND_ADDITIVE);
                        break;
                    case Data::BlendMode::Multiply:
                        BeginBlendMode(BLEND_MULTIPLIED);
                        break;
                    default:
                        BeginBlendMode(BLEND_ALPHA);
                        break;
                }
                
                for (const auto& p : emitter.particles) {
                    if (!p.active) continue;
                    
                    // 簡易パーティクル描画（テクスチャなし）
                    float size = 8.0f * p.scale;
                    DrawCircle(
                        static_cast<int>(p.x),
                        static_cast<int>(p.y),
                        size,
                        p.color
                    );
                }
                
                EndBlendMode();
            }
        }
    }
    
    void RenderScreenEffects() {
        // フラッシュ
        if (flashDuration_ > 0.0f && flashElapsed_ < flashDuration_) {
            float progress = flashElapsed_ / flashDuration_;
            float alpha = 1.0f - progress;
            Color c = flashColor_;
            c.a = static_cast<unsigned char>(c.a * alpha);
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), c);
        }
        
        // 定義ベースのフラッシュ
        for (const auto& effect : screenEffects_) {
            if (!effect.active || !effect.def) continue;
            
            if (effect.def->type == Data::ScreenEffectType::Flash) {
                Color c = {
                    static_cast<unsigned char>(effect.def->flashColor.r * 255),
                    static_cast<unsigned char>(effect.def->flashColor.g * 255),
                    static_cast<unsigned char>(effect.def->flashColor.b * 255),
                    static_cast<unsigned char>(effect.flashAlpha * 255)
                };
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), c);
            }
        }
    }
    
    void CleanupInactiveEffects() {
        // パーティクルエフェクト
        particleEffects_.erase(
            std::remove_if(particleEffects_.begin(), particleEffects_.end(),
                [](const ParticleEffectInstance& e) { return !e.active; }),
            particleEffects_.end()
        );
        
        // 画面エフェクト
        screenEffects_.erase(
            std::remove_if(screenEffects_.begin(), screenEffects_.end(),
                [](const ScreenEffectInstance& e) { return !e.active; }),
            screenEffects_.end()
        );
    }
};

} // namespace Core
