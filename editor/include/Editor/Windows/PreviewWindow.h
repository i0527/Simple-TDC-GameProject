#pragma once

#include "Editor/Windows/IEditorWindow.h"
#include "Shared/Simulation/SimulationContext.h"
#include <entt/entt.hpp>
#include <raylib.h>
#include <string>

namespace Editor::Windows {

class PreviewWindow : public IEditorWindow {
public:
  PreviewWindow();
  ~PreviewWindow() override;

  void Initialize(Shared::Core::GameContext &context,
                  Shared::Data::DefinitionRegistry &definitions) override;
  void Shutdown() override;
  void OnUpdate(float deltaTime) override;
  void OnDrawUI() override;
  std::string GetWindowTitle() const override { return "Preview"; }
  std::string GetWindowId() const override { return "preview_window"; }
  bool IsOpen() const override { return is_open_; }
  void SetOpen(bool open) override { is_open_ = open; }

  void LoadEntity(const std::string &definition_id);
  void Clear();

  // PropertyPanel連携用
  entt::entity GetPreviewEntity() const { return preview_entity_; }
  const Shared::Simulation::SimulationContext *GetSimulation() const {
    return simulation_;
  }

private:
  void UpdateAnimation(float deltaTime);
  void DrawPreviewArea();

  Shared::Core::GameContext *context_ = nullptr;
  Shared::Data::DefinitionRegistry *definitions_ = nullptr;
  Shared::Simulation::SimulationContext *simulation_ = nullptr;

  entt::entity preview_entity_ = entt::null;
  std::string current_entity_id_;
  bool is_open_ = true;
  int entity_reload_handle_ = 0;

  // アニメーション制御
  bool is_playing_ = true;
  float animation_speed_ = 1.0f;
  RenderTexture2D preview_texture_{};
  bool preview_texture_initialized_ = false;
  const float preview_size_ = 400.0f;
};

} // namespace Editor::Windows
