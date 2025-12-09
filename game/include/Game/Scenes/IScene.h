#pragma once

namespace Game::Scenes {

class IScene {
public:
  virtual ~IScene() = default;
  virtual void Update(float delta_time) = 0;
  virtual void Draw() = 0;
};

} // namespace Game::Scenes
