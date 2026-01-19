#include "../InputSystemAPI.hpp"

// 外部ライブラリ
#include <raylib.h>

// プロジェクト内
#include "../../../utils/Log.h"

namespace game {
namespace core {

InputSystemAPI::InputSystemAPI() : isInitialized_(false) {}

InputSystemAPI::~InputSystemAPI() = default;

bool InputSystemAPI::Initialize() {
    isInitialized_ = true;
    LOG_INFO("InputSystemAPI initialized");
    return true;
}

void InputSystemAPI::Shutdown() {
    if (!isInitialized_) {
        return;
    }
    LOG_INFO("InputSystemAPI shutdown");
    isInitialized_ = false;
}

bool InputSystemAPI::IsInitialized() const {
    return isInitialized_;
}

void InputSystemAPI::UpdateInput() {
    inputState_.mouseButtonConsumed.reset();
    Vector2 currentPos = ::GetMousePosition();
    inputState_.mouseDeltaX = currentPos.x - inputState_.mouseX;
    inputState_.mouseDeltaY = currentPos.y - inputState_.mouseY;
    inputState_.mouseX = currentPos.x;
    inputState_.mouseY = currentPos.y;
}

bool InputSystemAPI::IsKeyPressed(int key) const { return ::IsKeyPressed(key); }

bool InputSystemAPI::IsKeyPressedRepeat(int key) const {
    return ::IsKeyPressedRepeat(key);
}

bool InputSystemAPI::IsKeyDown(int key) const { return ::IsKeyDown(key); }

bool InputSystemAPI::IsKeyReleased(int key) const {
    return ::IsKeyReleased(key);
}

bool InputSystemAPI::IsKeyUp(int key) const { return ::IsKeyUp(key); }

int InputSystemAPI::GetKeyPressed() const { return ::GetKeyPressed(); }

int InputSystemAPI::GetCharPressed() const { return ::GetCharPressed(); }

void InputSystemAPI::SetExitKey(int key) { ::SetExitKey(key); }

bool InputSystemAPI::IsEscapePressed() const { return IsKeyPressed(KEY_ESCAPE); }

bool InputSystemAPI::IsSpacePressed() const { return IsKeyPressed(KEY_SPACE); }

bool InputSystemAPI::IsBackspacePressed() const {
    return IsKeyPressed(KEY_BACKSPACE);
}

bool InputSystemAPI::IsDeletePressed() const {
    return IsKeyPressed(KEY_DELETE);
}

bool InputSystemAPI::IsDebugTogglePressed() const {
    return IsKeyPressed(KEY_F1);
}

bool InputSystemAPI::IsEnterPressed() const {
    return IsKeyPressed(KEY_ENTER);
}

bool InputSystemAPI::IsUpPressed() const {
    return IsKeyPressed(KEY_UP);
}

bool InputSystemAPI::IsDownPressed() const {
    return IsKeyPressed(KEY_DOWN);
}

bool InputSystemAPI::IsPageUpPressed() const {
    return IsKeyPressed(KEY_PAGE_UP);
}

bool InputSystemAPI::IsPageDownPressed() const {
    return IsKeyPressed(KEY_PAGE_DOWN);
}

bool InputSystemAPI::IsMouseButtonPressed(int button) const {
    if (button < 0 || button >= 8) {
        return false;
    }
    return ::IsMouseButtonPressed(button) &&
           !inputState_.mouseButtonConsumed[button];
}

bool InputSystemAPI::IsMouseButtonDown(int button) const {
    return ::IsMouseButtonDown(button);
}

bool InputSystemAPI::IsMouseButtonReleased(int button) const {
    return ::IsMouseButtonReleased(button);
}

bool InputSystemAPI::IsMouseButtonUp(int button) const {
    return ::IsMouseButtonUp(button);
}

void InputSystemAPI::ConsumeMouseButton(int button) {
    if (button >= 0 && button < 8) {
        inputState_.mouseButtonConsumed[button] = true;
    }
}

Vec2 InputSystemAPI::GetMousePosition() const {
    return {inputState_.mouseX, inputState_.mouseY};
}

Vec2 InputSystemAPI::GetMouseDelta() const {
    return {inputState_.mouseDeltaX, inputState_.mouseDeltaY};
}

int InputSystemAPI::GetMouseX() const {
    return static_cast<int>(inputState_.mouseX);
}

int InputSystemAPI::GetMouseY() const {
    return static_cast<int>(inputState_.mouseY);
}

float InputSystemAPI::GetMouseWheelMove() const { return ::GetMouseWheelMove(); }

Vec2 InputSystemAPI::GetMouseWheelMoveV() const {
    const Vector2 v = ::GetMouseWheelMoveV();
    return {v.x, v.y};
}

bool InputSystemAPI::IsMouseOverRect(float x, float y, float width, float height) const {
    const Vec2 mouse = GetMousePosition();
    return mouse.x >= x && mouse.x <= x + width &&
           mouse.y >= y && mouse.y <= y + height;
}

std::pair<int, int> InputSystemAPI::GetMouseGridPosition(
    float originX, float originY, int cellSize, int gridWidth, int gridHeight) const {
    if (!IsMouseOverRect(originX, originY, gridWidth * cellSize, gridHeight * cellSize)) {
        return {-1, -1};
    }

    const Vec2 mouse = GetMousePosition();
    const int gx = static_cast<int>((mouse.x - originX) / cellSize);
    const int gy = static_cast<int>((mouse.y - originY) / cellSize);

    if (gx < 0 || gx >= gridWidth || gy < 0 || gy >= gridHeight) {
        return {-1, -1};
    }

    return {gx, gy};
}

bool InputSystemAPI::IsLeftClickPressed() const {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool InputSystemAPI::IsRightClickPressed() const {
    return IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
}

bool InputSystemAPI::IsLeftClickDown() const {
    return IsMouseButtonDown(MOUSE_BUTTON_LEFT);
}

bool InputSystemAPI::IsRightClickDown() const {
    return IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
}

bool InputSystemAPI::IsLeftClickReleased() const {
    return IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

bool InputSystemAPI::IsRightClickReleased() const {
    return IsMouseButtonReleased(MOUSE_BUTTON_RIGHT);
}

void InputSystemAPI::ConsumeLeftClick() {
    ConsumeMouseButton(MOUSE_BUTTON_LEFT);
}

void InputSystemAPI::ConsumeRightClick() {
    ConsumeMouseButton(MOUSE_BUTTON_RIGHT);
}

} // namespace core
} // namespace game
