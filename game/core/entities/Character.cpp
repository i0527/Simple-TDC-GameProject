#include "Character.hpp"

namespace game {
namespace core {
namespace entities {

int Character::GetTotalAttack() const {
    int total = attack;
    for (const auto& eq : equipment) {
        total += static_cast<int>(eq.attack_bonus);
    }
    return total;
}

int Character::GetTotalHP() const {
    int total = hp;
    for (const auto& eq : equipment) {
        total += static_cast<int>(eq.hp_bonus);
    }
    return total;
}

int Character::GetTotalDefense() const {
    int total = defense;
    for (const auto& eq : equipment) {
        total += static_cast<int>(eq.defense_bonus);
    }
    return total;
}

} // namespace entities
} // namespace core
} // namespace game
