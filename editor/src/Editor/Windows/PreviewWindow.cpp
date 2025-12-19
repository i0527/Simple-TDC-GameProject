#include "Editor/Windows/PreviewWindow.h"

#include "Core/GameContext.h"
#include "Game/Components/NewCoreComponents.h"
#include <imgui.h>
#include <raylib.h>

namespace Editor::Windows {

PreviewWindow::PreviewWindow() {
  preview_texture_initialized_ = false;
  preview_texture_ = {};
}

PreviewWindow::~PreviewWindow() {
  Shutdown();
}

void PreviewWindow::Initialize(Shared::Core::GameContext &context,
                               Shared::Data::DefinitionRegistry &definitions) {
  context_ = &context;
  definitions_ = &definitions;
  simulation_ = &context.GetSimulation();
  entity_reload_handle_ = definitions.OnEntityDefinitionReloaded.Connect(
      [this](const std::string &id) {
        if (id == current_entity_id_) {
          LoadEntity(id);
        }
      });
}

void PreviewWindow::Shutdown() {
  if (definitions_ && entity_reload_handle_ != 0) {
    definitions_->OnEntityDefinitionReloaded.Disconnect(entity_reload_handle_);
    entity_reload_handle_ = 0;
  }
  if (preview_texture_initialized_ && preview_texture_.texture.id != 0) {
    UnloadRenderTexture(preview_texture_);
    preview_texture_initialized_ = false;
  }
  Clear();
}

void PreviewWindow::OnUpdate(float deltaTime) {
  if (!is_open_ || !simulation_)
    return;
  
  // アニメーション更新（再生中の場合のみ）
  if (is_playing_ && preview_entity_ != entt::null) {
    UpdateAnimation(deltaTime * animation_speed_);
  }
  
  simulation_->Update(deltaTime);
}

void PreviewWindow::OnDrawUI() {
  if (!is_open_)
    return;

  ImGui::Begin(GetWindowTitle().c_str(), &is_open_);
  
  // エンティティ選択
  ImGui::Text("Preview Entity: %s",
              current_entity_id_.empty() ? "(none)" : current_entity_id_.c_str());
  
  // プレビューエリア
  DrawPreviewArea();
  
  ImGui::Separator();
  
  // アニメーション制御
  if (preview_entity_ != entt::null) {
    if (ImGui::Button(is_playing_ ? "Stop" : "Play")) {
      is_playing_ = !is_playing_;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
      if (preview_entity_ != entt::null && simulation_) {
        auto& registry = simulation_->GetRegistry();
        if (registry.all_of<Game::Components::Animation>(preview_entity_)) {
          auto& anim = registry.get<Game::Components::Animation>(preview_entity_);
          anim.current_frame = 0;
          anim.frame_timer = 0.0f;
          anim.elapsedTime = 0.0f;
          anim.frameIndex = 0;
        }
      }
    }
    
    ImGui::Text("Speed: %.2fx", animation_speed_);
    ImGui::SliderFloat("##speed", &animation_speed_, 0.0f, 3.0f, "%.2f");
  }
  
  ImGui::Separator();
  
  // 操作ボタン
  if (ImGui::Button("Reload") && !current_entity_id_.empty()) {
    LoadEntity(current_entity_id_);
  }
  ImGui::SameLine();
  if (ImGui::Button("Clear")) {
    Clear();
  }
  
  ImGui::End();
}

void PreviewWindow::LoadEntity(const std::string &definition_id) {
  if (!simulation_)
    return;

  Clear();
  preview_entity_ = simulation_->SpawnEntity(definition_id, {0.0f, 0.0f});
  if (preview_entity_ != entt::null) {
    current_entity_id_ = definition_id;
  }
}

void PreviewWindow::Clear() {
  if (simulation_ && preview_entity_ != entt::null) {
    simulation_->DestroyEntity(preview_entity_);
  }
  preview_entity_ = entt::null;
  current_entity_id_.clear();
  is_playing_ = true;
  animation_speed_ = 1.0f;
}

void PreviewWindow::UpdateAnimation(float deltaTime) {
  if (!simulation_ || preview_entity_ == entt::null)
    return;
  
  auto& registry = simulation_->GetRegistry();
  if (!registry.all_of<Game::Components::Animation>(preview_entity_))
    return;
  
  auto& anim = registry.get<Game::Components::Animation>(preview_entity_);
  
  // 新しいアニメーションシステム（クリップベース）
  if (!anim.currentClip.empty()) {
    anim.elapsedTime += deltaTime;
    // フレーム更新は実際のアニメーションシステムに任せる
  } else {
    // 古いアニメーションシステム（後方互換性）
    anim.frame_timer += deltaTime;
    if (anim.frame_timer >= anim.frame_duration) {
      anim.frame_timer = 0.0f;
      anim.current_frame = (anim.current_frame + 1) % std::max(1, anim.frames_per_state);
    }
  }
}

void PreviewWindow::DrawPreviewArea() {
  // RenderTextureの初期化
  if (!preview_texture_initialized_) {
    preview_texture_ = LoadRenderTexture(static_cast<int>(preview_size_), static_cast<int>(preview_size_));
    preview_texture_initialized_ = true;
  }
  
  // プレビューエリアの描画
  ImVec2 canvas_size = ImVec2(preview_size_, preview_size_);
  ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
  
  ImGui::InvisibleButton("canvas", canvas_size);
  
  // プレビュー背景
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(50, 50, 50, 255));
  draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(200, 200, 200, 255));
  
  // エンティティ情報表示
  if (preview_entity_ != entt::null) {
    ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x + 10, canvas_pos.y + 10));
    ImGui::Text("Entity: %u", static_cast<uint32_t>(preview_entity_));
    
    if (simulation_) {
      auto& registry = simulation_->GetRegistry();
      if (registry.all_of<Game::Components::Transform>(preview_entity_)) {
        const auto& tf = registry.get<Game::Components::Transform>(preview_entity_);
        ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x + 10, canvas_pos.y + 30));
        ImGui::Text("Pos: (%.1f, %.1f)", tf.x, tf.y);
      }
    }
  } else {
    ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x + canvas_size.x * 0.5f - 50, canvas_pos.y + canvas_size.y * 0.5f));
    ImGui::Text("No entity loaded");
  }
  
  ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x, canvas_pos.y + canvas_size.y + 10));
}

} // namespace Editor::Windows

