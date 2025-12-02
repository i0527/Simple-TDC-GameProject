/**
 * @file TurnManager.h
 * @brief ターン管理システム
 * 
 * NetHackのエネルギー制ターンシステムを管理。
 * 各アクターにエネルギーを供給し、行動可能なアクターを
 * 順番に処理する。
 */
#pragma once

#include <entt/entt.hpp>
#include "../Components/RoguelikeComponents.h"

namespace Roguelike::Managers {

/**
 * @brief ターン管理クラス
 * 
 * ゲームのターン進行を制御する。
 * - プレイヤー入力待ち状態の管理
 * - エネルギー供給と行動順の決定
 * - ターンカウント
 */
class TurnManager {
public:
    /**
     * @brief ターン管理の状態
     */
    enum class State {
        ProcessingTurns,  // ターン処理中（AIが行動中など）
        AwaitingInput,    // プレイヤー入力待ち
        Animating,        // アニメーション中（将来用）
        GameOver,         // ゲーム終了
    };
    
    TurnManager() = default;
    
    /**
     * @brief ターンシステムを更新
     * 
     * 毎フレーム呼び出され、以下を行う:
     * 1. 入力待ち中はプレイヤーの行動コマンドを確認
     * 2. 行動可能なアクターがいれば行動を実行
     * 3. 全員行動不可ならエネルギーを供給
     * 
     * @param registry ECSレジストリ
     * @return 現在の状態
     */
    State Update(entt::registry& registry) {
        // ゲームオーバー状態なら何もしない
        if (state_ == State::GameOver) {
            return state_;
        }
        
        // 入力待ち中の場合
        if (state_ == State::AwaitingInput) {
            // プレイヤーが行動コマンドを持っているか確認
            auto view = registry.view<Components::TurnActor, 
                                       Components::ActionCommand,
                                       Components::AwaitingInput>();
            
            for (auto entity : view) {
                auto& cmd = view.get<Components::ActionCommand>(entity);
                if (cmd.type != Components::ActionCommand::Type::None) {
                    // コマンドが設定された → 入力待ち解除
                    registry.remove<Components::AwaitingInput>(entity);
                    state_ = State::ProcessingTurns;
                    currentActor_ = entity;
                    return state_;
                }
            }
            
            // まだ入力待ち
            return state_;
        }
        
        // ターン処理中
        // 現在のアクターが行動待ちなら実行（呼び出し元で処理）
        if (currentActor_ != entt::null && registry.valid(currentActor_)) {
            auto* actor = registry.try_get<Components::TurnActor>(currentActor_);
            auto* cmd = registry.try_get<Components::ActionCommand>(currentActor_);
            
            if (actor && cmd && cmd->type != Components::ActionCommand::Type::None) {
                // 行動実行は外部システムで行う
                // ここでは行動コマンドがあることを確認するだけ
                return state_;
            }
        }
        
        // 次のアクターを探す
        currentActor_ = FindNextActor(registry);
        
        if (currentActor_ == entt::null) {
            // 行動可能なアクターがいない → エネルギー供給
            GiveEnergy(registry);
            currentActor_ = FindNextActor(registry);
        }
        
        if (currentActor_ != entt::null) {
            auto& actor = registry.get<Components::TurnActor>(currentActor_);
            
            if (actor.isPlayer) {
                // プレイヤーの番 → 入力待ちへ
                registry.emplace_or_replace<Components::AwaitingInput>(currentActor_);
                state_ = State::AwaitingInput;
            } else {
                // モンスターの番（AIシステムで行動決定）
                state_ = State::ProcessingTurns;
            }
        }
        
        return state_;
    }
    
    /**
     * @brief 現在のアクターの行動を完了
     * 
     * 行動を実行した後に呼び出す。
     * エネルギーを消費し、次のアクターへ移行。
     */
    void CompleteAction(entt::registry& registry) {
        if (currentActor_ != entt::null && registry.valid(currentActor_)) {
            auto* actor = registry.try_get<Components::TurnActor>(currentActor_);
            auto* cmd = registry.try_get<Components::ActionCommand>(currentActor_);
            
            if (actor) {
                actor->SpendEnergy();
                turnCount_++;
            }
            
            if (cmd) {
                cmd->Clear();
            }
        }
        
        currentActor_ = entt::null;
        state_ = State::ProcessingTurns;
    }
    
    /**
     * @brief ゲームオーバーを設定
     */
    void SetGameOver() {
        state_ = State::GameOver;
    }
    
    // アクセサ
    State GetState() const { return state_; }
    entt::entity GetCurrentActor() const { return currentActor_; }
    int GetTurnCount() const { return turnCount_; }
    bool IsAwaitingInput() const { return state_ == State::AwaitingInput; }
    bool IsGameOver() const { return state_ == State::GameOver; }

private:
    /**
     * @brief 全アクターにエネルギーを供給
     */
    void GiveEnergy(entt::registry& registry) {
        auto view = registry.view<Components::TurnActor>();
        for (auto entity : view) {
            auto& actor = view.get<Components::TurnActor>(entity);
            actor.GainEnergy();
        }
    }
    
    /**
     * @brief 次に行動可能なアクターを探す
     * 
     * energy >= 100 のアクターの中で最もenergyが高いものを返す。
     * 同率の場合はプレイヤー優先。
     */
    entt::entity FindNextActor(entt::registry& registry) {
        entt::entity bestActor = entt::null;
        int bestEnergy = 99;  // 100以上を探す
        bool bestIsPlayer = false;
        
        auto view = registry.view<Components::TurnActor>();
        for (auto entity : view) {
            auto& actor = view.get<Components::TurnActor>(entity);
            
            if (!actor.CanAct()) continue;
            
            // より高いエネルギーか、同率でプレイヤー優先
            bool isBetter = (actor.energy > bestEnergy) ||
                           (actor.energy == bestEnergy && actor.isPlayer && !bestIsPlayer);
            
            if (isBetter) {
                bestActor = entity;
                bestEnergy = actor.energy;
                bestIsPlayer = actor.isPlayer;
            }
        }
        
        return bestActor;
    }
    
    State state_ = State::ProcessingTurns;
    entt::entity currentActor_ = entt::null;
    int turnCount_ = 0;
};

} // namespace Roguelike::Managers

