#include "../TimingSystemAPI.hpp"
#include "../BaseSystemAPI.hpp"

namespace game {
namespace core {

TimingSystemAPI::TimingSystemAPI(BaseSystemAPI* owner) : owner_(owner) {}

float TimingSystemAPI::GetFrameTime() { return ::GetFrameTime(); }

int TimingSystemAPI::GetFPS() { return ::GetFPS(); }

void TimingSystemAPI::SetTargetFPS(int fps) { ::SetTargetFPS(fps); }

double TimingSystemAPI::GetTime() { return ::GetTime(); }

} // namespace core
} // namespace game

