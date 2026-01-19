#pragma once

#include <string>
#include <utility>
#include <vector>

// プロジェクト�E
#include "BattleSetupData.hpp"
#include "GameState.hpp"

namespace game {
    namespace core {
        
        // 前方宣言
        class BaseSystemAPI;
        class InputSystemAPI;
        class ECSystemAPI;
        class UISystemAPI;
        class SceneOverlayControlAPI;
        class GameplayDataAPI;
        class DebugUIAPI;
        class AudioControlAPI;
        class SetupAPI;
        class BattleProgressAPI;
        class BattleSetupAPI;
        namespace entities {
        }
        
        /// @brief 編成データ構造佁E
        struct FormationData {
            std::vector<std::pair<int, std::string>> slots; // (slot_id, character_id) のペア
            bool IsEmpty() const { return slots.empty(); }
            void Clear() { slots.clear(); }
        };
        
        /// @brief モジュール間で共有するコンチE��スチE
        /// 
        /// GameSystemが所有し、すべてのモジュールに共有されます、E
        /// モジュールはこ�EコンチE��ストを通じてシスチE��APIにアクセスします、E
        struct SharedContext {
            BaseSystemAPI* systemAPI = nullptr;
            InputSystemAPI* inputAPI = nullptr;  // 入力統吁EPI
            ECSystemAPI* ecsAPI = nullptr;
            AudioControlAPI* audioAPI = nullptr;  // オーチE��オ制御API
            GameplayDataAPI* gameplayDataAPI = nullptr;  // ゲームプレイチE�EタAPI
            SetupAPI* setupAPI = nullptr;  // セチE��アチE�EAPI
            BattleSetupAPI* battleSetupAPI = nullptr;  // 戦闘セチE��アチE�EAPI
            UISystemAPI* uiAPI = nullptr;  // UI共通API
            SceneOverlayControlAPI* sceneOverlayAPI = nullptr;  // Scene/Overlay制御API
            BattleProgressAPI* battleProgressAPI = nullptr;  // 戦闘進行API
            DebugUIAPI* debugUIAPI = nullptr;  // チE��チE��UI API
            std::string currentStageId = "";  // 現在選択されてぁE��スチE�EジID
            FormationData formationData;  // 編成データ
            BattleSetupData battleSetupData;  // 戦闘セチE��アチE�EチE�Eタ
            GameState currentState = GameState::Initializing;
            float deltaTime = 0.0f;
            bool isPaused = false;
            bool requestShutdown = false;
        };
        
    } // namespace core
} // namespace game
