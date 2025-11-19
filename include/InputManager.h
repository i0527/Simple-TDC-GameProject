#pragma once

#include <string>
#include <unordered_map>
#include <raylib.h>

namespace Core {
    // 入力管理クラス（Singleton）
    class InputManager {
    public:
        static InputManager& GetInstance();
        
        // インスタンスのコピーを禁止
        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;
        
        // 毎フレーム呼び出し - 入力状態を更新
        void Update();
        
        // キーが現在押されているか
        bool IsKeyPressed(int keyCode);
        
        // キーが押された瞬間か（1フレームのみ true）
        bool IsKeyDown(int keyCode);
        
        // キーが離された瞬間か（1フレームのみ true）
        bool IsKeyUp(int keyCode);
        
        // マウスボタンが押されているか
        bool IsMouseButtonPressed(int button);
        
        // マウスボタンが押された瞬間か
        bool IsMouseButtonDown(int button);
        
        // マウスボタンが離された瞬間か
        bool IsMouseButtonUp(int button);
        
        // マウス位置を取得
        float GetMouseX() const;
        float GetMouseY() const;
        
        // マウスホイール差分を取得
        float GetMouseWheelMove() const;
        
        // キーバインディング登録
        void RegisterKeyBinding(const std::string& action, int keyCode);
        
        // アクションが実行されたか確認
        bool IsActionPressed(const std::string& action);
        
    private:
        InputManager() = default;
        ~InputManager() = default;
        
        std::unordered_map<std::string, int> keyBindings_;  // アクション → キーコード
    };
}
