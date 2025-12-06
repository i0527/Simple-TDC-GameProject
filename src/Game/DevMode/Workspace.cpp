#include "Game/DevMode/Workspace.h"
#include <fstream>
#include <iostream>

namespace Game::DevMode {

void Workspace::SaveWindowState(const std::string& id, const ImVec2& pos, const ImVec2& size, bool visible) {
    for (auto& state : windowStates_) {
        if (state.id == id) {
            state.position = pos;
            state.size = size;
            state.visible = visible;
            return;
        }
    }
    windowStates_.push_back({id, pos, size, visible});
}

bool Workspace::LoadWindowState(const std::string& id, ImVec2& pos, ImVec2& size, bool& visible) {
    for (const auto& state : windowStates_) {
        if (state.id == id) {
            pos = state.position;
            size = state.size;
            visible = state.visible;
            return true;
        }
    }
    return false;
}

json Workspace::Serialize() const {
    json j;
    j["name"] = name_;
    j["windows"] = json::array();

    for (const auto& state : windowStates_) {
        json windowJson;
        windowJson["id"] = state.id;
        windowJson["position"] = {state.position.x, state.position.y};
        windowJson["size"] = {state.size.x, state.size.y};
        windowJson["visible"] = state.visible;
        j["windows"].push_back(windowJson);
    }

    return j;
}

bool Workspace::Deserialize(const json& data) {
    try {
        name_ = data["name"];
        windowStates_.clear();

        for (const auto& windowJson : data["windows"]) {
            WindowState state;
            state.id = windowJson["id"];
            state.position = ImVec2(windowJson["position"][0], windowJson["position"][1]);
            state.size = ImVec2(windowJson["size"][0], windowJson["size"][1]);
            state.visible = windowJson["visible"];
            windowStates_.push_back(state);
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to deserialize workspace: " << e.what() << "\n";
        return false;
    }
}

void Workspace::SaveToFile(const std::string& filepath) {
    try {
        json data = Serialize();
        std::ofstream file(filepath);
        file << data.dump(2);
    } catch (const std::exception& e) {
        std::cerr << "Failed to save workspace: " << e.what() << "\n";
    }
}

bool Workspace::LoadFromFile(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        json data;
        file >> data;
        return Deserialize(data);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load workspace: " << e.what() << "\n";
        return false;
    }
}

} // namespace Game::DevMode


