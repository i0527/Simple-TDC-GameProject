#pragma once

#include <string>
#include <vector>
#include "Editor/Windows/IEditorWindow.h"
#include "Editor/Windows/UnitEditorWindow.h"

namespace Editor::Windows { class UnitEditorWindow; }

namespace Shared::Core { class GameContext; }
namespace Shared::Data { class DefinitionRegistry; }

namespace Editor::Windows {

class ValidationPanel : public IEditorWindow {
public:
  ValidationPanel();
  ~ValidationPanel() override;

  void Initialize(Shared::Core::GameContext &context,
                  Shared::Data::DefinitionRegistry &definitions) override;
  void Shutdown() override;
  void OnUpdate(float deltaTime) override;
  void OnDrawUI() override;
  std::string GetWindowTitle() const override;
  std::string GetWindowId() const override;
  bool IsOpen() const override;
  void SetOpen(bool open) override;

  // 外部から検証実行を指示
  void RunValidation();
  void SetupFileWatches();
  void ClearFileWatches();

private:
  Shared::Core::GameContext* context_ = nullptr;
  Shared::Data::DefinitionRegistry* definitions_ = nullptr;
  UnitEditorWindow* unitEditor_ = nullptr;

  bool isOpen_ = true;
  bool lastOk_ = false;
  float lastRunSeconds_ = 0.0f;
  char filterBuf_[256];

  // オプション
  bool autoRevalidate_ = false;
  float intervalSeconds_ = 5.0f;
  float elapsedSeconds_ = 0.0f;
  bool tableView_ = false;

  bool watchFiles_ = false;
  std::vector<std::string> watchedPaths_;

  // 将来の他エディタ連携用（存在しなくても安全に無視）
  void FocusEntity(const std::string& id);
  void FocusSkill(const std::string& id);
  void FocusAbility(const std::string& id);
  void FocusStage(const std::string& id);
  void FocusWave(const std::string& id);

public:
  // EditorApp から連携用に設定
  void SetUnitEditor(UnitEditorWindow* w) { unitEditor_ = w; }
};

} // namespace Editor::Windows
