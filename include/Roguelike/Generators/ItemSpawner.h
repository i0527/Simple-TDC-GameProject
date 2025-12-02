/**
 * @file ItemSpawner.h
 * @brief アイテム生成システム
 * 
 * 階層に応じたアイテムをダンジョンに配置する。
 */
#pragma once

#include <entt/entt.hpp>
#include <random>
#include <vector>
#include "../Components/GridComponents.h"
#include "../Components/ItemComponents.h"

namespace Roguelike::Generators {

/**
 * @brief アイテム生成器
 */
class ItemSpawner {
public:
    /**
     * @brief 階層にアイテムを配置
     */
    void SpawnItemsForFloor(
        entt::registry& registry,
        Components::MapData& map,
        int floor,
        unsigned int seed
    ) {
        rng_.seed(seed + floor * 67890);
        
        // 階層に応じたアイテム数を決定
        int baseCount = 3 + floor / 2;
        std::uniform_int_distribution<> countDist(-1, 2);
        int itemCount = std::max(2, baseCount + countDist(rng_));
        
        // 配置可能な床タイルを収集
        std::vector<std::pair<int, int>> floorTiles;
        for (int y = 0; y < map.height; y++) {
            for (int x = 0; x < map.width; x++) {
                auto& tile = map.At(x, y);
                if ((tile.type == Components::TileType::Floor ||
                     tile.type == Components::TileType::Corridor) &&
                    tile.item == entt::null &&
                    tile.occupant == entt::null) {
                    floorTiles.push_back({x, y});
                }
            }
        }
        
        if (floorTiles.empty()) return;
        
        // この階層に出現するアイテムリストを取得
        auto availableItems = Components::GetItemsForFloor(floor);
        if (availableItems.empty()) return;
        
        // 重み付き抽選の準備
        float totalWeight = 0;
        for (const auto* data : availableItems) {
            totalWeight += data->spawnWeight;
        }
        
        // アイテムを生成
        std::uniform_int_distribution<> tileDist(0, static_cast<int>(floorTiles.size()) - 1);
        std::uniform_real_distribution<> weightDist(0, totalWeight);
        
        for (int i = 0; i < itemCount && !floorTiles.empty(); i++) {
            // 配置位置を選択
            int tileIndex = tileDist(rng_) % floorTiles.size();
            auto [x, y] = floorTiles[tileIndex];
            
            // アイテム種類を重み付き抽選
            const Components::ItemData* selectedItem = nullptr;
            float roll = static_cast<float>(weightDist(rng_));
            float cumWeight = 0;
            for (const auto* data : availableItems) {
                cumWeight += data->spawnWeight;
                if (roll <= cumWeight) {
                    selectedItem = data;
                    break;
                }
            }
            if (!selectedItem) selectedItem = availableItems[0];
            
            // アイテムエンティティを作成
            auto entity = CreateItem(registry, *selectedItem, x, y, floor);
            
            // マップにitemを設定
            map.At(x, y).item = entity;
            
            // 使用したタイルを除去
            floorTiles.erase(floorTiles.begin() + tileIndex);
            if (!floorTiles.empty()) {
                tileDist = std::uniform_int_distribution<>(0, static_cast<int>(floorTiles.size()) - 1);
            }
        }
    }
    
    /**
     * @brief 単体アイテムを生成
     */
    entt::entity CreateItem(
        entt::registry& registry,
        const Components::ItemData& data,
        int x, int y,
        int floor
    ) {
        auto entity = registry.create();
        
        // 位置
        registry.emplace<Components::GridPosition>(entity, x, y);
        
        // アイテム情報
        Components::Item item;
        item.id = data.id;
        item.name = data.name;
        item.description = data.description;
        item.type = data.type;
        item.symbol = data.symbol;
        item.r = data.r;
        item.g = data.g;
        item.b = data.b;
        item.weight = data.weight;
        item.value = data.value;
        
        // ゴールドは量をランダムに
        if (data.type == Components::ItemType::Gold) {
            std::uniform_int_distribution<> goldDist(5 + floor * 2, 20 + floor * 5);
            item.quantity = goldDist(rng_);
            item.value = item.quantity;
        }
        
        registry.emplace<Components::Item>(entity, item);
        
        // 見た目
        registry.emplace<Components::Appearance>(entity, data.symbol, data.r, data.g, data.b);
        
        // アイテムタグ
        registry.emplace<Components::ItemTag>(entity);
        
        // 装備可能アイテム
        if (data.equipSlot != Components::EquipSlot::None) {
            Components::Equippable equip;
            equip.slot = data.equipSlot;
            equip.attackBonus = data.attackBonus;
            equip.defenseBonus = data.defenseBonus;
            registry.emplace<Components::Equippable>(entity, equip);
        }
        
        // 消費アイテム
        if (data.effect != Components::Consumable::EffectType::None) {
            Components::Consumable consumable;
            consumable.effect = data.effect;
            consumable.value = data.effectValue;
            consumable.message = data.useMessage;
            registry.emplace<Components::Consumable>(entity, consumable);
        }
        
        return entity;
    }
    
    /**
     * @brief マップ上の全アイテムを削除
     */
    void ClearItems(entt::registry& registry, Components::MapData& map) {
        std::vector<entt::entity> toDestroy;
        
        registry.each([&](entt::entity entity) {
            if (registry.any_of<Components::ItemTag>(entity)) {
                auto* pos = registry.try_get<Components::GridPosition>(entity);
                if (pos && map.InBounds(pos->x, pos->y)) {
                    map.At(pos->x, pos->y).item = entt::null;
                }
                toDestroy.push_back(entity);
            }
        });
        
        for (auto entity : toDestroy) {
            registry.destroy(entity);
        }
    }
    
private:
    std::mt19937 rng_;
};

} // namespace Roguelike::Generators
