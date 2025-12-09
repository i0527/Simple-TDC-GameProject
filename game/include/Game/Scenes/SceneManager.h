#pragma once

#include <memory>
#include <vector>

#include <raylib.h>

#include "Game/Scenes/IScene.h"

namespace Game::Scenes {

class SceneManager {
public:
  enum class TransitionType { IMMEDIATE, FADE, SLIDE_LEFT, SLIDE_RIGHT };

  explicit SceneManager() = default;
  ~SceneManager();

  void PushScene(std::unique_ptr<IScene> scene,
                 TransitionType transition = TransitionType::IMMEDIATE);
  void PopScene(TransitionType transition = TransitionType::IMMEDIATE);
  void ReplaceScene(std::unique_ptr<IScene> scene,
                    TransitionType transition = TransitionType::IMMEDIATE);

  void Update(float delta_time);
  void Draw();

  IScene *GetCurrentScene() const;

private:
  void EnsureRenderTargets();
  void ClearRenderTargets();
  void RenderSceneToTarget(IScene *scene, RenderTexture2D &target);
  void DrawSlide(float progress);
  bool IsSlideTransition() const;

  enum class PendingAction { None, Push, Replace, Pop };

  std::vector<std::unique_ptr<IScene>> scene_stack_;
  std::unique_ptr<IScene> next_scene_;
  PendingAction pending_action_ = PendingAction::None;

  bool transitioning_ = false;
  float transition_elapsed_ = 0.0f;
  float transition_duration_ = 0.35f;
  TransitionType current_transition_ = TransitionType::IMMEDIATE;

  RenderTexture2D from_target_{};
  RenderTexture2D to_target_{};
  int render_width_ = 0;
  int render_height_ = 0;
};

} // namespace Game::Scenes
