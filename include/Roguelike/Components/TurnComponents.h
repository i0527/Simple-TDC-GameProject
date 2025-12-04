/**
 * @file TurnComponents.h
 * @brief ターン制システム関連のコンポーネント定義
 * 
 * NetHackのエネルギー制ターンシステムを再現。
 * 各アクターはspeedに応じてenergyを蓄積し、
 * 100に達すると行動可能になる。
 */
#pragma once

#include <entt/entt.hpp>

namespace Roguelike::Components {

/**
 * @brief ターン行動を持つエンティティ
 * 
 * プレイヤーとモンスターの両方に付与。
 * speedが高いほど頻繁に行動できる。
 */
struct TurnActor {
    int speed = 100;         // 行動速度（基準値100、高いほど速い）
    int energy = 0;          // 蓄積エネルギー（100で行動可能）
    bool isPlayer = false;   // プレイヤーか（入力待ちに使用）
    
    /**
     * @brief エネルギーを供給
     */
    void GainEnergy() {
        energy += speed;
    }
    
    /**
     * @brief 行動可能か判定
     */
    bool CanAct() const {
        return energy >= 100;
    }
    
    /**
     * @brief 行動消費（エネルギーを100減らす）
     */
    void SpendEnergy() {
        energy -= 100;
    }
};

/**
 * @brief プレイヤー入力待ち状態（タグコンポーネント）
 * 
 * このコンポーネントがプレイヤーに付与されている間、
 * ゲームは入力待ち状態となり、ターン進行が停止する。
 */
struct AwaitingInput {};

/**
 * @brief 行動コマンド
 * 
 * プレイヤーの入力またはAIにより設定され、
 * 各システムで処理される。
 */
struct ActionCommand {
    enum class Type {
        None,           // 未設定
        Move,           // 移動（またはその方向の敵を攻撃）
        Wait,           // 待機（ターン消費）
        PickUp,         // アイテムを拾う
        Use,            // アイテムを使う
        Drop,           // アイテムを落とす
        Open,           // ドアを開ける
        Close,          // ドアを閉める
        Descend,        // 下り階段で降りる
        Ascend,         // 上り階段で昇る
        Attack,         // 明示的攻撃（通常はMoveで自動判定）
    };
    
    Type type = Type::None;
    
    // 移動/攻撃の方向（-1, 0, 1）
    int dx = 0;
    int dy = 0;
    
    // 対象エンティティ（攻撃、アイテム使用等）
    entt::entity targetEntity = entt::null;
    
    // インベントリスロット番号（アイテム操作時）
    int itemSlot = -1;
    
    // ターゲット座標（攻撃時）
    int targetX = 0;
    int targetY = 0;
    
    /**
     * @brief コマンドをリセット
     */
    void Clear() {
        type = Type::None;
        dx = 0;
        dy = 0;
        targetEntity = entt::null;
        itemSlot = -1;
        targetX = 0;
        targetY = 0;
    }
    
    /**
     * @brief 移動コマンドを設定
     */
    static ActionCommand MakeMove(int dx, int dy) {
        ActionCommand cmd;
        cmd.type = Type::Move;
        cmd.dx = dx;
        cmd.dy = dy;
        return cmd;
    }
    
    /**
     * @brief 待機コマンドを設定
     */
    static ActionCommand MakeWait() {
        ActionCommand cmd;
        cmd.type = Type::Wait;
        return cmd;
    }
    
    /**
     * @brief 拾うコマンドを設定
     */
    static ActionCommand MakePickUp() {
        ActionCommand cmd;
        cmd.type = Type::PickUp;
        return cmd;
    }
    
    /**
     * @brief 階段降りるコマンドを設定
     */
    static ActionCommand MakeDescend() {
        ActionCommand cmd;
        cmd.type = Type::Descend;
        return cmd;
    }
    
    /**
     * @brief 階段昇るコマンドを設定
     */
    static ActionCommand MakeAscend() {
        ActionCommand cmd;
        cmd.type = Type::Ascend;
        return cmd;
    }
    
    /**
     * @brief アイテム使用コマンドを設定
     */
    static ActionCommand MakeUse(int slot) {
        ActionCommand cmd;
        cmd.type = Type::Use;
        cmd.itemSlot = slot;
        return cmd;
    }
    
    /**
     * @brief アイテムドロップコマンドを設定
     */
    static ActionCommand MakeDrop(int slot) {
        ActionCommand cmd;
        cmd.type = Type::Drop;
        cmd.itemSlot = slot;
        return cmd;
    }
};

/**
 * @brief 表示用の見た目情報
 * 
 * タイル描画時に使用する文字と色。
 */
struct Appearance {
    char symbol = '?';           // 表示文字
    unsigned char r = 255;       // 文字色R
    unsigned char g = 255;       // 文字色G
    unsigned char b = 255;       // 文字色B
    
    Appearance() = default;
    Appearance(char sym, unsigned char red, unsigned char green, unsigned char blue)
        : symbol(sym), r(red), g(green), b(blue) {}
};

/**
 * @brief 名前情報
 */
struct Name {
    std::string value;
    std::string description;
    
    Name() = default;
    explicit Name(const std::string& n) : value(n) {}
    Name(const std::string& n, const std::string& desc) : value(n), description(desc) {}
};

} // namespace Roguelike::Components

