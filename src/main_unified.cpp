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
    
    // HTTPサーバーを有効化（UIエディター用）
    // デフォルトでは無効化。有効化する場合は第2引数をtrueに
    bool enableHTTPServer = false;  // 必要に応じてtrueに変更
    int httpServerPort = 8080;
    
    if (!game.Initialize("assets/definitions", enableHTTPServer, httpServerPort)) {
        std::cerr << "Failed to initialize UnifiedGame\n";
        return 1;
    }
    
    // メニューモードで開始
    game.SetGameMode(Application::GameMode::Menu);
    
    // ゲームループ実行
    game.Run();
    
    return 0;
}

