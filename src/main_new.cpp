#include "Core/GameContext.h"
#include "raylib.h"


int main() {
  InitWindow(1280, 720, "SimpleTDCGame - NewArchNext");
  SetTargetFPS(60);

  New::Core::GameContext context;
  if (!context.Initialize()) {
    CloseWindow();
    return -1;
  }

  auto &renderer = context.GetRenderer();

  while (!WindowShouldClose()) {
    const float dt = GetFrameTime();
    context.GetSystemRunner().Update(context, dt);

    BeginDrawing();
    {
      ClearBackground(BLACK);

      renderer.BeginRender();
      renderer.Clear(BLANK);
      // TODO: add game rendering here
      renderer.EndRender();

      renderer.RenderScaled();
    }
    EndDrawing();
  }

  context.Shutdown();
  CloseWindow();
  return 0;
}
