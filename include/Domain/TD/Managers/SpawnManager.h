/**
 * @file SpawnManager.h
 * @brief プレイヤーユニットスポーン管理
 * 
 * コスト管理と味方ユニットのスポーンを担当。
 * Phase 2: 新しい統一アーキテクチャに移行
 */
#pragma once

#include "Core/World.h"
#include "Core/GameContext.h"
#include "Data/Definitions/CharacterDef.h"
#include "Data/Registry.h"
#include "Core/EntityFactory.h"
#include "Core/Events.h"
#include "Domain/TD/Components/TDComponents.h"

#include <vector>
#include <string>
#include <array>
#include <iostream>
#include <optional>

namespace Domain::TD::Managers {

// 既存のTD名前空間との互換性のためのエイリアス
namespace TD = Domain::TD;
// 注意: TDイベントはグローバル名前空間の ::TD に定義されているため、::TD:: で明示的に参照

/**
 * @brief デッキスロット情報
 */
struct DeckSlot {
    std::string characterId;
    float cooldownRemaining = 0.0f;
    bool isReady = true;
};

/**
 * @brief プレイヤースポーン管理クラス
 */
class SpawnManager {
public:
    static constexpr int MAX_DECK_SLOTS = 10;
    
    SpawnManager() = default;
    
    /**
     * @brief 初期化
     */
    void Initialize(float startingCost, float regenRate, float maxCost) {
        currentCost_ = startingCost;
        costRegenRate_ = regenRate;
        maxCost_ = maxCost;
        
        std::cout << "SpawnManager: Initialized with cost=" << currentCost_ 
                  << ", regen=" << regenRate << "/s, max=" << maxCost << "\n";
    }
    
    /**
     * @brief デッキをセット
     */
    void SetDeck(const std::vector<std::string>& characterIds) {
        deck_.clear();
        for (size_t i = 0; i < characterIds.size() && i < MAX_DECK_SLOTS; ++i) {
            DeckSlot slot;
            slot.characterId = characterIds[i];
            slot.cooldownRemaining = 0.0f;
            slot.isReady = true;
            deck_.push_back(slot);
        }
        
        std::cout << "SpawnManager: Deck set with " << deck_.size() << " characters\n";
    }
    
    /**
     * @brief 更新（コスト回復、クールダウン）
     */
    void Update(Core::World& world, float dt) {
        // コスト回復
        if (currentCost_ < maxCost_) {
            currentCost_ = std::min(maxCost_, currentCost_ + costRegenRate_ * dt);
        }
        
        // クールダウン更新
        for (auto& slot : deck_) {
            if (slot.cooldownRemaining > 0.0f) {
                slot.cooldownRemaining -= dt;
                if (slot.cooldownRemaining <= 0.0f) {
                    slot.cooldownRemaining = 0.0f;
                    slot.isReady = true;
                }
            }
        }
    }
    
    /**
     * @brief ユニットをスポーン可能か
     */
    bool CanSpawn(int slotIndex, Core::GameContext& ctx) const {
        if (slotIndex < 0 || slotIndex >= static_cast<int>(deck_.size())) {
            return false;
        }
        
        const auto& slot = deck_[slotIndex];
        if (!slot.isReady) {
            return false;
        }
        
        // コストチェック
        auto* registry = ctx.TryGet<Data::DefinitionRegistry>();
        if (!registry) return false;
        
        const auto* charDef = registry->TryGetCharacter(slot.characterId);
        if (!charDef) return false;
        
        // 新しいCharacterDef構造に対応（td.costを使用）
        return currentCost_ >= charDef->td.cost;
    }
    
    /**
     * @brief ユニットをスポーン
     */
    entt::entity SpawnUnit(int slotIndex, int lane, float laneY, 
                           Core::World& world, Core::GameContext& ctx) {
        if (!CanSpawn(slotIndex, ctx)) {
            return entt::null;
        }
        
        auto& slot = deck_[slotIndex];
        auto* registry = ctx.TryGet<Data::DefinitionRegistry>();
        auto* factory = ctx.TryGet<Core::EntityFactory>();
        
        if (!registry || !factory) {
            return entt::null;
        }
        
        const auto* charDef = registry->TryGetCharacter(slot.characterId);
        if (!charDef) {
            return entt::null;
        }
        
        // コスト消費（新しいCharacterDef構造に対応）
        currentCost_ -= charDef->td.cost;
        
        // クールダウン設定
        slot.cooldownRemaining = charDef->td.rechargeTime;
        slot.isReady = false;
        
        // ユニット生成（FHD座標: 1920x1080）
        float spawnX = 1820.0f;  // プレイヤー拠点付近から（右端）
        
        auto entity = factory->CreateCharacterInLane(
            slot.characterId,
            spawnX,
            lane,
            laneY,
            false,  // 味方
            1       // レベル
        );
        
        if (entity != entt::null) {
            // 移動方向を左に設定（敵陣へ向かう）
            if (auto* movement = world.TryGet<Domain::TD::Components::Movement>(entity)) {
                movement->direction = -1.0f;
            }
            
            // スポーンイベント
            world.Emit(::TD::UnitSpawned{
                entity,
                slot.characterId,
                lane,
                false
            });
            
            std::cout << "SpawnManager: Spawned " << slot.characterId 
                      << " at lane " << lane << ", cost remaining: " << currentCost_ << "\n";
        }
        
        return entity;
    }
    
    /**
     * @brief コスト加算（敵撃破ボーナスなど）
     */
    void AddCost(float amount) {
        currentCost_ = std::min(maxCost_, currentCost_ + amount);
    }
    
    /**
     * @brief 現在のコストを取得
     */
    float GetCurrentCost() const { return currentCost_; }
    
    /**
     * @brief 最大コストを取得
     */
    float GetMaxCost() const { return maxCost_; }
    
    /**
     * @brief コスト回復レートを取得
     */
    float GetCostRegenRate() const { return costRegenRate_; }
    
    /**
     * @brief デッキスロット情報を取得
     */
    const std::vector<DeckSlot>& GetDeck() const { return deck_; }
    
    /**
     * @brief 特定スロットのクールダウン残り時間を取得
     */
    float GetSlotCooldown(int slotIndex) const {
        if (slotIndex < 0 || slotIndex >= static_cast<int>(deck_.size())) {
            return 0.0f;
        }
        return deck_[slotIndex].cooldownRemaining;
    }
    
    /**
     * @brief キャラクターのコストを取得
     */
    std::optional<float> GetCharacterCost(int slotIndex, Core::GameContext& ctx) const {
        if (slotIndex < 0 || slotIndex >= static_cast<int>(deck_.size())) {
            return std::nullopt;
        }
        
        auto* registry = ctx.TryGet<Data::DefinitionRegistry>();
        if (!registry) return std::nullopt;
        
        const auto* charDef = registry->TryGetCharacter(deck_[slotIndex].characterId);
        if (!charDef) return std::nullopt;
        
        // 新しいCharacterDef構造に対応
        return charDef->td.cost;
    }

private:
    float currentCost_ = 0.0f;
    float costRegenRate_ = 10.0f;
    float maxCost_ = 9999.0f;
    std::vector<DeckSlot> deck_;
};

} // namespace Domain::TD::Managers

