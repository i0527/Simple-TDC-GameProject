/**
 * @file MonsterSpawner.h
 * @brief モンスター生成システム
 * 
 * 階層に応じたモンスターをダンジョンに配置する。
 */
#pragma once

#include <entt/entt.hpp>
#include <random>
#include <vector>
#include "../Components/GridComponents.h"
#include "../Components/TurnComponents.h"
#include "../Components/CombatComponents.h"

namespace Roguelike::Generators {

/**
 * @brief モンスター生成器
 */
class MonsterSpawner {
public:
    /**
     * @brief 階層にモンスターを配置
     * @param registry ECSレジストリ
     * @param map マップデータ
     * @param floor 現在の階層
     * @param playerX プレイヤーのX座標（近くに配置しない）
     * @param playerY プレイヤーのY座標
     * @param seed ランダムシード
     */
    void SpawnMonstersForFloor(
        entt::registry& registry,
        Components::MapData& map,
        int floor,
        int playerX,
        int playerY,
        unsigned int seed
    ) {
        rng_.seed(seed + floor * 54321);
        
        // 階層に応じたモンスター数を決定
        int baseCount = 3 + floor;
        std::uniform_int_distribution<> countDist(-2, 2);
        int monsterCount = std::max(2, baseCount + countDist(rng_));
        
        // 配置可能な床タイルを収集
        std::vector<std::pair<int, int>> floorTiles;
        for (int y = 0; y < map.height; y++) {
            for (int x = 0; x < map.width; x++) {
                auto& tile = map.At(x, y);
                if (tile.type == Components::TileType::Floor ||
                    tile.type == Components::TileType::Corridor) {
                    // プレイヤーから一定距離離れた場所のみ
                    int dx = x - playerX;
                    int dy = y - playerY;
                    if (dx * dx + dy * dy > 25) {  // 5タイル以上離れる
                        floorTiles.push_back({x, y});
                    }
                }
            }
        }
        
        if (floorTiles.empty()) return;
        
        // この階層に出現するモンスターリストを取得
        auto availableMonsters = Components::GetMonstersForFloor(floor);
        if (availableMonsters.empty()) return;
        
        // 重み付き抽選の準備
        float totalWeight = 0;
        for (const auto* data : availableMonsters) {
            totalWeight += data->spawnWeight;
        }
        
        // モンスターを生成
        std::uniform_int_distribution<> tileDist(0, static_cast<int>(floorTiles.size()) - 1);
        std::uniform_real_distribution<> weightDist(0, totalWeight);
        
        for (int i = 0; i < monsterCount && !floorTiles.empty(); i++) {
            // 配置位置を選択
            int tileIndex = tileDist(rng_) % floorTiles.size();
            auto [x, y] = floorTiles[tileIndex];
            
            // 既に何かがいる場合はスキップ
            if (map.At(x, y).occupant != entt::null) continue;
            
            // モンスター種類を重み付き抽選
            const Components::MonsterData* selectedMonster = nullptr;
            float roll = static_cast<float>(weightDist(rng_));
            float cumWeight = 0;
            for (const auto* data : availableMonsters) {
                cumWeight += data->spawnWeight;
                if (roll <= cumWeight) {
                    selectedMonster = data;
                    break;
                }
            }
            if (!selectedMonster) selectedMonster = availableMonsters[0];
            
            // モンスターエンティティを作成
            auto entity = CreateMonster(registry, *selectedMonster, x, y, floor);
            
            // マップにoccupantを設定
            map.At(x, y).occupant = entity;
            
            // 使用したタイルを除去
            floorTiles.erase(floorTiles.begin() + tileIndex);
            if (!floorTiles.empty()) {
                tileDist = std::uniform_int_distribution<>(0, static_cast<int>(floorTiles.size()) - 1);
            }
        }
    }
    
    /**
     * @brief 単体モンスターを生成
     */
    entt::entity CreateMonster(
        entt::registry& registry,
        const Components::MonsterData& data,
        int x, int y,
        int floor
    ) {
        auto entity = registry.create();
        
        // 位置
        registry.emplace<Components::GridPosition>(entity, x, y);
        
        // 見た目
        registry.emplace<Components::Appearance>(entity, data.symbol, data.r, data.g, data.b);
        
        // 名前
        registry.emplace<Components::Name>(entity, data.name, data.description);
        
        // ターンアクター
        Components::TurnActor actor;
        actor.speed = data.baseSpeed;
        actor.energy = 0;  // 最初はプレイヤーが先に動く
        actor.isPlayer = false;
        registry.emplace<Components::TurnActor>(entity, actor);
        
        // 行動コマンド
        registry.emplace<Components::ActionCommand>(entity);
        
        // ヘルス（階層ボーナス）
        int hpBonus = (floor - data.minFloor) * 2;
        Components::Health health;
        health.max = data.baseHP + hpBonus;
        health.current = health.max;
        registry.emplace<Components::Health>(entity, health);
        
        // 戦闘ステータス（階層ボーナス）
        int statBonus = (floor - data.minFloor) / 2;
        Components::CombatStats stats;
        stats.attack = data.baseAttack + statBonus;
        stats.defense = data.baseDefense;
        registry.emplace<Components::CombatStats>(entity, stats);
        
        // AI
        Components::AI ai;
        ai.type = data.aiType;
        ai.sightRange = data.sightRange;
        registry.emplace<Components::AI>(entity, ai);
        
        // モンスタータグ
        registry.emplace<Components::MonsterTag>(entity);
        
        return entity;
    }
    
    /**
     * @brief マップ上の全モンスターを削除
     */
    void ClearMonsters(entt::registry& registry, Components::MapData& map) {
        // 削除対象を収集
        std::vector<entt::entity> toDestroy;
        
        // すべてのエンティティをチェック
        registry.each([&](entt::entity entity) {
            if (registry.any_of<Components::MonsterTag>(entity)) {
                auto* pos = registry.try_get<Components::GridPosition>(entity);
                if (pos && map.InBounds(pos->x, pos->y)) {
                    map.At(pos->x, pos->y).occupant = entt::null;
                }
                toDestroy.push_back(entity);
            }
        });
        
        // モンスターエンティティを削除
        for (auto entity : toDestroy) {
            registry.destroy(entity);
        }
    }
    
private:
    std::mt19937 rng_;
};

} // namespace Roguelike::Generators
