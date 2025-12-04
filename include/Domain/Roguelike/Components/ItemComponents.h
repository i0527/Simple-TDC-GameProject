/**
 * @file ItemComponents.h
 * @brief アイテム関連コンポーネント
 * 
 * Phase 4: アイテム、インベントリ、装備システム
 * Phase 3: 新しい統一アーキテクチャに移行
 */
#pragma once

#include <entt/entt.hpp>
#include <string>
#include <array>
#include <vector>

namespace Domain::Roguelike::Components {

/**
 * @brief アイテム種別
 */
enum class ItemType : uint8_t {
    None = 0,
    Weapon,     ///< 武器
    Armor,      ///< 防具
    Potion,     ///< ポーション
    Scroll,     ///< 巻物
    Food,       ///< 食料
    Gold,       ///< お金
    Misc,       ///< その他
};

/**
 * @brief 装備スロット
 */
enum class EquipSlot : uint8_t {
    None = 0,
    Weapon,     ///< 武器
    Armor,      ///< 防具
    Ring,       ///< 指輪
    Amulet,     ///< 護符
};

/**
 * @brief アイテムタグ（床に落ちているアイテムを示す）
 */
struct ItemTag {};

/**
 * @brief アイテム基本情報
 */
struct Item {
    std::string id;             ///< 定義ID
    std::string name;           ///< 表示名
    std::string description;    ///< 説明
    ItemType type = ItemType::None;
    char symbol = '?';          ///< 表示記号
    unsigned char r = 255, g = 255, b = 255;  ///< 表示色
    int quantity = 1;           ///< スタック数
    int weight = 1;             ///< 重さ
    int value = 0;              ///< 価値（ゴールド）
    bool identified = true;     ///< 識別済みか
    
    Item() = default;
    Item(const std::string& itemId, const std::string& itemName, ItemType itemType, 
         char sym, unsigned char r_, unsigned char g_, unsigned char b_)
        : id(itemId), name(itemName), type(itemType), symbol(sym), r(r_), g(g_), b(b_) {}
};

/**
 * @brief 装備可能アイテム情報
 */
struct Equippable {
    EquipSlot slot = EquipSlot::None;
    int attackBonus = 0;        ///< 攻撃力ボーナス
    int defenseBonus = 0;       ///< 防御力ボーナス
    int accuracyBonus = 0;      ///< 命中ボーナス
    int evasionBonus = 0;       ///< 回避ボーナス
};

/**
 * @brief 消費アイテム効果
 */
struct Consumable {
    enum class EffectType {
        None,
        Heal,           ///< HP回復
        FullHeal,       ///< HP全回復
        Food,           ///< 空腹度回復（将来用）
        Teleport,       ///< テレポート
        Identify,       ///< 識別
        Damage,         ///< ダメージ（毒など）
    };
    
    EffectType effect = EffectType::None;
    int value = 0;              ///< 効果量
    std::string message;        ///< 使用時メッセージ
};

/**
 * @brief インベントリ
 */
struct Inventory {
    static constexpr int MAX_SLOTS = 26;  // a-z
    std::array<entt::entity, MAX_SLOTS> items{};
    int gold = 0;
    
    Inventory() {
        items.fill(entt::null);
    }
    
    /**
     * @brief 空きスロットを探す
     * @return スロットインデックス、見つからない場合は-1
     */
    int FindEmptySlot() const {
        for (int i = 0; i < MAX_SLOTS; i++) {
            if (items[i] == entt::null) {
                return i;
            }
        }
        return -1;
    }
    
    /**
     * @brief アイテム数をカウント
     */
    int CountItems() const {
        int count = 0;
        for (auto& item : items) {
            if (item != entt::null) {
                count++;
            }
        }
        return count;
    }
    
    /**
     * @brief アイテムを追加
     * @return 追加したスロット、失敗時は-1
     */
    int AddItem(entt::entity item) {
        int slot = FindEmptySlot();
        if (slot >= 0) {
            items[slot] = item;
        }
        return slot;
    }
    
    /**
     * @brief スロットからアイテムを削除
     */
    entt::entity RemoveItem(int slot) {
        if (slot >= 0 && slot < MAX_SLOTS) {
            entt::entity item = items[slot];
            items[slot] = entt::null;
            return item;
        }
        return entt::null;
    }
    
    /**
     * @brief スロット文字を取得
     */
    static char GetSlotChar(int slot) {
        return 'a' + static_cast<char>(slot);
    }
    
    /**
     * @brief 文字からスロットインデックスを取得
     */
    static int GetSlotIndex(char c) {
        if (c >= 'a' && c <= 'z') {
            return c - 'a';
        }
        return -1;
    }
};

/**
 * @brief 装備中アイテム
 */
struct Equipment {
    entt::entity weapon = entt::null;
    entt::entity armor = entt::null;
    entt::entity ring = entt::null;
    entt::entity amulet = entt::null;
    
    /**
     * @brief スロットに対応する装備を取得
     */
    entt::entity GetSlot(EquipSlot slot) const {
        switch (slot) {
            case EquipSlot::Weapon: return weapon;
            case EquipSlot::Armor: return armor;
            case EquipSlot::Ring: return ring;
            case EquipSlot::Amulet: return amulet;
            default: return entt::null;
        }
    }
    
    /**
     * @brief スロットに装備をセット
     */
    void SetSlot(EquipSlot slot, entt::entity item) {
        switch (slot) {
            case EquipSlot::Weapon: weapon = item; break;
            case EquipSlot::Armor: armor = item; break;
            case EquipSlot::Ring: ring = item; break;
            case EquipSlot::Amulet: amulet = item; break;
            default: break;
        }
    }
};

/**
 * @brief アイテムデータベース（定義済みアイテム）
 */
struct ItemData {
    std::string id;
    std::string name;
    std::string description;
    ItemType type;
    char symbol;
    unsigned char r, g, b;
    int weight;
    int value;
    
    // 装備用
    EquipSlot equipSlot = EquipSlot::None;
    int attackBonus = 0;
    int defenseBonus = 0;
    
    // 消費用
    Consumable::EffectType effect = Consumable::EffectType::None;
    int effectValue = 0;
    std::string useMessage;
    
    // 出現設定
    int minFloor = 1;
    int maxFloor = 10;
    float spawnWeight = 1.0f;
};

/**
 * @brief アイテムデータベースを取得
 */
inline const std::vector<ItemData>& GetItemDatabase() {
    static const std::vector<ItemData> database = {
        // ポーション
        {"potion_heal", "回復薬", "HPを回復する薬", ItemType::Potion, '!', 255, 100, 100,
         1, 50, EquipSlot::None, 0, 0, Consumable::EffectType::Heal, 20, "傷が癒えた！", 1, 10, 1.0f},
        
        {"potion_heal_major", "上級回復薬", "HPを大きく回復する薬", ItemType::Potion, '!', 255, 50, 50,
         1, 150, EquipSlot::None, 0, 0, Consumable::EffectType::Heal, 50, "傷が大きく癒えた！", 3, 10, 0.5f},
        
        {"potion_full_heal", "完全回復薬", "HPを全回復する薬", ItemType::Potion, '!', 255, 0, 0,
         1, 300, EquipSlot::None, 0, 0, Consumable::EffectType::FullHeal, 0, "傷が完全に癒えた！", 6, 10, 0.2f},
        
        // 武器
        {"weapon_dagger", "ダガー", "軽い短剣", ItemType::Weapon, ')', 200, 200, 200,
         2, 20, EquipSlot::Weapon, 2, 0, Consumable::EffectType::None, 0, "", 1, 3, 1.0f},
        
        {"weapon_short_sword", "ショートソード", "扱いやすい剣", ItemType::Weapon, ')', 220, 220, 220,
         3, 50, EquipSlot::Weapon, 4, 0, Consumable::EffectType::None, 0, "", 1, 5, 0.7f},
        
        {"weapon_long_sword", "ロングソード", "標準的な長剣", ItemType::Weapon, ')', 240, 240, 240,
         4, 100, EquipSlot::Weapon, 6, 0, Consumable::EffectType::None, 0, "", 3, 8, 0.5f},
        
        {"weapon_great_sword", "グレートソード", "強力な両手剣", ItemType::Weapon, ')', 255, 255, 255,
         6, 200, EquipSlot::Weapon, 10, 0, Consumable::EffectType::None, 0, "", 6, 10, 0.3f},
        
        // 防具
        {"armor_leather", "革の鎧", "軽い革製の防具", ItemType::Armor, '[', 139, 90, 43,
         5, 30, EquipSlot::Armor, 0, 2, Consumable::EffectType::None, 0, "", 1, 4, 1.0f},
        
        {"armor_chain", "チェインメイル", "鎖で編まれた防具", ItemType::Armor, '[', 180, 180, 180,
         10, 80, EquipSlot::Armor, 0, 4, Consumable::EffectType::None, 0, "", 3, 7, 0.6f},
        
        {"armor_plate", "プレートアーマー", "重厚な板金鎧", ItemType::Armor, '[', 200, 200, 220,
         15, 200, EquipSlot::Armor, 0, 7, Consumable::EffectType::None, 0, "", 5, 10, 0.3f},
        
        // 食料
        {"food_ration", "携帯食料", "腹持ちのよい食料", ItemType::Food, '%', 200, 150, 100,
         2, 10, EquipSlot::None, 0, 0, Consumable::EffectType::Food, 500, "食料を食べた。", 1, 10, 0.8f},
        
        {"food_bread", "パン", "焼きたてのパン", ItemType::Food, '%', 220, 180, 130,
         1, 5, EquipSlot::None, 0, 0, Consumable::EffectType::Food, 300, "パンを食べた。", 1, 5, 1.0f},
        
        // ゴールド
        {"gold_pile", "金貨", "輝く金貨の山", ItemType::Gold, '$', 255, 215, 0,
         0, 0, EquipSlot::None, 0, 0, Consumable::EffectType::None, 0, "", 1, 10, 2.0f},
    };
    return database;
}

/**
 * @brief 指定階層に出現可能なアイテムを取得
 */
inline std::vector<const ItemData*> GetItemsForFloor(int floor) {
    std::vector<const ItemData*> result;
    for (const auto& data : GetItemDatabase()) {
        if (floor >= data.minFloor && floor <= data.maxFloor) {
            result.push_back(&data);
        }
    }
    return result;
}

/**
 * @brief IDからアイテムデータを検索
 */
inline const ItemData* FindItemData(const std::string& id) {
    for (const auto& data : GetItemDatabase()) {
        if (data.id == id) {
            return &data;
        }
    }
    return nullptr;
}

} // namespace Domain::Roguelike::Components

