#pragma once

// 標準ライブラリ
#include <string>

// プロジェクト内
#include "../config/BattleSetupData.hpp"
#include "../config/SharedContext.hpp"

namespace game {
namespace core {

class GameplayDataAPI;
class SetupAPI;

/// @brief 戦闘セットアップを統合するAPI
class BattleSetupAPI {
public:
    BattleSetupAPI();
    ~BattleSetupAPI() = default;

    bool Initialize(GameplayDataAPI* gameplayDataAPI,
                    SetupAPI* setupAPI,
                    SharedContext* sharedContext);

    BattleSetupData BuildBattleSetupData(const std::string& stageId,
                                         const FormationData& formation) const;

private:
    GameplayDataAPI* gameplayDataAPI_;
    SetupAPI* setupAPI_;
    SharedContext* sharedContext_;
    bool isInitialized_;
};

} // namespace core
} // namespace game
