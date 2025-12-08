#pragma once

#include <string>

namespace New::Game::Components {

enum class SlotType {
  Main,    // メイン枠（召喚数制限1）
  Ability, // アビリティ枠（召喚数制限なし）
  Sub      // サブ枠（召喚数制限なし）
};

struct BattleState {
  int playerLife = 10;
  int cost = 0;
  int selectedSlot = 0;
  int selectedSlotCost = 30;
  int waveIndex = 0;
  int totalWaves = 1;
  bool victory = false;
  bool defeat = false;

  float costRegenPerSec = 5.0f;
  int waveBonusCost = 50;
  int killReward = 5;
  int baseArrivalDamage = 1;
  float minGap = 80.0f;        // 前線の最小間隔（敵味方間）
  float debugKnockback = 0.0f; // デバッグ上書き(0ならデータ値)
  int frontlineIterations = 3; // 前線補正反復回数

  // 配置/入力に関する簡易メッセージ
  std::string lastMessage;
  float messageTtl = 0.0f;

  // スロットクールダウン（秒）- 10スロット対応
  float slotCooldowns[10] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

  // スロットコスト（表示用）- 10スロット対応
  int slotCosts[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // スロットタイプ（メイン/アビリティ/サブ）- 10スロット対応
  SlotType slotTypes[10] = {
      SlotType::Main,    SlotType::Main,    SlotType::Main, // 0-2: メイン
      SlotType::Ability, SlotType::Ability, // 3-4: アビリティ
      SlotType::Sub,     SlotType::Sub,     SlotType::Sub,
      SlotType::Sub,     SlotType::Sub // 5-9: サブ
  };

  // 召喚数制限（メインは1、他は無制限）- 10スロット対応
  int slotSummonLimits[10] = {1,  1,  1,  -1, -1,
                              -1, -1, -1, -1, -1}; // -1は無制限

  // 現在の召喚数（メインスロット用）- 10スロット対応
  int slotSummonCounts[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // 配置リクエスト（入力からシステムへ受け渡し）
  bool hasPlacement = false;
  float placementX = 0.0f;
  float placementY = 0.0f;
};

} // namespace New::Game::Components
