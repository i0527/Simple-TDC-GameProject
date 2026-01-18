#pragma once

#include <string>
#include <vector>
#include <utility>

namespace game {
    namespace core {
        
        // 前方宣言
        class BaseSystemAPI;
        class GameModuleAPI;
        class OverlayManager;
        class PlayerDataManager;
        namespace entities {
            class CharacterManager;
            class ItemPassiveManager;
            class StageManager;
        }
        
        /// @brief 編成データ構造体
        struct FormationData {
            std::vector<std::pair<int, std::string>> slots; // (slot_id, character_id) のペア
            bool IsEmpty() const { return slots.empty(); }
            void Clear() { slots.clear(); }
        };
        
        /// @brief モジュール間で共有するコンテキスト
        /// 
        /// GameSystemが所有し、すべてのモジュールに共有されます。
        /// モジュールはこのコンテキストを通じてシステムAPIにアクセスします。
        struct SharedContext {
            BaseSystemAPI* systemAPI = nullptr;
            GameModuleAPI* gameAPI = nullptr;
            PlayerDataManager* playerDataManager = nullptr;  // プレイヤーデータ（単一JSON）
            entities::CharacterManager* characterManager = nullptr;  // キャラクターマネージャー
            entities::ItemPassiveManager* itemPassiveManager = nullptr; // 装備・パッシブマネージャー
            entities::StageManager* stageManager = nullptr;  // ステージマネージャー
            OverlayManager* overlayManager = nullptr;  // オーバーレイマネージャー
            std::string currentStageId = "";  // 現在選択されているステージID
            FormationData formationData;  // 編成データ
            float deltaTime = 0.0f;
            bool isPaused = false;
            bool requestShutdown = false;
        };
        
    } // namespace core
} // namespace game
