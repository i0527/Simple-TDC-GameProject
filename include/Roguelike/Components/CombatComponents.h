/**
 * @file CombatComponents.h
 * @brief 戦闘・モンスター関連コンポーネント
 * 
 * Phase 3: モンスターAI、戦闘システム用のコンポーネント定義。
 */
#pragma once

#include <entt/entt.hpp>
#include <string>
#include <vector>

namespace Roguelike::Components {

/**
 * @brief ヘルスコンポーネント
 */
struct Health {
    int current = 10;       ///< 現在HP
    int max = 10;           ///< 最大HP
    
    bool IsAlive() const { return current > 0; }
    float GetRatio() const { return static_cast<float>(current) / max; }
    
    void TakeDamage(int amount) {
        current -= amount;
        if (current < 0) current = 0;
    }
    
    void Heal(int amount) {
        current += amount;
        if (current > max) current = max;
    }
};

/**
 * @brief 戦闘ステータス
 */
struct CombatStats {
    int attack = 1;         ///< 攻撃力
    int defense = 0;        ///< 防御力
    int accuracy = 80;      ///< 命中率（%）
    int evasion = 10;       ///< 回避率（%）
    int critChance = 5;     ///< クリティカル率（%）
    float critMultiplier = 1.5f;  ///< クリティカルダメージ倍率
};

/**
 * @brief AIタイプ
 */
enum class AIType {
    None,           ///< AI無し（プレイヤーなど）
    Idle,           ///< 待機（動かない）
    Wander,         ///< 徘徊（ランダム移動）
    Hostile,        ///< 敵対（プレイヤーを追跡・攻撃）
    Cowardly,       ///< 臆病（ダメージを受けると逃げる）
    Patrol          ///< 巡回（決まったルートを移動）
};

/**
 * @brief AIコンポーネント
 */
struct AI {
    AIType type = AIType::Hostile;
    
    // 視界・探知
    int sightRange = 8;             ///< 視界範囲
    bool canSeePlayer = false;      ///< プレイヤーを視認中
    int lastKnownPlayerX = -1;      ///< プレイヤーの最後の位置X
    int lastKnownPlayerY = -1;      ///< プレイヤーの最後の位置Y
    int turnsLostPlayer = 0;        ///< プレイヤーを見失ってからのターン
    
    // 行動パターン
    int wanderCooldown = 0;         ///< 徘徊クールダウン
    float aggroChance = 1.0f;       ///< 敵対化確率
};

// MonsterTagはGridComponents.hで定義されています

/**
 * @brief モンスター種族
 */
enum class MonsterSpecies {
    // 弱い敵（序盤）
    Rat,            ///< ネズミ (r)
    Bat,            ///< コウモリ (B)
    Goblin,         ///< ゴブリン (g)
    Kobold,         ///< コボルド (k)
    
    // 中堅（中盤）
    Orc,            ///< オーク (o)
    Skeleton,       ///< スケルトン (s)
    Zombie,         ///< ゾンビ (Z)
    Snake,          ///< ヘビ (S)
    
    // 強敵（終盤）
    Troll,          ///< トロル (T)
    Ogre,           ///< オーガ (O)
    Wraith,         ///< レイス (W)
    Dragon          ///< ドラゴン (D)
};

/**
 * @brief モンスターデータ（種族情報）
 */
struct MonsterData {
    MonsterSpecies species;
    char symbol;
    unsigned char r, g, b;      ///< 表示色
    std::string name;
    std::string description;
    
    // 基本ステータス
    int baseHP;
    int baseAttack;
    int baseDefense;
    int baseSpeed;
    int expValue;               ///< 倒した時の経験値
    
    // AI設定
    AIType aiType;
    int sightRange;
    
    // 出現階層
    int minFloor;
    int maxFloor;
    float spawnWeight;          ///< 出現重み（高いほど出やすい）
};

/**
 * @brief モンスターテンプレートデータベース
 */
inline const std::vector<MonsterData>& GetMonsterDatabase() {
    static const std::vector<MonsterData> database = {
        // 弱い敵（1-3階）
        {MonsterSpecies::Rat,      'r', 139, 90, 43,  "ネズミ", "素早い小動物",
         4, 1, 0, 120, 5, AIType::Wander, 4, 1, 4, 1.5f},
        
        {MonsterSpecies::Bat,      'B', 100, 100, 100, "コウモリ", "闇を飛ぶ生物",
         3, 1, 0, 150, 3, AIType::Wander, 6, 1, 5, 1.2f},
        
        {MonsterSpecies::Goblin,   'g', 0, 200, 0,    "ゴブリン", "小柄な人型モンスター",
         8, 2, 1, 100, 10, AIType::Hostile, 6, 1, 5, 1.0f},
        
        {MonsterSpecies::Kobold,   'k', 255, 165, 0,  "コボルド", "卑怯な爬虫類人",
         6, 2, 0, 110, 8, AIType::Hostile, 5, 1, 4, 0.8f},
        
        // 中堅（3-6階）
        {MonsterSpecies::Orc,      'o', 150, 75, 0,   "オーク", "凶暴な戦士",
         15, 4, 2, 90, 25, AIType::Hostile, 6, 3, 7, 1.0f},
        
        {MonsterSpecies::Skeleton, 's', 200, 200, 200, "スケルトン", "動く骨",
         10, 3, 3, 80, 20, AIType::Hostile, 5, 3, 8, 1.0f},
        
        {MonsterSpecies::Zombie,   'Z', 100, 150, 100, "ゾンビ", "腐った死体",
         20, 3, 1, 60, 22, AIType::Hostile, 4, 3, 7, 0.8f},
        
        {MonsterSpecies::Snake,    'S', 0, 150, 0,    "ヘビ", "毒を持つ蛇",
         8, 4, 0, 130, 18, AIType::Hostile, 5, 2, 6, 0.6f},
        
        // 強敵（6-10階）
        {MonsterSpecies::Troll,    'T', 0, 100, 0,    "トロル", "再生する巨人",
         40, 6, 3, 70, 60, AIType::Hostile, 5, 6, 10, 0.7f},
        
        {MonsterSpecies::Ogre,     'O', 139, 69, 19,  "オーガ", "巨大な人喰い",
         35, 8, 2, 80, 55, AIType::Hostile, 4, 5, 9, 0.6f},
        
        {MonsterSpecies::Wraith,   'W', 100, 100, 150, "レイス", "実体なき霊",
         25, 5, 5, 100, 50, AIType::Hostile, 8, 7, 10, 0.4f},
        
        {MonsterSpecies::Dragon,   'D', 255, 50, 50,  "ドラゴン", "恐怖の竜",
         60, 10, 5, 90, 150, AIType::Hostile, 10, 9, 10, 0.2f}
    };
    return database;
}

/**
 * @brief 指定階層に出現可能なモンスターを取得
 */
inline std::vector<const MonsterData*> GetMonstersForFloor(int floor) {
    std::vector<const MonsterData*> result;
    for (const auto& data : GetMonsterDatabase()) {
        if (floor >= data.minFloor && floor <= data.maxFloor) {
            result.push_back(&data);
        }
    }
    return result;
}

/**
 * @brief 経験値コンポーネント（プレイヤー用）
 */
struct Experience {
    int current = 0;            ///< 現在の経験値
    int level = 1;              ///< 現在のレベル
    int toNextLevel = 100;      ///< 次のレベルまでに必要な経験値
    
    /**
     * @brief 経験値を追加
     * @return レベルアップした場合true
     */
    bool AddExp(int amount) {
        current += amount;
        if (current >= toNextLevel) {
            current -= toNextLevel;
            level++;
            toNextLevel = CalculateExpToNext(level);
            return true;
        }
        return false;
    }
    
    static int CalculateExpToNext(int level) {
        // レベルごとに必要経験値が増加
        return 100 + (level - 1) * 50;
    }
};

/**
 * @brief 死亡マーカー（削除待ち）
 */
struct Dead {};

} // namespace Roguelike::Components
