#include "Game/DevMode/DebugTools.h"
#include <imgui.h>
#include <sstream>

namespace Game::DevMode {

void Inspector::Render(bool* isVisible) {
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Inspector", isVisible)) {
        if (selectedEntity_ != entt::null) {
            ImGui::Text("Entity ID: %u", static_cast<uint32_t>(selectedEntity_));
        } else {
            ImGui::TextDisabled("No entity selected");
        }
    }
    ImGui::End();
}

void Hierarchy::Initialize(Core::World& world) {
    world_ = &world;
}

void Hierarchy::Render(bool* isVisible) {
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Hierarchy", isVisible)) {
        ImGui::Text("Entity Hierarchy - TODO");
    }
    ImGui::End();
}

void Console::AddLog(const std::string& message) {
    logs_.push_back(message);
    if (logs_.size() > 1000) {
        logs_.erase(logs_.begin());
    }
}

void Console::Render(bool* isVisible) {
    ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Console", isVisible)) {
        ImGui::BeginChild("LogArea", ImVec2(0, -30), true);
        for (const auto& log : logs_) {
            ImGui::TextUnformatted(log.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

        if (ImGui::Button("Clear", ImVec2(-1, 0))) {
            Clear();
        }
    }
    ImGui::End();
}

void AssetBrowser::Initialize(const std::string& rootPath) {
    rootPath_ = rootPath;
}

void AssetBrowser::Render(bool* isVisible) {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Asset Browser", isVisible)) {
        ImGui::Text("Root: %s", rootPath_.c_str());
        ImGui::TextDisabled("Asset Browser - TODO");
    }
    ImGui::End();
}

} // namespace Game::DevMode


