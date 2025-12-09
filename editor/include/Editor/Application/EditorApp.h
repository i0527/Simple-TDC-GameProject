#pragma once

#include <memory>

#include <imgui.h>
#include <raylib.h>

#include "Core/FontManager.h"
#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"

namespace Editor::Application {

/// @brief エディタアプリケーションのメインクラス
class EditorApp {
public:
  EditorApp();
  ~EditorApp();

  bool Initialize();
  void Run();
  void Shutdown();

private:
  // コンテキストと定義
  std::unique_ptr<Shared::Core::GameContext> context_;
  std::unique_ptr<Shared::Data::DefinitionRegistry> definitions_;

  // フォント
  std::unique_ptr<Shared::Core::FontManager> fontManager_;
  ImFont *defaultFont_ = nullptr;

  // ゲーム状態
  bool is_running_;
  float delta_time_;

  // 内部メソッド
  void Update(float delta_time);
  void Render();
  void RenderUI();
  void HandleResize();
  void LoadDefinitions();
};

} // namespace Editor::Application
