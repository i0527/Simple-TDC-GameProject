#pragma once

#include "Editor/Windows/IEditorWindow.h"
#include "Data/Definitions/EntityDef.h"
#include <entt/entt.hpp>
#include <string>

namespace Editor::Windows {

class PropertyPanel : public IEditorWindow {
public:
  PropertyPanel();
  ~PropertyPanel() override = default;

  void Initialize(Shared::Core::GameContext &context,
                  Shared::Data::DefinitionRegistry &definitions) override;
  void Shutdown() override;
  void OnUpdate(float deltaTime) override;
  void OnDrawUI() override;
  std::string GetWindowTitle() const override { return "Properties"; }
  std::string GetWindowId() const override { return "property_panel"; }
  bool IsOpen() const override { return is_open_; }
  void SetOpen(bool open) override { is_open_ = open; }

  void SetSelection(entt::entity entity, const entt::registry *registry);
  void SelectEntity(const std::string& entity_id);  // ユニット定義から選択

private:
  bool is_open_ = true;
  entt::entity selected_entity_ = entt::null;
  const entt::registry *registry_ = nullptr;
  
  // ユニット定義表示用
  std::string selected_entity_def_id_;
  Shared::Data::DefinitionRegistry *definitions_ = nullptr;
  Shared::Core::GameContext *context_ = nullptr;
  
  // 編集用
  Shared::Data::EntityDef editing_def_;
  bool is_dirty_ = false;

  void DrawEntityProperties();
  void DrawEntityDefinitionProperties();
  void SaveCurrentEntity();
};

} // namespace Editor::Windows

