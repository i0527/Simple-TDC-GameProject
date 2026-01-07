#include "GameModuleAPI.hpp"

namespace game {
    namespace core {

        GameModuleAPI::GameModuleAPI() {
            // レジストリはデフォルトコンストラクタで初期化される
        }

        entt::entity GameModuleAPI::Create() {
            return registry_.create();
        }

        void GameModuleAPI::Destroy(entt::entity e) {
            registry_.destroy(e);
        }

        bool GameModuleAPI::Valid(entt::entity e) const {
            return registry_.valid(e);
        }

        size_t GameModuleAPI::Count() const {
            return registry_.alive();
        }

        void GameModuleAPI::Clear() {
            registry_.clear();
        }

    } // namespace core
} // namespace game
