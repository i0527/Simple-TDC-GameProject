#include "Game/Scenes/SceneManager.h"
#include <algorithm>
#include <raylib.h>

namespace Game::Scenes {

SceneManager::~SceneManager() { ClearRenderTargets(); }

void SceneManager::PushScene(std::unique_ptr<IScene> scene,
                             TransitionType transition) {
  if (transition == TransitionType::IMMEDIATE) {
    scene_stack_.push_back(std::move(scene));
    return;
  }
  next_scene_ = std::move(scene);
  pending_action_ = PendingAction::Push;
  current_transition_ = transition;
  transitioning_ = true;
  transition_elapsed_ = 0.0f;
}

void SceneManager::PopScene(TransitionType transition) {
  if (scene_stack_.empty())
    return;
  if (transition == TransitionType::IMMEDIATE) {
    scene_stack_.pop_back();
    return;
  }
  pending_action_ = PendingAction::Pop;
  current_transition_ = transition;
  transitioning_ = true;
  transition_elapsed_ = 0.0f;
}

void SceneManager::ReplaceScene(std::unique_ptr<IScene> scene,
                                TransitionType transition) {
  if (transition == TransitionType::IMMEDIATE) {
    if (!scene_stack_.empty()) {
      scene_stack_.pop_back();
    }
    scene_stack_.push_back(std::move(scene));
    return;
  }
  next_scene_ = std::move(scene);
  pending_action_ = PendingAction::Replace;
  current_transition_ = transition;
  transitioning_ = true;
  transition_elapsed_ = 0.0f;
}

void SceneManager::Update(float delta_time) {
  if (transitioning_) {
    transition_elapsed_ += delta_time;
    if (transition_elapsed_ >= transition_duration_) {
      switch (pending_action_) {
      case PendingAction::Replace:
        if (!scene_stack_.empty()) {
          scene_stack_.pop_back();
        }
        scene_stack_.push_back(std::move(next_scene_));
        break;
      case PendingAction::Push:
        scene_stack_.push_back(std::move(next_scene_));
        break;
      case PendingAction::Pop:
        if (!scene_stack_.empty()) {
          scene_stack_.pop_back();
        }
        break;
      default:
        break;
      }
      pending_action_ = PendingAction::None;
      transitioning_ = false;
      transition_elapsed_ = 0.0f;
    }
  }

  if (!scene_stack_.empty()) {
    scene_stack_.back()->Update(delta_time);
  }
}

void SceneManager::Draw() {
  if (scene_stack_.empty())
    return;

  if (!transitioning_) {
    scene_stack_.back()->Draw();
    return;
  }

  const float progress = std::min(
      1.0f, transition_elapsed_ / std::max(0.001f, transition_duration_));

  if (IsSlideTransition()) {
    EnsureRenderTargets();

    IScene *current = scene_stack_.back().get();
    IScene *incoming = nullptr;
    if (pending_action_ == PendingAction::Pop) {
      if (scene_stack_.size() >= 2) {
        incoming = scene_stack_[scene_stack_.size() - 2].get();
      }
    } else {
      incoming = next_scene_.get();
    }

    RenderSceneToTarget(current, from_target_);
    if (incoming) {
      RenderSceneToTarget(incoming, to_target_);
    } else if (to_target_.id != 0) {
      BeginTextureMode(to_target_);
      ClearBackground(BLACK);
      EndTextureMode();
    }

    DrawSlide(progress);
    return;
  }

  scene_stack_.back()->Draw();

  if (transitioning_ && current_transition_ == TransitionType::FADE) {
    float alpha = progress;
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  Color{0, 0, 0, static_cast<unsigned char>(alpha * 255)});
  }
}

IScene *SceneManager::GetCurrentScene() const {
  if (scene_stack_.empty())
    return nullptr;
  return scene_stack_.back().get();
}

void SceneManager::EnsureRenderTargets() {
  int width = GetScreenWidth();
  int height = GetScreenHeight();
  if (width <= 0 || height <= 0) {
    return;
  }

  if (render_width_ == width && render_height_ == height &&
      from_target_.id != 0 && to_target_.id != 0) {
    return;
  }

  ClearRenderTargets();
  render_width_ = width;
  render_height_ = height;
  from_target_ = LoadRenderTexture(render_width_, render_height_);
  to_target_ = LoadRenderTexture(render_width_, render_height_);
}

void SceneManager::ClearRenderTargets() {
  if (from_target_.id != 0) {
    UnloadRenderTexture(from_target_);
    from_target_ = {};
  }
  if (to_target_.id != 0) {
    UnloadRenderTexture(to_target_);
    to_target_ = {};
  }
  render_width_ = 0;
  render_height_ = 0;
}

void SceneManager::RenderSceneToTarget(IScene *scene,
                                       RenderTexture2D &target) {
  if (!scene) {
    return;
  }
  if (target.id == 0) {
    scene->Draw();
    return;
  }

  BeginTextureMode(target);
  ClearBackground(BLANK);
  scene->Draw();
  EndTextureMode();
}

bool SceneManager::IsSlideTransition() const {
  return current_transition_ == TransitionType::SLIDE_LEFT ||
         current_transition_ == TransitionType::SLIDE_RIGHT;
}

void SceneManager::DrawSlide(float progress) {
  if (from_target_.id == 0 || to_target_.id == 0 || render_width_ <= 0 ||
      render_height_ <= 0) {
    scene_stack_.back()->Draw();
    return;
  }

  float t = std::clamp(progress, 0.0f, 1.0f);
  float dir = (current_transition_ == TransitionType::SLIDE_LEFT) ? -1.0f : 1.0f;
  float screen_w = static_cast<float>(render_width_);
  float screen_h = static_cast<float>(render_height_);

  float incoming_x = dir * screen_w * (1.0f - t);
  float outgoing_x = incoming_x - dir * screen_w;

  Rectangle src_from{0.0f, 0.0f, static_cast<float>(from_target_.texture.width),
                     -static_cast<float>(from_target_.texture.height)};
  Rectangle src_to{0.0f, 0.0f, static_cast<float>(to_target_.texture.width),
                   -static_cast<float>(to_target_.texture.height)};

  Rectangle dst_from{outgoing_x, 0.0f, screen_w, screen_h};
  Rectangle dst_to{incoming_x, 0.0f, screen_w, screen_h};

  DrawTexturePro(from_target_.texture, src_from, dst_from, {0.0f, 0.0f}, 0.0f,
                 WHITE);
  DrawTexturePro(to_target_.texture, src_to, dst_to, {0.0f, 0.0f}, 0.0f,
                 WHITE);
}

} // namespace Game::Scenes
