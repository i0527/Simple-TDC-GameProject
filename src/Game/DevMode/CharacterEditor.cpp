#include "Game/DevMode/CharacterEditor.h"
#include <imgui.h>

namespace Game::DevMode {

void CharacterEditor::Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader) {
    registry_ = registry;
    loader_ = loader;
}

void CharacterEditor::Render(bool* isVisible) {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Character Editor", isVisible)) {
        if (ImGui::BeginChild("CharacterList", ImVec2(200, 0), true)) {
            ImGui::Text("Characters");
            ImGui::Separator();
            
            if (registry_) {
                auto characterIds = registry_->GetAllCharacterIds();
                for (const auto& id : characterIds) {
                    if (ImGui::Selectable(id.c_str(), selectedCharacterId_ == id)) {
                        selectedCharacterId_ = id;
                    }
                }
            }
        }
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        if (ImGui::BeginChild("CharacterDetails", ImVec2(0, 0), true)) {
            if (!selectedCharacterId_.empty() && registry_) {
                auto* character = registry_->TryGetCharacter(selectedCharacterId_);
                if (character) {
                    ImGui::Text("ID: %s", selectedCharacterId_.c_str());
                    ImGui::Text("Name: %s", character->name.c_str());
                    ImGui::Separator();
                    ImGui::Text("Stats - TODO: Edit functionality");
                }
            } else {
                ImGui::TextDisabled("Select a character to edit");
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

} // namespace Game::DevMode


