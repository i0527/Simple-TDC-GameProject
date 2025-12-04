/**
 * @file RoguelikeGameScene.h
 * @brief ローグライクゲームシーン
 * 
 * ローグライクゲーム専用シーン。
 * HomeSceneから遷移され、Roguelike固有のゲームプレイを実行する。
 * ESCキーでHomeSceneに戻る。
 * 
 * Phase 2.2: ゲームシーン実装
 */
#pragma once

#include "Application/IScene.h"
#include "Domain/Roguelike/Components/RoguelikeComponents.h"
#include <string>
#include <memory>

namespace Application {

/**
 * @brief ローグライクゲームシーン
 * 
 * 機能:
 * - Roguelike固有のシステム登録
 * - ダンジョン生成
 * - ターン制ゲームループ
 * - ESC キーでホーム画面に戻る
 * - ゲーム状態管理（進行中/クリア/ゲームオーバー）
 */
class RoguelikeGameScene : public IScene {
public:
    RoguelikeGameScene();
    ~RoguelikeGameScene() override = default;
    
    // IScene実装
    std::string GetName() const override { return "Roguelike"; }
    void Initialize(Core::World& world, Core::GameContext& context) override;
    void Update(Core::World& world, Core::GameContext& context, float deltaTime) override;
    void Render(Core::World& world, Core::GameContext& context) override;
    void Shutdown(Core::World& world, Core::GameContext& context) override;
    std::string GetNextScene() const override { return nextScene_; }
    void ClearNextScene() override { nextScene_.clear(); }

private:
    /**
     * @brief Roguelike固有のシステムを登録
     */
    void RegisterRoguelikeSystems(Core::GameContext& context);
    
    /**
     * @brief ダンジョンを生成
     */
    void GenerateDungeon(Core::World& world, Core::GameContext& context);
    
    /**
     * @brief 入力処理（ESCでホームに戻る等）
     */
    void ProcessInput();
    
    /**
     * @brief ゲーム終了状態をチェック（クリア/ゲームオーバー）
     */
    void CheckGameEndCondition(Core::World& world, Core::GameContext& context);

private:
    std::string nextScene_;  // シーン遷移先
    bool initialized_ = false;
    int turnCount_ = 0;
    
    // MapData のライフタイム管理
    std::unique_ptr<Domain::Roguelike::Components::MapData> mapData_;
};

} // namespace Application

