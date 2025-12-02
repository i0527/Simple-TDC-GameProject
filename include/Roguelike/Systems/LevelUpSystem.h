/**
 * @file LevelUpSystem.h
 * @brief レベルアップシステム
 * 
 * Phase 5: 経験値獲得、レベルアップ時の能力値上昇
 */
#pragma once

#include <entt/entt.hpp>
#include <functional>
#include <string>
#include "../Components/CombatComponents.h"

namespace Roguelike::Systems {

/**
 * @brief レベルアップシステム
 */
class LevelUpSystem {
public:
    using MessageCallback = std::function<void(const std::string&)>;
    
    /**
     * @brief 経験値を獲得
     * @param registry ECSレジストリ
     * @param actor 経験値を得るエンティティ
     * @param exp 獲得経験値
     * @param callback メッセージコールバック
     * @return レベルアップした場合true
     */
    static bool GainExperience(entt::registry& registry,
                               entt::entity actor,
                               int exp,
                               MessageCallback callback = nullptr) {
        auto* experience = registry.try_get<Components::Experience>(actor);
        if (!experience) return false;
        
        int prevLevel = experience->level;
        
        if (callback) {
            callback(TextFormat("%d経験値を得た。", exp));
        }
        
        if (experience->AddExp(exp)) {
            // レベルアップ！
            int newLevel = experience->level;
            
            // 能力値上昇
            ApplyLevelUpBonuses(registry, actor, newLevel);
            
            if (callback) {
                callback(TextFormat("レベルアップ！ Lv.%d になった！", newLevel));
            }
            
            return true;
        }
        
        return false;
    }
    
    /**
     * @brief レベルアップボーナスを適用
     */
    static void ApplyLevelUpBonuses(entt::registry& registry,
                                     entt::entity actor,
                                     int newLevel) {
        // HP上昇
        auto* health = registry.try_get<Components::Health>(actor);
        if (health) {
            int hpGain = 5 + (newLevel / 3);  // レベルごとにHP+5〜
            health->max += hpGain;
            health->current = health->max;  // 全回復
        }
        
        // 攻撃/防御上昇
        auto* combat = registry.try_get<Components::CombatStats>(actor);
        if (combat) {
            // 2レベルごとに攻撃力+1
            if (newLevel % 2 == 0) {
                combat->attack++;
            }
            // 3レベルごとに防御力+1
            if (newLevel % 3 == 0) {
                combat->defense++;
            }
            // 5レベルごとにクリティカル率+1%
            if (newLevel % 5 == 0) {
                combat->critChance++;
            }
        }
    }
    
    /**
     * @brief モンスター撃破時の経験値を取得
     */
    static int GetMonsterExp(const Components::MonsterData& monsterData, int playerLevel) {
        int baseExp = monsterData.expValue;
        
        // レベル差によるボーナス/ペナルティ
        int floorDiff = monsterData.minFloor - playerLevel;
        if (floorDiff > 0) {
            // 強い敵はボーナス
            baseExp = static_cast<int>(baseExp * (1.0f + floorDiff * 0.1f));
        } else if (floorDiff < -3) {
            // 弱すぎる敵はペナルティ
            baseExp = static_cast<int>(baseExp * 0.5f);
            if (baseExp < 1) baseExp = 1;
        }
        
        return baseExp;
    }
    
    /**
     * @brief 経験値バーの割合を取得
     */
    static float GetExpRatio(entt::registry& registry, entt::entity actor) {
        auto* exp = registry.try_get<Components::Experience>(actor);
        if (!exp) return 0.0f;
        
        return static_cast<float>(exp->current) / exp->toNextLevel;
    }
    
    /**
     * @brief 次のレベルまでの残り経験値を取得
     */
    static int GetExpToNext(entt::registry& registry, entt::entity actor) {
        auto* exp = registry.try_get<Components::Experience>(actor);
        if (!exp) return 0;
        
        return exp->toNextLevel - exp->current;
    }
};

} // namespace Roguelike::Systems
