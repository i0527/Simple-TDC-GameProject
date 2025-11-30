/**
 * @file Events.h
 * @brief ゲームイベント定義
 * 
 * システム間の疎結合な通信を実現するイベント構造体。
 * World::Emit() / World::Subscribe() で使用。
 */
#pragma once

#include <entt/entt.hpp>
#include <string>

namespace Core {

// ===== ライフサイクルイベント =====

/**
 * @brief エンティティが生成された
 */
struct EntityCreated {
    entt::entity entity;
    std::string type;  // "unit", "projectile", "effect" など
};

/**
 * @brief エンティティが破棄される（破棄前に発火）
 */
struct EntityDestroying {
    entt::entity entity;
    std::string reason;  // "death", "expired", "cleanup" など
};

// ===== シーンイベント =====

/**
 * @brief シーン遷移リクエスト
 */
struct SceneChangeRequest {
    std::string nextScene;
    bool fade;
};

/**
 * @brief シーン遷移完了
 */
struct SceneChanged {
    std::string previousScene;
    std::string currentScene;
};

} // namespace Core


namespace Game {

// ===== アニメーションイベント =====

/**
 * @brief アニメーションが終了した
 */
struct AnimationFinished {
    entt::entity entity;
    std::string animationName;
};

/**
 * @brief アニメーションがループした
 */
struct AnimationLooped {
    entt::entity entity;
    std::string animationName;
    int loopCount;
};

/**
 * @brief アニメーションフレームが変わった
 */
struct AnimationFrameChanged {
    entt::entity entity;
    std::string animationName;
    int frameIndex;
    std::string frameTag;  // フレームに設定されたタグ（"attack_hit" など）
};

// ===== リソースイベント =====

/**
 * @brief リソースがロードされた
 */
struct ResourceLoaded {
    std::string path;
    std::string type;  // "texture", "sound", "font"
};

/**
 * @brief リソースロードエラー
 */
struct ResourceLoadFailed {
    std::string path;
    std::string error;
};

} // namespace Game


namespace TD {

// ===== ユニットイベント =====

/**
 * @brief ユニットがスポーンした
 */
struct UnitSpawned {
    entt::entity entity;
    std::string characterId;
    int lane;       // レーン番号
    bool isEnemy;   // 敵側か味方側か
};

/**
 * @brief ユニットが死亡した
 */
struct UnitDied {
    entt::entity entity;
    entt::entity killer;  // entt::null の場合もある
    std::string deathCause;  // "damage", "skill", "expired"
};

/**
 * @brief ユニットがノックバックされた
 */
struct UnitKnockedBack {
    entt::entity entity;
    float distance;
};

// ===== 戦闘イベント =====

/**
 * @brief ダメージが発生した
 */
struct DamageDealt {
    entt::entity source;
    entt::entity target;
    float damage;
    float actualDamage;  // 防御力適用後
    bool isCritical;
    std::string damageType;  // "normal", "skill", "area"
};

/**
 * @brief ユニットが回復した
 */
struct HealingReceived {
    entt::entity source;
    entt::entity target;
    float amount;
    float actualAmount;  // 上限考慮後
};

/**
 * @brief 攻撃がミスした（回避）
 */
struct AttackMissed {
    entt::entity attacker;
    entt::entity target;
};

/**
 * @brief バフ/デバフが適用された
 */
struct StatusEffectApplied {
    entt::entity source;
    entt::entity target;
    std::string effectId;
    float duration;
};

/**
 * @brief バフ/デバフが終了した
 */
struct StatusEffectExpired {
    entt::entity entity;
    std::string effectId;
};

// ===== Wave イベント =====

/**
 * @brief Waveが開始した
 */
struct WaveStarted {
    int waveNumber;
    int totalWaves;
};

/**
 * @brief Waveが完了した
 */
struct WaveCompleted {
    int waveNumber;
    int remainingWaves;
};

/**
 * @brief 全Waveが完了した
 */
struct AllWavesCompleted {
    int totalWaves;
    float elapsedTime;
};

// ===== ゲーム進行イベント =====

/**
 * @brief コストが変化した
 */
struct CostChanged {
    float currentCost;
    float maxCost;
    float regenRate;
};

/**
 * @brief ベースがダメージを受けた
 */
struct BaseDamaged {
    bool isEnemyBase;   // true=敵側, false=味方側
    float damage;
    float remainingHealth;
    float maxHealth;
};

/**
 * @brief ゲームが開始した
 */
struct GameStarted {};

/**
 * @brief ゲームが終了した
 */
struct GameEnded {
    bool isVictory;
    float elapsedTime;
    int score;
};

/**
 * @brief ゲーム結果が確定した
 */
struct GameResult {
    bool playerWon;
    int starsEarned;   // 0-3
    float elapsedTime;
    int unitsDeployed;
    int enemiesDefeated;
};

// ===== UI イベント =====

/**
 * @brief キャラクター選択がリクエストされた
 */
struct CharacterSelectRequest {
    int slotIndex;
    std::string characterId;
};

/**
 * @brief ユニット召喚がリクエストされた
 */
struct SummonRequest {
    std::string characterId;
    int lane;
};

/**
 * @brief 召喚がキャンセルされた（コスト不足など）
 */
struct SummonFailed {
    std::string characterId;
    std::string reason;  // "not_enough_cost", "cooldown", "lane_full"
};

// ===== スキルイベント =====

/**
 * @brief スキルが発動した
 */
struct SkillActivated {
    entt::entity source;
    std::string skillId;
};

/**
 * @brief スキルエフェクトがヒットした
 */
struct SkillHit {
    entt::entity source;
    entt::entity target;
    std::string skillId;
    std::string effectType;  // "damage", "heal", "buff", "debuff"
};

} // namespace TD
