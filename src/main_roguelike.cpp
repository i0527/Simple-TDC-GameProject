/**
 * @file main_roguelike.cpp
 * @brief NetHack風ローグライクゲームのエントリポイント
 * 
 * Phase 1: 基本移動とターン制の動作確認
 * 
 * 動作確認チェックリスト:
 * □ ゲーム起動時に固定マップが表示される
 * □ プレイヤー（@）が画面中央に表示される
 * □ 方向キーでプレイヤーが移動する
 * □ 壁にぶつかると移動できない
 * □ スペースキーで待機できる
 * □ ターン数がカウントアップされる（UI表示）
 */

#include "Roguelike/RoguelikeGame.h"

int main() {
    Roguelike::RoguelikeGame game;
    
    if (!game.Initialize()) {
        return 1;
    }
    
    game.Run();
    
    return 0;
}

