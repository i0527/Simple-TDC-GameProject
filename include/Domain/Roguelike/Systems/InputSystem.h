/**
 * @file InputSystem.h
 * @brief プレイヤー入力処理システム
 * 
 * シンプルな操作体系（エルローグ/RPGツクール風）:
 * - 矢印キー: 移動
 * - Enter/Space: 決定（足元メニュー、アイテム使用など）
 * - ESC: キャンセル/メニュー閉じる
 * - I: インベントリ
 * 
 * Phase 3: 新しい統一アーキテクチャに移行
 */
#pragma once

#include <raylib.h>
#include <entt/entt.hpp>
#include "../Components/RoguelikeComponents.h"

namespace Domain::Roguelike::Systems {

/**
 * @brief 入力システム
 * 
 * エルローグ/RPGツクール風のシンプル操作
 */
class InputSystem {
public:
    /**
     * @brief 入力を処理
     * 
     * @param registry ECSレジストリ
     * @return 入力があったらtrue
     */
    static bool ProcessInput(entt::registry& registry) {
        // 入力待ちのプレイヤーを探す
        auto view = registry.view<Components::TurnActor, 
                                   Components::ActionCommand,
                                   Components::AwaitingInput>();
        
        for (auto entity : view) {
            auto& cmd = view.get<Components::ActionCommand>(entity);
            
            // 既にコマンドがあれば処理しない
            if (cmd.type != Components::ActionCommand::Type::None) {
                return true;
            }
            
            // 移動キー（矢印キーのみ、4方向）
            if (IsKeyPressed(KEY_UP)) {
                cmd = Components::ActionCommand::MakeMove(0, -1);
                return true;
            }
            if (IsKeyPressed(KEY_DOWN)) {
                cmd = Components::ActionCommand::MakeMove(0, 1);
                return true;
            }
            if (IsKeyPressed(KEY_LEFT)) {
                cmd = Components::ActionCommand::MakeMove(-1, 0);
                return true;
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                cmd = Components::ActionCommand::MakeMove(1, 0);
                return true;
            }
            
            // 待機（Wキー）
            if (IsKeyPressed(KEY_W)) {
                cmd = Components::ActionCommand::MakeWait();
                return true;
            }
        }
        
        return false;
    }
    
    /**
     * @brief メニュー選択の入力処理
     * @param maxOptions 選択肢の最大数
     * @param currentSelection 現在の選択位置（参照で更新）
     * @return 1=決定, -1=キャンセル, 0=変更なし
     */
    static int ProcessMenuInput(int maxOptions, int& currentSelection) {
        // 上下で選択
        if (IsKeyPressed(KEY_UP)) {
            currentSelection = (currentSelection - 1 + maxOptions) % maxOptions;
            return 0;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            currentSelection = (currentSelection + 1) % maxOptions;
            return 0;
        }
        
        // Enter/Spaceで決定
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            return 1;
        }
        
        // ESCでキャンセル
        if (IsKeyPressed(KEY_ESCAPE)) {
            return -1;
        }
        
        return 0;
    }
    
    /**
     * @brief 決定キー（Enter/Space）が押されたか
     */
    static bool IsConfirmPressed() {
        return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    }
    
    /**
     * @brief キャンセルキー（ESC）が押されたか
     */
    static bool IsCancelPressed() {
        return IsKeyPressed(KEY_ESCAPE);
    }
    
    /**
     * @brief インベントリキーが押されたか
     */
    static bool IsInventoryPressed() {
        return IsKeyPressed(KEY_I);
    }
    
    /**
     * @brief ヘルプテキストを取得
     */
    static const char* GetHelpText() {
        return 
            "[矢印キー] 移動  "
            "[Enter/Space] 決定/調べる  "
            "[ESC] キャンセル  "
            "[I] 持ち物  "
            "[W] 待機";
    }
};

} // namespace Domain::Roguelike::Systems

