#pragma once

#include <string>
#include <vector>
#include "Editor/Windows/IEditorWindow.h"

namespace Shared::Core { class GameContext; }
namespace Shared::Data { class DefinitionRegistry; struct EntityDef; }

namespace Editor::Windows {
class UnitEditorWindow;
class SpriteEditorWindow;
class PreviewWindow;
class DefinitionEditorWindow;

class SearchPaletteWindow : public IEditorWindow {
public:
  SearchPaletteWindow();
  ~SearchPaletteWindow() override;

  void Initialize(Shared::Core::GameContext &context,
                  Shared::Data::DefinitionRegistry &definitions) override;
  void Shutdown() override {}
  void OnUpdate(float deltaTime) override;
  void OnDrawUI() override;
  std::string GetWindowTitle() const override { return "クイック検索"; }
  std::string GetWindowId() const override { return "search_palette"; }
  bool IsOpen() const override { return isOpen_; }
  void SetOpen(bool open) override { isOpen_ = open; if (open) focusInput_ = true; }

  void SetUnitEditor(UnitEditorWindow* w) { unitEditor_ = w; }
  void SetSpriteEditor(SpriteEditorWindow* w) { spriteEditor_ = w; }
  void SetPreviewWindow(PreviewWindow* w) { previewWindow_ = w; }
  void SetDefinitionEditor(DefinitionEditorWindow* w) { definitionEditor_ = w; }

private:
  Shared::Data::DefinitionRegistry* definitions_ = nullptr;

  UnitEditorWindow* unitEditor_ = nullptr;
  SpriteEditorWindow* spriteEditor_ = nullptr;
  PreviewWindow* previewWindow_ = nullptr;
  DefinitionEditorWindow* definitionEditor_ = nullptr;

  bool isOpen_ = false;
  bool focusInput_ = false;
  char query_[256];
  int selectedIndex_ = -1;
  bool togglePreviewPlayOnJump_ = false;

  enum class Kind { Entity, Skill, Stage, Wave, Ability };
  struct Item {
    Kind kind;
    std::string id;
    std::string name;
    std::string type;
  };
  std::vector<Item> items_;

  std::vector<std::string> recentEntities_;

  void RebuildList();
  void ActivateSelection(int idx);
  void PushRecentEntity(const std::string& id);
};

} // namespace Editor::Windows
