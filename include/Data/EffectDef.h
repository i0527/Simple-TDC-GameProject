/**
 * @file EffectDef.h
 * @brief エフェクト定義構造体
 * 
 * パーティクルエフェクト、視覚エフェクト、画面エフェクトの定義。
 * JSONから読み込まれるエフェクト設定を表現。
 */
#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <unordered_map>

namespace Data {

// ===== 基本型 =====

/**
 * @brief 2Dベクトル（値または範囲）
 */
struct Vec2Range {
    float minX = 0.0f;
    float maxX = 0.0f;
    float minY = 0.0f;
    float maxY = 0.0f;
    
    // 単一値コンストラクタ
    static Vec2Range Single(float x, float y) {
        return Vec2Range{x, x, y, y};
    }
    
    // 範囲コンストラクタ
    static Vec2Range Range(float minX, float maxX, float minY, float maxY) {
        return Vec2Range{minX, maxX, minY, maxY};
    }
};

/**
 * @brief 数値範囲
 */
struct FloatRange {
    float min = 0.0f;
    float max = 0.0f;
    
    static FloatRange Single(float v) { return FloatRange{v, v}; }
    static FloatRange Range(float min, float max) { return FloatRange{min, max}; }
};

/**
 * @brief カラー（RGBA）
 */
struct ColorDef {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
    
    static ColorDef White() { return ColorDef{1, 1, 1, 1}; }
    static ColorDef FromHex(unsigned int hex) {
        return ColorDef{
            ((hex >> 16) & 0xFF) / 255.0f,
            ((hex >> 8) & 0xFF) / 255.0f,
            (hex & 0xFF) / 255.0f,
            1.0f
        };
    }
};

/**
 * @brief カラー範囲（グラデーション/ランダム）
 */
struct ColorRange {
    ColorDef start;
    ColorDef end;
    bool isGradient = false;  // trueならグラデーション、falseならランダム
};

// ===== イージング =====

/**
 * @brief イージング関数タイプ
 */
enum class EaseType {
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,
    EaseInElastic,
    EaseOutElastic,
    EaseInBounce,
    EaseOutBounce,
};

// ===== ブレンドモード =====

/**
 * @brief ブレンドモード
 */
enum class BlendMode {
    Alpha,      // 通常アルファブレンド
    Additive,   // 加算ブレンド
    Multiply,   // 乗算ブレンド
    Screen,     // スクリーンブレンド
};

// ===== パーティクル定義 =====

/**
 * @brief エミッター形状
 */
enum class EmitterShape {
    Point,      // 点
    Circle,     // 円
    Rectangle,  // 矩形
    Line,       // 線
    Ring,       // リング（円周上）
    Cone,       // コーン（放射状）
};

/**
 * @brief エミッター形状設定
 */
struct EmitterShapeDef {
    EmitterShape shape = EmitterShape::Point;
    float width = 0.0f;         // 幅（Rectangle/Line）
    float height = 0.0f;        // 高さ（Rectangle）
    float radius = 0.0f;        // 半径（Circle/Ring/Cone）
    float innerRadius = 0.0f;   // 内径（Ring）
    float angle = 360.0f;       // 角度（Cone）
    float rotation = 0.0f;      // 回転
    bool edgeOnly = false;      // 縁のみから発生
};

/**
 * @brief パーティクル発生モード
 */
enum class EmissionMode {
    Continuous,     // 連続発生
    Burst,          // バースト（一度に複数）
    Distance,       // 移動距離ベース
};

/**
 * @brief バースト設定
 */
struct BurstDef {
    float time = 0.0f;          // 発生タイミング
    int minCount = 1;           // 最小数
    int maxCount = 1;           // 最大数
    float interval = 0.0f;      // 繰り返し間隔（0=1回のみ）
    int cycles = 1;             // 繰り返し回数
};

/**
 * @brief パーティクルのライフタイム中の変化
 */
struct ParticleOverLifetime {
    // サイズ変化
    float startScale = 1.0f;
    float endScale = 1.0f;
    EaseType scaleEasing = EaseType::Linear;
    
    // 色変化
    ColorDef startColor = ColorDef::White();
    ColorDef endColor = ColorDef::White();
    EaseType colorEasing = EaseType::Linear;
    
    // アルファ変化
    float startAlpha = 1.0f;
    float endAlpha = 0.0f;
    EaseType alphaEasing = EaseType::Linear;
    
    // 回転変化
    FloatRange rotationSpeed = FloatRange::Single(0.0f);
};

/**
 * @brief パーティクルエミッター定義
 */
struct ParticleEmitterDef {
    std::string id;
    std::string name;
    
    // === スプライト ===
    std::string textureId;          // テクスチャID
    int spriteIndex = 0;            // スプライトシート内のインデックス
    bool animated = false;          // アニメーションするか
    int frameCount = 1;             // アニメーションフレーム数
    float frameRate = 10.0f;        // アニメーションFPS
    
    // === エミッター設定 ===
    EmitterShapeDef shape;
    EmissionMode emissionMode = EmissionMode::Continuous;
    float emissionRate = 10.0f;     // 秒あたりの発生数
    std::vector<BurstDef> bursts;   // バースト設定
    
    // === 初期値 ===
    FloatRange lifetime = FloatRange::Range(1.0f, 2.0f);
    FloatRange speed = FloatRange::Range(50.0f, 100.0f);
    FloatRange angle = FloatRange::Range(0.0f, 360.0f);  // 発射角度
    FloatRange scale = FloatRange::Single(1.0f);
    FloatRange rotation = FloatRange::Single(0.0f);
    ColorRange color;
    
    // === ライフタイム中の変化 ===
    ParticleOverLifetime overLifetime;
    
    // === 物理 ===
    Vec2Range gravity = Vec2Range::Single(0.0f, 0.0f);
    float drag = 0.0f;              // 空気抵抗
    float velocityDamping = 1.0f;   // 速度減衰
    
    // === レンダリング ===
    BlendMode blendMode = BlendMode::Additive;
    int sortingOrder = 0;
    bool worldSpace = true;         // ワールド空間座標
    
    // === 制限 ===
    int maxParticles = 100;
};

/**
 * @brief パーティクルエフェクト定義（複数エミッターの組み合わせ）
 */
struct ParticleEffectDef {
    std::string id;
    std::string name;
    
    // 含まれるエミッター
    std::vector<ParticleEmitterDef> emitters;
    
    // === 全体設定 ===
    float duration = 1.0f;          // エフェクト全体の長さ
    bool loop = false;              // ループ
    bool autoDestroy = true;        // 終了時に自動破棄
    float scale = 1.0f;             // 全体スケール
    
    // === サウンド連携 ===
    std::string startSoundId;       // 開始時サウンド
    std::string endSoundId;         // 終了時サウンド
    
    // === タグ ===
    std::vector<std::string> tags;
};

// ===== 視覚エフェクト定義 =====

/**
 * @brief スプライトエフェクトタイプ
 */
enum class SpriteEffectType {
    Flash,          // フラッシュ（一瞬白く）
    ColorTint,      // 色変更
    FadeIn,         // フェードイン
    FadeOut,        // フェードアウト
    Scale,          // スケール変化
    Shake,          // 振動
    Pulse,          // パルス（拡大縮小繰り返し）
};

/**
 * @brief スプライトエフェクト定義
 */
struct SpriteEffectDef {
    std::string id;
    SpriteEffectType type = SpriteEffectType::Flash;
    
    float duration = 0.2f;          // エフェクト時間
    EaseType easing = EaseType::Linear;
    
    // Flash/ColorTint用
    ColorDef color = ColorDef::White();
    float intensity = 1.0f;         // 強度
    
    // Scale用
    float startScale = 1.0f;
    float endScale = 1.0f;
    
    // Shake用
    float shakeIntensity = 5.0f;
    float shakeFrequency = 30.0f;
    
    // Pulse用
    float pulseMin = 0.9f;
    float pulseMax = 1.1f;
    float pulseSpeed = 2.0f;
    
    bool loop = false;
};

// ===== 画面エフェクト定義 =====

/**
 * @brief 画面エフェクトタイプ
 */
enum class ScreenEffectType {
    Shake,          // 画面揺れ
    Flash,          // 画面フラッシュ
    FadeIn,         // フェードイン
    FadeOut,        // フェードアウト
    Vignette,       // ビネット
    ColorGrading,   // カラーグレーディング
    Zoom,           // ズーム
    Blur,           // ブラー
    Chromatic,      // 色収差
    SlowMotion,     // スローモーション
};

/**
 * @brief 画面エフェクト定義
 */
struct ScreenEffectDef {
    std::string id;
    std::string name;
    ScreenEffectType type = ScreenEffectType::Shake;
    
    float duration = 0.5f;
    EaseType easing = EaseType::EaseOut;
    
    // Shake用
    float shakeIntensity = 10.0f;
    float shakeFrequency = 20.0f;
    bool shakeDecay = true;         // 減衰するか
    
    // Flash用
    ColorDef flashColor = ColorDef::White();
    
    // Fade用
    ColorDef fadeColor = {0, 0, 0, 1};  // デフォルト黒
    
    // Vignette用
    float vignetteIntensity = 0.5f;
    float vignetteSmoothness = 0.5f;
    
    // ColorGrading用
    float saturation = 1.0f;
    float contrast = 1.0f;
    float brightness = 1.0f;
    ColorDef colorTint = ColorDef::White();
    
    // Zoom用
    float zoomAmount = 1.2f;
    Vec2Range zoomCenter = Vec2Range::Single(0.5f, 0.5f);  // 正規化座標
    
    // Blur用
    float blurRadius = 5.0f;
    
    // Chromatic用
    float chromaticIntensity = 0.02f;
    
    // SlowMotion用
    float timeScale = 0.5f;
};

// ===== 複合エフェクト定義 =====

/**
 * @brief エフェクトエントリ（タイミング付き）
 */
struct EffectEntry {
    std::string effectId;           // 参照するエフェクトID
    float startTime = 0.0f;         // 開始タイミング
    Vec2Range offset = Vec2Range::Single(0, 0);  // 位置オフセット
    float scale = 1.0f;             // スケール
};

/**
 * @brief 複合エフェクト定義
 * 
 * パーティクル、スプライト、画面エフェクト、サウンドを
 * タイムライン上で組み合わせたもの。
 */
struct CompositeEffectDef {
    std::string id;
    std::string name;
    
    // 各種エフェクト
    std::vector<EffectEntry> particles;
    std::vector<EffectEntry> sprites;
    std::vector<EffectEntry> screenEffects;
    
    // サウンド
    struct SoundEntry {
        std::string soundId;
        float startTime = 0.0f;
    };
    std::vector<SoundEntry> sounds;
    
    // 全体設定
    float duration = 1.0f;
    bool loop = false;
    
    // タグ
    std::vector<std::string> tags;
};

// ===== ヘルパー関数 =====

inline EaseType StringToEaseType(const std::string& str) {
    if (str == "linear" || str == "Linear") return EaseType::Linear;
    if (str == "easeIn" || str == "EaseIn") return EaseType::EaseIn;
    if (str == "easeOut" || str == "EaseOut") return EaseType::EaseOut;
    if (str == "easeInOut" || str == "EaseInOut") return EaseType::EaseInOut;
    if (str == "easeInQuad") return EaseType::EaseInQuad;
    if (str == "easeOutQuad") return EaseType::EaseOutQuad;
    if (str == "easeInOutQuad") return EaseType::EaseInOutQuad;
    if (str == "easeInCubic") return EaseType::EaseInCubic;
    if (str == "easeOutCubic") return EaseType::EaseOutCubic;
    if (str == "easeInOutCubic") return EaseType::EaseInOutCubic;
    if (str == "easeInElastic") return EaseType::EaseInElastic;
    if (str == "easeOutElastic") return EaseType::EaseOutElastic;
    if (str == "easeInBounce") return EaseType::EaseInBounce;
    if (str == "easeOutBounce") return EaseType::EaseOutBounce;
    return EaseType::Linear;
}

inline BlendMode StringToBlendMode(const std::string& str) {
    if (str == "alpha" || str == "Alpha") return BlendMode::Alpha;
    if (str == "additive" || str == "Additive") return BlendMode::Additive;
    if (str == "multiply" || str == "Multiply") return BlendMode::Multiply;
    if (str == "screen" || str == "Screen") return BlendMode::Screen;
    return BlendMode::Alpha;
}

inline EmitterShape StringToEmitterShape(const std::string& str) {
    if (str == "point" || str == "Point") return EmitterShape::Point;
    if (str == "circle" || str == "Circle") return EmitterShape::Circle;
    if (str == "rectangle" || str == "Rectangle") return EmitterShape::Rectangle;
    if (str == "line" || str == "Line") return EmitterShape::Line;
    if (str == "ring" || str == "Ring") return EmitterShape::Ring;
    if (str == "cone" || str == "Cone") return EmitterShape::Cone;
    return EmitterShape::Point;
}

inline ScreenEffectType StringToScreenEffectType(const std::string& str) {
    if (str == "shake" || str == "Shake") return ScreenEffectType::Shake;
    if (str == "flash" || str == "Flash") return ScreenEffectType::Flash;
    if (str == "fadeIn" || str == "FadeIn") return ScreenEffectType::FadeIn;
    if (str == "fadeOut" || str == "FadeOut") return ScreenEffectType::FadeOut;
    if (str == "vignette" || str == "Vignette") return ScreenEffectType::Vignette;
    if (str == "colorGrading" || str == "ColorGrading") return ScreenEffectType::ColorGrading;
    if (str == "zoom" || str == "Zoom") return ScreenEffectType::Zoom;
    if (str == "blur" || str == "Blur") return ScreenEffectType::Blur;
    if (str == "chromatic" || str == "Chromatic") return ScreenEffectType::Chromatic;
    if (str == "slowMotion" || str == "SlowMotion") return ScreenEffectType::SlowMotion;
    return ScreenEffectType::Shake;
}

} // namespace Data
