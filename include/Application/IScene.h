/**
 * @file IScene.h
 * @brief シーンインターフェース
 * 
 * 拡張可能なシーンベースアーキテクチャの基盤。
 * ホームシーン、ゲームシーン、設定シーンなどを統一インターフェースで管理。
 * 
 * Phase 3: 統合アーキテクチャの一部として実装
 */
#pragma once

#include "Core/World.h"
#include "Core/GameContext.h"
#include <string>

namespace Application {

/**
 * @brief シーンインターフェース
 * 
 * すべてのシーンはこのインターフェースを実装する。
 * 将来的に以下のようなシーンを追加可能：
 * - HomeScene: ホーム画面（ゲームモード選択）
 * - TDGameScene: TDゲーム画面
 * - RoguelikeGameScene: Roguelikeゲーム画面
 * - SettingsScene: 設定画面
 * - SaveLoadScene: セーブ/ロード画面
 */
class IScene {
public:
    virtual ~IScene() = default;
    
    /**
     * @brief シーン名を取得
     */
    virtual std::string GetName() const = 0;
    
    /**
     * @brief シーンを初期化
     * @param world ECSワールド
     * @param context ゲームコンテキスト
     */
    virtual void Initialize(Core::World& world, Core::GameContext& context) = 0;
    
    /**
     * @brief シーンを更新
     * @param world ECSワールド
     * @param context ゲームコンテキスト
     * @param deltaTime 前フレームからの経過時間（秒）
     */
    virtual void Update(Core::World& world, Core::GameContext& context, float deltaTime) = 0;
    
    /**
     * @brief シーンを描画
     * @param world ECSワールド
     * @param context ゲームコンテキスト
     */
    virtual void Render(Core::World& world, Core::GameContext& context) = 0;
    
    /**
     * @brief シーンを終了
     * @param world ECSワールド
     * @param context ゲームコンテキスト
     */
    virtual void Shutdown(Core::World& world, Core::GameContext& context) = 0;
    
    /**
     * @brief シーンが終了を要求しているか
     * @return 終了要求時は遷移先シーン名、継続時は空文字列
     */
    virtual std::string GetNextScene() const { return ""; }
    
    /**
     * @brief シーンの終了要求をクリア
     */
    virtual void ClearNextScene() {}
};

} // namespace Application

