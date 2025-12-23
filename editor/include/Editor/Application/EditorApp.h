#pragma once

#include <memory>
#include <vector>

#include <imgui.h>
#include <raylib.h>

#include "Core/FontManager.h"
#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"
#include "Editor/Windows/IEditorWindow.h"
#include "Editor/Windows/DefinitionEditorWindow.h"
#include "Editor/Windows/UnitEditorWindow.h"
#include "Editor/Windows/SpriteEditorWindow.h"
#include "Editor/Windows/PreviewWindow.h"
#include "Editor/Windows/ValidationPanel.h"
#include "Editor/Windows/SearchPaletteWindow.h"

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

  // ウィンドウ
  std::vector<std::unique_ptr<Editor::Windows::IEditorWindow>> windows_;
  
  // 特定ウィンドウへのポインタ（連携用）
  Editor::Windows::UnitEditorWindow* unitEditor_ = nullptr;
  Editor::Windows::SpriteEditorWindow* spriteEditor_ = nullptr;
  Editor::Windows::PreviewWindow* previewWindow_ = nullptr;
  Editor::Windows::DefinitionEditorWindow* definitionEditor_ = nullptr;
  Editor::Windows::ValidationPanel* validationPanel_ = nullptr;
  Editor::Windows::SearchPaletteWindow* searchPalette_ = nullptr;

  // ゲーム状態
  bool is_running_;
  float delta_time_;
  
  // 内部メソッド
  void Update(float delta_time);
  void Render();
  void RenderUI();
  void InitializeEditorWindows();
  void UpdateEditorWindows(float delta_time);
  void RenderEditorWindows();
  void HandleResize();
  void LoadDefinitions();
};

} // namespace Editor::Application
