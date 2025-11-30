/**
 * @file TDComponents.h
 * @brief TD層コンポーネント
 * 
 * タワーディフェンスゲーム固有のコンポーネント。
 * 戦闘、ユニット、レーンなど。
 */
#pragma once

#include "../../Core/Definitions.h"
#include <string>
#include <vector>
#include <entt/entt.hpp>

namespace TD::Components {

// ===== ユニット基本情報 =====

/**
 * @brief ユニット情報（定義への参照）
 */
struct Unit {
    std::string definitionId;  // CharacterDefのID
    bool isEnemy = false;      // 敵側かどうか
    int level = 1;             // 現在レベル
};

/**
 * @brief ステータス（ランタイム値）
 */
struct Stats {
    float maxHealth = 100.0f;
    float currentHealth = 100.0f;
    float attack = 10.0f;
    float defense = 0.0f;
    float moveSpeed = 50.0f;
    float attackInterval = 1.0f;
    float knockbackResist = 0.0f;
};

/**
 * @brief ステータス修正値（バフ/デバフ）
 */
struct StatModifiers {
    float attackMultiplier = 1.0f;
    float defenseMultiplier = 1.0f;
    float speedMultiplier = 1.0f;
    float damageMultiplier = 1.0f;
    float damageTakenMultiplier = 1.0f;
};

// ===== 戦闘関連 =====

/**
 * @brief 攻撃能力
 */
struct Combat {
    Data::AttackType attackType = Data::AttackType::Single;
    Data::Rect attackRange;
    Data::Rect hitbox;
    
    float attackCooldown = 0.0f;  // 現在のクールダウン
    int attackCount = 1;          // 多段攻撃回数
    int currentAttackHit = 0;     // 現在の攻撃何段目か
    
    float criticalChance = 0.0f;
    float criticalMultiplier = 1.5f;
    
    entt::entity currentTarget = entt::null;  // 現在のターゲット
};

/**
 * @brief 攻撃中フラグ
 */
struct Attacking {
    float attackProgress = 0.0f;  // 0.0-1.0 攻撃アニメーション進行
    bool hitApplied = false;      // ヒット判定適用済みか
};

/**
 * @brief ダメージを受けている
 */
struct TakingDamage {
    float damageAmount = 0.0f;
    entt::entity source = entt::null;
    std::string damageType;  // "normal", "skill", "poison"
};

/**
 * @brief ノックバック中
 */
struct KnockedBack {
    float distance = 0.0f;
    float progress = 0.0f;
    float startX = 0.0f;
};

/**
 * @brief 死亡中（死亡アニメーション再生）
 */
struct Dying {
    float animationProgress = 0.0f;
    bool skipAnimation = false;  // アニメーションをスキップするか
};

// ===== ステータス効果 =====

/**
 * @brief アクティブなステータス効果
 */
struct ActiveStatusEffect {
    std::string effectId;
    Data::StatusEffectType type;
    float value;
    float remainingDuration;
    float tickTimer;
    entt::entity source;
};

/**
 * @brief ステータス効果コンテナ
 */
struct StatusEffects {
    std::vector<ActiveStatusEffect> effects;
};

// ===== スキル関連 =====

/**
 * @brief スキルスロット
 */
struct SkillSlot {
    std::string skillId;
    float cooldown = 0.0f;      // 現在のクールダウン
    bool isReady = true;        // 発動可能か
};

/**
 * @brief スキルコンテナ
 */
struct Skills {
    std::vector<SkillSlot> slots;
};

/**
 * @brief スキル発動中
 */
struct CastingSkill {
    std::string skillId;
    float castProgress = 0.0f;
    std::vector<entt::entity> targets;
};

// ===== レーン関連 =====

/**
 * @brief レーン位置
 */
struct Lane {
    int laneIndex = 0;
    float laneY = 0.0f;  // レーンのY座標
};

/**
 * @brief 移動状態
 */
enum class MovementState {
    Moving,     // 移動中
    Engaging,   // 戦闘開始
    Stopped,    // 停止（敵と接触）
    Retreating, // 後退中
};

struct Movement {
    MovementState state = MovementState::Moving;
    float direction = 1.0f;  // 1.0=右, -1.0=左
};

// ===== 召喚関連 =====

/**
 * @brief 召喚クールダウン情報
 */
struct SummonInfo {
    std::string characterId;
    float cost = 100.0f;
    float cooldown = 0.0f;
    float maxCooldown = 5.0f;
    bool isReady = true;
};

// ===== ベース（拠点）=====

/**
 * @brief 拠点コンポーネント
 */
struct Base {
    bool isPlayerBase = true;
    float health = 1000.0f;
    float maxHealth = 1000.0f;
};

// ===== 投射物 =====

/**
 * @brief 投射物
 */
struct Projectile {
    entt::entity source = entt::null;
    entt::entity target = entt::null;
    float damage = 0.0f;
    float speed = 200.0f;
    bool isHoming = false;  // 追尾するか
};

// ===== タグコンポーネント =====

/**
 * @brief 味方ユニット
 */
struct AllyUnit {};

/**
 * @brief 敵ユニット
 */
struct EnemyUnit {};

/**
 * @brief ボスユニット
 */
struct BossUnit {};

/**
 * @brief 行動不能（スタン等）
 */
struct Stunned {
    float duration = 0.0f;
};

/**
 * @brief 無敵状態
 */
struct Invincible {
    float duration = 0.0f;
};

} // namespace TD::Components
