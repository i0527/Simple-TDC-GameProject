#pragma once

#include <string>
#include <vector>
#include <imgui.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Game::DevMode {

struct WindowState {
    std::string id;
    ImVec2 position;
    ImVec2 size;
    bool visible = true;
};

class Workspace {
public:
    Workspace() = default;
    ~Workspace() = default;

    void SetName(const std::string& name) { name_ = name; }
    std::string GetName() const { return name_; }

    void SaveWindowState(const std::string& id, const ImVec2& pos, const ImVec2& size, bool visible);
    bool LoadWindowState(const std::string& id, ImVec2& pos, ImVec2& size, bool& visible);

    void SaveToFile(const std::string& filepath);
    bool LoadFromFile(const std::string& filepath);

private:
    std::string name_;
    std::vector<WindowState> windowStates_;

    json Serialize() const;
    bool Deserialize(const json& data);
};

} // namespace Game::DevMode


