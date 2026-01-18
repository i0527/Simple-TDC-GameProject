#pragma once

#include "../api/BaseSystemAPI.hpp"
#include <raylib.h>
#include <functional>
#include <string>

namespace game {
namespace core {
namespace gamescene {

/// @brief 入力イベントタイプ
enum class InputEventType {
    None,
    FieldClick,        // フィールドクリック
    FieldRightClick,   // フィールド右クリック
    FieldHover,        // フィールドホバー
    ButtonClick,       // ボタンクリック
    KeyPress           // キー押下
};

/// @brief 入力イベント
struct InputEvent {
    InputEventType type;
    int gridX;          // グリッド座標X（フィールド操作時）
    int gridY;          // グリッド座標Y（フィールド操作時）
    std::string data;   // 追加データ（ボタンID、キー名など）
};

/// @brief 入力処理クラス
///
/// 責務:
/// - マウス入力の処理
/// - キーボード入力の処理
/// - 座標変換
/// - 入力イベントの生成
class InputHandler {
public:
    /// @brief コンストラクタ
    /// @param sysAPI BaseSystemAPIポインタ
    explicit InputHandler(BaseSystemAPI* sysAPI);
    
    ~InputHandler() = default;

    /// @brief 初期化
    bool Initialize();

    /// @brief シャットダウン
    void Shutdown();

    /// @brief 入力更新（毎フレーム呼び出し）
    void Update(float deltaTime);

    // ========== マウス状態 ==========

    /// @brief マウス位置を取得
    Vector2 GetMousePosition() const;

    /// @brief フィールド上でのマウスグリッド座標を取得
    /// @param fieldOriginX フィールド原点X座標
    /// @param fieldOriginY フィールド原点Y座標
    /// @param cellSize セルサイズ
    /// @param fieldWidth フィールド幅（グリッド数）
    /// @param fieldHeight フィールド高さ（グリッド数）
    /// @return グリッド座標（-1, -1の場合はフィールド外）
    std::pair<int, int> GetMouseGridPosition(float fieldOriginX, float fieldOriginY,
                                            int cellSize, int fieldWidth, int fieldHeight) const;

    /// @brief マウスがフィールド上にあるか
    /// @param fieldOriginX フィールド原点X座標
    /// @param fieldOriginY フィールド原点Y座標
    /// @param fieldWidth フィールド幅（ピクセル）
    /// @param fieldHeight フィールド高さ（ピクセル）
    bool IsMouseOverField(float fieldOriginX, float fieldOriginY,
                         float fieldWidth, float fieldHeight) const;

    /// @brief 左クリックされたか（1フレームのみ）
    bool IsLeftClickPressed() const;

    /// @brief 右クリックされたか（1フレームのみ）
    bool IsRightClickPressed() const;

    // ========== キーボード状態 ==========

    /// @brief キーが押されたか（1フレームのみ）
    /// @param key キーコード（例: KEY_ESCAPE）
    bool IsKeyPressed(int key) const;

    /// @brief Escapeキーが押されたか
    bool IsEscapePressed() const;

    /// @brief スペースキーが押されたか
    bool IsSpacePressed() const;

    // ========== イベント処理 ==========

    /// @brief イベントがあるかチェック
    bool HasEvent() const { return hasEvent_; }

    /// @brief イベントを取得
    InputEvent GetEvent() const { return currentEvent_; }

    /// @brief イベントをクリア
    void ClearEvent() { hasEvent_ = false; }

    /// @brief フィールドクリックイベントを生成
    /// @param gx グリッドX座標
    /// @param gy グリッドY座標
    /// @param isRightClick 右クリックかどうか
    void GenerateFieldClickEvent(int gx, int gy, bool isRightClick = false);

    /// @brief ボタンクリックイベントを生成
    /// @param buttonId ボタンID
    void GenerateButtonClickEvent(const std::string& buttonId);

private:
    BaseSystemAPI* sysAPI_;
    
    // イベント管理
    bool hasEvent_;
    InputEvent currentEvent_;
    
    // マウス状態キャッシュ
    Vector2 mousePosition_;
    bool leftClickPressed_;
    bool rightClickPressed_;
};

} // namespace gamescene
} // namespace core
} // namespace game
