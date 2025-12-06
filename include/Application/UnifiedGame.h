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
#include "Data/Registry.h"
#include "Data/Loaders/DefinitionLoader.h"
#include "Game/DevMode/DevModeManager.h"
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
        const std::string& definitionsPath = "assets/definitions"
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

    // ===== 開発者モード用エンティティ操作 =====

    /**
     * @brief 全エンティティ情報をJSON形式で取得
     */
    nlohmann::json GetEntities() const;

    /**
     * @brief 特定エンティティ情報をJSON形式で取得
     */
    nlohmann::json GetEntity(const std::string& entityId) const;

    /**
     * @brief エンティティ作成（テスト用）
     */
    nlohmann::json CreateEntity(const nlohmann::json& config) const;

    /**
     * @brief エンティティ更新
     */
    nlohmann::json UpdateEntity(const std::string& entityId, const nlohmann::json& config) const;

    /**
     * @brief エンティティ削除
     */
    bool DeleteEntity(const std::string& entityId) const;

    /**
     * @brief エンティティスポーン
     */
    nlohmann::json SpawnEntity(const std::string& entityId, const nlohmann::json& spawnParams) const;

    /**
     * @brief エンティティステータス設定
     */
    nlohmann::json SetEntityStats(const std::string& entityId, const nlohmann::json& stats) const;

    /**
     * @brief エンティティAI設定
     */
    nlohmann::json SetEntityAI(const std::string& entityId, const nlohmann::json& aiConfig) const;

    /**
     * @brief エンティティスキル設定
     */
    nlohmann::json SetEntitySkills(const std::string& entityId, const nlohmann::json& skills) const;

    /**
     * @brief スクリーンショット取得（開発者モード用）
     * @return Base64エンコードされたPNG画像データ（現在はプレースホルダー）
     */
    std::string GetScreenshot() const;

private:
    GameMode currentMode_ = GameMode::Menu;
    std::string currentSceneName_;
    
    std::unique_ptr<Core::World> world_;
    std::unique_ptr<Core::GameContext> context_;
    std::unique_ptr<Core::SystemRunner> systemRunner_;
    
    std::unique_ptr<Data::DefinitionRegistry> definitions_;
    std::unique_ptr<Data::DefinitionLoader> definitionLoader_;
    
    std::unordered_map<std::string, std::unique_ptr<IScene>> scenes_;
    std::string nextSceneName_;
    
    std::unique_ptr<Game::DevMode::DevModeManager> devMode_;
    
    // 状態
    bool initialized_ = false;
    bool running_ = false;
};

} // namespace Application

