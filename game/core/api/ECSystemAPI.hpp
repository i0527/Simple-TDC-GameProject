#pragma once

// 標準ライブラリ
#include <cassert>
#include <optional>
#include <utility>
#include <vector>

// 外部ライブラリ
#include <entt/entt.hpp>

// プロジェクト内
#include "../../utils/Log.h"
#include "../config/RenderTypes.hpp"
#include "../ecs/defineComponents.hpp"
#include "../ecs/entities/Character.hpp"
#include "../ecs/entities/EntityCreationData.hpp"

namespace game {
namespace core {

/// @brief 生成時の上書き値
struct SpawnOverrides {
    std::optional<int> maxHp;
    std::optional<int> attack;
    std::optional<int> defense;
    std::optional<float> moveSpeed;
    std::optional<Vector2> attackSize;
    std::optional<float> attackSpan;
};

/// @brief EnTTベースのECS統合API
///
/// GameModuleAPIの機能に加え、ECS生成ヘルパーを統合します。
class ECSystemAPI {
public:
    ECSystemAPI();
    ~ECSystemAPI() = default;

    // レジストリアクセス（必要な場合）
    entt::registry& Registry() { return registry_; }
    const entt::registry& Registry() const { return registry_; }

    // ========== エンティティ操作 ==========
    entt::entity Create();
    void Destroy(entt::entity entity);
    bool Valid(entt::entity entity) const;
    size_t Count() const;
    void Clear();

    // ========== コンポーネント操作 ==========
    template<typename T, typename... Args>
    T& Add(entt::entity entity, Args&&... args);

    template<typename T>
    T& Get(entt::entity entity);

    template<typename T>
    const T& Get(entt::entity entity) const;

    template<typename T>
    T* Try(entt::entity entity);

    template<typename T>
    const T* Try(entt::entity entity) const;

    template<typename T>
    void Remove(entt::entity entity);

    template<typename T>
    bool Has(entt::entity entity) const;

    template<typename... T>
    bool HasAll(entt::entity entity) const;

    // ========== ビュー作成 ==========
    template<typename... T>
    auto View();

    template<typename... T, typename... Exclude>
    auto View(entt::exclude_t<Exclude...>);

    // ========== コンテキスト変数 ==========
    template<typename T, typename... Args>
    T& Ctx(Args&&... args);

    template<typename T>
    T& Ctx();

    template<typename T>
    const T& Ctx() const;

    template<typename T>
    bool HasCtx() const;

    // ========== 生成ヘルパー ==========
    entt::entity CreateEntityFromCharacter(
        const entities::Character& character,
        const entities::EntityCreationData& creationData);

    entt::entity CreateBattleEntityFromCharacter(
        const entities::Character& character,
        const entities::EntityCreationData& creationData,
        ecs::components::Faction faction,
        const SpawnOverrides* overrides = nullptr);

    void QueueDestroy(entt::entity entity);
    void FlushDestroyQueue();
    size_t DestroyDeadEntities();
    void ResetForScene();

private:
    entt::registry registry_;
    std::vector<entt::entity> pendingDestroy_;
};

// ========== テンプレート実装 ==========

template<typename T, typename... Args>
inline T& ECSystemAPI::Add(entt::entity entity, Args&&... args) {
    return registry_.emplace<T>(entity, std::forward<Args>(args)...);
}

template<typename T>
inline T& ECSystemAPI::Get(entt::entity entity) {
    return registry_.get<T>(entity);
}

template<typename T>
inline const T& ECSystemAPI::Get(entt::entity entity) const {
    return registry_.get<T>(entity);
}

template<typename T>
inline T* ECSystemAPI::Try(entt::entity entity) {
    return registry_.try_get<T>(entity);
}

template<typename T>
inline const T* ECSystemAPI::Try(entt::entity entity) const {
    return registry_.try_get<T>(entity);
}

template<typename T>
inline void ECSystemAPI::Remove(entt::entity entity) {
    registry_.remove<T>(entity);
}

template<typename T>
inline bool ECSystemAPI::Has(entt::entity entity) const {
    return registry_.all_of<T>(entity);
}

template<typename... T>
inline bool ECSystemAPI::HasAll(entt::entity entity) const {
    return registry_.all_of<T...>(entity);
}

template<typename... T>
inline auto ECSystemAPI::View() {
    return registry_.view<T...>();
}

template<typename... T, typename... Exclude>
inline auto ECSystemAPI::View(entt::exclude_t<Exclude...>) {
    return registry_.view<T...>(entt::exclude<Exclude...>);
}

template<typename T, typename... Args>
inline T& ECSystemAPI::Ctx(Args&&... args) {
    if (registry_.ctx().contains<T>()) {
        return registry_.ctx().get<T>();
    }
    return registry_.ctx().emplace<T>(std::forward<Args>(args)...);
}

template<typename T>
inline T& ECSystemAPI::Ctx() {
    return registry_.ctx().get<T>();
}

template<typename T>
inline const T& ECSystemAPI::Ctx() const {
    return registry_.ctx().get<T>();
}

template<typename T>
inline bool ECSystemAPI::HasCtx() const {
    return registry_.ctx().contains<T>();
}

} // namespace core
} // namespace game
