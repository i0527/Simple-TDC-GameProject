#pragma once

#include <memory>
#include <string>
#include <vector>
#include <imgui.h>
#include "Core/GameContext.h"
#include "Core/HotReloadSystem.h"
#include "Data/Registry.h"
#include "Data/Loaders/DefinitionLoader.h"
#include "Game/DevMode/GameViewRenderer.h"
#include "Game/DevMode/WindowManager.h"
#include "Game/DevMode/Workspace.h"

namespace Game::DevMode {

class DevModeManager {
public:
    DevModeManager();
    ~DevModeManager();

    bool Initialize(
        Core::GameContext& context,
        Data::DefinitionRegistry* registry,
        Data::DefinitionLoader* loader
    );

    void Update(float deltaTime);
    void Render();

    void ToggleDevMode();
    bool IsDevModeActive() const { return active_; }

    void Show() { active_ = true; }
    void Hide() { active_ = false; }
    
    void SaveWorkspace(const std::string& name);
    void LoadWorkspace(const std::string& name);

private:
    bool active_ = false;
    bool initialized_ = false;

    Core::GameContext* context_ = nullptr;
    Data::DefinitionRegistry* registry_ = nullptr;
    Data::DefinitionLoader* loader_ = nullptr;

    GameViewRenderer gameViewRenderer_;
    WindowManager windowManager_;
    std::unique_ptr<Workspace> workspace_;
    std::unique_ptr<Core::HotReloadSystem> hotReload_;

    bool showCharacterEditor_ = false;
    bool showStageEditor_ = false;
    bool showInspector_ = true;
    bool showConsole_ = true;
    bool showHierarchy_ = true;

    std::vector<std::string> consoleLogs_;

    void RenderMenuBar();
    void RenderCharacterEditor();
    void RenderStageEditor();
    void RenderInspector();
    void RenderConsole();
    void RenderHierarchy();
};

} // namespace Game::DevMode

