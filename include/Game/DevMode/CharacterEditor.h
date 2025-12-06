#pragma once

#include <memory>
#include <string>
#include <optional>
#include "Core/GameContext.h"
#include "Data/Registry.h"
#include "Data/Loaders/DefinitionLoader.h"

namespace Game::DevMode {

class CharacterEditor {
public:
    CharacterEditor() = default;
    ~CharacterEditor() = default;

    void Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader);
    void Render(bool* isVisible);

private:
    Data::DefinitionRegistry* registry_ = nullptr;
    Data::DefinitionLoader* loader_ = nullptr;
    std::string selectedCharacterId_;
    
    void RenderEntityList();
    void RenderEntityDetails();
};

} // namespace Game::DevMode


