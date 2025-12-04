/**
 * @file StatsComponents.h
 * @brief ローグライク用ステータスコンポーネント
 * 
 * キャラクターのHP、攻撃力、防御力などのステータスを管理
 * 
 * Phase 3: Roguelike統合
 */
#pragma once

namespace Domain::Roguelike::Components {

/**
 * @brief キャラクターステータス
 */
struct Stats {
  int maxHp = 10;
  int hp = 10;
  int attack = 1;
  int defense = 0;
  int level = 1;
  int experience = 0;

  /**
   * @brief ダメージを受ける
   */
  void TakeDamage(int damage) {
    int actualDamage = std::max(1, damage - defense);
    hp -= actualDamage;
    if (hp < 0) hp = 0;
  }

  /**
   * @brief HP を回復
   */
  void Heal(int amount) {
    hp += amount;
    if (hp > maxHp) hp = maxHp;
  }

  /**
   * @brief HPが0か確認
   */
  bool IsDead() const { return hp <= 0; }
};

} // namespace Domain::Roguelike::Components

