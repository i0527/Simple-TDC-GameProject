#include "Game/Application/GameApp.h"
#include <iostream>

int main() {
    auto app = std::make_unique<Game::Application::GameApp>();

    if (!app->Initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return 1;
    }

    app->Run();
    app->Shutdown();

    return 0;
}
