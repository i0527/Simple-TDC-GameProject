#pragma once

#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <vector>

#include <raylib.h>

#include "Data/Graphics/FrameProviderManager.h"
#include "Game/Components/NewCoreComponents.h"
#include "Shared/Simulation/Factories/CharacterFactory.h"

namespace Shared::Core {
class GameContext;
}

namespace Shared::Data {
class DefinitionRegistry;
}

namespace Shared::Simulation {

/**
 * @brief ゲーム/エディタ共有のシミュレーションコンテキスト
 */
class SimulationContext {
public:
    SimulationContext(Shared::Core::GameContext* context = nullptr,
                      Shared::Data::DefinitionRegistry* definitions = nullptr);
    ~SimulationContext(); // 実装ファイルで定義（CharacterFactory の完全な型定義が必要）

    // エンティティ管理
    entt::entity SpawnEntity(const std::string& definitionId,
                             const Vector2& position,
                             Game::Components::Team::Type team = Game::Components::Team::Type::Player);
    void DestroyEntity(entt::entity entity);
    std::vector<entt::entity> FindEntitiesByDefinition(const std::string& definitionId) const;

    // 更新
    void Update(float deltaTime);

    // 状態取得
    entt::registry& GetRegistry() { return registry_; }
    const entt::registry& GetRegistry() const { return registry_; }
    size_t GetEntityCount() const;

    // FrameProviderManagerアクセス
    Shared::Data::Graphics::FrameProviderManager& GetFrameProviderManager() { return frame_provider_manager_; }
    const Shared::Data::Graphics::FrameProviderManager& GetFrameProviderManager() const { return frame_provider_manager_; }

    // 便利API
    void SetEntityPosition(entt::entity entity, const Vector2& position);
    Vector2 GetEntityPosition(entt::entity entity) const;

    struct RenderCommand {
        Texture2D texture{};
        Rectangle source_rect{};
        Rectangle dest_rect{};
        Vector2 origin{};
        float rotation = 0.0f;
        Color tint = RAYWHITE;
    };

    std::vector<RenderCommand> GetRenderCommands(const Rectangle& /*camera_bounds*/) const;

    // リセット
    void Clear();

    enum class ReloadPolicy { PreserveState, ResetToDefault, ScaleProportionally };
    bool ReloadEntity(entt::entity entity, ReloadPolicy policy = ReloadPolicy::PreserveState);
    size_t ReloadAllInstances(const std::string& definitionId, ReloadPolicy policy = ReloadPolicy::PreserveState);

    // 依存設定（必要に応じて後から設定）
    void SetContext(Shared::Core::GameContext* context) { context_ = context; character_factory_.reset(); }
    void SetDefinitions(Shared::Data::DefinitionRegistry* definitions) { definitions_ = definitions; character_factory_.reset(); }

private:
    void EnsureFactory();

    entt::registry registry_;

    Shared::Core::GameContext* context_ = nullptr;
    Shared::Data::DefinitionRegistry* definitions_ = nullptr;
    std::unique_ptr<CharacterFactory> character_factory_;
    Shared::Data::Graphics::FrameProviderManager frame_provider_manager_;
};

} // namespace Shared::Simulation

