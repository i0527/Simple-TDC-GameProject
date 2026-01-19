#pragma once

namespace game {
namespace core {

class BaseSystemAPI;

/// @brief タイミングAPI
class TimingSystemAPI {
public:
  explicit TimingSystemAPI(BaseSystemAPI* owner);
  ~TimingSystemAPI() = default;

  float GetFrameTime();
  int GetFPS();
  void SetTargetFPS(int fps);
  double GetTime();

private:
  BaseSystemAPI* owner_;
};

} // namespace core
} // namespace game
