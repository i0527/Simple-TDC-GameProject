/**
 * @file HomeScene.h
 * @brief ホームシーン
 * 
 * ゲームモード選択を行うホーム画面。
 * TD/Roguelikeを選択できるUIを提供。
 * 将来的に設定、セーブ/ロード、クレジットなども追加可能。
 * 
 * Phase 3: 拡張可能なシーンベース設計
 */
#pragma once

#include "Application/IScene.h"
#include "Application/GameMode.h"
#include <string>
#include <vector>
#include <functional>

namespace Application {

/**
 * @brief ホームシーンのメニュー項目
 */
struct MenuItem {
    std::string label;
    GameMode gameMode;
    bool enabled;
    
    MenuItem(const std::string& l, GameMode mode, bool e = true)
        : label(l), gameMode(mode), enabled(e) {}
};

/**
 * @brief ホームシーン
 * 
 * 機能:
 * - ゲームモード選択（TD/Roguelike）
 * - 将来の拡張ポイント（設定、セーブ/ロード、クレジットなど）
 */
class HomeScene : public IScene {
public:
    HomeScene();
    ~HomeScene() override = default;
    
    // IScene実装
    std::string GetName() const override { return "Home"; }
    void Initialize(Core::World& world, Core::GameContext& context) override;
    void Update(Core::World& world, Core::GameContext& context, float deltaTime) override;
    void Render(Core::World& world, Core::GameContext& context) override;
    void Shutdown(Core::World& world, Core::GameContext& context) override;
    std::string GetNextScene() const override { return nextScene_; }
    void ClearNextScene() override { nextScene_.clear(); }
    
    /**
     * @brief ゲームモード選択時のコールバック設定
     */
    void SetGameModeCallback(std::function<void(GameMode)> callback) {
        gameModeCallback_ = std::move(callback);
    }

private:
    /**
     * @brief メニュー項目を追加（拡張可能）
     */
    void AddMenuItem(const std::string& label, GameMode mode, bool enabled = true);
    
    /**
     * @brief 入力処理
     */
    void ProcessInput();
    
    /**
     * @brief メニュー選択を実行
     */
    void ExecuteSelection();
    
    /**
     * @brief UI描画
     */
    void DrawUI();

private:
    std::vector<MenuItem> menuItems_;
    int selectedIndex_ = 0;
    std::string nextScene_;  // シーン遷移先（例: "TDGame", "RoguelikeGame"）
    
    std::function<void(GameMode)> gameModeCallback_;
    
    bool initialized_ = false;
    
    // UI定数（FHD座標: 1920x1080）
    static constexpr int CENTER_X = 1920 / 2;
    static constexpr int CENTER_Y = 1080 / 2;
    static constexpr int MENU_ITEM_HEIGHT = 80;
    static constexpr int MENU_ITEM_SPACING = 20;
    static constexpr int MENU_FONT_SIZE = 48;
    static constexpr int TITLE_FONT_SIZE = 72;
};

} // namespace Application

