/**
 * @file ItemSystem.h
 * @brief アイテム操作システム
 * 
 * アイテムの拾得、使用、装備、ドロップを処理する。
 * 
 * Phase 3: 新しい統一アーキテクチャに移行
 */
#pragma once

#include <entt/entt.hpp>
#include <string>
#include <functional>
#include "../Components/RoguelikeComponents.h"

namespace Domain::Roguelike::Systems {

/**
 * @brief アイテムシステム
 */
class ItemSystem {
public:
    using MessageCallback = std::function<void(const std::string&)>;
    
    /**
     * @brief アイテムを拾う
     * @return 成功したらtrue
     */
    static bool PickupItem(
        entt::registry& registry,
        Components::MapData& map,
        entt::entity actor,
        MessageCallback onMessage = nullptr
    ) {
        auto* pos = registry.try_get<Components::GridPosition>(actor);
        auto* inventory = registry.try_get<Components::Inventory>(actor);
        
        if (!pos || !inventory) return false;
        if (!map.InBounds(pos->x, pos->y)) return false;
        
        auto& tile = map.At(pos->x, pos->y);
        if (tile.item == entt::null) {
            if (onMessage) onMessage("ここには何もない。");
            return false;
        }
        
        auto itemEntity = tile.item;
        auto* item = registry.try_get<Components::Item>(itemEntity);
        if (!item) return false;
        
        // ゴールドは直接加算
        if (item->type == Components::ItemType::Gold) {
            inventory->gold += item->quantity;
            if (onMessage) {
                onMessage(std::to_string(item->quantity) + "ゴールドを拾った。");
            }
            tile.item = entt::null;
            registry.destroy(itemEntity);
            return true;
        }
        
        // インベントリに追加
        int slot = inventory->AddItem(itemEntity);
        if (slot < 0) {
            if (onMessage) onMessage("持ち物がいっぱいだ！");
            return false;
        }
        
        // マップから削除、位置コンポーネントを削除
        tile.item = entt::null;
        registry.remove<Components::GridPosition>(itemEntity);
        registry.remove<Components::ItemTag>(itemEntity);
        
        if (onMessage) {
            char slotChar = Components::Inventory::GetSlotChar(slot);
            onMessage(std::string(1, slotChar) + " - " + item->name + "を拾った。");
        }
        
        return true;
    }
    
    /**
     * @brief アイテムを落とす
     */
    static bool DropItem(
        entt::registry& registry,
        Components::MapData& map,
        entt::entity actor,
        int slot,
        MessageCallback onMessage = nullptr
    ) {
        auto* pos = registry.try_get<Components::GridPosition>(actor);
        auto* inventory = registry.try_get<Components::Inventory>(actor);
        
        if (!pos || !inventory) return false;
        if (!map.InBounds(pos->x, pos->y)) return false;
        if (slot < 0 || slot >= Components::Inventory::MAX_SLOTS) return false;
        
        auto& tile = map.At(pos->x, pos->y);
        if (tile.item != entt::null) {
            if (onMessage) onMessage("ここには既にアイテムがある。");
            return false;
        }
        
        auto itemEntity = inventory->RemoveItem(slot);
        if (itemEntity == entt::null) {
            if (onMessage) onMessage("そのスロットにはアイテムがない。");
            return false;
        }
        
        // 装備中なら外す
        auto* equipment = registry.try_get<Components::Equipment>(actor);
        if (equipment) {
            UnequipIfEquipped(registry, actor, itemEntity);
        }
        
        // マップに配置
        registry.emplace<Components::GridPosition>(itemEntity, pos->x, pos->y);
        registry.emplace<Components::ItemTag>(itemEntity);
        tile.item = itemEntity;
        
        auto* item = registry.try_get<Components::Item>(itemEntity);
        if (item && onMessage) {
            onMessage(item->name + "を落とした。");
        }
        
        return true;
    }
    
    /**
     * @brief アイテムを使用
     */
    static bool UseItem(
        entt::registry& registry,
        entt::entity actor,
        int slot,
        MessageCallback onMessage = nullptr
    ) {
        auto* inventory = registry.try_get<Components::Inventory>(actor);
        if (!inventory) return false;
        if (slot < 0 || slot >= Components::Inventory::MAX_SLOTS) return false;
        
        auto itemEntity = inventory->items[slot];
        if (itemEntity == entt::null) {
            if (onMessage) onMessage("そのスロットにはアイテムがない。");
            return false;
        }
        
        auto* item = registry.try_get<Components::Item>(itemEntity);
        if (!item) return false;
        
        // 装備可能アイテムなら装備
        auto* equippable = registry.try_get<Components::Equippable>(itemEntity);
        if (equippable) {
            return EquipItem(registry, actor, slot, onMessage);
        }
        
        // 消費アイテムなら使用
        auto* consumable = registry.try_get<Components::Consumable>(itemEntity);
        if (consumable) {
            return ConsumeItem(registry, actor, slot, onMessage);
        }
        
        if (onMessage) {
            onMessage(item->name + "は使用できない。");
        }
        return false;
    }
    
    /**
     * @brief 消費アイテムを使用
     */
    static bool ConsumeItem(
        entt::registry& registry,
        entt::entity actor,
        int slot,
        MessageCallback onMessage = nullptr
    ) {
        auto* inventory = registry.try_get<Components::Inventory>(actor);
        if (!inventory) return false;
        
        auto itemEntity = inventory->items[slot];
        if (itemEntity == entt::null) return false;
        
        auto* item = registry.try_get<Components::Item>(itemEntity);
        auto* consumable = registry.try_get<Components::Consumable>(itemEntity);
        if (!item || !consumable) return false;
        
        bool used = false;
        
        switch (consumable->effect) {
            case Components::Consumable::EffectType::Heal: {
                auto* health = registry.try_get<Components::Health>(actor);
                if (health) {
                    int oldHP = health->current;
                    health->Heal(consumable->value);
                    int healed = health->current - oldHP;
                    if (onMessage) {
                        onMessage(consumable->message + " (+" + std::to_string(healed) + "HP)");
                    }
                    used = true;
                }
                break;
            }
            
            case Components::Consumable::EffectType::FullHeal: {
                auto* health = registry.try_get<Components::Health>(actor);
                if (health) {
                    int oldHP = health->current;
                    health->current = health->max;
                    int healed = health->current - oldHP;
                    if (onMessage) {
                        onMessage(consumable->message + " (+" + std::to_string(healed) + "HP)");
                    }
                    used = true;
                }
                break;
            }
            
            case Components::Consumable::EffectType::Food: {
                // 空腹度回復
                auto* hunger = registry.try_get<Components::Hunger>(actor);
                if (hunger) {
                    auto prevState = hunger->GetState();
                    hunger->Eat(consumable->value);
                    auto newState = hunger->GetState();
                    
                    if (onMessage) {
                        onMessage(consumable->message);
                        if (newState == Components::HungerState::Satiated && 
                            prevState != Components::HungerState::Satiated) {
                            onMessage("満腹になった！");
                        } else if (prevState >= Components::HungerState::Weak) {
                            onMessage("お腹が落ち着いた。");
                        }
                    }
                } else {
                    if (onMessage) {
                        onMessage(consumable->message);
                    }
                }
                used = true;
                break;
            }
            
            default:
                if (onMessage) {
                    onMessage(item->name + "を使用した。");
                }
                used = true;
                break;
        }
        
        if (used) {
            // アイテム消費（数量を減らすかスロットから削除）
            item->quantity--;
            if (item->quantity <= 0) {
                inventory->RemoveItem(slot);
                registry.destroy(itemEntity);
            }
        }
        
        return used;
    }
    
    /**
     * @brief アイテムを装備
     */
    static bool EquipItem(
        entt::registry& registry,
        entt::entity actor,
        int slot,
        MessageCallback onMessage = nullptr
    ) {
        auto* inventory = registry.try_get<Components::Inventory>(actor);
        auto* equipment = registry.try_get<Components::Equipment>(actor);
        auto* stats = registry.try_get<Components::CombatStats>(actor);
        
        if (!inventory || !equipment) return false;
        
        auto itemEntity = inventory->items[slot];
        if (itemEntity == entt::null) return false;
        
        auto* item = registry.try_get<Components::Item>(itemEntity);
        auto* equippable = registry.try_get<Components::Equippable>(itemEntity);
        if (!item || !equippable) return false;
        
        auto equipSlot = equippable->slot;
        
        // 既に装備中のアイテムがあれば外す
        auto currentEquip = equipment->GetSlot(equipSlot);
        if (currentEquip != entt::null) {
            // 現在の装備のボーナスを外す
            if (stats) {
                auto* oldEquippable = registry.try_get<Components::Equippable>(currentEquip);
                if (oldEquippable) {
                    stats->attack -= oldEquippable->attackBonus;
                    stats->defense -= oldEquippable->defenseBonus;
                }
            }
            
            auto* oldItem = registry.try_get<Components::Item>(currentEquip);
            if (oldItem && onMessage) {
                onMessage(oldItem->name + "を外した。");
            }
        }
        
        // 新しいアイテムを装備
        equipment->SetSlot(equipSlot, itemEntity);
        
        // ボーナスを適用
        if (stats) {
            stats->attack += equippable->attackBonus;
            stats->defense += equippable->defenseBonus;
        }
        
        if (onMessage) {
            onMessage(item->name + "を装備した。");
        }
        
        return true;
    }
    
    /**
     * @brief 装備を外す
     */
    static bool UnequipItem(
        entt::registry& registry,
        entt::entity actor,
        Components::EquipSlot slot,
        MessageCallback onMessage = nullptr
    ) {
        auto* equipment = registry.try_get<Components::Equipment>(actor);
        auto* stats = registry.try_get<Components::CombatStats>(actor);
        
        if (!equipment) return false;
        
        auto itemEntity = equipment->GetSlot(slot);
        if (itemEntity == entt::null) {
            if (onMessage) onMessage("何も装備していない。");
            return false;
        }
        
        // ボーナスを外す
        if (stats) {
            auto* equippable = registry.try_get<Components::Equippable>(itemEntity);
            if (equippable) {
                stats->attack -= equippable->attackBonus;
                stats->defense -= equippable->defenseBonus;
            }
        }
        
        equipment->SetSlot(slot, entt::null);
        
        auto* item = registry.try_get<Components::Item>(itemEntity);
        if (item && onMessage) {
            onMessage(item->name + "を外した。");
        }
        
        return true;
    }
    
    /**
     * @brief 装備中のアイテムを外す（ヘルパー）
     */
    static void UnequipIfEquipped(
        entt::registry& registry,
        entt::entity actor,
        entt::entity itemEntity
    ) {
        auto* equipment = registry.try_get<Components::Equipment>(actor);
        auto* stats = registry.try_get<Components::CombatStats>(actor);
        if (!equipment) return;
        
        auto* equippable = registry.try_get<Components::Equippable>(itemEntity);
        if (!equippable) return;
        
        if (equipment->GetSlot(equippable->slot) == itemEntity) {
            if (stats) {
                stats->attack -= equippable->attackBonus;
                stats->defense -= equippable->defenseBonus;
            }
            equipment->SetSlot(equippable->slot, entt::null);
        }
    }
};

} // namespace Domain::Roguelike::Systems

