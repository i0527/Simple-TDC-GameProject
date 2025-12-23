#include "Editor/Windows/SearchPaletteWindow.h"

#include <imgui.h>
#include <algorithm>
#include <cstring>

#include "Data/DefinitionRegistry.h"
#include "Data/Definitions/EntityDef.h"
#include "Data/Definitions/SkillDef.h"
#include "Data/Definitions/StageDef.h"
#include "Data/Definitions/WaveDef.h"
#include "Data/Definitions/AbilityDef.h"
#include "Editor/Windows/UnitEditorWindow.h"
#include "Editor/Windows/SpriteEditorWindow.h"
#include "Editor/Windows/PreviewWindow.h"
#include "Editor/Windows/DefinitionEditorWindow.h"

namespace Editor::Windows {

SearchPaletteWindow::SearchPaletteWindow() {
  std::memset(query_, 0, sizeof(query_));
}

SearchPaletteWindow::~SearchPaletteWindow() {}

void SearchPaletteWindow::Initialize(Shared::Core::GameContext & /*context*/,
                                     Shared::Data::DefinitionRegistry &definitions) {
  definitions_ = &definitions;
  RebuildList();
}

void SearchPaletteWindow::RebuildList() {
  items_.clear();
  if (!definitions_) return;
  const auto entities = definitions_->GetAllEntities();
  const auto skills = definitions_->GetAllSkills();
  const auto stages = definitions_->GetAllStages();
  const auto waves = definitions_->GetAllWaves();
  const auto abilities = definitions_->GetAllAbilities();

  items_.reserve(entities.size() + skills.size() + stages.size() + waves.size() + abilities.size());

  for (const auto *e : entities) {
    items_.push_back({Kind::Entity, e->id, e->name, e->type});
  }
  for (const auto *s : skills) {
    items_.push_back({Kind::Skill, s->id, s->name, s->type});
  }
  for (const auto *s : stages) {
    items_.push_back({Kind::Stage, s->id, s->name, s->domain});
  }
  for (const auto *w : waves) {
    std::string label = w->spawn_groups.empty() ? "group0" : w->spawn_groups.front().entity_id;
    items_.push_back({Kind::Wave, w->id, label, "wave"});
  }
  for (const auto *a : abilities) {
    items_.push_back({Kind::Ability, a->id, a->name, a->type});
  }
  selectedIndex_ = items_.empty() ? -1 : 0;
}

void SearchPaletteWindow::ActivateSelection(int idx) {
  if (idx < 0 || idx >= static_cast<int>(items_.size())) return;
  const auto &it = items_[idx];
  switch (it.kind) {
  case Kind::Entity:
    if (unitEditor_) {
      unitEditor_->SetOpen(true);
      unitEditor_->SetActiveEntity(it.id);
    }
    if (spriteEditor_) {
      spriteEditor_->SetOpen(true);
      spriteEditor_->SetActiveEntity(it.id);
    }
    if (previewWindow_) {
      previewWindow_->SetOpen(true);
      previewWindow_->LoadEntity(it.id);
      if (togglePreviewPlayOnJump_) {
        previewWindow_->SetPlaying(!previewWindow_->IsPlaying());
      }
    }
    PushRecentEntity(it.id);
    break;
  case Kind::Skill:
    if (definitionEditor_) definitionEditor_->FocusTabSkills();
    break;
  case Kind::Stage:
    if (definitionEditor_) definitionEditor_->FocusTabStages();
    break;
  case Kind::Wave:
    if (definitionEditor_) definitionEditor_->FocusTabWaves();
    break;
  case Kind::Ability:
    if (definitionEditor_) definitionEditor_->FocusTabAbilities();
    break;
  }
  isOpen_ = false;
}

void SearchPaletteWindow::OnUpdate(float /*deltaTime*/) {}

void SearchPaletteWindow::OnDrawUI() {
  if (!isOpen_) return;
  if (!definitions_) return;

  const ImGuiIO &io = ImGui::GetIO();
  if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_P)) {
    // 再度ショートカットで閉じる
    isOpen_ = false;
    return;
  }

  ImGui::SetNextWindowSize(ImVec2(520, 420), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("クイック検索", &isOpen_, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse)) {
    if (focusInput_) {
      ImGui::SetKeyboardFocusHere();
      focusInput_ = false;
    }

    ImGui::InputText("検索 (ID/名前/タイプ)", query_, sizeof(query_));
    ImGui::SameLine();
    ImGui::Checkbox("プレビュー再生をトグル", &togglePreviewPlayOnJump_);

    std::string q = query_;
    std::transform(q.begin(), q.end(), q.begin(), [](unsigned char c){ return static_cast<char>(::tolower(c)); });

    ImGui::Separator();

    ImGui::BeginChild("palette_list", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()*3.0f), true);
    int idx = 0;
    for (const auto &it : items_) {
      std::string hay = it.id + " " + it.name + " " + it.type;
      std::string hayLower = hay;
      std::transform(hayLower.begin(), hayLower.end(), hayLower.begin(), [](unsigned char c){ return static_cast<char>(::tolower(c)); });
      if (!q.empty() && hayLower.find(q) == std::string::npos) { ++idx; continue; }

      const bool selected = (idx == selectedIndex_);
      std::string label;
      switch (it.kind) {
      case Kind::Entity: label = "[Entity] "; break;
      case Kind::Skill: label = "[Skill] "; break;
      case Kind::Stage: label = "[Stage] "; break;
      case Kind::Wave: label = "[Wave] "; break;
      case Kind::Ability: label = "[Ability] "; break;
      }
      label += it.id + " | " + it.name + " | " + it.type;

      if (ImGui::Selectable(label.c_str(), selected)) {
        selectedIndex_ = idx;
        ActivateSelection(idx);
      }
      if (selected) {
        ImGui::SetItemDefaultFocus();
      }
      ++idx;
    }
    ImGui::EndChild();

    if (ImGui::Button("再読込")) {
      RebuildList();
    }
    ImGui::SameLine();
    if (ImGui::Button("閉じる")) {
      isOpen_ = false;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("最近使ったエンティティ");
    ImGui::BeginChild("recent_entities", ImVec2(0, 80), true);
    for (const auto &e : recentEntities_) {
      if (ImGui::Selectable(e.c_str(), false)) {
        // find index for this entity and activate
        for (int i = 0; i < static_cast<int>(items_.size()); ++i) {
          if (items_[i].kind == Kind::Entity && items_[i].id == e) {
            ActivateSelection(i);
            break;
          }
        }
      }
    }
    ImGui::EndChild();

    // キー操作: 上下/Enter
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
      if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        if (selectedIndex_ > 0) selectedIndex_--;
      }
      if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        if (selectedIndex_ + 1 < static_cast<int>(items_.size())) selectedIndex_++;
      }
      if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
        ActivateSelection(selectedIndex_);
      }
    }
  }
  ImGui::End();
}

void SearchPaletteWindow::PushRecentEntity(const std::string& id) {
  if (id.empty()) return;
  recentEntities_.erase(std::remove(recentEntities_.begin(), recentEntities_.end(), id), recentEntities_.end());
  recentEntities_.insert(recentEntities_.begin(), id);
  constexpr size_t kMax = 10;
  if (recentEntities_.size() > kMax) recentEntities_.resize(kMax);
}

} // namespace Editor::Windows
