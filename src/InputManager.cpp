#include "InputManager.h"
#include <iostream>

namespace Core {
    InputManager& InputManager::GetInstance() {
        static InputManager instance;
        return instance;
    }
    
    void InputManager::Update() {
        // Raylibの入力更新は自動的に行われる
        // このメソッドは拡張性のため保持
    }
    
    bool InputManager::IsKeyPressed(int keyCode) {
        return ::IsKeyPressed(keyCode);
    }
    
    bool InputManager::IsKeyDown(int keyCode) {
        return ::IsKeyDown(keyCode);
    }
    
    bool InputManager::IsKeyUp(int keyCode) {
        return ::IsKeyUp(keyCode);
    }
    
    bool InputManager::IsMouseButtonPressed(int button) {
        return ::IsMouseButtonPressed(button);
    }
    
    bool InputManager::IsMouseButtonDown(int button) {
        return ::IsMouseButtonDown(button);
    }
    
    bool InputManager::IsMouseButtonUp(int button) {
        return ::IsMouseButtonUp(button);
    }
    
    float InputManager::GetMouseX() const {
        return ::GetMouseX();
    }
    
    float InputManager::GetMouseY() const {
        return ::GetMouseY();
    }
    
    float InputManager::GetMouseWheelMove() const {
        return ::GetMouseWheelMove();
    }
    
    void InputManager::RegisterKeyBinding(const std::string& action, int keyCode) {
        keyBindings_[action] = keyCode;
        std::cout << "Key binding registered: " << action << " (" << keyCode << ")" << std::endl;
    }
    
    bool InputManager::IsActionPressed(const std::string& action) {
        auto it = keyBindings_.find(action);
        if (it == keyBindings_.end()) {
            std::cerr << "Action not found: " << action << std::endl;
            return false;
        }
        return IsKeyPressed(it->second);
    }
}
