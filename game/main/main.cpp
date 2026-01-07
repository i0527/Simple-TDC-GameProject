#include "core/system/GameSystem.hpp"
#include "utils/Log.h"

int main() {
  game::core::GameSystem system;

  // ゲームの初期化
  int initResult = system.Initialize();
  if (initResult != 0) {
    return initResult;
  }

  // メインループ実行（初期化シーン→タイトル画面）
  int runResult = system.Run();

  // ゲームのシャットダウン
  system.Shutdown();

  return runResult;
}
