/**
 * @file SoundDef.h
 * @brief サウンド定義構造体
 * 
 * サウンドエフェクト、BGM、サウンドバンクの定義。
 * JSONから読み込まれるサウンド設定を表現。
 */
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace Data {

// ===== サウンドタイプ =====

/**
 * @brief サウンドの種類
 */
enum class SoundType {
    SFX,        // 効果音
    Voice,      // ボイス
    Ambient,    // 環境音
    UI,         // UI音
    Music,      // BGM
};

/**
 * @brief サウンドの優先度
 */
enum class SoundPriority {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3,  // 常に再生（爆発、重要なボイス等）
};

/**
 * @brief フェードタイプ
 */
enum class FadeType {
    None,       // フェードなし
    Linear,     // 線形フェード
    EaseIn,     // イーズイン
    EaseOut,    // イーズアウト
    EaseInOut,  // イーズインアウト
};

// ===== 基本サウンド定義 =====

/**
 * @brief サウンドバリエーション
 * 
 * 同じサウンドの複数バリエーションを表現。
 * ランダムまたは順番で再生される。
 */
struct SoundVariation {
    std::string filePath;       // サウンドファイルパス
    float weight = 1.0f;        // 選択確率の重み
    float pitchOffset = 0.0f;   // ピッチオフセット（-1.0〜1.0）
    float volumeOffset = 0.0f;  // ボリュームオフセット（-1.0〜1.0）
};

/**
 * @brief サウンドエフェクト定義
 */
struct SoundDef {
    std::string id;
    std::string name;
    SoundType type = SoundType::SFX;
    SoundPriority priority = SoundPriority::Normal;
    
    // === ファイル ===
    std::vector<SoundVariation> variations;  // バリエーション（複数可）
    
    // === 再生設定 ===
    float volume = 1.0f;            // 基本ボリューム（0.0〜1.0）
    float pitch = 1.0f;             // 基本ピッチ（0.5〜2.0）
    float pitchVariation = 0.0f;    // ピッチランダム幅
    float volumeVariation = 0.0f;   // ボリュームランダム幅
    bool loop = false;              // ループ再生
    
    // === 3Dサウンド ===
    bool is3D = false;              // 3D音源かどうか
    float minDistance = 1.0f;       // 最小距離（これより近いと最大音量）
    float maxDistance = 100.0f;     // 最大距離（これより遠いと無音）
    float rolloffFactor = 1.0f;     // 減衰係数
    
    // === 再生制限 ===
    int maxInstances = 4;           // 同時再生数上限
    float cooldown = 0.0f;          // 再生間隔（秒）
    bool stopOldest = true;         // 上限時に最古を停止
    
    // === フェード ===
    float fadeInTime = 0.0f;        // フェードイン時間
    float fadeOutTime = 0.0f;       // フェードアウト時間
    FadeType fadeType = FadeType::Linear;
    
    // === グループ ===
    std::string group;              // サウンドグループ名（音量調整用）
    std::vector<std::string> tags;  // タグ（フィルタリング用）
};

// ===== BGM定義 =====

/**
 * @brief BGMループ設定
 */
struct MusicLoopSettings {
    bool enabled = true;        // ループ有効
    float loopStart = 0.0f;     // ループ開始位置（秒）
    float loopEnd = 0.0f;       // ループ終了位置（秒、0=末尾）
    int loopCount = -1;         // ループ回数（-1=無限）
};

/**
 * @brief BGMレイヤー
 * 
 * インタラクティブミュージックのレイヤー定義。
 */
struct MusicLayer {
    std::string id;
    std::string filePath;
    float volume = 1.0f;
    bool enabled = true;
    std::string condition;      // 有効条件（例: "intensity > 0.5"）
};

/**
 * @brief BGM定義
 */
struct MusicDef {
    std::string id;
    std::string name;
    std::string filePath;       // メインBGMファイル
    
    // === 基本設定 ===
    float volume = 0.8f;
    float bpm = 120.0f;         // テンポ（ビート同期用）
    int beatsPerBar = 4;        // 拍子
    
    // === ループ設定 ===
    MusicLoopSettings loop;
    
    // === イントロ/アウトロ ===
    std::string introFilePath;  // イントロファイル
    std::string outroFilePath;  // アウトロファイル
    
    // === トランジション ===
    float crossfadeDuration = 2.0f;  // クロスフェード時間
    FadeType crossfadeType = FadeType::EaseInOut;
    
    // === レイヤー（インタラクティブミュージック） ===
    std::vector<MusicLayer> layers;
    
    // === グループ ===
    std::string group = "music";
    std::vector<std::string> tags;
};

// ===== サウンドバンク定義 =====

/**
 * @brief サウンドキュー（イベントベースの再生）
 */
struct SoundCue {
    std::string id;
    std::string soundId;        // 再生するサウンドID
    float delay = 0.0f;         // 遅延時間
    float probability = 1.0f;   // 再生確率
    
    // 条件付き再生
    std::string condition;      // 再生条件
};

/**
 * @brief サウンドイベント
 * 
 * ゲームイベントに対応するサウンド群。
 */
struct SoundEvent {
    std::string id;
    std::string name;
    std::vector<SoundCue> cues;
    
    // 再生モード
    enum class PlayMode {
        All,        // 全て再生
        Random,     // ランダムに1つ
        Sequence,   // 順番に再生
    } playMode = PlayMode::All;
    
    float cooldown = 0.0f;      // イベント発火間隔
};

/**
 * @brief サウンドバンク定義
 * 
 * 関連するサウンドをグループ化したもの。
 */
struct SoundBankDef {
    std::string id;
    std::string name;
    
    // 含まれるサウンド
    std::vector<std::string> soundIds;
    std::vector<std::string> musicIds;
    
    // サウンドイベント
    std::unordered_map<std::string, SoundEvent> events;
    
    // バンク設定
    bool preload = false;       // 起動時にプリロード
    bool persistent = false;    // メモリに常駐
    
    // タグ
    std::vector<std::string> tags;
};

// ===== サウンドグループ設定 =====

/**
 * @brief サウンドグループ設定
 * 
 * カテゴリごとの音量設定。
 */
struct SoundGroupSettings {
    std::string id;
    std::string name;
    float volume = 1.0f;        // グループ音量
    bool muted = false;         // ミュート
    int maxInstances = 16;      // 同時再生数上限
    SoundPriority minPriority = SoundPriority::Low;  // 最低優先度
};

// ===== ヘルパー関数 =====

/**
 * @brief SoundType文字列変換
 */
inline SoundType StringToSoundType(const std::string& str) {
    if (str == "sfx" || str == "SFX") return SoundType::SFX;
    if (str == "voice" || str == "Voice") return SoundType::Voice;
    if (str == "ambient" || str == "Ambient") return SoundType::Ambient;
    if (str == "ui" || str == "UI") return SoundType::UI;
    if (str == "music" || str == "Music") return SoundType::Music;
    return SoundType::SFX;
}

inline std::string SoundTypeToString(SoundType type) {
    switch (type) {
        case SoundType::SFX: return "sfx";
        case SoundType::Voice: return "voice";
        case SoundType::Ambient: return "ambient";
        case SoundType::UI: return "ui";
        case SoundType::Music: return "music";
    }
    return "sfx";
}

/**
 * @brief SoundPriority文字列変換
 */
inline SoundPriority StringToSoundPriority(const std::string& str) {
    if (str == "low" || str == "Low") return SoundPriority::Low;
    if (str == "normal" || str == "Normal") return SoundPriority::Normal;
    if (str == "high" || str == "High") return SoundPriority::High;
    if (str == "critical" || str == "Critical") return SoundPriority::Critical;
    return SoundPriority::Normal;
}

/**
 * @brief FadeType文字列変換
 */
inline FadeType StringToFadeType(const std::string& str) {
    if (str == "none" || str == "None") return FadeType::None;
    if (str == "linear" || str == "Linear") return FadeType::Linear;
    if (str == "easeIn" || str == "EaseIn") return FadeType::EaseIn;
    if (str == "easeOut" || str == "EaseOut") return FadeType::EaseOut;
    if (str == "easeInOut" || str == "EaseInOut") return FadeType::EaseInOut;
    return FadeType::Linear;
}

} // namespace Data
