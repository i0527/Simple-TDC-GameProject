/**
 * @file Definitions.h
 * @brief データ駆動設計のための定義構造体
 * 
 * JSONから読み込まれるキャラクター、ステージ、スキルなどの
 * 静的データを表現する構造体群。
 * これらはランタイムで変更されない「設計図」として機能する。
 */
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <variant>

namespace Data {

// ===== 基本型 =====

/**
 * @brief 2D矩形（ヒットボックス、エフェクトエリア等）
 */
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

/**
 * @brief アニメーションフレーム定義
 */
struct FrameDef {
    int index = 0;           // スプライトシート内のフレーム番号
    float duration = 0.1f;   // フレーム表示時間（秒）
    std::string tag;         // オプショナルタグ（"attack_hit" 等）
};

/**
 * @brief アニメーション定義
 */
struct AnimationDef {
    std::string name;
    std::vector<FrameDef> frames;
    bool loop = true;
    std::string nextAnimation;  // ループしない場合の次アニメーション
};

// ===== ステータス効果定義 =====

/**
 * @brief ステータス効果の種類
 */
enum class StatusEffectType {
    None,
    Slow,           // 移動速度低下
    Stun,           // 行動不能
    Poison,         // 継続ダメージ
    Burn,           // 継続ダメージ（炎）
    Freeze,         // 凍結（行動不能＋ダメージ増加）
    AttackUp,       // 攻撃力上昇
    AttackDown,     // 攻撃力低下
    DefenseUp,      // 防御力上昇
    DefenseDown,    // 防御力低下
    SpeedUp,        // 移動速度上昇
    Regeneration,   // 継続回復
    Shield,         // ダメージ吸収
    Invincible,     // 無敵
};

/**
 * @brief ステータス効果定義
 */
struct StatusEffectDef {
    std::string id;
    StatusEffectType type = StatusEffectType::None;
    float value = 0.0f;         // 効果値（%またはフラット値）
    float duration = 0.0f;      // 効果時間（秒）
    float tickInterval = 0.0f;  // 継続ダメージ/回復の間隔
    bool isPercentage = true;   // 値が%かどうか
    std::string iconPath;       // UIアイコン
};

// ===== スキル定義 =====

/**
 * @brief スキルのターゲットタイプ
 */
enum class SkillTargetType {
    Self,           // 自分
    SingleEnemy,    // 敵単体
    SingleAlly,     // 味方単体
    AllEnemies,     // 敵全体
    AllAllies,      // 味方全体
    Area,           // 範囲
};

/**
 * @brief スキル効果の種類
 */
enum class SkillEffectType {
    Damage,         // ダメージ
    Heal,           // 回復
    StatusApply,    // ステータス効果付与
    Summon,         // 召喚
    Knockback,      // ノックバック
    Pull,           // 引き寄せ
};

/**
 * @brief スキル効果定義
 */
struct SkillEffectDef {
    SkillEffectType type = SkillEffectType::Damage;
    float value = 0.0f;             // ダメージ量/回復量等
    bool isPercentage = false;      // 値が%かどうか
    std::string statusEffectId;     // StatusApplyの場合
    std::string summonCharacterId;  // Summonの場合
    int summonCount = 1;            // 召喚数
};

/**
 * @brief スキル定義
 */
struct SkillDef {
    std::string id;
    std::string name;
    std::string description;
    
    // 発動条件
    float cooldown = 10.0f;             // クールダウン（秒）
    float activationChance = 1.0f;      // 発動確率（0.0-1.0）
    bool activateOnAttack = false;      // 攻撃時に発動
    bool activateOnDamaged = false;     // 被ダメージ時に発動
    bool activateOnDeath = false;       // 死亡時に発動
    float healthThreshold = 0.0f;       // HP閾値（これ以下で発動）
    
    // ターゲティング
    SkillTargetType targetType = SkillTargetType::SingleEnemy;
    Rect effectArea;                    // Area型の場合
    int maxTargets = 1;                 // 最大ターゲット数
    
    // 効果
    std::vector<SkillEffectDef> effects;
    
    // アニメーション
    std::string animationName;          // スキル発動アニメーション
    std::string effectSpritePath;       // エフェクトスプライト
};

// ===== キャラクター定義 =====

/**
 * @brief キャラクターのレアリティ
 */
enum class Rarity {
    Normal,
    Rare,
    SuperRare,
    UberRare,
    Legend,
};

/**
 * @brief キャラクターの攻撃タイプ
 */
enum class AttackType {
    Single,     // 単体攻撃
    Area,       // 範囲攻撃
    Wave,       // 波動（貫通）
};

/**
 * @brief キャラクター定義
 * 
 * JSONからロードされるキャラクターの全パラメータ。
 * ランタイムで変更されない静的データ。
 */
struct CharacterDef {
    // === 識別 ===
    std::string id;              // 一意のID（例: "cupslime"）
    std::string name;            // 表示名
    std::string description;     // 説明文
    Rarity rarity = Rarity::Normal;
    std::vector<std::string> traits;  // 特性タグ（"floating", "metal" 等）
    
    // === スプライト ===
    std::string spritePath;      // スプライトシートのパス
    int frameWidth = 64;
    int frameHeight = 64;
    int framesPerRow = 8;
    float scale = 1.0f;
    
    // === アニメーション ===
    std::unordered_map<std::string, AnimationDef> animations;
    std::string defaultAnimation = "idle";
    
    // === ステータス ===
    float maxHealth = 100.0f;
    float attack = 10.0f;
    float defense = 0.0f;
    float attackInterval = 1.0f;   // 攻撃間隔（秒）
    float moveSpeed = 50.0f;       // 移動速度
    float knockbackResist = 0.0f;  // ノックバック耐性（0-1）
    
    // === 戦闘 ===
    AttackType attackType = AttackType::Single;
    Rect attackRange;              // 攻撃範囲
    Rect hitbox;                   // 当たり判定
    int attackCount = 1;           // 多段攻撃の回数
    float criticalChance = 0.0f;   // クリティカル率
    float criticalMultiplier = 1.5f;
    
    // === スキル ===
    std::vector<std::string> skillIds;  // 所持スキルID一覧
    
    // === コスト ===
    float cost = 100.0f;           // 召喚コスト
    float cooldownTime = 5.0f;     // 再召喚までの時間
    
    // === レベルアップ時の成長率 ===
    float healthGrowth = 1.1f;     // 1レベルあたりHP倍率
    float attackGrowth = 1.1f;     // 1レベルあたり攻撃倍率
    
    // === オプション ===
    bool isEnemy = false;          // 敵ユニットかどうか
    int maxSpawnCount = 0;         // 最大同時出現数（0=無制限）
};

// ===== Wave定義 =====

/**
 * @brief Wave内の敵出現エントリ
 */
struct EnemySpawnEntry {
    std::string characterId;
    int count = 1;
    float delay = 0.0f;          // Wave開始からの遅延
    float interval = 1.0f;       // 複数体の出現間隔
    int lane = 0;                // 出現レーン
};

/**
 * @brief Wave定義
 */
struct WaveDef {
    int waveNumber = 0;
    std::vector<EnemySpawnEntry> enemies;
    float duration = 30.0f;       // Wave最大継続時間
    std::string triggerCondition; // 次Wave発動条件（"time", "all_dead"）
};

// ===== ステージ定義 =====

/**
 * @brief ステージ定義
 */
struct StageDef {
    std::string id;
    std::string name;
    std::string description;
    std::string backgroundPath;
    
    // Wave構成
    std::vector<WaveDef> waves;
    
    // 勝利条件
    float baseHealth = 1000.0f;      // 味方ベースHP
    float enemyBaseHealth = 1000.0f; // 敵ベースHP
    float timeLimit = 0.0f;          // 制限時間（0=無制限）
    
    // 報酬
    int clearReward = 100;           // クリア報酬
    int firstClearBonus = 50;        // 初回クリアボーナス
    std::vector<std::string> dropCharacterIds;  // ドロップするキャラID
    
    // コスト
    float startingCost = 500.0f;     // 開始時コスト
    float costRegenRate = 10.0f;     // コスト回復速度（/秒）
    float maxCost = 9999.0f;         // 最大コスト
    
    // レーン設定
    int laneCount = 1;               // レーン数
    float laneHeight = 100.0f;       // 各レーンの高さ
    
    // 難易度調整
    float enemyHealthMultiplier = 1.0f;
    float enemyAttackMultiplier = 1.0f;
};

// ===== UIパーツ定義（将来用）=====

/**
 * @brief ボタン定義
 */
struct ButtonDef {
    std::string id;
    Rect bounds;
    std::string normalSprite;
    std::string pressedSprite;
    std::string disabledSprite;
    std::string text;
    std::string fontId;
    float fontSize = 16.0f;
};

} // namespace Data
