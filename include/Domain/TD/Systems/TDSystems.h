/**
 * @file TDSystems.h
 * @brief タワーディフェンス用システム群
 * 
 * レーン移動、戦闘、Wave管理などのTD固有システム。
 * Phase 2: 新しい統一アーキテクチャに移行
 */
#pragma once

#include "Core/World.h"
#include "Core/GameContext.h"
#include "Core/Events.h"
#include "Core/Components/CoreComponents.h"
#include "Game/Components/GameComponents.h"
#include "Domain/TD/Components/TDComponents.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace Domain::TD::Systems {

// 既存のTD::Systemsとの互換性のためのエイリアス
namespace TD = Domain::TD;
// 注意: TDイベントはグローバル名前空間の ::TD に定義されているため、::TD:: で明示的に参照

// ===== LaneSystem =====

/**
 * @brief レーン上の移動を管理
 * 
 * - 敵との衝突検出
 * - 移動状態の更新
 * - 停止/移動の切り替え
 */
inline void LaneMovementSystem(Core::World& world, Core::GameContext& ctx, float dt) {
    using namespace Domain::TD::Components;
    using namespace Core::Components;
    
    auto view = world.View<Position, Stats, Movement, Combat, Lane>();
    
    for (auto entity : view) {
        auto& pos = view.get<Position>(entity);
        auto& stats = view.get<Stats>(entity);
        auto& movement = view.get<Movement>(entity);
        auto& combat = view.get<Combat>(entity);
        const auto& lane = view.get<Lane>(entity);
        
        // スタン中は移動しない
        if (world.HasAll<Stunned>(entity)) {
            movement.state = MovementState::Stopped;
            continue;
        }
        
        // ノックバック中は移動システムで処理しない
        if (world.HasAll<KnockedBack>(entity)) {
            continue;
        }
        
        // 死亡中は移動しない
        if (world.HasAll<Dying>(entity)) {
            movement.state = MovementState::Stopped;
            continue;
        }
        
        // 攻撃中は移動しない
        if (world.HasAll<Attacking>(entity)) {
            movement.state = MovementState::Engaging;
            continue;
        }
        
        // 敵を検索（同じレーン内で最も近い敵）
        bool isEnemy = world.HasAll<EnemyUnit>(entity);
        float closestDist = 99999.0f;
        entt::entity closestEnemy = entt::null;
        
        auto searchView = world.View<Position, Lane, Combat>();
        for (auto other : searchView) {
            if (other == entity) continue;
            
            // 敵味方判定
            bool otherIsEnemy = world.HasAll<EnemyUnit>(other);
            if (isEnemy == otherIsEnemy) continue;  // 同じ陣営はスキップ
            
            // 死亡中はスキップ
            if (world.HasAll<Dying>(other)) continue;
            
            const auto& otherLane = searchView.get<Lane>(other);
            if (otherLane.laneIndex != lane.laneIndex) continue;  // 別レーンはスキップ
            
            const auto& otherPos = searchView.get<Position>(other);
            float dist = std::abs(otherPos.x - pos.x);
            
            if (dist < closestDist) {
                closestDist = dist;
                closestEnemy = other;
            }
        }
        
        // 攻撃範囲の計算
        float attackRangeStart = movement.direction > 0 
            ? pos.x + combat.attackRange.x 
            : pos.x - combat.attackRange.x - combat.attackRange.width;
        float attackRangeEnd = attackRangeStart + combat.attackRange.width;
        
        // 敵が攻撃範囲内にいるか
        bool enemyInRange = false;
        if (closestEnemy != entt::null) {
            const auto& enemyPos = world.Get<Position>(closestEnemy);
            
            // 敵のヒットボックス中心
            float enemyCenter = enemyPos.x;
            enemyInRange = enemyCenter >= attackRangeStart && enemyCenter <= attackRangeEnd;
            
            combat.currentTarget = closestEnemy;
        } else {
            combat.currentTarget = entt::null;
        }
        
        // 移動状態の更新
        if (enemyInRange) {
            movement.state = MovementState::Engaging;
        } else if (closestEnemy != entt::null) {
            movement.state = MovementState::Moving;
            
            // 移動速度にモディファイアを適用
            float speed = stats.moveSpeed;
            if (auto* mods = world.TryGet<StatModifiers>(entity)) {
                speed *= mods->speedMultiplier;
            }
            
            pos.x += speed * movement.direction * dt;
        } else {
            // 敵がいない場合も前進
            movement.state = MovementState::Moving;
            
            float speed = stats.moveSpeed;
            if (auto* mods = world.TryGet<StatModifiers>(entity)) {
                speed *= mods->speedMultiplier;
            }
            
            pos.x += speed * movement.direction * dt;
        }
    }
}

/**
 * @brief ノックバック処理
 */
inline void KnockbackSystem(Core::World& world, Core::GameContext& ctx, float dt) {
    using namespace Domain::TD::Components;
    using namespace Core::Components;
    
    auto view = world.View<Position, KnockedBack, Movement>();
    
    std::vector<entt::entity> toRemove;
    
    for (auto entity : view) {
        auto& pos = view.get<Position>(entity);
        auto& kb = view.get<KnockedBack>(entity);
        const auto& movement = view.get<Movement>(entity);
        
        const float knockbackSpeed = 200.0f;  // ノックバック速度
        float moveAmount = knockbackSpeed * dt;
        
        kb.progress += moveAmount;
        
        // 移動方向と逆にノックバック
        float knockbackDir = -movement.direction;
        pos.x = kb.startX + kb.progress * knockbackDir;
        
        if (kb.progress >= kb.distance) {
            toRemove.push_back(entity);
        }
    }
    
    for (auto entity : toRemove) {
        world.Remove<KnockedBack>(entity);
    }
}

// ===== CombatSystem =====

/**
 * @brief 攻撃クールダウン管理
 */
inline void AttackCooldownSystem(Core::World& world, Core::GameContext& ctx, float dt) {
    using namespace Domain::TD::Components;
    
    auto view = world.View<Combat>();
    
    for (auto entity : view) {
        auto& combat = view.get<Combat>(entity);
        
        if (combat.attackCooldown > 0.0f) {
            combat.attackCooldown -= dt;
        }
    }
}

/**
 * @brief 攻撃開始判定
 */
inline void AttackTriggerSystem(Core::World& world, Core::GameContext& ctx, float dt) {
    using namespace Domain::TD::Components;
    using namespace Core::Components;
    
    auto view = world.View<Combat, Movement, Stats>();
    
    for (auto entity : view) {
        auto& combat = view.get<Combat>(entity);
        const auto& movement = view.get<Movement>(entity);
        const auto& stats = view.get<Stats>(entity);
        
        // すでに攻撃中、スタン中、死亡中はスキップ
        if (world.HasAll<Attacking>(entity) || 
            world.HasAll<Stunned>(entity) || 
            world.HasAll<Dying>(entity)) {
            continue;
        }
        
        // 戦闘状態で、ターゲットがいて、クールダウンが終わっている
        if (movement.state == MovementState::Engaging && 
            combat.currentTarget != entt::null &&
            combat.attackCooldown <= 0.0f) {
            
            // ターゲットが有効か確認
            if (!world.Valid(combat.currentTarget) || 
                world.HasAll<Dying>(combat.currentTarget)) {
                combat.currentTarget = entt::null;
                continue;
            }
            
            // 攻撃開始
            Attacking attacking;
            attacking.attackProgress = 0.0f;
            attacking.hitApplied = false;
            world.Emplace<Attacking>(entity, attacking);
            
            // アニメーション変更
            if (auto* anim = world.TryGet<Game::Components::Animation>(entity)) {
                anim->currentAnimation = "attack";
                anim->currentFrameIndex = 0;
                anim->elapsedTime = 0.0f;
                anim->isPlaying = true;
            }
        }
    }
}

/**
 * @brief 攻撃処理（ダメージ適用）
 */
inline void AttackExecutionSystem(Core::World& world, Core::GameContext& ctx, float dt) {
    using namespace Domain::TD::Components;
    using namespace Core::Components;
    
    const float ATTACK_HIT_POINT = 0.5f;  // 攻撃アニメーションの50%でヒット判定
    const float ATTACK_DURATION = 0.5f;   // 攻撃アニメーション全体の長さ
    
    auto view = world.View<Attacking, Combat, Stats>();
    
    std::vector<entt::entity> finishedAttacks;
    
    for (auto entity : view) {
        auto& attacking = view.get<Attacking>(entity);
        auto& combat = view.get<Combat>(entity);
        const auto& stats = view.get<Stats>(entity);
        
        attacking.attackProgress += dt / ATTACK_DURATION;
        
        // ヒット判定タイミング
        if (!attacking.hitApplied && attacking.attackProgress >= ATTACK_HIT_POINT) {
            attacking.hitApplied = true;
            
            if (combat.currentTarget != entt::null && 
                world.Valid(combat.currentTarget) &&
                !world.HasAll<Dying>(combat.currentTarget)) {
                
                // ダメージ計算
                float damage = stats.attack;
                
                // モディファイア適用
                if (auto* mods = world.TryGet<StatModifiers>(entity)) {
                    damage *= mods->attackMultiplier * mods->damageMultiplier;
                }
                
                // クリティカル判定
                bool isCritical = false;
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                
                if (dist(gen) < combat.criticalChance) {
                    damage *= combat.criticalMultiplier;
                    isCritical = true;
                }
                
                // ターゲットの防御力適用
                auto& targetStats = world.Get<Stats>(combat.currentTarget);
                float defense = targetStats.defense;
                
                if (auto* targetMods = world.TryGet<StatModifiers>(combat.currentTarget)) {
                    defense *= targetMods->defenseMultiplier;
                }
                
                float actualDamage = std::max(1.0f, damage - defense);
                
                // 被ダメージモディファイア
                if (auto* targetMods = world.TryGet<StatModifiers>(combat.currentTarget)) {
                    actualDamage *= targetMods->damageTakenMultiplier;
                }
                
                // ダメージ適用
                targetStats.currentHealth -= actualDamage;
                
                // ダメージイベント発火（グローバルTD名前空間のイベント）
                world.Emit(::TD::DamageDealt{
                    entity,
                    combat.currentTarget,
                    damage,
                    actualDamage,
                    isCritical,
                    "normal"
                });
                
                // ノックバック処理
                if (targetStats.knockbackResist < 1.0f) {
                    float kbDistance = 30.0f * (1.0f - targetStats.knockbackResist);
                    
                    if (kbDistance > 1.0f && !world.HasAll<KnockedBack>(combat.currentTarget)) {
                        const auto& targetPos = world.Get<Position>(combat.currentTarget);
                        
                        KnockedBack kb;
                        kb.distance = kbDistance;
                        kb.progress = 0.0f;
                        kb.startX = targetPos.x;
                        world.Emplace<KnockedBack>(combat.currentTarget, kb);
                    }
                }
            }
        }
        
        // 攻撃終了
        if (attacking.attackProgress >= 1.0f) {
            finishedAttacks.push_back(entity);
            combat.attackCooldown = stats.attackInterval;
            
            // アニメーションをidleに戻す
            if (auto* anim = world.TryGet<Game::Components::Animation>(entity)) {
                anim->currentAnimation = "idle";
                anim->currentFrameIndex = 0;
                anim->elapsedTime = 0.0f;
            }
        }
    }
    
    for (auto entity : finishedAttacks) {
        world.Remove<Attacking>(entity);
    }
}

/**
 * @brief 死亡判定
 */
inline void DeathCheckSystem(Core::World& world, Core::GameContext& ctx, float dt) {
    using namespace Domain::TD::Components;
    using namespace Core::Components;
    
    auto view = world.View<Stats, Unit>();
    
    for (auto entity : view) {
        const auto& stats = view.get<Stats>(entity);
        
        // すでに死亡中はスキップ
        if (world.HasAll<Dying>(entity)) continue;
        
        if (stats.currentHealth <= 0.0f) {
            // 死亡開始
            Dying dying;
            dying.animationProgress = 0.0f;
            world.Emplace<Dying>(entity, dying);
            
            // 死亡アニメーション開始
            if (auto* anim = world.TryGet<Game::Components::Animation>(entity)) {
                anim->currentAnimation = "death";
                anim->currentFrameIndex = 0;
                anim->elapsedTime = 0.0f;
                anim->isPlaying = true;
                anim->isLooping = false;
            }
            
            // 攻撃状態をクリア
            if (world.HasAll<Attacking>(entity)) {
                world.Remove<Attacking>(entity);
            }
            
            // イベント発火
            world.Emit(::TD::UnitDied{
                entity,
                entt::null,
                "damage"
            });
        }
    }
}

/**
 * @brief 死亡アニメーション処理
 */
inline void DeathAnimationSystem(Core::World& world, Core::GameContext& ctx, float dt) {
    using namespace Domain::TD::Components;
    
    const float DEATH_DURATION = 1.0f;
    
    auto view = world.View<Dying>();
    
    for (auto entity : view) {
        auto& dying = view.get<Dying>(entity);
        
        // skipAnimationがtrueなら即座に破棄
        if (dying.skipAnimation) {
            world.MarkForDestruction(entity);
            continue;
        }
        
        dying.animationProgress += dt / DEATH_DURATION;
        
        if (dying.animationProgress >= 1.0f) {
            world.MarkForDestruction(entity);
        }
    }
}

// ===== StatusEffectSystem =====

/**
 * @brief ステータス効果の更新
 */
inline void StatusEffectSystem(Core::World& world, Core::GameContext& ctx, float dt) {
    using namespace Domain::TD::Components;
    
    auto view = world.View<StatusEffects, Stats, StatModifiers>();
    
    for (auto entity : view) {
        auto& effects = view.get<StatusEffects>(entity);
        auto& stats = view.get<Stats>(entity);
        auto& mods = view.get<StatModifiers>(entity);
        
        // モディファイアをリセット
        mods = StatModifiers{};
        
        // 期限切れの効果を削除しながら処理
        effects.effects.erase(
            std::remove_if(effects.effects.begin(), effects.effects.end(),
                [&](ActiveStatusEffect& effect) {
                    effect.remainingDuration -= dt;
                    
                    if (effect.remainingDuration <= 0.0f) {
                        // 効果終了イベント
                        world.Emit(::TD::StatusEffectExpired{entity, effect.effectId});
                        return true;
                    }
                    
                    // 効果適用
                    switch (effect.type) {
                        case Data::StatusEffectType::Slow:
                            mods.speedMultiplier *= (1.0f - effect.value);
                            break;
                        case Data::StatusEffectType::AttackUp:
                            mods.attackMultiplier *= (1.0f + effect.value);
                            break;
                        case Data::StatusEffectType::AttackDown:
                            mods.attackMultiplier *= (1.0f - effect.value);
                            break;
                        case Data::StatusEffectType::DefenseUp:
                            mods.defenseMultiplier *= (1.0f + effect.value);
                            break;
                        case Data::StatusEffectType::DefenseDown:
                            mods.defenseMultiplier *= (1.0f - effect.value);
                            break;
                        case Data::StatusEffectType::SpeedUp:
                            mods.speedMultiplier *= (1.0f + effect.value);
                            break;
                        case Data::StatusEffectType::Poison:
                        case Data::StatusEffectType::Burn:
                            effect.tickTimer -= dt;
                            if (effect.tickTimer <= 0.0f) {
                                stats.currentHealth -= effect.value;
                                effect.tickTimer = 1.0f;  // 1秒ごとにダメージ
                            }
                            break;
                        case Data::StatusEffectType::Regeneration:
                            effect.tickTimer -= dt;
                            if (effect.tickTimer <= 0.0f) {
                                stats.currentHealth = std::min(
                                    stats.maxHealth, 
                                    stats.currentHealth + effect.value
                                );
                                effect.tickTimer = 1.0f;
                            }
                            break;
                        default:
                            break;
                    }
                    
                    return false;
                }),
            effects.effects.end()
        );
    }
}

/**
 * @brief スタン状態の更新
 */
inline void StunSystem(Core::World& world, Core::GameContext& ctx, float dt) {
    using namespace Domain::TD::Components;
    
    auto view = world.View<Stunned>();
    
    std::vector<entt::entity> toRemove;
    
    for (auto entity : view) {
        auto& stun = view.get<Stunned>(entity);
        stun.duration -= dt;
        
        if (stun.duration <= 0.0f) {
            toRemove.push_back(entity);
        }
    }
    
    for (auto entity : toRemove) {
        world.Remove<Stunned>(entity);
    }
}

/**
 * @brief 無敵状態の更新
 */
inline void InvincibleSystem(Core::World& world, Core::GameContext& ctx, float dt) {
    using namespace Domain::TD::Components;
    
    auto view = world.View<Invincible>();
    
    std::vector<entt::entity> toRemove;
    
    for (auto entity : view) {
        auto& inv = view.get<Invincible>(entity);
        inv.duration -= dt;
        
        if (inv.duration <= 0.0f) {
            toRemove.push_back(entity);
        }
    }
    
    for (auto entity : toRemove) {
        world.Remove<Invincible>(entity);
    }
}

} // namespace Domain::TD::Systems
