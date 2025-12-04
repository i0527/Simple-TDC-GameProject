/**
 * @file EditorManager.cpp
 * @brief ImGuiベースのゲーム内蔵エディター実装
 */

#include "Game/Editor/EditorManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace Game {
namespace Editor {

EditorManager::EditorManager() {
}

EditorManager::~EditorManager() {
}

bool EditorManager::Initialize(Data::DefinitionRegistry* registry, Data::DefinitionLoader* loader) {
    if (!registry || !loader) {
        std::cerr << "EditorManager: Invalid registry or loader\n";
        return false;
    }

    registry_ = registry;
    loader_ = loader;
    initialized_ = true;

    AddLog("Editor initialized successfully");
    AddLog("Press F1 to toggle editor visibility");
    
    return true;
}

void EditorManager::Update(float deltaTime) {
    if (!initialized_) return;

    // F1キーでエディターの表示/非表示切り替え
    if (IsKeyPressed(KEY_F1)) {
        ToggleVisibility();
    }
}

void EditorManager::Render() {
    if (!initialized_ || !visible_) return;

    // ImGuiの描画開始
    rlImGuiBegin();

    // メニューバー
    RenderMenuBar();

    // 各ウィンドウの描画
    if (showCharacters_) RenderCharacterEditor();
    if (showStages_) RenderStageEditor();
    if (showUI_) RenderUIEditor();
    if (showSkills_) RenderSkillEditor();
    if (showEffects_) RenderEffectEditor();
    if (showSounds_) RenderSoundEditor();
    if (showInspector_) RenderInspector();
    if (showConsole_) RenderConsole();
    if (showHierarchy_) RenderHierarchy();
    if (showAssets_) RenderAssetBrowser();

    // ImGuiの描画終了
    rlImGuiEnd();
}

void EditorManager::ToggleVisibility() {
    visible_ = !visible_;
    AddLog(visible_ ? "Editor shown" : "Editor hidden");
}

void EditorManager::OpenWindow(EditorWindow window) {
    switch (window) {
        case EditorWindow::Characters: showCharacters_ = true; break;
        case EditorWindow::Stages: showStages_ = true; break;
        case EditorWindow::UI: showUI_ = true; break;
        case EditorWindow::Skills: showSkills_ = true; break;
        case EditorWindow::Effects: showEffects_ = true; break;
        case EditorWindow::Sounds: showSounds_ = true; break;
        case EditorWindow::Inspector: showInspector_ = true; break;
        case EditorWindow::Console: showConsole_ = true; break;
        case EditorWindow::Hierarchy: showHierarchy_ = true; break;
        case EditorWindow::Assets: showAssets_ = true; break;
        default: break;
    }
}

void EditorManager::AddLog(const std::string& message) {
    // タイムスタンプ付きログ
    std::ostringstream oss;
    oss << "[" << std::setw(6) << logs_.size() << "] " << message;
    logs_.push_back(oss.str());

    // ログが多すぎる場合は古いものを削除
    if (logs_.size() > 1000) {
        logs_.erase(logs_.begin());
    }
}

void EditorManager::RenderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save All", "Ctrl+S")) {
                AddLog("Save All - Not implemented yet");
            }
            if (ImGui::MenuItem("Reload All", "Ctrl+R")) {
                AddLog("Reload All - Not implemented yet");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit Editor", "F1")) {
                Hide();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows")) {
            ImGui::MenuItem("Characters", nullptr, &showCharacters_);
            ImGui::MenuItem("Stages", nullptr, &showStages_);
            ImGui::MenuItem("UI", nullptr, &showUI_);
            ImGui::MenuItem("Skills", nullptr, &showSkills_);
            ImGui::MenuItem("Effects", nullptr, &showEffects_);
            ImGui::MenuItem("Sounds", nullptr, &showSounds_);
            ImGui::Separator();
            ImGui::MenuItem("Inspector", nullptr, &showInspector_);
            ImGui::MenuItem("Console", nullptr, &showConsole_);
            ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy_);
            ImGui::MenuItem("Assets", nullptr, &showAssets_);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                AddLog("Simple TDC Game - Built-in Editor");
                AddLog("Press F1 to toggle editor");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void EditorManager::RenderCharacterEditor() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Character Editor", &showCharacters_)) {
        ImGui::Text("Character Editor");
        ImGui::Separator();

        if (!registry_) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Registry not available");
            ImGui::End();
            return;
        }

        // 左側: キャラクター一覧
        ImGui::BeginChild("CharacterList", ImVec2(200, 0), true);
        ImGui::Text("Characters");
        ImGui::Separator();

        auto characterIds = registry_->GetAllCharacterIds();
        for (const auto& id : characterIds) {
            if (ImGui::Selectable(id.c_str(), selectedCharacterId_ == id)) {
                selectedCharacterId_ = id;
            }
        }

        ImGui::EndChild();
        ImGui::SameLine();

        // 右側: キャラクター詳細
        ImGui::BeginChild("CharacterDetails");
        if (!selectedCharacterId_.empty()) {
            auto* character = registry_->TryGetCharacter(selectedCharacterId_);
            if (character) {
                ImGui::Text("ID: %s", selectedCharacterId_.c_str());
                ImGui::Text("Name: %s", character->name.c_str());
                ImGui::Separator();
                
                ImGui::Text("Stats:");
                ImGui::BulletText("HP: %.0f", character->stats.hp);
                ImGui::BulletText("Attack: %.0f", character->stats.attack);
                ImGui::BulletText("Defense: %.0f", character->stats.defense);
                ImGui::BulletText("Move Speed: %.0f", character->stats.moveSpeed);
                ImGui::BulletText("Attack Interval: %.2f", character->stats.attackInterval);
                
                // TODO: 編集機能を追加
                ImGui::Separator();
                ImGui::TextDisabled("(Edit功能は実装予定)");
            }
        } else {
            ImGui::TextDisabled("Select a character to edit");
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void EditorManager::RenderStageEditor() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Stage Editor", &showStages_)) {
        ImGui::Text("Stage Editor");
        ImGui::Separator();
        ImGui::TextDisabled("(実装予定)");
    }
    ImGui::End();
}

void EditorManager::RenderUIEditor() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("UI Editor", &showUI_)) {
        ImGui::Text("UI Editor");
        ImGui::Separator();
        ImGui::TextDisabled("(実装予定)");
    }
    ImGui::End();
}

void EditorManager::RenderSkillEditor() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Skill Editor", &showSkills_)) {
        ImGui::Text("Skill Editor");
        ImGui::Separator();
        ImGui::TextDisabled("(実装予定)");
    }
    ImGui::End();
}

void EditorManager::RenderEffectEditor() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Effect Editor", &showEffects_)) {
        ImGui::Text("Effect Editor");
        ImGui::Separator();
        ImGui::TextDisabled("(実装予定)");
    }
    ImGui::End();
}

void EditorManager::RenderSoundEditor() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Sound Editor", &showSounds_)) {
        ImGui::Text("Sound Editor");
        ImGui::Separator();
        ImGui::TextDisabled("(実装予定)");
    }
    ImGui::End();
}

void EditorManager::RenderInspector() {
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Inspector", &showInspector_)) {
        ImGui::Text("Inspector");
        ImGui::Separator();
        ImGui::TextDisabled("(選択したオブジェクトの詳細を表示)");
    }
    ImGui::End();
}

void EditorManager::RenderConsole() {
    ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Console", &showConsole_)) {
        ImGui::Text("Console");
        ImGui::Separator();

        // ログ表示
        ImGui::BeginChild("LogArea", ImVec2(0, -30), true);
        for (const auto& log : logs_) {
            ImGui::TextUnformatted(log.c_str());
        }
        // 自動スクロール
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

        // クリアボタン
        if (ImGui::Button("Clear")) {
            logs_.clear();
            AddLog("Console cleared");
        }
    }
    ImGui::End();
}

void EditorManager::RenderHierarchy() {
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Hierarchy", &showHierarchy_)) {
        ImGui::Text("Hierarchy");
        ImGui::Separator();
        ImGui::TextDisabled("(エンティティ一覧を表示)");
    }
    ImGui::End();
}

void EditorManager::RenderAssetBrowser() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Asset Browser", &showAssets_)) {
        ImGui::Text("Asset Browser");
        ImGui::Separator();
        ImGui::TextDisabled("(アセットファイルを表示)");
    }
    ImGui::End();
}

void EditorManager::ShowHelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

} // namespace Editor
} // namespace Game
