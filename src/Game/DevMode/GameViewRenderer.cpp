#include "Game/DevMode/GameViewRenderer.h"
#include <iostream>

namespace Game::DevMode {

GameViewRenderer::GameViewRenderer() = default;

GameViewRenderer::~GameViewRenderer() {
    Shutdown();
}

void GameViewRenderer::Initialize(int width, int height) {
    width_ = width;
    height_ = height;

    renderTarget_ = LoadRenderTexture(width_, height_);
    if (renderTarget_.id == 0) {
        std::cerr << "Failed to create RenderTexture\n";
    }
}

void GameViewRenderer::Shutdown() {
    if (renderTarget_.id != 0) {
        UnloadRenderTexture(renderTarget_);
        renderTarget_ = {0};
    }
}

void GameViewRenderer::BeginRender() {
    if (renderTarget_.id == 0) return;
    BeginTextureMode(renderTarget_);
}

void GameViewRenderer::EndRender() {
    if (renderTarget_.id == 0) return;
    EndTextureMode();
}

} // namespace Game::DevMode


