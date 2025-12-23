#include "Editor/Windows/AssetBrowserWindow.h"

#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"
#include <algorithm>
#include <imgui.h>
#include <string>

namespace Editor::Windows {

namespace {
// 小さなユーティリティ: forward slashで正規化
std::string ToGenericString(const std::filesystem::path& p) {
  return p.generic_string();
}
}

AssetBrowserWindow::AssetBrowserWindow(const std::string& root)
    : rootPath_(root) {}

void AssetBrowserWindow::Initialize(Shared::Core::GameContext &context,
                                    Shared::Data::DefinitionRegistry &definitions) {
  context_ = &context;
  definitions_ = &definitions;
}

void AssetBrowserWindow::OnDrawUI() {
  if (!isOpen_) return;

  ImGuiWindowFlags flags = ImGuiWindowFlags_None;
  if (ImGui::Begin(GetWindowTitle().c_str(), &isOpen_, flags)) {
    ImGui::Text("assetsフォルダのファイルをD&Dで入力欄へドロップできます。");
    ImGui::Separator();

    std::filesystem::path root(rootPath_);
    if (!std::filesystem::exists(root)) {
      ImGui::TextDisabled("Not found: %s", rootPath_.c_str());
    } else {
      DrawDirectory(root, 0);
    }
  }
  ImGui::End();
}

void AssetBrowserWindow::DrawDirectory(const std::filesystem::path& path, int depth) {
  if (!std::filesystem::exists(path)) return;

  std::vector<std::filesystem::directory_entry> entries;
  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    entries.push_back(entry);
  }

  // ディレクトリを先、ファイルを後にして並べる
  std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
    if (a.is_directory() != b.is_directory()) {
      return a.is_directory() && !b.is_directory();
    }
    return a.path().filename().string() < b.path().filename().string();
  });

  for (const auto& entry : entries) {
    const auto& p = entry.path();
    const std::string label = p.filename().string();

    if (entry.is_directory()) {
      ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
      bool open = ImGui::TreeNodeEx(p.string().c_str(), flags, "%s", label.c_str());
      if (open) {
        DrawDirectory(p, depth + 1);
        ImGui::TreePop();
      }
    } else if (entry.is_regular_file() && IsVisibleExtension(p)) {
      ImGui::Selectable(label.c_str(), false);
      MakeDragSource(p);
    }
  }
}

bool AssetBrowserWindow::IsVisibleExtension(const std::filesystem::path& p) const {
  std::string ext = p.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  return std::find(allowedExtensions_.begin(), allowedExtensions_.end(), ext) != allowedExtensions_.end();
}

void AssetBrowserWindow::MakeDragSource(const std::filesystem::path& path) const {
  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
    std::string payload = ToGenericString(path.lexically_normal());
    ImGui::SetDragDropPayload("ASSET_PATH", payload.c_str(), payload.size() + 1);
    ImGui::TextUnformatted(path.filename().string().c_str());
    ImGui::EndDragDropSource();
  }
}

} // namespace Editor::Windows
