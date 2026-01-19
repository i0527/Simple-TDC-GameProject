#include "../ECSystemAPI.hpp"

namespace game {
namespace core {

ECSystemAPI::ECSystemAPI() {
    // registry_ はデフォルトコンストラクタで初期化される
}

entt::entity ECSystemAPI::Create() {
    return registry_.create();
}

void ECSystemAPI::Destroy(entt::entity entity) {
    registry_.destroy(entity);
}

bool ECSystemAPI::Valid(entt::entity entity) const {
    return registry_.valid(entity);
}

size_t ECSystemAPI::Count() const {
    return registry_.alive();
}

void ECSystemAPI::Clear() {
    registry_.clear();
}

} // namespace core
} // namespace game
