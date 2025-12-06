#pragma once

#include <string>
#include <vector>
#include <imgui.h>

namespace Game::DevMode {

struct WindowLayout {
    std::string windowId;
    ImVec2 position;
    ImVec2 size;
    bool isVisible = true;
};

class WindowManager {
public:
    WindowManager() = default;
    ~WindowManager() = default;

    void AddWindow(const std::string& id, const ImVec2& pos, const ImVec2& size);
    WindowLayout* GetWindow(const std::string& id);

    void SetupDockSpace();

    std::vector<WindowLayout>& GetWindows() { return windows_; }

private:
    std::vector<WindowLayout> windows_;
};

} // namespace Game::DevMode


