/**
 * @file main_unified.cpp
 * @brief 統合ゲームエントリポイント
 * 
 * TDとRoguelikeを統合した単一エントリポイント。
 * UnifiedGameを使用してゲームモードを切り替えながら実行する。
 */

#include "Core/Platform.h"
#include "Application/UnifiedGame.h"
#include "Application/GameMode.h"

int main() {
    // 統合ゲームの初期化
    Application::UnifiedGame game;
    
    if (!game.Initialize("assets/definitions")) {
        std::cerr << "Failed to initialize UnifiedGame\n";
        return 1;
    }
    
    // メニューモードで開始
    game.SetGameMode(Application::GameMode::Menu);
    
    // ゲームループ実行
    game.Run();
    
    return 0;
}

