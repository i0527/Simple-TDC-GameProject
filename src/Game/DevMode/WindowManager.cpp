#include "Game/DevMode/WindowManager.h"

namespace Game::DevMode {

void WindowManager::AddWindow(const std::string& id, const ImVec2& pos, const ImVec2& size) {
    windows_.push_back({id, pos, size, true});
}

WindowLayout* WindowManager::GetWindow(const std::string& id) {
    for (auto& window : windows_) {
        if (window.windowId == id) {
            return &window;
        }
    }
    return nullptr;
}

void WindowManager::SetupDockSpace() {
}

} // namespace Game::DevMode


