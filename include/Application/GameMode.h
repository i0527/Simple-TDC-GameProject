/**
 * @file GameMode.h
 * @brief ゲームモード定義
 * 
 * 統合ゲームプラットフォームでサポートするゲームモードを定義
 */
#pragma once

#include <string>

namespace Application {

/**
 * @brief ゲームモード列挙型
 */
enum class GameMode {
    Menu,           // メニュー画面
    TD,             // タワーディフェンス
    Roguelike       // ローグライク
};

/**
 * @brief ゲームモード名を文字列で取得
 */
inline const char* GameModeToString(GameMode mode) {
    switch (mode) {
        case GameMode::Menu: return "Menu";
        case GameMode::TD: return "TD";
        case GameMode::Roguelike: return "Roguelike";
        default: return "Unknown";
    }
}

/**
 * @brief 文字列からゲームモードを取得
 */
inline GameMode GameModeFromString(const std::string& str) {
    if (str == "Menu" || str == "menu") return GameMode::Menu;
    if (str == "TD" || str == "td") return GameMode::TD;
    if (str == "Roguelike" || str == "roguelike") return GameMode::Roguelike;
    return GameMode::Menu; // デフォルト
}

} // namespace Application

