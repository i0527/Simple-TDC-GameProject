/**
 * @file UnifiedGame.h
 * @brief 統合ゲームプラットフォーム
 * 
 * TD/Roguelike統合プラットフォームの中核クラス。
 * ゲームモードの切り替えと統合実行を管理する。
 */
#pragma once

#include "Application/GameMode.h"
#include "Application/IScene.h"
#include "Core/World.h"
#include "Core/GameContext.h"
#include "Core/SystemRunner.h"
#include "Core/HTTPServer.h"
#include "Data/Registry.h"
#include "Data/Loaders/DefinitionLoader.h"
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace Application {

/**
 * @brief 統合ゲームクラス
 * 
 * TDとRoguelikeを統合した単一プラットフォーム。
 * ゲームモードの切り替え、システム実行、データ定義管理を行う。
 */
class UnifiedGame {
public:
    UnifiedGame();
    ~UnifiedGame() = default;
    
    // コピー禁止
    UnifiedGame(const UnifiedGame&) = delete;
    UnifiedGame& operator=(const UnifiedGame&) = delete;
    
    // ムーブ許可
    UnifiedGame(UnifiedGame&&) = default;
    UnifiedGame& operator=(UnifiedGame&&) = default;
    
    /**
     * @brief 初期化
     * @param definitionsPath 定義ファイルのベースパス
     */
        bool Initialize(
            const std::string& definitionsPath = "assets/definitions",
            bool enableHTTPServer = false,
            int httpServerPort = 8080
        );
    
    /**
     * @brief ゲームモードを設定
     */
    void SetGameMode(GameMode mode);
    
    /**
     * @brief 現在のゲームモードを取得
     */
    GameMode GetCurrentGameMode() const { return currentMode_; }
    
    /**
     * @brief ゲームループの実行
     */
    void Run();
    
    /**
     * @brief 更新処理
     * @param deltaTime 前フレームからの経過時間（秒）
     */
    void Update(float deltaTime);
    
    /**
     * @brief 描画処理
     */
    void Render();
    
    /**
     * @brief 終了処理
     */
    void Shutdown();

private:
    /**
     * @brief ゲームモード固有の初期化
     */
    void InitializeGameMode(GameMode mode);
    
    /**
     * @brief 現在のゲームモードのシステムをクリア
     */
    void ClearCurrentSystems();
    
    /**
     * @brief TDモードのシステムを登録
     */
    void RegisterTDSystems();
    
    /**
     * @brief Roguelikeモードのシステムを登録
     */
    void RegisterRoguelikeSystems();
    
    /**
     * @brief メニューモードのシステムを登録
     */
    void RegisterMenuSystems();
    
    /**
     * @brief シーンを登録
     */
    void RegisterScene(const std::string& name, std::unique_ptr<IScene> scene);
    
    /**
     * @brief シーンを切り替え
     */
    void ChangeScene(const std::string& name);
    
    /**
     * @brief 現在のシーン名を取得
     */
    std::string GetCurrentSceneName() const { return currentSceneName_; }

    /**
     * @brief ゲーム状態をJSON形式で取得（デバッグ/プレビュー用）
     */
    nlohmann::json GetGameState() const;

private:
    GameMode currentMode_ = GameMode::Menu;
    std::string currentSceneName_;
    
    // コアコンポーネント
    std::unique_ptr<Core::World> world_;
    std::unique_ptr<Core::GameContext> context_;
    std::unique_ptr<Core::SystemRunner> systemRunner_;
    
    // データ定義
    std::unique_ptr<Data::DefinitionRegistry> definitions_;
    std::unique_ptr<Data::DefinitionLoader> definitionLoader_;
    
    // HTTPサーバー（UIエディター用）
    std::unique_ptr<Core::HTTPServer> httpServer_;
    int httpServerPort_ = 8080;
    bool enableHTTPServer_ = false;
    
    // シーン管理
    std::unordered_map<std::string, std::unique_ptr<IScene>> scenes_;
    std::string nextSceneName_;  // 次のフレームに切り替えるシーン
    
    // 状態
    bool initialized_ = false;
    bool running_ = false;
};

} // namespace Application

