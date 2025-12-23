#pragma once

#include "Editor/Windows/IEditorWindow.h"
#include <filesystem>
#include <string>
#include <vector>

namespace Shared {
namespace Core {
class GameContext;
}
namespace Data {
class DefinitionRegistry;
}
} // namespace Shared

namespace Editor::Windows {

class AssetBrowserWindow : public IEditorWindow {
public:
  explicit AssetBrowserWindow(const std::string& root = "assets");
  ~AssetBrowserWindow() override = default;

  void Initialize(Shared::Core::GameContext &context,
                  Shared::Data::DefinitionRegistry &definitions) override;
  void Shutdown() override {}
  void OnUpdate(float deltaTime) override {}
  void OnDrawUI() override;

  std::string GetWindowTitle() const override { return "アセットブラウザ"; }
  std::string GetWindowId() const override { return "asset_browser"; }
  bool IsOpen() const override { return isOpen_; }
  void SetOpen(bool open) override { isOpen_ = open; }

private:
  void DrawDirectory(const std::filesystem::path& path, int depth);
  bool IsVisibleExtension(const std::filesystem::path& p) const;
  void MakeDragSource(const std::filesystem::path& path) const;

  Shared::Core::GameContext* context_ = nullptr;
  Shared::Data::DefinitionRegistry* definitions_ = nullptr;
  std::string rootPath_;
  bool isOpen_ = true;
  std::vector<std::string> allowedExtensions_ = {
      ".png", ".jpg", ".jpeg", ".bmp", ".ase", ".json", ".wav", ".ogg", ".mp3"};
};

} // namespace Editor::Windows
