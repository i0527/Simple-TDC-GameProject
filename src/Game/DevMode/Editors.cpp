#include "Game/DevMode/Editors.h"
#include <imgui.h>
#include <string>

namespace Game::DevMode {

void StageEditor::Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader) {
    registry_ = registry;
    loader_ = loader;
}

void StageEditor::Render(bool* isVisible) {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Stage Editor", isVisible)) {
        ImGui::Text("Stage Editor - TODO");
    }
    ImGui::End();
}

void SkillEditor::Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader) {
    registry_ = registry;
    loader_ = loader;
}

void SkillEditor::Render(bool* isVisible) {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Skill Editor", isVisible)) {
        ImGui::Text("Skill Editor - TODO");
    }
    ImGui::End();
}

void EffectEditor::Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader) {
    registry_ = registry;
    loader_ = loader;
}

void EffectEditor::Render(bool* isVisible) {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Effect Editor", isVisible)) {
        ImGui::Text("Effect Editor - TODO");
    }
    ImGui::End();
}

void SoundEditor::Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader) {
    registry_ = registry;
    loader_ = loader;
}

void SoundEditor::Render(bool* isVisible) {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Sound Editor", isVisible)) {
        ImGui::Text("Sound Editor - TODO");
    }
    ImGui::End();
}

void UIEditor::Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader) {
    registry_ = registry;
    loader_ = loader;
}

void UIEditor::Render(bool* isVisible) {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("UI Editor", isVisible)) {
        ImGui::Text("UI Editor - TODO");
    }
    ImGui::End();
}

} // namespace Game::DevMode


