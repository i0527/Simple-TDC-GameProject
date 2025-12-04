/**
 * @file HungerSystem.h
 * @brief 空腹度システム
 * 
 * Phase 5: 毎ターンの空腹度減少、餓死判定、食事処理
 * 
 * Phase 3: 新しい統一アーキテクチャに移行
 */
#pragma once

#include <entt/entt.hpp>
#include <functional>
#include <string>
#include <cstdlib>
#include "../Components/RoguelikeComponents.h"

namespace Domain::Roguelike::Systems {

/**
 * @brief 空腹度システム
 */
class HungerSystem {
public:
    using MessageCallback = std::function<void(const std::string&)>;
    
    /**
     * @brief プレイヤーの行動後に空腹度を更新
     * @param registry ECSレジストリ
     * @param actor 行動したエンティティ
     * @param actionType 行動タイプ
     * @param callback メッセージコールバック
     */
    static void OnAction(entt::registry& registry, 
                         entt::entity actor,
                         Components::ActionCommand::Type actionType,
                         MessageCallback callback = nullptr) {
        // プレイヤーのみ処理
        if (!registry.any_of<Components::PlayerTag>(actor)) return;
        
        auto* hunger = registry.try_get<Components::Hunger>(actor);
        if (!hunger) return;
        
        // 前の状態を記録
        auto prevState = hunger->GetState();
        
        // 行動に応じて空腹度減少
        int decrease = Components::Hunger::HUNGER_PER_ACTION;
        switch (actionType) {
            case Components::ActionCommand::Type::Move:
                decrease = Components::Hunger::HUNGER_PER_MOVE;
                break;
            case Components::ActionCommand::Type::Attack:
                decrease = Components::Hunger::HUNGER_PER_ATTACK;
                break;
            case Components::ActionCommand::Type::Wait:
                decrease = Components::Hunger::HUNGER_PER_WAIT;
                break;
            default:
                break;
        }
        
        hunger->Decrease(decrease);
        
        // 状態変化をチェック
        auto newState = hunger->GetState();
        if (newState != prevState && callback) {
            NotifyStateChange(prevState, newState, callback);
        }
        
        // 餓死ダメージ
        if (newState == Components::HungerState::Starving) {
            auto* health = registry.try_get<Components::Health>(actor);
            if (health && callback) {
                health->TakeDamage(1);
                callback("空腹で体力が奪われている！");
            }
        }
        
        // 満腹時のHP回復
        if (newState == Components::HungerState::Satiated) {
            auto* health = registry.try_get<Components::Health>(actor);
            if (health && health->current < health->max) {
                // 10ターンに1回程度回復
                if (std::rand() % 10 == 0) {
                    health->Heal(1);
                    if (callback) {
                        callback("満腹で傷が癒えた。");
                    }
                }
            }
        }
    }
    
    /**
     * @brief 食事処理
     * @param registry ECSレジストリ
     * @param actor 食べるエンティティ
     * @param nutrition 栄養価
     * @param callback メッセージコールバック
     */
    static void Eat(entt::registry& registry,
                    entt::entity actor,
                    int nutrition,
                    MessageCallback callback = nullptr) {
        auto* hunger = registry.try_get<Components::Hunger>(actor);
        if (!hunger) return;
        
        auto prevState = hunger->GetState();
        hunger->Eat(nutrition);
        auto newState = hunger->GetState();
        
        if (callback) {
            if (newState == Components::HungerState::Satiated && 
                prevState != Components::HungerState::Satiated) {
                callback("満腹になった！");
            } else if (prevState == Components::HungerState::Starving ||
                       prevState == Components::HungerState::Fainting ||
                       prevState == Components::HungerState::Weak) {
                callback("お腹が落ち着いた。");
            }
        }
    }
    
    /**
     * @brief 気絶判定
     * @param registry ECSレジストリ
     * @param actor 判定対象
     * @return 気絶する場合true
     */
    static bool CheckFaint(entt::registry& registry, entt::entity actor) {
        auto* hunger = registry.try_get<Components::Hunger>(actor);
        if (!hunger) return false;
        
        int chance = hunger->GetFaintChance();
        if (chance > 0) {
            return (std::rand() % 100) < chance;
        }
        return false;
    }
    
    /**
     * @brief 戦闘ペナルティを適用した攻撃力を取得
     */
    static int GetEffectiveAttack(entt::registry& registry, entt::entity actor) {
        auto* combat = registry.try_get<Components::CombatStats>(actor);
        if (!combat) return 0;
        
        int attack = combat->attack;
        
        auto* hunger = registry.try_get<Components::Hunger>(actor);
        if (hunger) {
            attack -= hunger->GetPenalty();
            if (attack < 1) attack = 1;
        }
        
        return attack;
    }
    
    /**
     * @brief 戦闘ペナルティを適用した防御力を取得
     */
    static int GetEffectiveDefense(entt::registry& registry, entt::entity actor) {
        auto* combat = registry.try_get<Components::CombatStats>(actor);
        if (!combat) return 0;
        
        int defense = combat->defense;
        
        auto* hunger = registry.try_get<Components::Hunger>(actor);
        if (hunger) {
            defense -= hunger->GetPenalty();
            if (defense < 0) defense = 0;
        }
        
        return defense;
    }

private:
    /**
     * @brief 状態変化を通知
     */
    static void NotifyStateChange(Components::HungerState prev,
                                   Components::HungerState next,
                                   MessageCallback callback) {
        // 悪化した場合のみ通知
        if (next > prev) {
            switch (next) {
                case Components::HungerState::Hungry:
                    callback("お腹が空いてきた。");
                    break;
                case Components::HungerState::Weak:
                    callback("空腹で体が弱ってきた...");
                    break;
                case Components::HungerState::Fainting:
                    callback("空腹で意識が朦朧としてきた！");
                    break;
                case Components::HungerState::Starving:
                    callback("餓死寸前だ！何か食べなければ！");
                    break;
                default:
                    break;
            }
        }
    }
};

} // namespace Domain::Roguelike::Systems
