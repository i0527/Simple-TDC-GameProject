#pragma once

namespace game {
    namespace core {
        
        // 前方宣言
        class BaseSystemAPI;
        class GameModuleAPI;
        class OverlayManager;
        namespace entities {
            class CharacterManager;
        }
        
        /// @brief モジュール間で共有するコンテキスト
        /// 
        /// GameSystemが所有し、すべてのモジュールに共有されます。
        /// モジュールはこのコンテキストを通じてシステムAPIにアクセスします。
        struct SharedContext {
            BaseSystemAPI* systemAPI = nullptr;
            GameModuleAPI* gameAPI = nullptr;
            entities::CharacterManager* characterManager = nullptr;  // キャラクターマネージャー
            OverlayManager* overlayManager = nullptr;  // オーバーレイマネージャー
            float deltaTime = 0.0f;
            bool isPaused = false;
            bool requestShutdown = false;
        };
        
    } // namespace core
} // namespace game
