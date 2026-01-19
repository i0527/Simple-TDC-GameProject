#pragma once

// 標準ライブラリ
#include <vector>
#include <memory>

// 外部ライブラリ
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

// プロジェクト内
#include "../ecs/defineComponents.hpp"
#include "../game/WaveLoader.hpp"

namespace game {
namespace core {

class BaseSystemAPI;
class GameplayDataAPI;
class ECSystemAPI;
struct SpawnOverrides;
struct SharedContext;

namespace entities {
    class Character;
    struct EntityCreationData;
}

/// @brief セットアップを統合するAPI
///
/// 責務:
/// - 非戦闘ロード（GameplayDataAPIの初期化/SharedContext反映）
/// - Wave/ステージロード（WaveLoader）
/// - ECS生成（ECSystemAPI::CreateEntityFromCharacter）
class SetupAPI {
public:
    SetupAPI();
    ~SetupAPI();

    /// @brief API初期化（依存注入 + 非戦闘ロード）
    bool Initialize(BaseSystemAPI* systemAPI,
                    GameplayDataAPI* gameplayDataAPI,
                    ECSystemAPI* ecsAPI,
                    SharedContext* sharedContext);

    // ========== Wave/ステージロード ==========
    std::vector<::game::core::game::SpawnEvent> LoadStageSpawnEvents(const nlohmann::json& stageData);

    // ========== ECS生成 ==========
    entt::entity CreateBattleEntityFromCharacter(
        const entities::Character& character,
        const entities::EntityCreationData& creationData,
        ecs::components::Faction faction,
        const SpawnOverrides* overrides = nullptr);

private:
    BaseSystemAPI* systemAPI_;
    GameplayDataAPI* gameplayDataAPI_;
    ECSystemAPI* ecsAPI_;
    SharedContext* sharedContext_;

    std::unique_ptr<::game::core::game::WaveLoader> waveLoader_;
    bool isInitialized_;
};

} // namespace core
} // namespace game
