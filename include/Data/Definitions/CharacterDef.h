/**
 * @file CharacterDef.h
 * @brief キャラクター定義構造体
 *
 * TD/Roguelike統合プラットフォーム用の統一キャラクター定義。
 * JSONから読み込まれる静的データを表現する。
 */
#pragma once

#include "Data/Definitions/AnimationDef.h"
#include "Data/Definitions/CommonTypes.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace Data {

/**
 * @brief ゲームモード種別
 */
enum class GameModeType {
  TD,        // タワーディフェンス
  Roguelike, // ローグライク
  Both       // 両方
};

/**
 * @brief キャラクターのレアリティ
 */
enum class Rarity { Normal, Rare, SuperRare, UberRare, Legend };

/**
 * @brief 攻撃タイプ
 */
enum class AttackType {
  Single, // 単体攻撃
  Area,   // 範囲攻撃
  Wave    // 波動（貫通）
};

/**
 * @brief TD固有設定
 */
struct TDConfig {
  float cost = 100.0f;       // 召喚コスト
  float rechargeTime = 3.0f; // 再召喚までの時間
  bool laneMovement = true;  // レーン移動可能か
  float laneSpeed = 50.0f;   // レーン移動速度
};

/**
 * @brief Roguelike固有設定
 */
struct RoguelikeConfig {
  bool gridMovement = true; // グリッド移動可能か
  bool turnBased = true;    // ターン制か
  int movementCost = 1;     // 移動コスト
};

/**
 * @brief キャラクター定義
 *
 * JSONからロードされるキャラクターの全パラメータ。
 * TD/Roguelike両方で使用可能。
 */
struct CharacterDef {
  // === 識別 ===
  std::string id;          // 一意のID（例: "cupslime"）
  std::string name;        // 表示名
  std::string description; // 説明文
  Rarity rarity = Rarity::Normal;
  std::vector<std::string> traits; // 特性タグ（"floating", "metal" 等）

  // === 使用可能なゲームモード ===
  GameModeType gameMode = GameModeType::Both; // "td" | "roguelike" | "both"

  // === ビジュアル ===
  struct Visual {
    struct Sprite {
      std::string atlasPath; // スプライトアトラスのパス
      std::string jsonPath;  // アトラスJSON定義のパス
    } sprite;

    int frameWidth = 64;
    int frameHeight = 64;
    int framesPerRow = 8;
    float scale = 1.0f;
    std::unordered_map<std::string, AnimationDef> animations;
    std::string defaultAnimation = "idle";
  } visual;

  // === ステータス ===
  struct Stats {
    float hp = 100.0f;
    float attack = 15.0f;
    float defense = 5.0f;
    float moveSpeed = 120.0f;
    float attackInterval = 1.5f;
    float knockbackResist = 0.0f;
  } stats;

  // === 戦闘 ===
  struct Combat {
    AttackType attackType = AttackType::Single;
    float attackRange = 50.0f;
    float attackCooldown = 1.5f;
    Rect hitbox;                 // 当たり判定
    Rect attackRangeArea;        // 攻撃範囲（Area型の場合）
    float criticalChance = 0.0f; // クリティカル率
    float criticalMultiplier = 1.5f;
    int attackCount = 1;
  } combat;

  // === ゲームモード固有設定 ===
  TDConfig td;
  RoguelikeConfig roguelike;

  // === スキル ===
  std::vector<std::string> skillIds; // 所持スキルID一覧

  // === レベルアップ時の成長率 ===
  float healthGrowth = 1.1f; // 1レベルあたりHP倍率
  float attackGrowth = 1.1f; // 1レベルあたり攻撃倍率
};

} // namespace Data
