/**
 * @file TDGameScene.h
 * @brief TDゲームシーン
 * 
 * タワーディフェンスゲーム専用シーン。
 * HomeSceneから遷移され、TD固有のゲームプレイを実行する。
 * ESCキーでHomeSceneに戻る。
 * 
 * Phase 2.1: ゲームシーン実装
 */
#pragma once

#include "Application/IScene.h"
#include <string>

namespace Application {

/**
 * @brief TDゲームシーン
 * 
 * 機能:
 * - TD固有のシステム登録
 * - ステージ管理
 * - ESC キーでホーム画面に戻る
 * - ゲーム状態管理（進行中/クリア/ゲームオーバー）
 */
class TDGameScene : public IScene {
public:
    TDGameScene();
    ~TDGameScene() override = default;
    
    // IScene実装
    std::string GetName() const override { return "TDGame"; }
    void Initialize(Core::World& world, Core::GameContext& context) override;
    void Update(Core::World& world, Core::GameContext& context, float deltaTime) override;
    void Render(Core::World& world, Core::GameContext& context) override;
    void Shutdown(Core::World& world, Core::GameContext& context) override;
    std::string GetNextScene() const override { return nextScene_; }
    void ClearNextScene() override { nextScene_.clear(); }

private:
    /**
     * @brief TD固有のシステムを登録
     */
    void RegisterTDSystems(Core::GameContext& context);
    
    /**
     * @brief デフォルトステージを読み込む
     */
    void LoadDefaultStage(Core::World& world, Core::GameContext& context);
    
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
    float elapsedTime_ = 0.0f;
};

} // namespace Application

