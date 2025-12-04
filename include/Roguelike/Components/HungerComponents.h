/**
 * @file HungerComponents.h
 * @brief 空腹度システム用コンポーネント
 * 
 * Phase 5: 空腹度管理、食事による回復
 */
#pragma once

#include <string>

namespace Roguelike::Components {

/**
 * @brief 空腹状態
 */
enum class HungerState {
    Satiated,       ///< 満腹（ボーナス状態）
    Normal,         ///< 通常
    Hungry,         ///< 空腹（警告）
    Weak,           ///< 衰弱（ペナルティ開始）
    Fainting,       ///< 気絶寸前（行動制限）
    Starving        ///< 餓死寸前（HP減少）
};

/**
 * @brief 空腹度コンポーネント
 */
struct Hunger {
    int current = 1500;         ///< 現在の満腹度
    int max = 2000;             ///< 最大満腹度
    
    // 状態閾値
    static constexpr int SATIATED_THRESHOLD = 1800;   ///< 満腹
    static constexpr int HUNGRY_THRESHOLD = 500;      ///< 空腹
    static constexpr int WEAK_THRESHOLD = 200;        ///< 衰弱
    static constexpr int FAINTING_THRESHOLD = 50;     ///< 気絶寸前
    
    // 減少量（行動ごと）
    static constexpr int HUNGER_PER_ACTION = 1;       ///< 通常行動
    static constexpr int HUNGER_PER_MOVE = 1;         ///< 移動
    static constexpr int HUNGER_PER_ATTACK = 2;       ///< 攻撃
    static constexpr int HUNGER_PER_WAIT = 1;         ///< 待機
    
    /**
     * @brief 現在の空腹状態を取得
     */
    HungerState GetState() const {
        if (current >= SATIATED_THRESHOLD) return HungerState::Satiated;
        if (current >= HUNGRY_THRESHOLD) return HungerState::Normal;
        if (current >= WEAK_THRESHOLD) return HungerState::Hungry;
        if (current >= FAINTING_THRESHOLD) return HungerState::Weak;
        if (current > 0) return HungerState::Fainting;
        return HungerState::Starving;
    }
    
    /**
     * @brief 状態の日本語名を取得
     */
    const char* GetStateName() const {
        switch (GetState()) {
            case HungerState::Satiated: return "満腹";
            case HungerState::Normal:   return "";  // 通常時は非表示
            case HungerState::Hungry:   return "空腹";
            case HungerState::Weak:     return "衰弱";
            case HungerState::Fainting: return "気絶寸前";
            case HungerState::Starving: return "餓死寸前";
        }
        return "";
    }
    
    /**
     * @brief 状態の表示色を取得（R, G, B）
     */
    void GetStateColor(unsigned char& r, unsigned char& g, unsigned char& b) const {
        switch (GetState()) {
            case HungerState::Satiated:
                r = 100; g = 200; b = 100;  // 緑
                break;
            case HungerState::Normal:
                r = 200; g = 200; b = 200;  // 白
                break;
            case HungerState::Hungry:
                r = 255; g = 255; b = 0;    // 黄
                break;
            case HungerState::Weak:
                r = 255; g = 165; b = 0;    // オレンジ
                break;
            case HungerState::Fainting:
                r = 255; g = 100; b = 100;  // 薄赤
                break;
            case HungerState::Starving:
                r = 255; g = 0; b = 0;      // 赤
                break;
        }
    }
    
    /**
     * @brief 空腹度を減少
     * @param amount 減少量
     */
    void Decrease(int amount) {
        current -= amount;
        if (current < 0) current = 0;
    }
    
    /**
     * @brief 食事で回復
     * @param amount 回復量
     */
    void Eat(int amount) {
        current += amount;
        if (current > max) current = max;
    }
    
    /**
     * @brief 満腹度の割合を取得
     */
    float GetRatio() const {
        return static_cast<float>(current) / max;
    }
    
    /**
     * @brief 衰弱によるペナルティを取得
     * @return 攻撃/防御への減算値
     */
    int GetPenalty() const {
        switch (GetState()) {
            case HungerState::Weak:     return 1;
            case HungerState::Fainting: return 3;
            case HungerState::Starving: return 5;
            default: return 0;
        }
    }
    
    /**
     * @brief 満腹によるボーナスを取得
     * @return HP回復ボーナス（ターンごと）
     */
    int GetRegenBonus() const {
        return (GetState() == HungerState::Satiated) ? 1 : 0;
    }
    
    /**
     * @brief 気絶判定
     * @return 気絶する確率（%）
     */
    int GetFaintChance() const {
        if (GetState() == HungerState::Fainting) return 20;
        return 0;
    }
};

/**
 * @brief 食料の栄養価
 */
struct FoodNutrition {
    int nutrition = 500;        ///< 回復する満腹度
    bool isRotted = false;      ///< 腐っているか（マイナス効果）
};

} // namespace Roguelike::Components
