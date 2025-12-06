#include "Game/DevMode/DevModeManager.h"
#include <iostream>

namespace Game::DevMode {

DevModeManager::DevModeManager() 
    : workspace_(std::make_unique<Workspace>()),
      hotReload_(std::make_unique<Core::HotReloadSystem>())
{
}

DevModeManager::~DevModeManager() = default;

bool DevModeManager::Initialize(
    Core::GameContext& context,
    Data::DefinitionRegistry* registry,
    Data::DefinitionLoader* loader
) {
    if (!registry || !loader) {
        std::cerr << "DevModeManager: Invalid registry or loader\n";
        return false;
    }

    context_ = &context;
    registry_ = registry;
    loader_ = loader;

    gameViewRenderer_.Initialize(1920, 1080);
    workspace_->SetName("default");
    
    hotReload_->Initialize("assets/definitions", 500.0f);
    hotReload_->RegisterCallback("*.json", [this](const std::string& filepath) {
        std::cout << "HotReload: File changed: " << filepath << "\n";
    });

    initialized_ = true;

    std::cout << "DevModeManager initialized successfully\n";
    return true;
}

void DevModeManager::Update(float deltaTime) {
    if (!initialized_) return;

    if (IsKeyPressed(KEY_F1)) {
        ToggleDevMode();
    }
}

void DevModeManager::Render() {
    if (!initialized_ || !active_) return;

    rlImGuiBegin();

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_FirstUseEver);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Workspace", "Ctrl+S")) {
                SaveWorkspace("default");
            }
            if (ImGui::MenuItem("Load Workspace", "Ctrl+L")) {
                LoadWorkspace("default");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Close Editor", "F1")) {
                Hide();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows")) {
            ImGui::MenuItem("Characters", nullptr, &showCharacterEditor_);
            ImGui::MenuItem("Stages", nullptr, &showStageEditor_);
            ImGui::Separator();
            ImGui::MenuItem("Inspector", nullptr, &showInspector_);
            ImGui::MenuItem("Console", nullptr, &showConsole_);
            ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy_);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (showCharacterEditor_) RenderCharacterEditor();
    if (showStageEditor_) RenderStageEditor();
    if (showInspector_) RenderInspector();
    if (showConsole_) RenderConsole();
    if (showHierarchy_) RenderHierarchy();

    rlImGuiEnd();
}

void DevModeManager::ToggleDevMode() {
    active_ = !active_;
}

void DevModeManager::SaveWorkspace(const std::string& name) {
    if (workspace_) {
        workspace_->SetName(name);
        workspace_->SaveToFile("workspaces/" + name + ".json");
        std::cout << "Workspace saved: " << name << "\n";
    }
}

void DevModeManager::LoadWorkspace(const std::string& name) {
    if (workspace_) {
        if (workspace_->LoadFromFile("workspaces/" + name + ".json")) {
            std::cout << "Workspace loaded: " << name << "\n";
        } else {
            std::cout << "Failed to load workspace: " << name << "\n";
        }
    }
}

void DevModeManager::RenderMenuBar() {
}

void DevModeManager::RenderCharacterEditor() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Character Editor", &showCharacterEditor_)) {
        ImGui::Text("Character Editor - TODO");
    }
    ImGui::End();
}

void DevModeManager::RenderStageEditor() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Stage Editor", &showStageEditor_)) {
        ImGui::Text("Stage Editor - TODO");
    }
    ImGui::End();
}

void DevModeManager::RenderInspector() {
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Inspector", &showInspector_)) {
        ImGui::Text("Inspector - TODO");
    }
    ImGui::End();
}

void DevModeManager::RenderConsole() {
    ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Console", &showConsole_)) {
        ImGui::BeginChild("LogArea", ImVec2(0, -30), true);
        for (const auto& log : consoleLogs_) {
            ImGui::TextUnformatted(log.c_str());
        }
        ImGui::EndChild();

        if (ImGui::Button("Clear")) {
            consoleLogs_.clear();
        }
    }
    ImGui::End();
}

void DevModeManager::RenderHierarchy() {
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Hierarchy", &showHierarchy_)) {
        ImGui::Text("Hierarchy - TODO");
    }
    ImGui::End();
}

} // namespace Game::DevMode

