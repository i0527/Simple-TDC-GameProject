#include "Editor/Windows/PropertyPanel.h"

#include "Game/Components/NewCoreComponents.h"
#include <imgui.h>
#include <sstream>

namespace Editor::Windows {

PropertyPanel::PropertyPanel() = default;

void PropertyPanel::Initialize(Shared::Core::GameContext & /*context*/,
                               Shared::Data::DefinitionRegistry & /*definitions*/) {}

void PropertyPanel::Shutdown() {
  selected_entity_ = entt::null;
  registry_ = nullptr;
}

void PropertyPanel::OnUpdate(float /*deltaTime*/) {}

void PropertyPanel::OnDrawUI() {
  if (!is_open_)
    return;

  ImGui::Begin(GetWindowTitle().c_str(), &is_open_);
  
  if (selected_entity_ == entt::null || registry_ == nullptr) {
    ImGui::Text("No entity selected");
    ImGui::End();
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

  ImGui::End();
}

void PropertyPanel::SetSelection(entt::entity entity, const entt::registry *registry) {
  selected_entity_ = entity;
  registry_ = registry;
}

} // namespace Editor::Windows

