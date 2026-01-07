# Cat Tower Defense - キャラクターシステム設計仕様書

**最終更新**: 2026-01-07  
**実装状況**: ✅ 全フェーズ実装完了  
**対象**: キャラクター構造体 + キャラクター管理システム  
**バージョン**: 統合版（味方 + 敵共通）  

---

## 📋 目次

1. [概要](#概要)
2. [キャラクター構造体設計](#キャラクター構造体設計)
3. [アニメーション定義](#アニメーション定義)
4. [キャラクターマスターデータ](#キャラクターマスターデータ)
5. [キャラクター管理システム](#キャラクター管理システム)
6. [ゲーム・UI連携](#ゲームui連携)
7. [実装フロー](#実装フロー)

---

## 🎯 概要

### 設計方針

✅ **共通管理**: 味方キャラ・敵キャラを同じ構造体で管理  
✅ **JSON駆動**: マスターデータはJSON、起動時にハードコード化  
✅ **アニメーション対応**: スプライトシートベースのシンプルアニメーション  
✅ **ゲーム + UI**: ゲーム画面・UI両方での利用を想定  

### ディレクトリ構造

```
game/
├── core/
│   ├── entities/                    # NEW: エンティティ関連
│   │   ├── Character.hpp            # キャラクター構造体
│   │   ├── CharacterManager.hpp/cpp # キャラクター管理
│   │   └── animation/
│   │       ├── SpriteAnimation.hpp  # スプライトアニメーション定義
│   │       └── AnimationData.hpp    # アニメーション設定
│   └── ...
│
├── assets/
│   └── data/
│       └── characters/              # NEW: キャラクターデータ
│           ├── characters_master.json
│           ├── sprites/             # スプライトシート
│           │   └── {character_id}/
│           │       ├── idle.png
│           │       ├── move.png
│           │       ├── attack.png
│           │       └── ...
│           └── icons/               # UIアイコン
│               └── {character_id}.png
```

---

## 🎨 キャラクター構造体設計

### Character.hpp

```cpp
// game/core/entities/Character.hpp
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace game {
namespace core {
namespace entities {

// 攻撃タイプ
enum class AttackType {
    Single,     // 単体攻撃
    Range,      // 範囲攻撃
    Line,       // 直線攻撃
};

// エフェクトタイプ
enum class EffectType {
    Normal,     // 通常攻撃エフェクト
    Fire,       // 炎エフェクト
    Ice,        // 氷エフェクト
    Lightning,  // 雷エフェクト
    Heal,       // 回復エフェクト
};

// パッシブスキル定義
struct PassiveSkill {
    std::string id;           // スキルID
    std::string name;         // スキル名
    std::string description;  // スキル説明
    float value;              // パラメータ値（加算 or 乗算）
};

// 装備定義
struct Equipment {
    std::string id;           // 装備ID
    std::string name;         // 装備名
    float attack_bonus;       // 攻撃力ボーナス
    float hp_bonus;           // HP ボーナス
};

// ***** キャラクター構造体（共通） *****
struct Character {
    // ===== 基本情報 =====
    std::string id;           // キャラクターID（ユニーク）
    std::string name;         // キャラクター名
    int rarity;               // レアリティ（1-5）
    int level;                // レベル

    // ===== ステータス =====
    int hp;                   // HP
    int attack;               // 攻撃力
    int defense;              // 防御力
    float move_speed;         // 移動速度（ピクセル/秒）
    float attack_span;        // 攻撃スパン（秒）

    // ===== 攻撃設定 =====
    AttackType attack_type;   // 攻撃タイプ
    glm::vec2 attack_size;    // (攻撃有効範囲の距離, キャラ-攻撃有効範囲距離)
    EffectType effect_type;   // エフェクトタイプ

    // ===== UIリソース =====
    std::string icon_path;    // UIアイコンパス

    // ===== スプライト情報 =====
    struct SpriteInfo {
        std::string sheet_path;     // スプライトシートパス
        int frame_width;            // 1フレームの幅
        int frame_height;           // 1フレームの高さ
        int frame_count;            // 総フレーム数
        float frame_duration;       // 1フレームの表示時間（秒）
    };

    SpriteInfo move_sprite;   // 移動スプライト情報
    SpriteInfo attack_sprite; // 攻撃スプライト情報

    // ===== スキル・装備 =====
    std::vector<PassiveSkill> passive_skills;  // パッシブスキル
    std::vector<Equipment> equipment;          // 装備

    // ===== オプショナル情報 =====
    std::string description;  // キャラクター説明
    std::string rarity_name;  // レアリティ名（N, R, SR, SSRなど）

    // ===== コンストラクタ =====
    Character() = default;
    ~Character() = default;

    // ===== ユーティリティメソッド =====
    
    // 総攻撃力を計算（装備ボーナス込み）
    int GetTotalAttack() const;
    
    // 総HPを計算（装備ボーナス込み）
    int GetTotalHP() const;

    // アニメーション総フレーム数取得
    int GetMoveFrameCount() const { return move_sprite.frame_count; }
    int GetAttackFrameCount() const { return attack_sprite.frame_count; }
};

} // namespace entities
} // namespace core
} // namespace game
```

### Character.cpp（実装例）

```cpp
// game/core/entities/Character.cpp
#include "Character.hpp"

namespace game {
namespace core {
namespace entities {

int Character::GetTotalAttack() const {
    int total = attack;
    for (const auto& eq : equipment) {
        total += static_cast<int>(eq.attack_bonus);
    }
    return total;
}

int Character::GetTotalHP() const {
    int total = hp;
    for (const auto& eq : equipment) {
        total += static_cast<int>(eq.hp_bonus);
    }
    return total;
}

} // namespace entities
} // namespace core
} // namespace game
```

---

## 🎬 アニメーション定義

### AnimationData.hpp

```cpp
// game/core/entities/animation/AnimationData.hpp
#pragma once

#include <string>
#include <glm/glm.hpp>

namespace game {
namespace core {
namespace entities {
namespace animation {

// スプライトアニメーションのメタデータ
struct SpriteAnimationData {
    std::string animation_name;   // アニメーション名（"move", "attack"）
    std::string sprite_sheet_path; // スプライトシートパス
    
    int frame_width;              // 1フレームの幅（ピクセル）
    int frame_height;             // 1フレームの高さ（ピクセル）
    int frame_count;              // 総フレーム数
    
    float frame_duration;         // 各フレームの表示時間（秒）
    bool is_looping;              // ループするか
    
    // デフォルトコンストラクタ
    SpriteAnimationData()
        : animation_name("")
        , sprite_sheet_path("")
        , frame_width(0)
        , frame_height(0)
        , frame_count(0)
        , frame_duration(0.1f)
        , is_looping(true)
    {}
};

} // namespace animation
} // namespace entities
} // namespace core
} // namespace game
```

---

## 📊 キャラクターマスターデータ

### JSON スキーマ

```json
{
  "characters": [
    {
      "id": "cat_001",
      "name": "勇敢な猫",
      "rarity": 4,
      "description": "勇敢でバランスの取れた猫戦士",
      "rarity_name": "SSR",
      
      "status": {
        "level": 1,
        "hp": 100,
        "attack": 80,
        "defense": 40,
        "move_speed": 150.0,
        "attack_span": 1.5
      },
      
      "attack": {
        "type": "single",
        "size": [80.0, 20.0],
        "effect_type": "normal"
      },
      
      "sprites": {
        "icon_path": "assets/icons/cat_001.png",
        "move": {
          "sheet_path": "assets/sprites/cat_001/move.png",
          "frame_width": 64,
          "frame_height": 64,
          "frame_count": 8,
          "frame_duration": 0.1
        },
        "attack": {
          "sheet_path": "assets/sprites/cat_001/attack.png",
          "frame_width": 80,
          "frame_height": 80,
          "frame_count": 6,
          "frame_duration": 0.08
        }
      },
      
      "passive_skills": [
        {
          "id": "skill_defense_up",
          "name": "防御アップ",
          "description": "防御力が10%上昇",
          "value": 0.1
        }
      ],
      
      "equipment": [
        {
          "id": "eq_sword_001",
          "name": "鋼の剣",
          "attack_bonus": 15.0,
          "hp_bonus": 0.0
        }
      ]
    },
    {
      "id": "dog_001",
      "name": "強気な犬",
      "rarity": 3,
      "description": "攻撃型のキャラクター",
      "rarity_name": "SR",
      
      "status": {
        "level": 1,
        "hp": 80,
        "attack": 100,
        "defense": 30,
        "move_speed": 180.0,
        "attack_span": 1.2
      },
      
      "attack": {
        "type": "range",
        "size": [120.0, 50.0],
        "effect_type": "fire"
      },
      
      "sprites": {
        "icon_path": "assets/icons/dog_001.png",
        "move": {
          "sheet_path": "assets/sprites/dog_001/move.png",
          "frame_width": 64,
          "frame_height": 64,
          "frame_count": 8,
          "frame_duration": 0.1
        },
        "attack": {
          "sheet_path": "assets/sprites/dog_001/attack.png",
          "frame_width": 80,
          "frame_height": 80,
          "frame_count": 6,
          "frame_duration": 0.08
        }
      },
      
      "passive_skills": [],
      "equipment": []
    }
  ]
}
```

---

## 🔧 キャラクター管理システム

### CharacterManager.hpp

```cpp
// game/core/entities/CharacterManager.hpp
#pragma once

#include "Character.hpp"
#include <string>
#include <unordered_map>
#include <memory>

namespace game {
namespace core {
namespace entities {

// キャラクターマスターデータ管理
class CharacterManager {
public:
    CharacterManager();
    ~CharacterManager();

    // 初期化（JSON / ハードコードからデータロード）
    bool Initialize(const std::string& json_path = "");

    // マスターデータからキャラクターを取得
    // （フレッシュなインスタンスを返す）
    std::shared_ptr<Character> GetCharacterTemplate(const std::string& character_id);

    // 全キャラクターIDを取得
    std::vector<std::string> GetAllCharacterIds() const;

    // キャラクター存在確認
    bool HasCharacter(const std::string& character_id) const;

    // キャラクター数
    size_t GetCharacterCount() const { return masters_.size(); }

    // 全マスターデータ取得（デバッグ用）
    const std::unordered_map<std::string, Character>& GetAllMasters() const {
        return masters_;
    }

    // 終了処理
    void Shutdown();

private:
    // マスターデータ（ID -> Character）
    std::unordered_map<std::string, Character> masters_;

    // JSONからデータロード
    bool LoadFromJSON(const std::string& json_path);

    // ハードコードデータを初期化（JSON不要な場合）
    void InitializeHardcodedData();
};

} // namespace entities
} // namespace core
} // namespace game
```

### CharacterManager.cpp

```cpp
// game/core/entities/CharacterManager.cpp
#include "CharacterManager.hpp"
#include "utils/Log.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace game {
namespace core {
namespace entities {

CharacterManager::CharacterManager() {
}

CharacterManager::~CharacterManager() {
    Shutdown();
}

bool CharacterManager::Initialize(const std::string& json_path) {
    if (!json_path.empty()) {
        // JSON からロード
        return LoadFromJSON(json_path);
    } else {
        // ハードコード初期化（開発速度優先）
        InitializeHardcodedData();
        return true;
    }
}

bool CharacterManager::LoadFromJSON(const std::string& json_path) {
    try {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open character data file: {}", json_path);
            return false;
        }

        json data;
        file >> data;

        for (const auto& ch_json : data["characters"]) {
            Character ch;
            
            // 基本情報
            ch.id = ch_json["id"].get<std::string>();
            ch.name = ch_json["name"].get<std::string>();
            ch.rarity = ch_json["rarity"].get<int>();
            ch.description = ch_json.value("description", "");
            ch.rarity_name = ch_json.value("rarity_name", "");
            
            // ステータス
            ch.level = ch_json["status"]["level"].get<int>();
            ch.hp = ch_json["status"]["hp"].get<int>();
            ch.attack = ch_json["status"]["attack"].get<int>();
            ch.defense = ch_json["status"]["defense"].get<int>();
            ch.move_speed = ch_json["status"]["move_speed"].get<float>();
            ch.attack_span = ch_json["status"]["attack_span"].get<float>();
            
            // 攻撃設定
            std::string attack_type_str = ch_json["attack"]["type"].get<std::string>();
            if (attack_type_str == "single") {
                ch.attack_type = AttackType::Single;
            } else if (attack_type_str == "range") {
                ch.attack_type = AttackType::Range;
            } else if (attack_type_str == "line") {
                ch.attack_type = AttackType::Line;
            }
            
            ch.attack_size.x = ch_json["attack"]["size"][0].get<float>();
            ch.attack_size.y = ch_json["attack"]["size"][1].get<float>();
            
            std::string effect_str = ch_json["attack"]["effect_type"].get<std::string>();
            if (effect_str == "fire") ch.effect_type = EffectType::Fire;
            else if (effect_str == "ice") ch.effect_type = EffectType::Ice;
            else if (effect_str == "lightning") ch.effect_type = EffectType::Lightning;
            else if (effect_str == "heal") ch.effect_type = EffectType::Heal;
            else ch.effect_type = EffectType::Normal;
            
            // UIリソース
            ch.icon_path = ch_json["sprites"]["icon_path"].get<std::string>();
            
            // スプライト情報
            ch.move_sprite.sheet_path = ch_json["sprites"]["move"]["sheet_path"].get<std::string>();
            ch.move_sprite.frame_width = ch_json["sprites"]["move"]["frame_width"].get<int>();
            ch.move_sprite.frame_height = ch_json["sprites"]["move"]["frame_height"].get<int>();
            ch.move_sprite.frame_count = ch_json["sprites"]["move"]["frame_count"].get<int>();
            ch.move_sprite.frame_duration = ch_json["sprites"]["move"]["frame_duration"].get<float>();
            
            ch.attack_sprite.sheet_path = ch_json["sprites"]["attack"]["sheet_path"].get<std::string>();
            ch.attack_sprite.frame_width = ch_json["sprites"]["attack"]["frame_width"].get<int>();
            ch.attack_sprite.frame_height = ch_json["sprites"]["attack"]["frame_height"].get<int>();
            ch.attack_sprite.frame_count = ch_json["sprites"]["attack"]["frame_count"].get<int>();
            ch.attack_sprite.frame_duration = ch_json["sprites"]["attack"]["frame_duration"].get<float>();
            
            // パッシブスキル
            for (const auto& skill_json : ch_json.value("passive_skills", json::array())) {
                PassiveSkill skill;
                skill.id = skill_json["id"].get<std::string>();
                skill.name = skill_json["name"].get<std::string>();
                skill.description = skill_json.value("description", "");
                skill.value = skill_json.value("value", 0.0f);
                ch.passive_skills.push_back(skill);
            }
            
            // 装備
            for (const auto& eq_json : ch_json.value("equipment", json::array())) {
                Equipment eq;
                eq.id = eq_json["id"].get<std::string>();
                eq.name = eq_json["name"].get<std::string>();
                eq.attack_bonus = eq_json.value("attack_bonus", 0.0f);
                eq.hp_bonus = eq_json.value("hp_bonus", 0.0f);
                ch.equipment.push_back(eq);
            }
            
            masters_[ch.id] = ch;
        }
        
        LOG_INFO("Loaded {} characters from JSON", masters_.size());
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse character data: {}", e.what());
        return false;
    }
}

void CharacterManager::InitializeHardcodedData() {
    // 猫戦士
    Character cat;
    cat.id = "cat_001";
    cat.name = "勇敢な猫";
    cat.rarity = 4;
    cat.rarity_name = "SSR";
    cat.description = "勇敢でバランスの取れた猫戦士";
    cat.level = 1;
    cat.hp = 100;
    cat.attack = 80;
    cat.defense = 40;
    cat.move_speed = 150.0f;
    cat.attack_span = 1.5f;
    cat.attack_type = AttackType::Single;
    cat.attack_size = glm::vec2(80.0f, 20.0f);
    cat.effect_type = EffectType::Normal;
    cat.icon_path = "assets/icons/cat_001.png";
    cat.move_sprite.sheet_path = "assets/sprites/cat_001/move.png";
    cat.move_sprite.frame_width = 64;
    cat.move_sprite.frame_height = 64;
    cat.move_sprite.frame_count = 8;
    cat.move_sprite.frame_duration = 0.1f;
    cat.attack_sprite.sheet_path = "assets/sprites/cat_001/attack.png";
    cat.attack_sprite.frame_width = 80;
    cat.attack_sprite.frame_height = 80;
    cat.attack_sprite.frame_count = 6;
    cat.attack_sprite.frame_duration = 0.08f;
    
    PassiveSkill skill;
    skill.id = "skill_defense_up";
    skill.name = "防御アップ";
    skill.description = "防御力が10%上昇";
    skill.value = 0.1f;
    cat.passive_skills.push_back(skill);
    
    Equipment eq;
    eq.id = "eq_sword_001";
    eq.name = "鋼の剣";
    eq.attack_bonus = 15.0f;
    eq.hp_bonus = 0.0f;
    cat.equipment.push_back(eq);
    
    masters_["cat_001"] = cat;
    
    // 犬戦士
    Character dog;
    dog.id = "dog_001";
    dog.name = "強気な犬";
    dog.rarity = 3;
    dog.rarity_name = "SR";
    dog.description = "攻撃型のキャラクター";
    dog.level = 1;
    dog.hp = 80;
    dog.attack = 100;
    dog.defense = 30;
    dog.move_speed = 180.0f;
    dog.attack_span = 1.2f;
    dog.attack_type = AttackType::Range;
    dog.attack_size = glm::vec2(120.0f, 50.0f);
    dog.effect_type = EffectType::Fire;
    dog.icon_path = "assets/icons/dog_001.png";
    dog.move_sprite.sheet_path = "assets/sprites/dog_001/move.png";
    dog.move_sprite.frame_width = 64;
    dog.move_sprite.frame_height = 64;
    dog.move_sprite.frame_count = 8;
    dog.move_sprite.frame_duration = 0.1f;
    dog.attack_sprite.sheet_path = "assets/sprites/dog_001/attack.png";
    dog.attack_sprite.frame_width = 80;
    dog.attack_sprite.frame_height = 80;
    dog.attack_sprite.frame_count = 6;
    dog.attack_sprite.frame_duration = 0.08f;
    
    masters_["dog_001"] = dog;
    
    LOG_INFO("Initialized {} hardcoded characters", masters_.size());
}

std::shared_ptr<Character> CharacterManager::GetCharacterTemplate(
    const std::string& character_id) {
    auto it = masters_.find(character_id);
    if (it == masters_.end()) {
        LOG_WARN("Character not found: {}", character_id);
        return nullptr;
    }
    
    // マスターデータをコピーして返す
    auto ch = std::make_shared<Character>(it->second);
    return ch;
}

std::vector<std::string> CharacterManager::GetAllCharacterIds() const {
    std::vector<std::string> ids;
    for (const auto& [id, _] : masters_) {
        ids.push_back(id);
    }
    return ids;
}

bool CharacterManager::HasCharacter(const std::string& character_id) const {
    return masters_.find(character_id) != masters_.end();
}

void CharacterManager::Shutdown() {
    masters_.clear();
}

} // namespace entities
} // namespace core
} // namespace game
```

---

## 🎮 ゲーム・UI連携

### ゲーム画面での使用例

```cpp
// GameScreen::Initialize()
bool GameScreen::Initialize(BaseSystemAPI* systemAPI) {
    systemAPI_ = systemAPI;
    
    // キャラクターマネージャー初期化
    characterManager_ = std::make_unique<CharacterManager>();
    characterManager_->Initialize();  // ハードコード
    // または：
    // characterManager_->Initialize("assets/data/characters/characters_master.json");
    
    // プレイヤーが所有するキャラクターを取得
    auto cat_ch = characterManager_->GetCharacterTemplate("cat_001");
    if (cat_ch) {
        // ゲーム内エンティティ化（例：GameCharacterEntity作成）
        // ...
    }
    
    return true;
}
```

### UI（編成画面）での使用例

```cpp
// FormationOverlay::Initialize()
bool FormationOverlay::Initialize(BaseSystemAPI* systemAPI) {
    systemAPI_ = systemAPI;
    
    // 親画面（GameScreen）のキャラクターマネージャーを利用
    auto* gameScreen = /* 親へのポインタ取得 */;
    characterManager_ = gameScreen->GetCharacterManager();
    
    // 全キャラクターリストを作成（UI表示用）
    auto character_ids = characterManager_->GetAllCharacterIds();
    
    for (const auto& id : character_ids) {
        auto ch = characterManager_->GetCharacterTemplate(id);
        if (ch) {
            // List コンポーネントにアイテム追加
            ui::ListItem item;
            item.id = ch->id;
            item.label = ch->name;
            item.value = "Lv." + std::to_string(ch->level);
            
            characterList_->AddItem(item);
        }
    }
    
    return true;
}
```

---

## 🔄 実装フロー

### フェーズ1: キャラクター構造体 ✅ 実装完了

**タスク:**
1. ✅ `Character.hpp` 定義 - **実装済み**
2. ✅ `Character.cpp` 実装（GetTotalAttack, GetTotalHP） - **実装済み**
3. ✅ `AnimationData.hpp` 定義 - **実装済み**
4. ✅ コンパイル・動作確認 - **完了**

**出力物:**
- ✅ キャラクター属性完全定義
- ✅ スプライトアニメーション対応

---

### フェーズ2: キャラクター管理 ✅ 実装完了

**タスク:**
1. ✅ `CharacterManager.hpp` 定義 - **実装済み**
2. ✅ `CharacterManager.cpp` 実装 - **実装済み**
   - ✅ ハードコード初期化（猫・犬）
   - ✅ JSON ロード機能
   - ✅ テンプレート取得
3. ✅ JSON スキーマ定義 - **対応済み**
4. ✅ コンパイル・動作確認 - **完了**

**出力物:**
- ✅ マスターデータ管理完全対応
- ✅ JSON / ハードコード両対応

---

### フェーズ3: ゲーム・UI統合 ✅ 実装完了

**タスク:**
1. ✅ GameSystem に CharacterManager 統合 - **実装済み**
2. ✅ FormationOverlay でキャラクターリスト表示 - **実装済み**
3. ✅ CodexOverlay でキャラクター詳細表示 - **実装済み**
4. ✅ 統合動作確認 - **完了**

**出力物:**
- ✅ ゲーム + UI完全連携

---

## ✅ チェックリスト

### フェーズ1: キャラクター構造体 ✅ 完了

- [x] `Character.hpp` enum/struct 定義完了
- [x] `Character.cpp` ユーティリティ実装完了
- [x] `AnimationData.hpp` 定義完了
- [x] コンパイル成功

### フェーズ2: キャラクター管理 ✅ 完了

- [x] `CharacterManager.hpp` インターフェース完成
- [x] ハードコード初期化実装（cat_001, dog_001）
- [x] JSON ロード機能実装
- [x] GetCharacterTemplate() 動作確認
- [x] GetAllCharacterIds() 動作確認

### フェーズ3: ゲーム・UI統合 ✅ 完了

- [x] GameSystem::Initialize() 統合完了
- [x] FormationOverlay キャラクターリスト表示
- [x] CodexOverlay キャラクター詳細表示
- [x] 複数キャラクター管理確認
- [x] JSON ロード機能動作確認（オプション）

---

## 📝 設計原則

### 1. シンプル・実装速度優先

- ハードコード初期化で即座に開発開始
- JSON は「後で」追加する柔軟性
- マスターデータは読み取り専用

### 2. 共通管理

- 味方 + 敵キャラを同じ構造体で管理
- Level, Experience 等は個別に管理（後で追加可能）

### 3. アニメーション対応

- スプライトシート前提
- フレーム数 + 表示時間で自動化
- 複雑なアニメーションはシンプル化

### 4. 拡張性

- PassiveSkill, Equipment は可変長
- 属性フラグは enum で管理
- JSON スキーマで将来追加も容易

---

**これでゲーム + UI両対応の共通キャラクターシステムが実装できます！** 🎯