#pragma once

#include <vector>
#include <string>
#include "Core/World.h"

namespace Game::DevMode {

class Inspector {
public:
    void Render(bool* isVisible);
    void SetSelectedEntity(entt::entity entity) { selectedEntity_ = entity; }

private:
    entt::entity selectedEntity_ = entt::null;
};

class Hierarchy {
public:
    void Initialize(Core::World& world);
    void Render(bool* isVisible);
    void SetSelectedEntity(entt::entity entity) { selectedEntity_ = entity; }

private:
    Core::World* world_ = nullptr;
    entt::entity selectedEntity_ = entt::null;
};

class Console {
public:
    void AddLog(const std::string& message);
    void Render(bool* isVisible);
    void Clear() { logs_.clear(); }

private:
    std::vector<std::string> logs_;
};

class AssetBrowser {
public:
    void Initialize(const std::string& rootPath);
    void Render(bool* isVisible);

private:
    std::string rootPath_;
};

} // namespace Game::DevMode


