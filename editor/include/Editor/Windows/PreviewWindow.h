#pragma once

#include "Editor/Windows/IEditorWindow.h"
#include "Shared/Simulation/SimulationContext.h"
#include <entt/entt.hpp>
#include <raylib.h>
#include <string>

namespace Shared {
namespace Data {
struct EntityDef;
class DefinitionRegistry;
} // namespace Data
} // namespace Shared

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
  const std::string& GetCurrentEntityId() const { return current_entity_id_; }
  bool IsPlaying() const { return is_playing_; }
  void SetPlaying(bool playing) { is_playing_ = playing; }

  void LoadEntity(const std::string &definition_id);
  void Clear();
  void PreviewEntity(const std::string &entity_id);  // エディタから選択されたユニットをプレビュー
  void SetCurrentAction(const std::string &action_name);  // アクション指定で表示

  // PropertyPanel連携用
  entt::entity GetPreviewEntity() const { return preview_entity_; }
  const Shared::Simulation::SimulationContext *GetSimulation() const {
    return simulation_;
  }

private:
  void UpdateAnimation(float deltaTime);
  void DrawPreviewArea();
  void DrawFormationPreview();
  std::string ResolveIconPath(const Shared::Data::EntityDef& def) const;
  void LoadPreviewAssets(const Shared::Data::EntityDef& def);
  void UnloadPreviewAssets();
  bool LoadIdleFrame(const std::string& animation_path);
  bool LoadActionFrames(const std::string& animation_path);
  void ApplyPreviewSettings();

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
  Texture2D preview_atlas_texture_{};
  Texture2D preview_icon_texture_{};
  std::string preview_atlas_path_;
  std::string preview_icon_path_;
  std::string preview_idle_path_;
  Rectangle preview_frame_{};
  bool preview_frame_valid_ = false;
  int atlas_width_ = 0;
  int atlas_height_ = 0;
  struct FrameInfo { Rectangle rect; float duration; };
  std::vector<FrameInfo> current_frames_{};
  std::string forced_action_;  // 外部から指定されたアクション名
  int current_frame_index_ = 0;
  float current_frame_timer_ = 0.0f;
  
  // 表示設定
  bool show_hitbox_ = true;
  bool show_attack_point_ = true;
  // ミラー設定（メタからの既定＋UIで変更可能）
  bool mirror_h_ = false;
  bool mirror_v_ = false;
  // フレーム時間の上書き設定（UI）
  bool override_frame_duration_ = false;
  float frame_duration_override_ = 0.10f;

  // ステージ動作設定
  bool simulate_movement_ = false;
  float move_speed_ = 60.0f;
  float patrol_width_ = 200.0f;
  float ground_y_ = 0.0f;
  std::string current_action_;
  float patrol_phase_ = 0.0f;

  // 編成プレビュー用
  std::string formation_selected_action_ = "idle";
  entt::entity formation_preview_entity_ = entt::null;
  float formation_anim_time_ = 0.0f;
};

} // namespace Editor::Windows
