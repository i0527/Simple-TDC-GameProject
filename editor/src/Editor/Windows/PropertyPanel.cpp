#include "Editor/Windows/PropertyPanel.h"

#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"
#include "Data/Loaders/EntityLoader.h"
#include "Game/Components/NewCoreComponents.h"
#include "Data/Definitions/EntityDef.h"
#include <imgui.h>
#include <sstream>
#include <filesystem>

namespace Editor::Windows {

namespace {
// Helper for std::string input
bool InputTextString(const char* label, std::string& str) {
    char buffer[256];
    strncpy(buffer, str.c_str(), sizeof(buffer));
    buffer[sizeof(buffer) - 1] = 0;
    if (ImGui::InputText(label, buffer, sizeof(buffer))) {
        str = buffer;
        return true;
    }
    return false;
}
}

PropertyPanel::PropertyPanel() = default;

void PropertyPanel::Initialize(Shared::Core::GameContext &context,
                               Shared::Data::DefinitionRegistry &definitions) {
  context_ = &context;
  definitions_ = &definitions;
}

void PropertyPanel::Shutdown() {
  selected_entity_ = entt::null;
  registry_ = nullptr;
  selected_entity_def_id_.clear();
}

void PropertyPanel::OnUpdate(float /*deltaTime*/) {}

void PropertyPanel::OnDrawUI() {
  if (!is_open_)
    return;

  ImGui::Begin(GetWindowTitle().c_str(), &is_open_);
  
  // ユニット定義表示優先
  if (!selected_entity_def_id_.empty()) {
    DrawEntityDefinitionProperties();
  } else if (selected_entity_ != entt::null && registry_ != nullptr) {
    DrawEntityProperties();
  } else {
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "エンティティが選択されていません");
  }

  ImGui::End();
}

void PropertyPanel::DrawEntityProperties() {
  if (selected_entity_ == entt::null || registry_ == nullptr) {
    return;
  }

  ImGui::Text("Entity ID: %u", static_cast<uint32_t>(selected_entity_));
  ImGui::Separator();

  // Transform コンポーネント
  if (registry_->all_of<Game::Components::Transform>(selected_entity_)) {
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      const auto& tf = registry_->get<Game::Components::Transform>(selected_entity_);
      ImGui::Text("Position: (%.2f, %.2f)", tf.x, tf.y);
      ImGui::Text("Scale: (%.2f, %.2f)", tf.scaleX, tf.scaleY);
      ImGui::Text("Rotation: %.2f", tf.rotation);
      ImGui::Text("Flip H: %s", tf.flipH ? "Yes" : "No");
      ImGui::Text("Flip V: %s", tf.flipV ? "Yes" : "No");
    }
  }

  // Team コンポーネント
  if (registry_->all_of<Game::Components::Team>(selected_entity_)) {
    if (ImGui::CollapsingHeader("Team")) {
      const auto& team = registry_->get<Game::Components::Team>(selected_entity_);
      const char* team_name = (team.type == Game::Components::Team::Type::Player) ? "Player" : "Enemy";
      ImGui::Text("Team: %s", team_name);
    }
  }

  // Stats コンポーネント
  if (registry_->all_of<Game::Components::Stats>(selected_entity_)) {
    if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
      const auto& stats = registry_->get<Game::Components::Stats>(selected_entity_);
      ImGui::Text("HP: %d / %d", stats.current_hp, stats.max_hp);
      ImGui::Text("Attack: %d", stats.attack);
      ImGui::Text("Attack Speed: %.2f", stats.attack_speed);
      ImGui::Text("Range: %d", stats.range);
      ImGui::Text("Move Speed: %.2f", stats.move_speed);
      ImGui::Text("Knockback: %.2f", stats.knockback);
    }
  }

  // Animation コンポーネント
  if (registry_->all_of<Game::Components::Animation>(selected_entity_)) {
    if (ImGui::CollapsingHeader("Animation")) {
      const auto& anim = registry_->get<Game::Components::Animation>(selected_entity_);
      ImGui::Text("Current Clip: %s", anim.currentClip.empty() ? "(none)" : anim.currentClip.c_str());
      ImGui::Text("Frame Index: %d", anim.frameIndex);
      ImGui::Text("Elapsed Time: %.2f", anim.elapsedTime);
      ImGui::Text("Is Playing: %s", anim.isPlaying ? "Yes" : "No");
      ImGui::Text("Current Frame: %d", anim.current_frame);
      ImGui::Text("Frame Timer: %.2f", anim.frame_timer);
      ImGui::Text("Frame Duration: %.2f", anim.frame_duration);
    }
  }

  // Sprite コンポーネント
  if (registry_->all_of<Game::Components::Sprite>(selected_entity_)) {
    if (ImGui::CollapsingHeader("Sprite")) {
      const auto& sprite = registry_->get<Game::Components::Sprite>(selected_entity_);
      ImGui::Text("Texture Path: %s", sprite.texturePath.empty() ? "(none)" : sprite.texturePath.c_str());
      ImGui::Text("Loaded: %s", sprite.loaded ? "Yes" : "No");
      ImGui::Text("Failed: %s", sprite.failed ? "Yes" : "No");
    }
  }

  // EntityDefId コンポーネント
  if (registry_->all_of<Game::Components::EntityDefId>(selected_entity_)) {
    if (ImGui::CollapsingHeader("Entity Definition")) {
      const auto& defId = registry_->get<Game::Components::EntityDefId>(selected_entity_);
      ImGui::Text("Definition ID: %s", defId.id.c_str());
    }
  }
}

void PropertyPanel::DrawEntityDefinitionProperties() {
  if (!definitions_) {
    ImGui::TextColored(ImVec4(1, 0, 0, 1), "DefinitionRegistry が初期化されていません");
    return;
  }

  if (selected_entity_def_id_.empty()) return;

  // Header with Save button
  ImGui::Text("📋 %s", editing_def_.name.c_str());
  ImGui::SameLine();
  if (is_dirty_) {
      if (ImGui::Button("💾 Save")) {
          SaveCurrentEntity();
      }
  } else {
      ImGui::BeginDisabled();
      ImGui::Button("Saved");
      ImGui::EndDisabled();
  }
  
  ImGui::Separator();

  // 基本情報
  if (ImGui::CollapsingHeader("基本情報", ImGuiTreeNodeFlags_DefaultOpen)) {
    // ID is usually immutable after creation
    ImGui::LabelText("ID", "%s", editing_def_.id.c_str());
    
    if (InputTextString("名前", editing_def_.name)) is_dirty_ = true;
    if (InputTextString("説明", editing_def_.description)) is_dirty_ = true;
    
    // Type
    const char* types[] = { "main", "sub", "enemy" };
    int current_type = 0;
    if (editing_def_.type == "sub") current_type = 1;
    else if (editing_def_.type == "enemy") current_type = 2;
    
    if (ImGui::Combo("Type", &current_type, types, IM_ARRAYSIZE(types))) {
        editing_def_.type = types[current_type];
        editing_def_.is_enemy = (editing_def_.type == "enemy");
        is_dirty_ = true;
    }

    if (ImGui::SliderInt("レアリティ", &editing_def_.rarity, 1, 5)) is_dirty_ = true;
    if (ImGui::InputInt("コスト", &editing_def_.cost)) is_dirty_ = true;
  }

  // ステータス
  if (ImGui::CollapsingHeader("ステータス", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::InputInt("HP", &editing_def_.stats.hp)) is_dirty_ = true;
    if (ImGui::InputInt("攻撃力", &editing_def_.stats.attack)) is_dirty_ = true;
    if (ImGui::DragFloat("攻撃速度", &editing_def_.stats.attack_speed, 0.1f, 0.1f, 10.0f)) is_dirty_ = true;
    if (ImGui::DragFloat("移動速度", &editing_def_.stats.move_speed, 1.0f, 0.0f, 500.0f)) is_dirty_ = true;
    if (ImGui::InputInt("範囲", &editing_def_.stats.range)) is_dirty_ = true;
    if (ImGui::InputInt("ノックバック", &editing_def_.stats.knockback)) is_dirty_ = true;
  }

  // Combat
  if (ImGui::CollapsingHeader("Combat", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::SliderFloat("Attack Point (0.0-1.0)", &editing_def_.combat.attack_point, 0.0f, 1.0f)) is_dirty_ = true;
    if (ImGui::InputInt("Attack Frame (-1=Auto)", &editing_def_.combat.attack_frame)) is_dirty_ = true;
    
    ImGui::Text("Hitbox:");
    if (ImGui::DragFloat("Width", &editing_def_.combat.hitbox.width, 1.0f, 0.0f, 1000.0f)) is_dirty_ = true;
    if (ImGui::DragFloat("Height", &editing_def_.combat.hitbox.height, 1.0f, 0.0f, 1000.0f)) is_dirty_ = true;
    if (ImGui::DragFloat("Offset X", &editing_def_.combat.hitbox.offset_x, 1.0f, -500.0f, 500.0f)) is_dirty_ = true;
    if (ImGui::DragFloat("Offset Y", &editing_def_.combat.hitbox.offset_y, 1.0f, -500.0f, 500.0f)) is_dirty_ = true;
  }

  // グラフィックス
  if (ImGui::CollapsingHeader("グラフィックス", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (InputTextString("Icon", editing_def_.display.icon)) is_dirty_ = true;
    if (InputTextString("Atlas", editing_def_.display.atlas_texture)) is_dirty_ = true;
    
    ImGui::Text("Animations:");
    for (auto& pair : editing_def_.display.sprite_actions) {
        std::string key = pair.first;
        std::string val = pair.second;
        ImGui::Text("%s:", key.c_str());
        ImGui::SameLine();
        if (InputTextString(("##" + key).c_str(), val)) {
            editing_def_.display.sprite_actions[key] = val;
            is_dirty_ = true;
        }
    }
  }

  // アビリティ・スキル
  if (ImGui::CollapsingHeader("アビリティ")) {
    if (editing_def_.ability_ids.empty()) {
      ImGui::TextUnformatted("(なし)");
    } else {
      for (size_t i = 0; i < editing_def_.ability_ids.size(); ++i) {
        ImGui::BulletText("%s", editing_def_.ability_ids[i].c_str());
      }
    }
  }

  if (ImGui::CollapsingHeader("スキル")) {
    if (editing_def_.skill_ids.empty()) {
      ImGui::TextUnformatted("(なし)");
    } else {
      for (size_t i = 0; i < editing_def_.skill_ids.size(); ++i) {
        ImGui::BulletText("%s", editing_def_.skill_ids[i].c_str());
      }
    }
  }
}

void PropertyPanel::SetSelection(entt::entity entity, const entt::registry *registry) {
  selected_entity_ = entity;
  registry_ = registry;
  selected_entity_def_id_.clear();
}

void PropertyPanel::SelectEntity(const std::string& entity_id) {
  selected_entity_def_id_ = entity_id;
  selected_entity_ = entt::null;
  registry_ = nullptr;

  if (definitions_) {
    const auto* def = definitions_->GetEntity(entity_id);
    if (def) {
      editing_def_ = *def;
      is_dirty_ = false;
    }
  }
}

void PropertyPanel::SaveCurrentEntity() {
  if (selected_entity_def_id_.empty() || !definitions_ || !context_) return;

  // 1. Update Registry
  definitions_->RegisterEntity(editing_def_);

  // 2. Save to File
  // TODO: Handle different directories based on type or existing file location
  // For now, assume assets/definitions/entities/characters/{id}.json
  std::string base_dir = context_->GetDataPath("entities/characters");
  std::filesystem::create_directories(base_dir);
  std::string filepath = base_dir + "/" + editing_def_.id + ".json";

  if (Shared::Data::EntityLoader::SaveSingleEntity(filepath, editing_def_)) {
    is_dirty_ = false;
  }
}

} // namespace Editor::Windows
