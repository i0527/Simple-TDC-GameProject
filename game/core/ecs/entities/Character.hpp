#pragma once

#include <string>
#include <vector>
#include "../../config/RenderTypes.hpp"

namespace game {
namespace core {
namespace entities {

// 攻撃タイプ
enum class AttackType {
    Single,     // 単体攻撃
    Range,      // 範囲攻撃
    Line,       // 直線攻撃
};

// エフェクトタイプ
enum class EffectType {
    Normal,     // 通常攻撃エフェクト
    Fire,       // 炎エフェクト
    Ice,        // 氷エフェクト
    Lightning,  // 雷エフェクト
    Heal,       // 回復エフェクト
};

// パッシブ効果タイプ
enum class PassiveEffectType {
    Percentage, // 割合（例: +10% => value=0.10）
    Flat        // 固定値（例: +15 => value=15）
};

// パッシブ対象ステータス
enum class PassiveTargetStat {
    Attack,      // 攻撃力
    Defense,     // 防御力
    Hp,          // 最大HP
    MoveSpeed,   // 移動速度
    AttackSpeed, // 攻撃速度（attack_spanに反映）
    Range,       // 攻撃射程（attack_size.xに反映）
    CritChance,  // クリティカル率（未接続: 将来拡張用）
    CritDamage,  // クリティカルダメージ（未接続: 将来拡張用）
    GoldGain,    // ゴールド獲得（未接続: 将来拡張用）
    ExpGain      // 経験値獲得（未接続: 将来拡張用）
};

// パッシブスキル定義
struct PassiveSkill {
    std::string id;           // スキルID
    std::string name;         // スキル名
    std::string description;  // スキル説明
    float value;              // パラメータ値（加算 or 乗算）
    PassiveEffectType effect_type = PassiveEffectType::Percentage; // 効果タイプ
    PassiveTargetStat target_stat = PassiveTargetStat::Attack;     // 対象ステータス
    int rarity = 1;           // レアリティ (1-5)
};

// 装備定義
struct Equipment {
    std::string id;           // 装備ID
    std::string name;         // 装備名
    std::string description;  // 装備説明
    std::string icon_path;    // UIアイコンパス
    float attack_bonus;       // 攻撃力ボーナス
    float defense_bonus;      // 防御力ボーナス
    float hp_bonus;           // HP ボーナス
};

// ***** キャラクター構造体（共通） *****
struct Character {
    // ===== 基本情報 =====
    std::string id;           // キャラクターID（ユニーク）
    std::string name;         // キャラクター名
    int rarity;               // レアリティ（1-5）
    int default_level = 1;    // 初期レベル（マスターの初期値）

    // ===== ステータス =====
    int hp;                   // HP
    int attack;               // 攻撃力
    int defense;              // 防御力
    float move_speed;         // 移動速度（ピクセル/秒）
    float attack_span;        // 攻撃スパン（秒）

    // ===== 攻撃設定 =====
    AttackType attack_type;   // 攻撃タイプ
    Vector2 attack_size;      // (攻撃有効範囲の距離, キャラ-攻撃有効範囲距離)
    EffectType effect_type;   // エフェクトタイプ
    float attack_hit_time = 0.0f;  // 攻撃開始からのヒット発生タイミング（秒）

    // ===== UIリソース =====
    std::string icon_path;    // UIアイコンパス

    // ===== スプライト情報 =====
    struct SpriteInfo {
        std::string sheet_path;     // スプライトシートパス
        int frame_width;            // 1フレームの幅
        int frame_height;           // 1フレームの高さ
        int frame_count;            // 総フレーム数
        float frame_duration;       // 1フレームの表示時間（秒）
    };

    SpriteInfo move_sprite;   // 移動スプライト情報
    SpriteInfo attack_sprite; // 攻撃スプライト情報

    // ===== スキル・装備 =====
    std::vector<PassiveSkill> default_passive_skills;  // 初期パッシブ
    std::vector<Equipment> default_equipment;          // 初期装備

    // ===== オプショナル情報 =====
    std::string description;  // キャラクター説明
    std::string rarity_name;  // レアリティ名（N, R, SR, SSRなど）

    // ===== 図鑑情報 =====
    int cost = 1;                      // 編成時のコスト消費量
    bool default_unlocked = false;     // 初期解放フラグ

    // ===== コンストラクタ =====
    Character() = default;
    ~Character() = default;

    // ===== ユーティリティメソッド =====
    
    // 総攻撃力を計算（装備ボーナス込み）
    int GetTotalAttack() const;
    
    // 総HPを計算（装備ボーナス込み）
    int GetTotalHP() const;

    // 総防御力を計算（装備ボーナス込み）
    int GetTotalDefense() const;

    // アニメーション総フレーム数取得
    int GetMoveFrameCount() const { return move_sprite.frame_count; }
    int GetAttackFrameCount() const { return attack_sprite.frame_count; }
};

} // namespace entities
} // namespace core
} // namespace game
