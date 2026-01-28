#pragma once

// 標準ライブラリ
#include <bitset>
#include <utility>

// プロジェクト内
#include "../config/RenderPrimitives.hpp"

namespace game {
namespace core {

class BaseSystemAPI;

/// @brief 入力統合API
///
/// BaseSystemAPI の入力機能をまとめ、シーン共通で利用するためのラッパー。
/// 論理座標（1920x1080）でのマウス位置は GetMousePositionInternal() で取得する。
class InputSystemAPI {
public:
    InputSystemAPI();
    ~InputSystemAPI();

    /// @brief 初期化。BaseSystemAPI を渡すと GetMousePositionInternal で論理座標変換を行う。
    bool Initialize();
    bool Initialize(BaseSystemAPI* systemAPI);
    void Shutdown();
    bool IsInitialized() const;

    // 入力状態更新（毎フレーム呼び出し必須）
    void UpdateInput();

    // ========== キーボード ==========
    bool IsKeyPressed(int key) const;
    bool IsKeyPressedRepeat(int key) const;
    bool IsKeyDown(int key) const;
    bool IsKeyReleased(int key) const;
    bool IsKeyUp(int key) const;
    int GetKeyPressed() const;
    int GetCharPressed() const;
    void SetExitKey(int key);

    // 便利ショートカット（共通化しない方針のため最低限のみ提供）
    bool IsEscapePressed() const;
    bool IsSpacePressed() const;
    bool IsBackspacePressed() const;
    bool IsDeletePressed() const;
    bool IsDebugTogglePressed() const;
    bool IsEnterPressed() const;
    bool IsUpPressed() const;
    bool IsDownPressed() const;
    bool IsPageUpPressed() const;
    bool IsPageDownPressed() const;

    // ========== マウス ==========
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonDown(int button) const;
    bool IsMouseButtonReleased(int button) const;
    bool IsMouseButtonUp(int button) const;
    void ConsumeMouseButton(int button);

    Vec2 GetMousePosition() const;
    /// @brief 論理座標（内部解像度 1920x1080）でのマウス位置。Initialize(BaseSystemAPI*) 済み時のみ変換。
    Vec2 GetMousePositionInternal() const;
    Vec2 GetMouseDelta() const;
    int GetMouseX() const;
    int GetMouseY() const;
    float GetMouseWheelMove() const;
    Vec2 GetMouseWheelMoveV() const;

    // 座標補助（論理座標の rect / グリッド用。GetMousePositionInternal を使用）
    bool IsMouseOverRect(float x, float y, float width, float height) const;
    std::pair<int, int> GetMouseGridPosition(float originX, float originY,
                                             int cellSize, int gridWidth, int gridHeight) const;

    // クリック判定（1フレームのみ）
    bool IsLeftClickPressed() const;
    bool IsRightClickPressed() const;
    bool IsLeftClickDown() const;
    bool IsRightClickDown() const;
    bool IsLeftClickReleased() const;
    bool IsRightClickReleased() const;
    void ConsumeLeftClick();
    void ConsumeRightClick();

private:
    bool isInitialized_;
    BaseSystemAPI* systemAPI_ = nullptr;

    struct InputState {
        float mouseX = 0.0f;
        float mouseY = 0.0f;
        float mouseDeltaX = 0.0f;
        float mouseDeltaY = 0.0f;
        std::bitset<8> mouseButtonConsumed;
    };
    InputState inputState_;
};

} // namespace core
} // namespace game
