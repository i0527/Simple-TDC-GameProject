#include "Editor/Application/EditorApp.h"
#include <iostream>
#include <memory>

int main() {
    auto app = std::make_unique<Editor::Application::EditorApp>();

    if (!app->Initialize()) {
        std::cerr << "Failed to initialize editor" << std::endl;
        return 1;
    }

    app->Run();
    app->Shutdown();

    return 0;
}
