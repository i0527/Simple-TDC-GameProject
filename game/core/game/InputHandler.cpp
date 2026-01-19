#include "InputHandler.hpp"
#include "../../utils/Log.h"
#include <raylib.h>

namespace game {
namespace core {
namespace gamescene {

InputHandler::InputHandler(BaseSystemAPI* sysAPI)
    : sysAPI_(sysAPI), hasEvent_(false), leftClickPressed_(false),
      rightClickPressed_(false) {
    currentEvent_.type = InputEventType::None;
    mousePosition_ = {0, 0};
}

bool InputHandler::Initialize() {
    LOG_INFO("InputHandler initialized");
    return true;
}

void InputHandler::Shutdown() {
    LOG_INFO("InputHandler shutdown");
}

void InputHandler::Update(float deltaTime) {
    // マウス状態更新
    mousePosition_ = ::GetMousePosition();
    leftClickPressed_ = ::IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    rightClickPressed_ = ::IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    
    // イベントはシーン側で生成されるため、ここでは状態のみ更新
}

Vector2 InputHandler::GetMousePosition() const {
    return mousePosition_;
}

std::pair<int, int> InputHandler::GetMouseGridPosition(
    float fieldOriginX, float fieldOriginY,
    int cellSize, int fieldWidth, int fieldHeight) const {
    
    // マウスがフィールド上にない場合
    if (!IsMouseOverField(fieldOriginX, fieldOriginY,
                         fieldWidth * cellSize, fieldHeight * cellSize)) {
        return {-1, -1};
    }
    
    // グリッド座標に変換
    int gx = static_cast<int>((mousePosition_.x - fieldOriginX) / cellSize);
    int gy = static_cast<int>((mousePosition_.y - fieldOriginY) / cellSize);
    
    // 範囲チェック
    if (gx < 0 || gx >= fieldWidth || gy < 0 || gy >= fieldHeight) {
        return {-1, -1};
    }
    
    return {gx, gy};
}

bool InputHandler::IsMouseOverField(float fieldOriginX, float fieldOriginY,
                                   float fieldWidth, float fieldHeight) const {
    return mousePosition_.x >= fieldOriginX &&
           mousePosition_.x <= fieldOriginX + fieldWidth &&
           mousePosition_.y >= fieldOriginY &&
           mousePosition_.y <= fieldOriginY + fieldHeight;
}

bool InputHandler::IsLeftClickPressed() const {
    return leftClickPressed_;
}

bool InputHandler::IsRightClickPressed() const {
    return rightClickPressed_;
}

bool InputHandler::IsKeyPressed(int key) const {
    return ::IsKeyPressed(key);
}

bool InputHandler::IsEscapePressed() const {
    return IsKeyPressed(KEY_ESCAPE);
}

bool InputHandler::IsSpacePressed() const {
    return IsKeyPressed(KEY_SPACE);
}

void InputHandler::GenerateFieldClickEvent(int gx, int gy, bool isRightClick) {
    currentEvent_.type = isRightClick ? InputEventType::FieldRightClick : InputEventType::FieldClick;
    currentEvent_.gridX = gx;
    currentEvent_.gridY = gy;
    currentEvent_.data = "";
    hasEvent_ = true;
    
    LOG_DEBUG("Field click event: ({}, {}) right={}", gx, gy, isRightClick);
}

void InputHandler::GenerateButtonClickEvent(const std::string& buttonId) {
    currentEvent_.type = InputEventType::ButtonClick;
    currentEvent_.gridX = -1;
    currentEvent_.gridY = -1;
    currentEvent_.data = buttonId;
    hasEvent_ = true;
    
    LOG_DEBUG("Button click event: {}", buttonId);
}

} // namespace gamescene
} // namespace core
} // namespace game
