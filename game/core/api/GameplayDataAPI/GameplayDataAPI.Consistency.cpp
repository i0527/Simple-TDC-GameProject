#include "../GameplayDataAPI.hpp"

namespace game {
namespace core {

bool GameplayDataAPI::ValidateFormation(const FormationData& formation,
                                        std::vector<std::string>* invalidCharacterIds) const {
    if (!characterManager_) {
        return false;
    }

    bool ok = true;
    for (const auto& slot : formation.slots) {
        const std::string& id = slot.second;
        if (id.empty()) {
            continue;
        }
        if (!characterManager_->HasCharacter(id)) {
            ok = false;
            if (invalidCharacterIds) {
                invalidCharacterIds->push_back(id);
            }
        }
    }
    return ok;
}

} // namespace core
} // namespace game
