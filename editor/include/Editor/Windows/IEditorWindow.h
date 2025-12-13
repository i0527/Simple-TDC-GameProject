#pragma once

#include <string>

namespace Shared::Core {
class GameContext;
}

namespace Shared::Data {
class DefinitionRegistry;
}

namespace Editor::Windows {

/// @brief エディタウィンドウ共通インターフェース
class IEditorWindow {
public:
  virtual ~IEditorWindow() = default;

  /// @brief 初期化（依存するコンテキストと定義レジストリを受け取る）
  virtual void Initialize(Shared::Core::GameContext &context,
                          Shared::Data::DefinitionRegistry &definitions) = 0;

  /// @brief ウィンドウ終了処理
  virtual void Shutdown() = 0;

  /// @brief フレーム更新
  virtual void OnUpdate(float deltaTime) = 0;

  /// @brief UI描画
  virtual void OnDrawUI() = 0;

  /// @brief タイトル取得
  virtual std::string GetWindowTitle() const = 0;

  /// @brief 識別子取得
  virtual std::string GetWindowId() const = 0;

  /// @brief 表示状態
  virtual bool IsOpen() const = 0;
  virtual void SetOpen(bool open) = 0;
};

} // namespace Editor::Windows

