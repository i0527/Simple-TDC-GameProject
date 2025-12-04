/**
 * @file AISystem.h
 * @brief モンスターAIシステム
 * 
 * モンスターの思考・行動決定を行う。
 * - プレイヤー追跡
 * - 攻撃判断
 * - 経路探索（簡易A*）
 * 
 * Phase 3: 新しい統一アーキテクチャに移行
 */
#pragma once

#include <entt/entt.hpp>
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <cstdlib>
#include "../Components/RoguelikeComponents.h"

namespace Domain::Roguelike::Systems {

/**
 * @brief AIシステム
 */
class AISystem {
public:
    /**
     * @brief モンスターの行動を決定
     * @param registry ECSレジストリ
     * @param map マップデータ
     * @param monster モンスターエンティティ
     * @param playerEntity プレイヤーエンティティ
     */
    static void DecideAction(
        entt::registry& registry,
        const Components::MapData& map,
        entt::entity monster,
        entt::entity playerEntity
    ) {
        auto* ai = registry.try_get<Components::AI>(monster);
        auto* monsterPos = registry.try_get<Components::GridPosition>(monster);
        auto* cmd = registry.try_get<Components::ActionCommand>(monster);
        
        if (!ai || !monsterPos || !cmd) return;
        
        // プレイヤー位置を取得
        auto* playerPos = registry.try_get<Components::GridPosition>(playerEntity);
        if (!playerPos) return;
        
        // プレイヤーとの距離を計算
        int dx = playerPos->x - monsterPos->x;
        int dy = playerPos->y - monsterPos->y;
        int distSq = dx * dx + dy * dy;
        float dist = std::sqrt(static_cast<float>(distSq));
        
        // プレイヤーが視界内か確認
        ai->canSeePlayer = CanSeeTarget(map, monsterPos->x, monsterPos->y, 
                                         playerPos->x, playerPos->y, ai->sightRange);
        
        if (ai->canSeePlayer) {
            ai->lastKnownPlayerX = playerPos->x;
            ai->lastKnownPlayerY = playerPos->y;
            ai->turnsLostPlayer = 0;
        } else {
            ai->turnsLostPlayer++;
        }
        
        // AIタイプに応じた行動決定
        switch (ai->type) {
            case Components::AIType::Hostile:
                DecideHostileAction(registry, map, monster, playerPos->x, playerPos->y, 
                                    monsterPos, ai, cmd, dist);
                break;
                
            case Components::AIType::Wander:
                DecideWanderAction(registry, map, monster, monsterPos, ai, cmd);
                break;
                
            case Components::AIType::Cowardly:
                DecideCowardlyAction(registry, map, monster, playerPos->x, playerPos->y,
                                     monsterPos, ai, cmd, dist);
                break;
                
            case Components::AIType::Idle:
            default:
                // 何もしない
                cmd->type = Components::ActionCommand::Type::Wait;
                break;
        }
    }
    
private:
    /**
     * @brief 敵対AIの行動決定
     */
    static void DecideHostileAction(
        entt::registry& registry,
        const Components::MapData& map,
        entt::entity monster,
        int playerX, int playerY,
        Components::GridPosition* pos,
        Components::AI* ai,
        Components::ActionCommand* cmd,
        float dist
    ) {
        // プレイヤーに隣接していれば攻撃
        if (dist < 1.5f) {
            cmd->type = Components::ActionCommand::Type::Attack;
            cmd->targetX = playerX;
            cmd->targetY = playerY;
            return;
        }
        
        // プレイヤーが見えていれば追跡
        if (ai->canSeePlayer) {
            MoveTowards(map, pos->x, pos->y, playerX, playerY, cmd);
            return;
        }
        
        // 最後に見た位置に向かう（数ターン以内）
        if (ai->turnsLostPlayer < 5 && ai->lastKnownPlayerX >= 0) {
            MoveTowards(map, pos->x, pos->y, ai->lastKnownPlayerX, ai->lastKnownPlayerY, cmd);
            return;
        }
        
        // 見失ったらランダム移動
        RandomMove(map, pos->x, pos->y, cmd);
    }
    
    /**
     * @brief 徘徊AIの行動決定
     */
    static void DecideWanderAction(
        entt::registry& registry,
        const Components::MapData& map,
        entt::entity monster,
        Components::GridPosition* pos,
        Components::AI* ai,
        Components::ActionCommand* cmd
    ) {
        if (ai->wanderCooldown > 0) {
            ai->wanderCooldown--;
            cmd->type = Components::ActionCommand::Type::Wait;
            return;
        }
        
        // ランダム移動
        RandomMove(map, pos->x, pos->y, cmd);
        
        // 次の移動まで少し待つ
        ai->wanderCooldown = 1 + (std::rand() % 3);
    }
    
    /**
     * @brief 臆病AIの行動決定
     */
    static void DecideCowardlyAction(
        entt::registry& registry,
        const Components::MapData& map,
        entt::entity monster,
        int playerX, int playerY,
        Components::GridPosition* pos,
        Components::AI* ai,
        Components::ActionCommand* cmd,
        float dist
    ) {
        // HPが半分以下なら逃げる
        auto* health = registry.try_get<Components::Health>(monster);
        if (health && health->GetRatio() < 0.5f && ai->canSeePlayer) {
            // プレイヤーと反対方向に移動
            int fleeX = pos->x - (playerX - pos->x);
            int fleeY = pos->y - (playerY - pos->y);
            MoveTowards(map, pos->x, pos->y, fleeX, fleeY, cmd);
            return;
        }
        
        // それ以外は敵対AIと同じ
        DecideHostileAction(registry, map, monster, playerX, playerY, pos, ai, cmd, dist);
    }
    
    /**
     * @brief ターゲットに向かって移動
     */
    static void MoveTowards(
        const Components::MapData& map,
        int fromX, int fromY,
        int toX, int toY,
        Components::ActionCommand* cmd
    ) {
        // 簡易的な方向計算（A*は重いので省略）
        int dx = 0, dy = 0;
        
        if (toX > fromX) dx = 1;
        else if (toX < fromX) dx = -1;
        
        if (toY > fromY) dy = 1;
        else if (toY < fromY) dy = -1;
        
        // 直線移動を試みる
        int newX = fromX + dx;
        int newY = fromY + dy;
        
        if (CanMoveTo(map, newX, newY)) {
            cmd->type = Components::ActionCommand::Type::Move;
            cmd->dx = dx;
            cmd->dy = dy;
            return;
        }
        
        // 斜め移動がダメなら軸移動を試みる
        if (dx != 0 && CanMoveTo(map, fromX + dx, fromY)) {
            cmd->type = Components::ActionCommand::Type::Move;
            cmd->dx = dx;
            cmd->dy = 0;
            return;
        }
        
        if (dy != 0 && CanMoveTo(map, fromX, fromY + dy)) {
            cmd->type = Components::ActionCommand::Type::Move;
            cmd->dx = 0;
            cmd->dy = dy;
            return;
        }
        
        // どこにも行けない
        cmd->type = Components::ActionCommand::Type::Wait;
    }
    
    /**
     * @brief ランダム移動
     */
    static void RandomMove(
        const Components::MapData& map,
        int fromX, int fromY,
        Components::ActionCommand* cmd
    ) {
        static const int dirs[8][2] = {
            {-1, -1}, {0, -1}, {1, -1},
            {-1,  0},          {1,  0},
            {-1,  1}, {0,  1}, {1,  1}
        };
        
        // ランダムな方向を試す
        int startDir = std::rand() % 8;
        for (int i = 0; i < 8; i++) {
            int d = (startDir + i) % 8;
            int newX = fromX + dirs[d][0];
            int newY = fromY + dirs[d][1];
            
            if (CanMoveTo(map, newX, newY)) {
                cmd->type = Components::ActionCommand::Type::Move;
                cmd->dx = dirs[d][0];
                cmd->dy = dirs[d][1];
                return;
            }
        }
        
        // どこにも行けない
        cmd->type = Components::ActionCommand::Type::Wait;
    }
    
    /**
     * @brief 移動可能かチェック
     */
    static bool CanMoveTo(const Components::MapData& map, int x, int y) {
        if (!map.InBounds(x, y)) return false;
        if (!map.IsWalkable(x, y)) return false;
        if (map.At(x, y).occupant != entt::null) return false;
        return true;
    }
    
    /**
     * @brief 視線が通るかチェック（簡易レイキャスト）
     */
    static bool CanSeeTarget(
        const Components::MapData& map,
        int fromX, int fromY,
        int toX, int toY,
        int maxRange
    ) {
        int dx = toX - fromX;
        int dy = toY - fromY;
        int distSq = dx * dx + dy * dy;
        
        // 範囲外
        if (distSq > maxRange * maxRange) return false;
        
        // Bresenhamのような簡易ラインチェック
        int steps = std::max(std::abs(dx), std::abs(dy));
        if (steps == 0) return true;
        
        float xStep = static_cast<float>(dx) / steps;
        float yStep = static_cast<float>(dy) / steps;
        
        float x = static_cast<float>(fromX);
        float y = static_cast<float>(fromY);
        
        for (int i = 1; i < steps; i++) {
            x += xStep;
            y += yStep;
            
            int checkX = static_cast<int>(x + 0.5f);
            int checkY = static_cast<int>(y + 0.5f);
            
            if (!map.InBounds(checkX, checkY)) return false;
            auto tileType = map.At(checkX, checkY).type;
            if (tileType == Components::TileType::Wall || tileType == Components::TileType::Void) return false;
        }
        
        return true;
    }
};

} // namespace Domain::Roguelike::Systems

