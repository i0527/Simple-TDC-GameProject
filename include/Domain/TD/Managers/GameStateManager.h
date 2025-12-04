/**
 * @file GameStateManager.h
 * @brief ゲーム状態管理
 * 
 * 勝利/敗北判定、ゲーム進行状態を管理。
 * Phase 2: 新しい統一アーキテクチャに移行
 */
#pragma once

#include "Core/World.h"
#include "Core/GameContext.h"
#include "Core/Events.h"
#include "Core/Components/CoreComponents.h"
#include "Domain/TD/Components/TDComponents.h"
#include "Domain/TD/Managers/WaveManager.h"
#include "Domain/TD/Managers/SpawnManager.h"

#include <iostream>

namespace Domain::TD::Managers {

// 既存のTD名前空間との互換性のためのエイリアス
namespace TD = Domain::TD;
// 注意: TDイベントはグローバル名前空間の ::TD に定義されているため、::TD:: で明示的に参照

/**
 * @brief ゲームの進行状態
 */
enum class GamePhase {
    NotStarted,     // 開始前
    Playing,        // プレイ中
    Paused,         // 一時停止
    Victory,        // 勝利
    Defeat          // 敗北
};

/**
 * @brief ゲーム状態管理クラス
 */
class GameStateManager {
public:
    GameStateManager() = default;
    
    /**
     * @brief 初期化
     */
    void Initialize() {
        phase_ = GamePhase::NotStarted;
        elapsedTime_ = 0.0f;
        baseHealth_ = 100.0f;
        maxBaseHealth_ = 100.0f;
        enemyBaseHealth_ = 100.0f;
        enemyMaxBaseHealth_ = 100.0f;
        
        std::cout << "GameStateManager: Initialized\n";
    }
    
    /**
     * @brief 拠点HPを設定
     */
    void SetBaseHealth(float playerBase, float enemyBase) {
        baseHealth_ = playerBase;
        maxBaseHealth_ = playerBase;
        enemyBaseHealth_ = enemyBase;
        enemyMaxBaseHealth_ = enemyBase;
    }
    
    /**
     * @brief ゲームを開始
     */
    void StartGame(Core::World& world, WaveManager& waveManager) {
        if (phase_ != GamePhase::NotStarted && phase_ != GamePhase::Victory && phase_ != GamePhase::Defeat) {
            return;
        }
        
        phase_ = GamePhase::Playing;
        elapsedTime_ = 0.0f;
        baseHealth_ = maxBaseHealth_;
        enemyBaseHealth_ = enemyMaxBaseHealth_;
        
        // 最初のWaveを開始
        waveManager.StartNextWave(world);
        
        // イベント発火
        world.Emit(::TD::GameStarted{});
        
        std::cout << "GameStateManager: Game started\n";
    }
    
    /**
     * @brief 一時停止
     */
    void PauseGame() {
        if (phase_ == GamePhase::Playing) {
            phase_ = GamePhase::Paused;
            std::cout << "GameStateManager: Game paused\n";
        }
    }
    
    /**
     * @brief 再開
     */
    void ResumeGame() {
        if (phase_ == GamePhase::Paused) {
            phase_ = GamePhase::Playing;
            std::cout << "GameStateManager: Game resumed\n";
        }
    }
    
    /**
     * @brief 更新
     */
    void Update(Core::World& world, WaveManager& waveManager, float dt) {
        if (phase_ != GamePhase::Playing) return;
        
        elapsedTime_ += dt;
        
        // 敗北判定：味方拠点が破壊された
        CheckDefeat(world);
        
        // 勝利判定：全Wave完了 かつ 敵全滅
        CheckVictory(world, waveManager);
        
        // 拠点への攻撃チェック（敵が画面左端到達）
        CheckBaseAttack(world);
    }
    
    /**
     * @brief 味方拠点にダメージ
     */
    void DamagePlayerBase(Core::World& world, float damage) {
        if (phase_ != GamePhase::Playing) return;
        
        baseHealth_ -= damage;
        
        world.Emit(::TD::BaseDamaged{
            false,  // プレイヤー側
            damage,
            baseHealth_,
            maxBaseHealth_
        });
        
        if (baseHealth_ <= 0.0f) {
            baseHealth_ = 0.0f;
            TriggerDefeat(world);
        }
    }
    
    /**
     * @brief 敵拠点にダメージ
     */
    void DamageEnemyBase(Core::World& world, float damage) {
        if (phase_ != GamePhase::Playing) return;
        
        enemyBaseHealth_ -= damage;
        
        world.Emit(::TD::BaseDamaged{
            true,   // 敵側
            damage,
            enemyBaseHealth_,
            enemyMaxBaseHealth_
        });
        
        if (enemyBaseHealth_ <= 0.0f) {
            enemyBaseHealth_ = 0.0f;
            TriggerVictory(world);
        }
    }
    
    /**
     * @brief ゲームフェーズを取得
     */
    GamePhase GetPhase() const { return phase_; }
    
    /**
     * @brief プレイ中か
     */
    bool IsPlaying() const { return phase_ == GamePhase::Playing; }
    
    /**
     * @brief ゲームオーバーか（勝敗問わず）
     */
    bool IsGameOver() const { 
        return phase_ == GamePhase::Victory || phase_ == GamePhase::Defeat; 
    }
    
    /**
     * @brief 経過時間を取得
     */
    float GetElapsedTime() const { return elapsedTime_; }
    
    /**
     * @brief 味方拠点HP
     */
    float GetBaseHealth() const { return baseHealth_; }
    float GetMaxBaseHealth() const { return maxBaseHealth_; }
    float GetBaseHealthPercent() const { 
        return maxBaseHealth_ > 0 ? baseHealth_ / maxBaseHealth_ : 0.0f; 
    }
    
    /**
     * @brief 敵拠点HP
     */
    float GetEnemyBaseHealth() const { return enemyBaseHealth_; }
    float GetEnemyMaxBaseHealth() const { return enemyMaxBaseHealth_; }
    float GetEnemyBaseHealthPercent() const { 
        return enemyMaxBaseHealth_ > 0 ? enemyBaseHealth_ / enemyMaxBaseHealth_ : 0.0f; 
    }

private:
    void CheckDefeat(Core::World& world) {
        if (baseHealth_ <= 0.0f) {
            TriggerDefeat(world);
        }
    }
    
    void CheckVictory(Core::World& world, WaveManager& waveManager) {
        if (!waveManager.IsAllWavesCompleted()) return;
        
        // 敵が全滅しているか確認
        auto view = world.View<Domain::TD::Components::EnemyUnit>();
        bool hasEnemies = false;
        for (auto entity : view) {
            if (!world.HasAll<Domain::TD::Components::Dying>(entity)) {
                hasEnemies = true;
                break;
            }
        }
        
        if (!hasEnemies) {
            TriggerVictory(world);
        }
    }
    
    void CheckBaseAttack(Core::World& world) {
        using namespace Core::Components;
        using namespace Domain::TD::Components;
        
        // FHD座標基準（1920x1080）
        // 敵が右端（プレイヤー拠点位置）に到達したらダメージ
        constexpr float PLAYER_BASE_X = 1850.0f;  // 右端=プレイヤー拠点（FHD）
        
        auto view = world.View<EnemyUnit, Position, Stats>();
        for (auto entity : view) {
            const auto& pos = view.get<Position>(entity);
            const auto& stats = view.get<Stats>(entity);
            
            if (pos.x >= PLAYER_BASE_X) {
                // プレイヤー拠点にダメージを与えて自壊
                DamagePlayerBase(world, stats.attack);
                
                // このユニットを削除
                if (!world.HasAll<Dying>(entity)) {
                    Dying dying;
                    dying.animationProgress = 0.0f;
                    dying.skipAnimation = true;
                    world.Emplace<Dying>(entity, dying);
                }
            }
        }
        
        // 味方が左端（敵拠点位置）に到達したらダメージ
        constexpr float ENEMY_BASE_X = 70.0f;  // 左端=敵拠点（FHD）
        
        auto allyView = world.View<AllyUnit, Position, Stats>();
        for (auto entity : allyView) {
            const auto& pos = allyView.get<Position>(entity);
            const auto& stats = allyView.get<Stats>(entity);
            
            if (pos.x <= ENEMY_BASE_X) {
                // 敵拠点にダメージを与えて自壊
                DamageEnemyBase(world, stats.attack);
                
                // このユニットを削除
                if (!world.HasAll<Dying>(entity)) {
                    Dying dying;
                    dying.animationProgress = 0.0f;
                    dying.skipAnimation = true;
                    world.Emplace<Dying>(entity, dying);
                }
            }
        }
    }
    
    void TriggerVictory(Core::World& world) {
        if (phase_ == GamePhase::Victory) return;
        
        phase_ = GamePhase::Victory;
        
        world.Emit(::TD::GameEnded{
            true,   // isVictory
            elapsedTime_,
            0       // TODO: score計算
        });
        
        std::cout << "GameStateManager: VICTORY! Time: " << elapsedTime_ << "s\n";
    }
    
    void TriggerDefeat(Core::World& world) {
        if (phase_ == GamePhase::Defeat) return;
        
        phase_ = GamePhase::Defeat;
        
        world.Emit(::TD::GameEnded{
            false,  // isVictory
            elapsedTime_,
            0
        });
        
        std::cout << "GameStateManager: DEFEAT! Time: " << elapsedTime_ << "s\n";
    }
    
    GamePhase phase_ = GamePhase::NotStarted;
    float elapsedTime_ = 0.0f;
    float baseHealth_ = 100.0f;
    float maxBaseHealth_ = 100.0f;
    float enemyBaseHealth_ = 100.0f;
    float enemyMaxBaseHealth_ = 100.0f;
};

} // namespace Domain::TD::Managers

