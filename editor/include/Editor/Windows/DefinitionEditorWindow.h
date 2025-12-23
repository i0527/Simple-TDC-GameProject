#pragma once

#include <array>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <optional>
#include <nlohmann/json.hpp>

#include <imgui.h>

#include "Data/Definitions/AbilityDef.h"
#include "Data/Definitions/EntityDef.h"
#include "Data/Definitions/SkillDef.h"
#include "Data/Definitions/StageDef.h"
#include "Data/Definitions/WaveDef.h"
#include "Data/Validators/DataValidator.h"
#include "Editor/Windows/IEditorWindow.h"

namespace Editor::Windows {

/// @brief 定義の閲覧・作成・検証を行う最小ウィンドウ
class DefinitionEditorWindow : public IEditorWindow {
public:
  DefinitionEditorWindow() = default;
  ~DefinitionEditorWindow() override = default;

  void Initialize(Shared::Core::GameContext &context,
                  Shared::Data::DefinitionRegistry &definitions) override;
  void Shutdown() override;
  void OnUpdate(float deltaTime) override;
  void OnDrawUI() override;
  
  // PropertyPanel 統合用
  void SetPropertyPanel(class PropertyPanel *panel) { propertyPanel_ = panel; }

  std::string GetWindowTitle() const override { return "Definitions"; }
  std::string GetWindowId() const override { return "definition_editor"; }
  bool IsOpen() const override { return isOpen_; }
  void SetOpen(bool open) override { isOpen_ = open; }

  // 外部からタブを開かせるための要求
  void FocusTabEntities();
  void FocusTabSkills();
  void FocusTabStages();
  void FocusTabWaves();
  void FocusTabAbilities();

private:
  enum class DefinitionTab { Entities, Skills, Stages, Waves, Abilities, Validate };

  std::optional<DefinitionTab> requestedTab_;

  // 依存
  Shared::Core::GameContext *context_ = nullptr;
  Shared::Data::DefinitionRegistry *definitions_ = nullptr;
  class PropertyPanel *propertyPanel_ = nullptr;

  // 状態
  bool isOpen_ = true;
  DefinitionTab activeTab_ = DefinitionTab::Entities;
  std::string statusMessage_;
  bool lastValidationOk_ = true;
  
  // Template & batch operation state
  std::vector<std::string> entityList_;  // List of all entity IDs
  int selectedEntityIdx_ = -1;  // For duplication/selection
  std::vector<int> selectedBatchIndices_;  // For batch operations
  
  // Search & filter
  std::array<char, 128> searchQuery_{};
  std::array<const char*, 4> typeFilters_{"all", "main", "sub", "enemy"};
  int selectedTypeFilter_ = 0;  // "all"
  int selectedRarityFilter_ = 0;  // 0 = all, 1-5 = specific
  bool showEnemyUnits_ = false;
  int sortMethod_ = 0;  // 0=ID, 1=Name, 2=Rarity, 3=Cost

  // 作成フォーム
  struct EntityForm {
    std::array<char, 64> id{};
    std::array<char, 128> name{};
    std::array<char, 256> description{};
    int rarity = 1;
    bool isEnemy = false;
    int cost = 0;
    float cooldown = 0.0f;
    int hp = 100;
    int attack = 10;
    float attackSpeed = 1.0f;
    int range = 100;
    float moveSpeed = 50.0f;
    int knockback = 0;
    // Display fields
    std::array<char, 256> atlasTexture{};
    std::array<char, 256> icon{};
    std::array<char, 64> type{"sub"}; // main or sub
    std::array<char, 256> spriteActionFile{}; // Aseprite JSONアニメーションファイル
    std::string idleAnimation = "animations.json";
    std::string walkAnimation = "animations.json";
    std::string attackAnimation = "animations.json";
  } entityForm_;

  struct SkillForm {
    std::array<char, 64> id{};
    std::array<char, 128> name{};
    std::array<char, 256> description{};
    std::array<char, 32> type{"passive"};
    float cooldown = 0.0f;
    float activationChance = 1.0f;
  } skillForm_;

  struct StageForm {
    std::array<char, 64> id{};
    std::array<char, 128> name{};
    std::array<char, 256> description{};
    int difficulty = 1;
    std::array<char, 64> domain{"World1"};
    int subdomain = 1;
  } stageForm_;

  struct WaveForm {
    std::array<char, 64> id{};
    std::array<char, 64> stageId{};
    int waveNumber = 1;
    std::array<char, 64> entityId{};
    int count = 1;
    float spawnInterval = 1.0f;
  } waveForm_;

  struct AbilityForm {
    std::array<char, 64> id{};
    std::array<char, 128> name{};
    std::array<char, 256> description{};
    std::array<char, 64> type{"stat_boost"};
    std::array<char, 64> statType{"hp"};
    float value = 0.0f;
    bool isPercentage = false;
  } abilityForm_;

  // 内部処理
  void DrawTabs();
  void DrawEntitiesTab();
  void DrawSkillsTab();
  void DrawStagesTab();
  void DrawWavesTab();
  void DrawAbilitiesTab();
  void DrawValidationTab();

  void DrawEntityCreateForm();
  void DrawSkillCreateForm();
  void DrawStageCreateForm();
  void DrawWaveCreateForm();
  void DrawAbilityCreateForm();

  // Entity save/load
  bool SaveEntityDefinition(const Shared::Data::EntityDef &def);
  void LoadEntityDefinitions();

  // Template, validation & batch
  void DuplicateEntity(const std::string &fromId);
  bool ValidateEntityDef(const Shared::Data::EntityDef &def, std::string &errorMsg);
  void RefreshEntityList();
  void DeleteSelectedEntities();
  void ExportSelectedEntities();
  
  // 参照チェック
  std::vector<std::string> CheckEntityReferences(const std::string& entityId);

  template <typename Fn> void WithTwoColumns(const char *id, Fn &&fn);
};

} // namespace Editor::Windows

