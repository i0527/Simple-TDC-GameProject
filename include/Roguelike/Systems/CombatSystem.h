/**
 * @file CombatSystem.h
 * @brief 戦闘システム
 * 
 * 攻撃判定、ダメージ計算、死亡処理を行う。
 */
#pragma once

#include <entt/entt.hpp>
#include <string>
#include <functional>
#include <cstdlib>
#include "../Components/GridComponents.h"
#include "../Components/TurnComponents.h"
#include "../Components/CombatComponents.h"

namespace Roguelike::Systems {

/**
 * @brief 戦闘結果
 */
struct CombatResult {
    bool hit = false;           ///< 命中したか
    bool critical = false;      ///< クリティカルか
    int damage = 0;             ///< 与えたダメージ
    bool killed = false;        ///< 倒したか
    std::string message;        ///< 戦闘メッセージ
};

/**
 * @brief 戦闘システム
 */
class CombatSystem {
public:
    using MessageCallback = std::function<void(const std::string&)>;
    
    /**
     * @brief 攻撃を実行
     * @param registry ECSレジストリ
     * @param attacker 攻撃者
     * @param defender 防御者
     * @return 戦闘結果
     */
    static CombatResult Attack(
        entt::registry& registry,
        entt::entity attacker,
        entt::entity defender
    ) {
        CombatResult result;
        
        // コンポーネント取得
        auto* attackerStats = registry.try_get<Components::CombatStats>(attacker);
        auto* attackerName = registry.try_get<Components::Name>(attacker);
        auto* defenderStats = registry.try_get<Components::CombatStats>(defender);
        auto* defenderHealth = registry.try_get<Components::Health>(defender);
        auto* defenderName = registry.try_get<Components::Name>(defender);
        
        if (!defenderHealth) return result;
        
        std::string atkName = attackerName ? attackerName->value : "???";
        std::string defName = defenderName ? defenderName->value : "???";
        
        // デフォルトステータス
        int attackPower = attackerStats ? attackerStats->attack : 1;
        int accuracy = attackerStats ? attackerStats->accuracy : 80;
        int critChance = attackerStats ? attackerStats->critChance : 5;
        float critMult = attackerStats ? attackerStats->critMultiplier : 1.5f;
        int defense = defenderStats ? defenderStats->defense : 0;
        int evasion = defenderStats ? defenderStats->evasion : 10;
        
        // 命中判定
        int hitRoll = std::rand() % 100;
        int hitChance = accuracy - evasion;
        if (hitChance < 5) hitChance = 5;    // 最低5%
        if (hitChance > 95) hitChance = 95;  // 最高95%
        
        if (hitRoll >= hitChance) {
            // ミス
            result.hit = false;
            result.message = atkName + "の攻撃は" + defName + "に当たらなかった。";
            return result;
        }
        
        result.hit = true;
        
        // クリティカル判定
        int critRoll = std::rand() % 100;
        if (critRoll < critChance) {
            result.critical = true;
        }
        
        // ダメージ計算
        int baseDamage = attackPower;
        if (result.critical) {
            baseDamage = static_cast<int>(baseDamage * critMult);
        }
        
        // ダメージにランダム性を追加（±20%）
        int variance = baseDamage / 5;
        if (variance < 1) variance = 1;
        baseDamage += (std::rand() % (variance * 2 + 1)) - variance;
        
        // 防御力でダメージ軽減
        result.damage = baseDamage - defense;
        if (result.damage < 1) result.damage = 1;  // 最低1ダメージ
        
        // ダメージ適用
        defenderHealth->TakeDamage(result.damage);
        
        // 死亡判定
        if (!defenderHealth->IsAlive()) {
            result.killed = true;
            registry.emplace_or_replace<Components::Dead>(defender);
        }
        
        // メッセージ生成
        if (result.critical) {
            result.message = atkName + "の会心の一撃！" + defName + "に" + 
                            std::to_string(result.damage) + "ダメージ！";
        } else {
            result.message = atkName + "は" + defName + "に" + 
                            std::to_string(result.damage) + "ダメージを与えた。";
        }
        
        if (result.killed) {
            result.message += " " + defName + "を倒した！";
        }
        
        return result;
    }
    
    /**
     * @brief 死亡エンティティを処理
     * @param registry ECSレジストリ
     * @param map マップデータ
     * @param onMonsterKilled モンスター死亡時のコールバック（経験値など）
     */
    static void ProcessDeaths(
        entt::registry& registry,
        Components::MapData& map,
        MessageCallback onMessage = nullptr
    ) {
        auto view = registry.view<Components::Dead, Components::GridPosition>();
        
        std::vector<entt::entity> toDestroy;
        
        for (auto entity : view) {
            auto& pos = view.get<Components::GridPosition>(entity);
            
            // プレイヤーの死亡チェック
            if (registry.any_of<Components::PlayerTag>(entity)) {
                // ゲームオーバー処理（後で実装）
                if (onMessage) {
                    onMessage("あなたは死んだ...");
                }
                continue;  // プレイヤーは削除しない
            }
            
            // モンスターの死亡処理
            if (registry.any_of<Components::MonsterTag>(entity)) {
                // マップからoccupantを削除
                if (map.InBounds(pos.x, pos.y)) {
                    if (map.At(pos.x, pos.y).occupant == entity) {
                        map.At(pos.x, pos.y).occupant = entt::null;
                    }
                }
                
                toDestroy.push_back(entity);
            }
        }
        
        // エンティティを削除
        for (auto entity : toDestroy) {
            registry.destroy(entity);
        }
    }
    
    /**
     * @brief プレイヤーがモンスターを倒した時の経験値処理
     */
    static bool GiveExperience(
        entt::registry& registry,
        entt::entity player,
        int expAmount,
        MessageCallback onMessage = nullptr
    ) {
        auto* exp = registry.try_get<Components::Experience>(player);
        if (!exp) return false;
        
        bool leveledUp = exp->AddExp(expAmount);
        
        if (onMessage) {
            onMessage(std::to_string(expAmount) + "の経験値を得た。");
            
            if (leveledUp) {
                onMessage("レベルアップ！ レベル" + std::to_string(exp->level) + "になった！");
                
                // レベルアップ時のステータス上昇
                auto* health = registry.try_get<Components::Health>(player);
                auto* stats = registry.try_get<Components::CombatStats>(player);
                
                if (health) {
                    health->max += 5;
                    health->current = health->max;  // 全回復
                }
                if (stats) {
                    stats->attack += 1;
                }
            }
        }
        
        return leveledUp;
    }
};

} // namespace Roguelike::Systems
