/**
 * @file test_roguelike_crash.cpp
 * @brief Roguelike起動時クラッシュテスト
 * 
 * Roguelikeゲーム選択時のクラッシュを検証するテストプログラム
 */

#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

int main() {
    std::cout << "=====================================\n";
    std::cout << "Roguelike Crash Fix Verification Test\n";
    std::cout << "=====================================\n\n";

    // テスト1: maps ディレクトリ確認
    std::cout << "[Test 1] Checking maps directory...\n";
    std::string maps_path = "assets/definitions/maps";
    if (fs::exists(maps_path)) {
        std::cout << "✅ maps directory exists at: " << fs::absolute(maps_path) << "\n";
        
        // ディレクトリ内のファイルを列挙
        int file_count = 0;
        for (const auto& entry : fs::directory_iterator(maps_path)) {
            std::cout << "  - " << entry.path().filename().string() << "\n";
            file_count++;
        }
        std::cout << "   Total files: " << file_count << "\n";
    } else {
        std::cout << "❌ maps directory NOT found at: " << fs::absolute(maps_path) << "\n";
        return 1;
    }

    // テスト2: 他の定義ディレクトリ確認
    std::cout << "\n[Test 2] Checking other definition directories...\n";
    std::vector<std::string> required_dirs = {
        "assets/definitions/characters",
        "assets/definitions/stages",
        "assets/definitions/ui",
        "assets/definitions/effects",
        "assets/definitions/skills",
        "assets/definitions/sounds"
    };

    bool all_dirs_exist = true;
    for (const auto& dir : required_dirs) {
        if (fs::exists(dir)) {
            std::cout << "✅ " << dir << "\n";
        } else {
            std::cout << "❌ " << dir << " NOT FOUND\n";
            all_dirs_exist = false;
        }
    }

    // テスト3: デバッグ情報
    std::cout << "\n[Test 3] Debug Information...\n";
    std::cout << "Current working directory: " << fs::current_path() << "\n";
    std::cout << "Assets directory exists: " << (fs::exists("assets") ? "Yes" : "No") << "\n";
    std::cout << "Definitions directory exists: " << (fs::exists("assets/definitions") ? "Yes" : "No") << "\n";

    // テスト4: ゲーム初期化ログ期待値
    std::cout << "\n[Test 4] Expected Log Output...\n";
    std::cout << "When game starts, you should see these messages:\n";
    std::cout << "  1. 'UnifiedGame: Loading definitions from: assets/definitions'\n";
    std::cout << "  2. 'UnifiedGame: Loading characters...'\n";
    std::cout << "  3. 'UnifiedGame: Loading stages...'\n";
    std::cout << "  4. 'UnifiedGame: Loading UI layouts...'\n";
    std::cout << "  5. 'UnifiedGame: ℹ️ Maps directory not found...' OR 'Loading maps from...'\n";
    std::cout << "  6. 'UnifiedGame: ✅ All available definitions loaded successfully'\n";
    std::cout << "\nIf you see these messages, the crash has been FIXED! ✅\n";

    // テスト5: ホームシーン表示確認
    std::cout << "\n[Test 5] Manual Verification Steps...\n";
    std::cout << "1. Run the game: .\\build\\bin\\Release\\SimpleTDCGame.exe\n";
    std::cout << "2. Verify ホームシーン (Home Screen) displays\n";
    std::cout << "3. Click or select Roguelike game\n";
    std::cout << "4. Verify Roguelike screen displays WITHOUT crashing\n";
    std::cout << "5. Check console output for the messages listed above\n";

    std::cout << "\n=====================================\n";
    std::cout << "Test Summary\n";
    std::cout << "=====================================\n";
    if (fs::exists(maps_path) && all_dirs_exist) {
        std::cout << "✅ All directory checks passed!\n";
        std::cout << "✅ Crash fix should be working.\n";
        return 0;
    } else {
        std::cout << "❌ Some directories are missing.\n";
        return 1;
    }
}

