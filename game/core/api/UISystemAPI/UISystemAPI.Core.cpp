#include "../UISystemAPI.hpp"

namespace game {
namespace core {

UISystemAPI::UISystemAPI()
    : systemAPI_(nullptr),
      isInitialized_(false) {}

UISystemAPI::~UISystemAPI() {
    Shutdown();
}

bool UISystemAPI::Initialize(BaseSystemAPI* systemAPI) {
    if (isInitialized_) {
        return true;
    }
    systemAPI_ = systemAPI;
    isInitialized_ = (systemAPI_ != nullptr);
    return isInitialized_;
}

void UISystemAPI::Shutdown() {
    systemAPI_ = nullptr;
    isInitialized_ = false;
}

bool UISystemAPI::IsInitialized() const {
    return isInitialized_;
}

} // namespace core
} // namespace game
