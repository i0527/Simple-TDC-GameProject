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
#include <thread>
#include <chrono>
#include <cstdlib>

/**
 * @brief ブラウザでURLを開く（クロスプラットフォーム）
 * system() コマンドでシンプルに実装（マクロ競合を避ける）
 */
void OpenBrowserUrl(const std::string& url) {
#ifdef _WIN32
    // Windows: start コマンド使用
    std::string command = "start " + url;
    system(command.c_str());
#elif defined(__APPLE__)
    // macOS: open コマンド使用
    std::string command = "open " + url;
    system(command.c_str());
#else
    // Linux: xdg-open コマンド使用
    std::string command = "xdg-open " + url;
    system(command.c_str());
#endif
}

int main() {
    // 統合ゲームの初期化
    Application::UnifiedGame game;
    
    // 開発者モード: HTTPサーバーを自動有効化
    // DEVELOPER_MODE=1 でHTTPサーバーと WebUI を自動起動
    bool enableHTTPServer = true;  // ✅ デフォルトで開発者モード有効
    int httpServerPort = 8080;
    
    if (!game.Initialize("assets/definitions", enableHTTPServer, httpServerPort)) {
        std::cerr << "Failed to initialize UnifiedGame\n";
        return 1;
    }
    
    // HTTPサーバーが起動してから、WebUIをブラウザで開く
    if (enableHTTPServer) {
        // サーバー起動待機（200ms）
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // WebUI を自動起動
        std::string webUIUrl = "http://localhost:3000";
        std::cout << "\n=== Opening WebUI Editor ===\n";
        std::cout << "URL: " << webUIUrl << "\n";
        std::cout << "API: http://localhost:" << httpServerPort << "/api\n";
        std::cout << "Launching browser...\n\n";
        
        OpenBrowserUrl(webUIUrl);
    }
    
    // メニューモードで開始
    game.SetGameMode(Application::GameMode::Menu);
    
    // ゲームループ実行
    game.Run();
    
    return 0;
}

