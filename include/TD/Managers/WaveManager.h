/**
 * @file WaveManager.h
 * @brief Wave進行管理
 * 
 * ステージのWave進行、敵スポーン、クリア判定を管理。
 */
#pragma once

#include "Core/World.h"
#include "Core/GameContext.h"
#include "Core/Definitions.h"
#include "Core/DefinitionRegistry.h"
#include "Core/EntityFactory.h"
#include "Core/Events.h"
#include "TD/Components/TDComponents.h"

#include <vector>
#include <string>
#include <iostream>

namespace TD {

/**
 * @brief スポーン待機中のエントリ
 */
struct PendingSpawn {
    std::string characterId;
    int lane;
    float spawnTime;        // Wave開始からの時間
    int remainingCount;     // 残りスポーン数
    float nextSpawnTime;    // 次のスポーン時間
    float interval;         // スポーン間隔
};

/**
 * @brief Wave管理クラス
 */
class WaveManager {
public:
    WaveManager() = default;
    
    /**
     * @brief ステージをロード
     */
    void LoadStage(const Data::StageDef& stageDef) {
        stageDef_ = &stageDef;
        currentWaveIndex_ = 0;
        waveTimer_ = 0.0f;
        isWaveActive_ = false;
        allWavesCompleted_ = false;
        pendingSpawns_.clear();
        
        std::cout << "WaveManager: Loaded stage '" << stageDef.name 
                  << "' with " << stageDef.waves.size() << " waves\n";
    }
    
    /**
     * @brief 次のWaveを開始
     */
    void StartNextWave(Core::World& world) {
        if (!stageDef_ || currentWaveIndex_ >= stageDef_->waves.size()) {
            return;
        }
        
        const auto& waveDef = stageDef_->waves[currentWaveIndex_];
        waveTimer_ = 0.0f;
        isWaveActive_ = true;
        pendingSpawns_.clear();
        
        // スポーンエントリを準備
        for (const auto& entry : waveDef.enemies) {
            PendingSpawn pending;
            pending.characterId = entry.characterId;
            pending.lane = entry.lane;
            pending.spawnTime = entry.delay;
            pending.remainingCount = entry.count;
            pending.nextSpawnTime = entry.delay;
            pending.interval = entry.interval;
            pendingSpawns_.push_back(pending);
        }
        
        // イベント発火
        world.Emit(TD::WaveStarted{
            waveDef.waveNumber,
            static_cast<int>(stageDef_->waves.size())
        });
        
        std::cout << "WaveManager: Starting wave " << (currentWaveIndex_ + 1) 
                  << "/" << stageDef_->waves.size() << "\n";
    }
    
    /**
     * @brief 更新
     */
    void Update(Core::World& world, Core::GameContext& ctx, float dt) {
        if (!stageDef_ || !isWaveActive_) return;
        
        waveTimer_ += dt;
        
        // スポーン処理
        ProcessSpawns(world, ctx);
        
        // Wave完了チェック
        CheckWaveCompletion(world);
    }
    
    /**
     * @brief 現在のWave番号を取得（1始まり）
     */
    int GetCurrentWaveNumber() const {
        return currentWaveIndex_ + 1;
    }
    
    /**
     * @brief 総Wave数を取得
     */
    int GetTotalWaves() const {
        return stageDef_ ? static_cast<int>(stageDef_->waves.size()) : 0;
    }
    
    /**
     * @brief Waveがアクティブか
     */
    bool IsWaveActive() const { return isWaveActive_; }
    
    /**
     * @brief 全Wave完了したか
     */
    bool IsAllWavesCompleted() const { return allWavesCompleted_; }
    
    /**
     * @brief ステージのコスト設定を取得
     */
    float GetStartingCost() const { return stageDef_ ? stageDef_->startingCost : 500.0f; }
    float GetCostRegenRate() const { return stageDef_ ? stageDef_->costRegenRate : 10.0f; }
    float GetMaxCost() const { return stageDef_ ? stageDef_->maxCost : 9999.0f; }
    
    /**
     * @brief レーン情報を取得
     */
    int GetLaneCount() const { return stageDef_ ? stageDef_->laneCount : 1; }
    float GetLaneHeight() const { return stageDef_ ? stageDef_->laneHeight : 100.0f; }
    
    /**
     * @brief レーンのY座標を計算
     */
    float GetLaneY(int laneIndex, float screenHeight) const {
        int laneCount = GetLaneCount();
        float totalLaneArea = laneCount * GetLaneHeight();
        float startY = (screenHeight - totalLaneArea) / 2.0f + GetLaneHeight() / 2.0f;
        return startY + laneIndex * GetLaneHeight();
    }

private:
    void ProcessSpawns(Core::World& world, Core::GameContext& ctx) {
        auto* factory = ctx.TryGet<Core::EntityFactory>();
        if (!factory) return;
        
        for (auto& pending : pendingSpawns_) {
            while (pending.remainingCount > 0 && waveTimer_ >= pending.nextSpawnTime) {
                // 敵をスポーン（FHD座標: 1920x1080）
                constexpr float FHD_HEIGHT = 1080.0f;
                float spawnX = 100.0f;  // 敵拠点付近から（左端）
                float laneY = GetLaneY(pending.lane, FHD_HEIGHT);
                
                auto entity = factory->CreateCharacterInLane(
                    pending.characterId,
                    spawnX,
                    pending.lane,
                    laneY,
                    true,  // 敵
                    1      // レベル
                );
                
                if (entity != entt::null) {
                    // 移動方向を右に設定（自陣へ向かう）
                    if (auto* movement = world.TryGet<TD::Components::Movement>(entity)) {
                        movement->direction = 1.0f;
                    }
                    
                    // スポーンイベント
                    world.Emit(TD::UnitSpawned{
                        entity,
                        pending.characterId,
                        pending.lane,
                        true
                    });
                }
                
                pending.remainingCount--;
                pending.nextSpawnTime += pending.interval;
            }
        }
    }
    
    void CheckWaveCompletion(Core::World& world) {
        // 全スポーン完了かつ敵が全滅したらWave完了
        bool allSpawned = true;
        for (const auto& pending : pendingSpawns_) {
            if (pending.remainingCount > 0) {
                allSpawned = false;
                break;
            }
        }
        
        if (!allSpawned) return;
        
        // 敵の生存チェック
        auto view = world.View<TD::Components::EnemyUnit, TD::Components::Unit>();
        int aliveEnemies = 0;
        for (auto entity : view) {
            if (!world.HasAll<TD::Components::Dying>(entity)) {
                aliveEnemies++;
            }
        }
        
        if (aliveEnemies == 0) {
            // Wave完了
            isWaveActive_ = false;
            
            world.Emit(TD::WaveCompleted{
                static_cast<int>(currentWaveIndex_ + 1),
                static_cast<int>(stageDef_->waves.size() - currentWaveIndex_ - 1)
            });
            
            std::cout << "WaveManager: Wave " << (currentWaveIndex_ + 1) << " completed\n";
            
            currentWaveIndex_++;
            
            if (currentWaveIndex_ >= stageDef_->waves.size()) {
                allWavesCompleted_ = true;
                world.Emit(TD::AllWavesCompleted{
                    static_cast<int>(stageDef_->waves.size()),
                    waveTimer_
                });
                std::cout << "WaveManager: All waves completed!\n";
            } else {
                // 次のWaveを自動開始（または手動トリガーを待つ）
                StartNextWave(world);
            }
        }
    }
    
    const Data::StageDef* stageDef_ = nullptr;
    size_t currentWaveIndex_ = 0;
    float waveTimer_ = 0.0f;
    bool isWaveActive_ = false;
    bool allWavesCompleted_ = false;
    std::vector<PendingSpawn> pendingSpawns_;
};

} // namespace TD
