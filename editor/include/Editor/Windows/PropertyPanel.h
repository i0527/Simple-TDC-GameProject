#pragma once

#include "Editor/Windows/IEditorWindow.h"
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

private:
  bool is_open_ = true;
  entt::entity selected_entity_ = entt::null;
  const entt::registry *registry_ = nullptr;
};

} // namespace Editor::Windows

