#pragma once

#include "Data/Registry.h"
#include "Data/Loaders/DefinitionLoader.h"

namespace Game::DevMode {

class StageEditor {
public:
    void Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader);
    void Render(bool* isVisible);

private:
    Data::DefinitionRegistry* registry_ = nullptr;
    Data::DefinitionLoader* loader_ = nullptr;
    std::string selectedStageId_;
};

class SkillEditor {
public:
    void Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader);
    void Render(bool* isVisible);

private:
    Data::DefinitionRegistry* registry_ = nullptr;
    Data::DefinitionLoader* loader_ = nullptr;
};

class EffectEditor {
public:
    void Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader);
    void Render(bool* isVisible);

private:
    Data::DefinitionRegistry* registry_ = nullptr;
    Data::DefinitionLoader* loader_ = nullptr;
};

class SoundEditor {
public:
    void Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader);
    void Render(bool* isVisible);

private:
    Data::DefinitionRegistry* registry_ = nullptr;
    Data::DefinitionLoader* loader_ = nullptr;
};

class UIEditor {
public:
    void Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader);
    void Render(bool* isVisible);

private:
    Data::DefinitionRegistry* registry_ = nullptr;
    Data::DefinitionLoader* loader_ = nullptr;
};

} // namespace Game::DevMode


