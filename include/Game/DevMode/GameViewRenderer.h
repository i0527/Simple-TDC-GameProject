#pragma once

#include <raylib.h>

namespace Game::DevMode {

class GameViewRenderer {
public:
    GameViewRenderer();
    ~GameViewRenderer();

    void Initialize(int width = 1920, int height = 1080);
    void Shutdown();

    void BeginRender();
    void EndRender();

    RenderTexture2D& GetRenderTarget() { return renderTarget_; }

    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }

private:
    RenderTexture2D renderTarget_ = {0};
    int width_ = 1920;
    int height_ = 1080;
};

} // namespace Game::DevMode


