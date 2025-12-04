/**
 * @file ActionSystem.h
 * @brief 行動実行システム
 * 
 * ActionCommandを読み取り、実際の行動を実行する。
 * 移動、攻撃、アイテム拾得などを処理。
 * 
 * Phase 3: 新しい統一アーキテクチャに移行
 */
#pragma once

#include <entt/entt.hpp>
#include "../Components/RoguelikeComponents.h"

namespace Domain::Roguelike::Systems {

/**
 * @brief 行動実行システム
 * 
 * 現在のアクターのActionCommandを処理して
 * ゲーム状態を更新する。
 */
class ActionSystem {
public:
    /**
     * @brief 行動を実行
     * 
     * @param registry ECSレジストリ
     * @param map マップデータ
     * @param entity 行動するエンティティ
     * @return 行動が成功したらtrue
     */
    static bool ExecuteAction(entt::registry& registry, 
                              Components::MapData& map,
                              entt::entity entity) {
        auto* cmd = registry.try_get<Components::ActionCommand>(entity);
        if (!cmd || cmd->type == Components::ActionCommand::Type::None) {
            return false;
        }
        
        bool success = false;
        
        switch (cmd->type) {
            case Components::ActionCommand::Type::Move:
                success = ExecuteMove(registry, map, entity, cmd->dx, cmd->dy);
                break;
                
            case Components::ActionCommand::Type::Wait:
                success = ExecuteWait(registry, entity);
                break;
                
            case Components::ActionCommand::Type::PickUp:
                success = ExecutePickUp(registry, map, entity);
                break;
                
            case Components::ActionCommand::Type::Descend:
                success = ExecuteDescend(registry, map, entity);
                break;
                
            case Components::ActionCommand::Type::Ascend:
                success = ExecuteAscend(registry, map, entity);
                break;
                
            default:
                // 未実装のコマンド
                success = false;
                break;
        }
        
        return success;
    }

private:
    /**
     * @brief 移動を実行
     */
    static bool ExecuteMove(entt::registry& registry,
                            Components::MapData& map,
                            entt::entity entity,
                            int dx, int dy) {
        auto* pos = registry.try_get<Components::GridPosition>(entity);
        if (!pos) return false;
        
        int newX = pos->x + dx;
        int newY = pos->y + dy;
        
        // 範囲外チェック
        if (!map.InBounds(newX, newY)) {
            return false;
        }
        
        // 歩行可能チェック
        if (!map.IsWalkable(newX, newY)) {
            return false;
        }
        
        // 他のエンティティがいるかチェック
        auto& targetTile = map.At(newX, newY);
        if (targetTile.occupant != entt::null && targetTile.occupant != entity) {
            // TODO: 攻撃処理（Phase 3で実装）
            // 今は移動できないとする
            return false;
        }
        
        // 現在位置のoccupantをクリア
        if (map.InBounds(pos->x, pos->y)) {
            map.At(pos->x, pos->y).occupant = entt::null;
        }
        
        // 移動
        pos->x = newX;
        pos->y = newY;
        
        // 新しい位置のoccupantを設定
        targetTile.occupant = entity;
        
        return true;
    }
    
    /**
     * @brief 待機を実行
     */
    static bool ExecuteWait(entt::registry& registry, entt::entity entity) {
        // 待機は常に成功（ターンを消費するだけ）
        (void)registry;
        (void)entity;
        return true;
    }
    
    /**
     * @brief アイテム拾得を実行
     */
    static bool ExecutePickUp(entt::registry& registry,
                              Components::MapData& map,
                              entt::entity entity) {
        auto* pos = registry.try_get<Components::GridPosition>(entity);
        if (!pos) return false;
        
        auto& tile = map.At(pos->x, pos->y);
        if (tile.item == entt::null) {
            // アイテムがない
            return false;
        }
        
        // TODO: インベントリシステム実装後に処理（Phase 4）
        // 今はアイテムがあることを確認するだけ
        return false;
    }
    
    /**
     * @brief 下り階段を使う
     */
    static bool ExecuteDescend(entt::registry& registry,
                               Components::MapData& map,
                               entt::entity entity) {
        auto* pos = registry.try_get<Components::GridPosition>(entity);
        if (!pos) return false;
        
        if (!map.InBounds(pos->x, pos->y)) return false;
        
        auto& tile = map.At(pos->x, pos->y);
        if (tile.type != Components::TileType::StairsDown) {
            // 下り階段の上にいない
            return false;
        }
        
        // TODO: 新フロア生成（Phase 2で実装）
        // 今は階段にいることを確認するだけ
        return true;
    }
    
    /**
     * @brief 上り階段を使う
     */
    static bool ExecuteAscend(entt::registry& registry,
                              Components::MapData& map,
                              entt::entity entity) {
        auto* pos = registry.try_get<Components::GridPosition>(entity);
        if (!pos) return false;
        
        if (!map.InBounds(pos->x, pos->y)) return false;
        
        auto& tile = map.At(pos->x, pos->y);
        if (tile.type != Components::TileType::StairsUp) {
            // 上り階段の上にいない
            return false;
        }
        
        // TODO: 前フロアへ戻る処理（Phase 2で実装）
        return true;
    }
};

} // namespace Domain::Roguelike::Systems

