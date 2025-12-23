#include "Editor/Windows/DefinitionEditorWindow.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>

#include <raylib.h>

#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"
#include "Data/Loaders/EntityLoader.h"
#include "Data/Loaders/SkillLoader.h"
#include "Data/Loaders/StageLoader.h"
#include "Data/Loaders/WaveLoader.h"
#include "Data/Loaders/AbilityLoader.h"
#include "Editor/Windows/PropertyPanel.h"
#include <filesystem>

namespace Editor::Windows {

namespace {
constexpr float kLeftColumnWidth = 160.0f;

bool IsEmpty(const std::array<char, 64> &buf) { return buf[0] == '\0'; }
bool IsEmpty(const std::array<char, 128> &buf) { return buf[0] == '\0'; }
bool IsEmpty(const std::array<char, 256> &buf) { return buf[0] == '\0'; }

template <size_t N>
std::string ToString(const std::array<char, N> &buf) {
  return std::string(buf.data());
}

template <size_t N>
void CopyToBuffer(std::array<char, N> &buf, const std::string &src) {
  if (src.empty()) {
    buf.fill('\0');
    return;
  }
  size_t copy_len = std::min(src.length(), buf.size() - 1);
  std::copy(src.begin(), src.begin() + copy_len, buf.begin());
  buf[copy_len] = '\0';
}
} // namespace

void DefinitionEditorWindow::Initialize(Shared::Core::GameContext &context,
                                        Shared::Data::DefinitionRegistry &definitions) {
  context_ = &context;
  definitions_ = &definitions;
  statusMessage_.clear();
  
  // 初期化時にエンティティリストを構築
  RefreshEntityList();
  
  std::cout << "[DefinitionEditor] Initialized with " << entityList_.size() << " entities." << std::endl;
}

void DefinitionEditorWindow::Shutdown() {
  // 今のところ解放すべきリソースなし
}

void DefinitionEditorWindow::OnUpdate(float deltaTime) {
  (void)deltaTime;
  // 現状更新処理はなし（将来の拡張ポイント）
}

void DefinitionEditorWindow::OnDrawUI() {
  if (!isOpen_) {
    return;
  }

  ImGui::SetNextWindowSize(ImVec2(960, 640), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(GetWindowTitle().c_str(), &isOpen_)) {
    ImGui::End();
    return;
  }

  DrawTabs();

  if (!statusMessage_.empty()) {
    ImGui::Separator();
    ImGui::TextWrapped("%s", statusMessage_.c_str());
  }

  ImGui::End();
}

void DefinitionEditorWindow::DrawTabs() {
  if (ImGui::BeginTabBar("definition_tabs")) {
    auto tabFlag = [&](DefinitionTab tab){
      return (requestedTab_.has_value() && requestedTab_.value() == tab)
                 ? ImGuiTabItemFlags_SetSelected
                 : ImGuiTabItemFlags_None;
    };

    if (ImGui::BeginTabItem("Entities", nullptr, tabFlag(DefinitionTab::Entities))) {
      activeTab_ = DefinitionTab::Entities;
      DrawEntitiesTab();
      if (requestedTab_ == DefinitionTab::Entities) requestedTab_.reset();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Skills", nullptr, tabFlag(DefinitionTab::Skills))) {
      activeTab_ = DefinitionTab::Skills;
      DrawSkillsTab();
      if (requestedTab_ == DefinitionTab::Skills) requestedTab_.reset();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Stages", nullptr, tabFlag(DefinitionTab::Stages))) {
      activeTab_ = DefinitionTab::Stages;
      DrawStagesTab();
      if (requestedTab_ == DefinitionTab::Stages) requestedTab_.reset();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Waves", nullptr, tabFlag(DefinitionTab::Waves))) {
      activeTab_ = DefinitionTab::Waves;
      DrawWavesTab();
      if (requestedTab_ == DefinitionTab::Waves) requestedTab_.reset();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Abilities", nullptr, tabFlag(DefinitionTab::Abilities))) {
      activeTab_ = DefinitionTab::Abilities;
      DrawAbilitiesTab();
      if (requestedTab_ == DefinitionTab::Abilities) requestedTab_.reset();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Validation")) {
      activeTab_ = DefinitionTab::Validate;
      DrawValidationTab();
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
}

void DefinitionEditorWindow::FocusTabEntities() { requestedTab_ = DefinitionTab::Entities; SetOpen(true); }
void DefinitionEditorWindow::FocusTabSkills() { requestedTab_ = DefinitionTab::Skills; SetOpen(true); }
void DefinitionEditorWindow::FocusTabStages() { requestedTab_ = DefinitionTab::Stages; SetOpen(true); }
void DefinitionEditorWindow::FocusTabWaves() { requestedTab_ = DefinitionTab::Waves; SetOpen(true); }
void DefinitionEditorWindow::FocusTabAbilities() { requestedTab_ = DefinitionTab::Abilities; SetOpen(true); }

template <typename Fn>
void DefinitionEditorWindow::WithTwoColumns(const char *id, Fn &&fn) {
  if (ImGui::BeginTable(id, 2, ImGuiTableFlags_BordersInnerV)) {
    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, kLeftColumnWidth);
    ImGui::TableSetupColumn("Value");
    fn();
    ImGui::EndTable();
  }
}

void DefinitionEditorWindow::DrawEntitiesTab() {
  ImGui::TextUnformatted("登録済みエンティティ");
  ImGui::Separator();
  
  // 検索・フィルタバー
  ImGui::SetNextItemWidth(200);
  ImGui::InputTextWithHint("##search", "ユニット検索...", searchQuery_.data(), searchQuery_.size());
  ImGui::SameLine();
  ImGui::SetNextItemWidth(120);
  ImGui::Combo("##typeFilter", &selectedTypeFilter_, typeFilters_.data(), static_cast<int>(typeFilters_.size()));
  ImGui::SameLine();
  ImGui::SetNextItemWidth(100);
  const char* rarityItems[] = {"全レアリティ", "★", "★★", "★★★", "★★★★", "★★★★★"};
  ImGui::Combo("##rarityFilter", &selectedRarityFilter_, rarityItems, 6);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(100);
  const char* sortItems[] = {"ID順", "名前順", "レアリティ順", "コスト順"};
  ImGui::Combo("##sort", &sortMethod_, sortItems, 4);
  
  ImGui::Separator();
  
  // フィルタ処理
  std::string search_query = searchQuery_.data();
  std::string type_filter = selectedTypeFilter_ == 0 ? "" : typeFilters_[selectedTypeFilter_];
  
  // ユニット一覧の取得と フィルタリング
  const auto all_entities = definitions_->GetAllEntities();
  std::vector<const Shared::Data::EntityDef*> filtered_entities;
  
  for (const auto *entity : all_entities) {
    if (!entity) continue;
    
    // 検索フィルタ
    if (!search_query.empty()) {
      if (entity->id.find(search_query) == std::string::npos &&
          entity->name.find(search_query) == std::string::npos) {
        continue;
      }
    }
    
    // Type フィルタ
    if (!type_filter.empty() && entity->type != type_filter) {
      continue;
    }
    
    // Rarity フィルタ
    if (selectedRarityFilter_ > 0 && entity->rarity != selectedRarityFilter_) {
      continue;
    }
    
    filtered_entities.push_back(entity);
  }
  
  // 並び替え
  std::sort(filtered_entities.begin(), filtered_entities.end(),
    [this](const Shared::Data::EntityDef* a, const Shared::Data::EntityDef* b) {
      switch (sortMethod_) {
        case 1: return a->name < b->name;
        case 2: return a->rarity > b->rarity;
        case 3: return a->cost < b->cost;
        default: return a->id < b->id;
      }
    });
  
  // ユニット一覧テーブル
  if (ImGui::BeginTable("entity_list", 6, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
                                              ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY,
                        ImVec2(0, 300))) {
    ImGui::TableSetupColumn("選択");
    ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 120);
    ImGui::TableSetupColumn("名前", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 60);
    ImGui::TableSetupColumn("レアリティ", ImGuiTableColumnFlags_WidthFixed, 80);
    ImGui::TableSetupColumn("操作", ImGuiTableColumnFlags_WidthFixed, 140);
    ImGui::TableHeadersRow();

    for (size_t idx = 0; idx < filtered_entities.size(); ++idx) {
      const auto *entity = filtered_entities[idx];
      ImGui::TableNextRow();
      
      // 行全体をクリック可能にする
      ImGui::PushID(static_cast<int>(idx));
      if (ImGui::Selectable("##row", false, ImGuiSelectableFlags_SpanAllColumns)) {
        if (propertyPanel_) {
          propertyPanel_->SelectEntity(entity->id);
        }
      }
      ImGui::PopID();
      
      // チェックボックス
      ImGui::TableSetColumnIndex(0);
      bool is_selected = std::find(selectedBatchIndices_.begin(), selectedBatchIndices_.end(), 
                                   (int)idx) != selectedBatchIndices_.end();
      if (ImGui::Checkbox(("##sel_" + std::to_string(idx)).c_str(), &is_selected)) {
        if (is_selected) {
          selectedBatchIndices_.push_back((int)idx);
        } else {
          auto it = std::find(selectedBatchIndices_.begin(), selectedBatchIndices_.end(), (int)idx);
          if (it != selectedBatchIndices_.end()) {
            selectedBatchIndices_.erase(it);
          }
        }
      }
      
      // ID
      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(entity->id.c_str());
      
      // 名前
      ImGui::TableSetColumnIndex(2);
      ImGui::TextUnformatted(entity->name.c_str());
      
      // Type
      ImGui::TableSetColumnIndex(3);
      ImGui::TextUnformatted(entity->type.c_str());
      
      // レアリティ（★表示）
      ImGui::TableSetColumnIndex(4);
      std::string rarity_str(entity->rarity, '*');
      ImGui::TextUnformatted(rarity_str.c_str());
      
      // 操作ボタン
      ImGui::TableSetColumnIndex(5);
      ImGui::PushID(entity->id.c_str());
      if (ImGui::Button("複製", ImVec2(50, 0))) {
        DuplicateEntity(entity->id);
      }
      ImGui::SameLine();
      if (ImGui::Button("削除", ImVec2(50, 0))) {
        selectedBatchIndices_.clear();
        selectedBatchIndices_.push_back((int)idx);
        DeleteSelectedEntities();
      }
      ImGui::PopID();
    }
    ImGui::EndTable();
  }
  
  // 統計表示
  ImGui::Text("フィルタ結果: %zu / %zu ユニット", filtered_entities.size(), all_entities.size());

  ImGui::Separator();
  if (!selectedBatchIndices_.empty()) {
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "選択中: %zu 件", selectedBatchIndices_.size());
    ImGui::SameLine();
    if (ImGui::Button("選択削除##batch", ImVec2(100, 0))) {
      DeleteSelectedEntities();
    }
    ImGui::SameLine();
    if (ImGui::Button("選択複製##batch", ImVec2(100, 0))) {
      // 選択された最初のユニットを複製
      if (!selectedBatchIndices_.empty() && selectedBatchIndices_[0] < static_cast<int>(filtered_entities.size())) {
        DuplicateEntity(filtered_entities[selectedBatchIndices_[0]]->id);
      }
    }
  }

  ImGui::Separator();
  ImGui::TextUnformatted("🔧 エンティティ作成");
  ImGui::Separator();
  DrawEntityCreateForm();
}

void DefinitionEditorWindow::DrawSkillsTab() {
  ImGui::TextUnformatted("登録済みスキル");
  if (ImGui::BeginTable("skill_list", 3,
                        ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV)) {
    ImGui::TableSetupColumn("ID");
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Type");
    ImGui::TableHeadersRow();

    const auto skills = definitions_->GetAllSkills();
    for (const auto *skill : skills) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted(skill->id.c_str());
      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(skill->name.c_str());
      ImGui::TableSetColumnIndex(2);
      ImGui::TextUnformatted(skill->type.c_str());
    }
    ImGui::EndTable();
  }

  ImGui::Separator();
  ImGui::TextUnformatted("スキル作成（簡易）");
  DrawSkillCreateForm();
}

void DefinitionEditorWindow::DrawStagesTab() {
  ImGui::TextUnformatted("登録済みステージ");
  if (ImGui::BeginTable("stage_list", 3,
                        ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV)) {
    ImGui::TableSetupColumn("ID");
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Domain");
    ImGui::TableHeadersRow();

    const auto stages = definitions_->GetAllStages();
    for (const auto *stage : stages) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted(stage->id.c_str());
      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(stage->name.c_str());
      ImGui::TableSetColumnIndex(2);
      ImGui::TextUnformatted(stage->domain.c_str());
    }
    ImGui::EndTable();
  }

  ImGui::Separator();
  ImGui::TextUnformatted("ステージ作成（簡易）");
  DrawStageCreateForm();
}

void DefinitionEditorWindow::DrawWavesTab() {
  ImGui::TextUnformatted("登録済みWave");
  if (ImGui::BeginTable("wave_list", 3,
                        ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV)) {
    ImGui::TableSetupColumn("ID");
    ImGui::TableSetupColumn("Stage");
    ImGui::TableSetupColumn("Wave #");
    ImGui::TableHeadersRow();

    const auto waves = definitions_->GetAllWaves();
    for (const auto *wave : waves) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted(wave->id.c_str());
      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(wave->stage_id.c_str());
      ImGui::TableSetColumnIndex(2);
      ImGui::Text("%d", wave->wave_number);
    }
    ImGui::EndTable();
  }

  ImGui::Separator();
  ImGui::TextUnformatted("Wave作成（簡易）");
  DrawWaveCreateForm();
}

void DefinitionEditorWindow::DrawAbilitiesTab() {
  ImGui::TextUnformatted("登録済みアビリティ");
  if (ImGui::BeginTable("ability_list", 3,
                        ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV)) {
    ImGui::TableSetupColumn("ID");
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Type");
    ImGui::TableHeadersRow();

    const auto abilities = definitions_->GetAllAbilities();
    for (const auto *ability : abilities) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted(ability->id.c_str());
      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(ability->name.c_str());
      ImGui::TableSetColumnIndex(2);
      ImGui::TextUnformatted(ability->type.c_str());
    }
    ImGui::EndTable();
  }

  ImGui::Separator();
  ImGui::TextUnformatted("アビリティ作成（簡易）");
  DrawAbilityCreateForm();
}

void DefinitionEditorWindow::DrawValidationTab() {
  ImGui::TextUnformatted("定義のバリデーション");
  if (ImGui::Button("Validate definitions")) {
    Shared::Data::DataValidator::ClearErrors();
    lastValidationOk_ = Shared::Data::DataValidator::Validate(*definitions_);
    const auto &errors = Shared::Data::DataValidator::GetErrors();
    if (lastValidationOk_) {
      statusMessage_ = "Validation succeeded.";
    } else if (!errors.empty()) {
      statusMessage_ = errors.front();
    } else {
      statusMessage_ = "Validation failed.";
    }
  }

  ImGui::SameLine();
  ImGui::TextUnformatted("| バッチ操作:");
  ImGui::SameLine();
  if (ImGui::Button("全ユニット更新リスト")) {
    RefreshEntityList();
    statusMessage_ = "ユニットリスト更新: " + std::to_string(entityList_.size()) + " 件";
  }
  ImGui::SameLine();
  if (ImGui::Button("エクスポート")) {
    ExportSelectedEntities();
  }

  const auto &errors = Shared::Data::DataValidator::GetErrors();
  if (errors.empty() && lastValidationOk_) {
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "エラーはありません。");
    return;
  }

  ImGui::Separator();
  ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.3f, 1.0f), "エラー一覧");
  for (const auto &err : errors) {
    ImGui::BulletText("%s", err.c_str());
  }
}

void DefinitionEditorWindow::DrawEntityCreateForm() {
  // 基本情報セクション
  if (ImGui::CollapsingHeader("📋 基本情報", ImGuiTreeNodeFlags_DefaultOpen)) {
    WithTwoColumns("entity_form_basic", [this]() {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("ID");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputText("##entity_id", entityForm_.id.data(), entityForm_.id.size());

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("名前");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputText("##entity_name", entityForm_.name.data(), entityForm_.name.size());

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("説明");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputTextMultiline("##entity_desc", entityForm_.description.data(),
                                entityForm_.description.size(), ImVec2(-FLT_MIN, 50));

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("Type");
      ImGui::TableSetColumnIndex(1);
      const char* type_items[] = {"main", "sub", "enemy"};
      int type_idx = std::string(entityForm_.type.data()) == "main" ? 0 : 
                     std::string(entityForm_.type.data()) == "sub" ? 1 : 2;
      if (ImGui::Combo("##entity_type", &type_idx, type_items, 3)) {
        std::string type_str = type_items[type_idx];
        std::fill(entityForm_.type.begin(), entityForm_.type.end(), 0);
        std::copy(type_str.begin(), type_str.end(), entityForm_.type.begin());
      }

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("レアリティ");
      ImGui::TableSetColumnIndex(1);
      ImGui::SliderInt("##entity_rarity", &entityForm_.rarity, 1, 5);
      ImGui::SameLine();
      ImGui::TextUnformatted(std::string(entityForm_.rarity, '*').c_str());

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("コスト");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputInt("##entity_cost", &entityForm_.cost);
    });
  }

  // ステータスセクション
  if (ImGui::CollapsingHeader("⚔️ ステータス", ImGuiTreeNodeFlags_DefaultOpen)) {
    WithTwoColumns("entity_form_stats", [this]() {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("HP");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputInt("##entity_hp", &entityForm_.hp);

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("攻撃力");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputInt("##entity_atk", &entityForm_.attack);

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("防御力");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputInt("##entity_def", &entityForm_.knockback);

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("攻撃速度");
      ImGui::TableSetColumnIndex(1);
      ImGui::SliderFloat("##entity_as", &entityForm_.attackSpeed, 0.1f, 3.0f, "%.2f");

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("移動速度");
      ImGui::TableSetColumnIndex(1);
      ImGui::SliderFloat("##entity_ms", &entityForm_.moveSpeed, 0.0f, 200.0f, "%.1f");

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("範囲");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputInt("##entity_range", &entityForm_.range);

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("クールダウン");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputFloat("##entity_cd", &entityForm_.cooldown);
    });
  }

  // グラフィックスセクション
  if (ImGui::CollapsingHeader("🎨 グラフィックス", ImGuiTreeNodeFlags_DefaultOpen)) {
    WithTwoColumns("entity_form_graphics", [this]() {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("アイコン");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputText("##entity_icon", entityForm_.icon.data(), entityForm_.icon.size());
      ImGui::SameLine();
      if (ImGui::Button("📁##icon")) {
        // ファイルダイアログのプレースホルダー
        ImGui::OpenPopup("icon_browser");
      }

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("アトラステクスチャ");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputText("##entity_atlas", entityForm_.atlasTexture.data(), entityForm_.atlasTexture.size());
      ImGui::SameLine();
      if (ImGui::Button("📁##atlas")) {
        ImGui::OpenPopup("atlas_browser");
      }

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted("スプライトアクション");
      ImGui::TableSetColumnIndex(1);
      ImGui::InputText("##entity_actions", entityForm_.spriteActionFile.data(), 
                       entityForm_.spriteActionFile.size());
      ImGui::SameLine();
      if (ImGui::Button("📁##actions")) {
        ImGui::OpenPopup("actions_browser");
      }
    });
  }

  ImGui::Separator();

  // 作成ボタン
  ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 240);
  if (ImGui::Button("🔄 リセット", ImVec2(100, 0))) {
    std::fill(entityForm_.id.begin(), entityForm_.id.end(), 0);
    std::fill(entityForm_.name.begin(), entityForm_.name.end(), 0);
    std::fill(entityForm_.description.begin(), entityForm_.description.end(), 0);
    entityForm_.rarity = 1;
    entityForm_.hp = 100;
    entityForm_.attack = 10;
    entityForm_.attackSpeed = 1.0f;
  }

  ImGui::SameLine();
  if (ImGui::Button("✨ 作成", ImVec2(100, 0))) {
    if (IsEmpty(entityForm_.id) || IsEmpty(entityForm_.name)) {
      statusMessage_ = "エラー: ID と名前は必須です。";
      return;
    }
    Shared::Data::EntityDef def;
    def.id = ToString(entityForm_.id);
    def.name = ToString(entityForm_.name);
    def.description = ToString(entityForm_.description);
    def.rarity = std::max(1, std::min(5, entityForm_.rarity));
    def.is_enemy = ToString(entityForm_.type) == "enemy";
    def.cost = entityForm_.cost;
    def.cooldown = entityForm_.cooldown;
    def.stats.hp = entityForm_.hp;
    def.stats.attack = entityForm_.attack;
    def.stats.attack_speed = entityForm_.attackSpeed;
    def.stats.range = entityForm_.range;
    def.stats.move_speed = entityForm_.moveSpeed;
    def.stats.knockback = entityForm_.knockback;
    def.type = std::string(entityForm_.type.data());
    def.draw_type = "sprite";
    
    // Display fields
    if (!IsEmpty(entityForm_.atlasTexture)) {
      def.display.atlas_texture = ToString(entityForm_.atlasTexture);
    }
    if (!IsEmpty(entityForm_.icon)) {
      def.display.icon = ToString(entityForm_.icon);
    }
    if (!IsEmpty(entityForm_.spriteActionFile)) {
      std::string action_file = ToString(entityForm_.spriteActionFile);
      def.display.sprite_actions["idle"] = action_file;
      def.display.sprite_actions["walk"] = action_file;
      def.display.sprite_actions["attack"] = action_file;
      def.display.sprite_actions["death"] = action_file;
    }

    if (definitions_->HasEntity(def.id)) {
      statusMessage_ = "エラー: 同じ ID のエンティティが既に存在します。";
      return;
    }

    // Validate entity definition
    std::string validationError;
    if (!ValidateEntityDef(def, validationError)) {
      statusMessage_ = "検証失敗: " + validationError;
      return;
    }

    if (definitions_->RegisterEntity(def)) {
      // Save to file
      if (SaveEntityDefinition(def)) {
        statusMessage_ = "✅ ユニットを作成・保存しました: " + def.id;
        // リセット
        std::fill(entityForm_.id.begin(), entityForm_.id.end(), 0);
        std::fill(entityForm_.name.begin(), entityForm_.name.end(), 0);
      } else {
        statusMessage_ = "⚠️ ユニットを追加しましたが、保存に失敗: " + def.id;
      }
    } else {
      statusMessage_ = "❌ ユニットの追加に失敗しました。";
    }
  }
}

void DefinitionEditorWindow::DrawSkillCreateForm() {
  WithTwoColumns("skill_form", [this]() {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("ID");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##skill_id", skillForm_.id.data(), skillForm_.id.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Name");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##skill_name", skillForm_.name.data(), skillForm_.name.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Description");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputTextMultiline("##skill_desc", skillForm_.description.data(),
                              skillForm_.description.size(), ImVec2(-FLT_MIN, 80));

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Type");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##skill_type", skillForm_.type.data(), skillForm_.type.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Cooldown");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputFloat("##skill_cd", &skillForm_.cooldown);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Activation Chance");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputFloat("##skill_chance", &skillForm_.activationChance);
  });

  if (ImGui::Button("Create Skill")) {
    if (IsEmpty(skillForm_.id) || IsEmpty(skillForm_.name)) {
      statusMessage_ = "Skill: ID と Name は必須です。";
      return;
    }
    Shared::Data::SkillDef def;
    def.id = ToString(skillForm_.id);
    def.name = ToString(skillForm_.name);
    def.description = ToString(skillForm_.description);
    def.type = ToString(skillForm_.type);
    def.cooldown = std::max(0.0f, skillForm_.cooldown);
    def.activation_chance = std::clamp(skillForm_.activationChance, 0.0f, 1.0f);

    if (definitions_->HasSkill(def.id)) {
      statusMessage_ = "同じIDのスキルが既に存在します。";
      return;
    }

    if (definitions_->RegisterSkill(def)) {
      // Save to file
      std::string base_dir = context_->GetDataPath("definitions/skills");
      std::filesystem::create_directories(base_dir);
      std::string filepath = base_dir + "/" + def.id + ".json";
      
      if (Shared::Data::SkillLoader::SaveSingleSkill(filepath, def)) {
        statusMessage_ = "Skill を追加・保存しました: " + def.id;
      } else {
        statusMessage_ = "Skill を追加しましたが、保存に失敗しました。";
      }
    } else {
      statusMessage_ = "Skill の追加に失敗しました。";
    }
  }
}

void DefinitionEditorWindow::DrawStageCreateForm() {
  WithTwoColumns("stage_form", [this]() {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("ID");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##stage_id", stageForm_.id.data(), stageForm_.id.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Name");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##stage_name", stageForm_.name.data(), stageForm_.name.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Description");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputTextMultiline("##stage_desc", stageForm_.description.data(),
                              stageForm_.description.size(), ImVec2(-FLT_MIN, 80));

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Difficulty");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputInt("##stage_diff", &stageForm_.difficulty);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Domain");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##stage_domain", stageForm_.domain.data(), stageForm_.domain.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Subdomain");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputInt("##stage_subdomain", &stageForm_.subdomain);
  });

  if (ImGui::Button("Create Stage")) {
    if (IsEmpty(stageForm_.id) || IsEmpty(stageForm_.name)) {
      statusMessage_ = "Stage: ID と Name は必須です。";
      return;
    }
    Shared::Data::StageDef def;
    def.id = ToString(stageForm_.id);
    def.name = ToString(stageForm_.name);
    def.description = ToString(stageForm_.description);
    def.difficulty = std::max(1, stageForm_.difficulty);
    def.domain = ToString(stageForm_.domain);
    def.subdomain = std::max(0, stageForm_.subdomain);

    if (definitions_->HasStage(def.id)) {
      statusMessage_ = "同じIDのステージが既に存在します。";
      return;
    }

    if (definitions_->RegisterStage(def)) {
      // Save to file
      std::string base_dir = context_->GetDataPath("definitions/stages");
      std::filesystem::create_directories(base_dir);
      std::string filepath = base_dir + "/" + def.id + ".json";
      
      if (Shared::Data::StageLoader::SaveSingleStage(filepath, def)) {
        statusMessage_ = "Stage を追加・保存しました: " + def.id;
      } else {
        statusMessage_ = "Stage を追加しましたが、保存に失敗しました。";
      }
    } else {
      statusMessage_ = "Stage の追加に失敗しました。";
    }
  }
}

void DefinitionEditorWindow::DrawWaveCreateForm() {
  WithTwoColumns("wave_form", [this]() {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("ID");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##wave_id", waveForm_.id.data(), waveForm_.id.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Stage ID");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##wave_stage_id", waveForm_.stageId.data(), waveForm_.stageId.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Wave Number");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputInt("##wave_number", &waveForm_.waveNumber);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Entity ID");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##wave_entity_id", waveForm_.entityId.data(), waveForm_.entityId.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Count");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputInt("##wave_count", &waveForm_.count);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Spawn Interval");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputFloat("##wave_interval", &waveForm_.spawnInterval);
  });

  if (ImGui::Button("Create Wave")) {
    if (IsEmpty(waveForm_.id) || IsEmpty(waveForm_.stageId) || IsEmpty(waveForm_.entityId)) {
      statusMessage_ = "Wave: ID / Stage ID / Entity ID は必須です。";
      return;
    }
    if (!definitions_->HasStage(ToString(waveForm_.stageId))) {
      statusMessage_ = "指定された Stage ID が存在しません。";
      return;
    }
    if (!definitions_->HasEntity(ToString(waveForm_.entityId))) {
      statusMessage_ = "指定された Entity ID が存在しません。";
      return;
    }

    Shared::Data::WaveDef def;
    def.id = ToString(waveForm_.id);
    def.stage_id = ToString(waveForm_.stageId);
    def.wave_number = std::max(1, waveForm_.waveNumber);
    Shared::Data::WaveDef::SpawnGroup group;
    group.entity_id = ToString(waveForm_.entityId);
    group.count = std::max(1, waveForm_.count);
    group.spawn_interval = std::max(0.0f, waveForm_.spawnInterval);
    def.spawn_groups.push_back(group);

    if (definitions_->HasWave(def.id)) {
      statusMessage_ = "同じIDのWaveが既に存在します。";
      return;
    }

    if (definitions_->RegisterWave(def)) {
      // Save to file
      std::string base_dir = context_->GetDataPath("definitions/waves");
      std::filesystem::create_directories(base_dir);
      std::string filepath = base_dir + "/" + def.id + ".json";
      
      if (Shared::Data::WaveLoader::SaveSingleWave(filepath, def)) {
        statusMessage_ = "Wave を追加・保存しました: " + def.id;
      } else {
        statusMessage_ = "Wave を追加しましたが、保存に失敗しました。";
      }
    } else {
      statusMessage_ = "Wave の追加に失敗しました。";
    }
  }
}

void DefinitionEditorWindow::DrawAbilityCreateForm() {
  WithTwoColumns("ability_form", [this]() {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("ID");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##ability_id", abilityForm_.id.data(), abilityForm_.id.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Name");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##ability_name", abilityForm_.name.data(), abilityForm_.name.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Description");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputTextMultiline("##ability_desc", abilityForm_.description.data(),
                              abilityForm_.description.size(), ImVec2(-FLT_MIN, 80));

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Type");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##ability_type", abilityForm_.type.data(), abilityForm_.type.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Stat Type");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##ability_stat", abilityForm_.statType.data(), abilityForm_.statType.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Value");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputFloat("##ability_value", &abilityForm_.value);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Is Percentage");
    ImGui::TableSetColumnIndex(1);
    ImGui::Checkbox("##ability_percent", &abilityForm_.isPercentage);
  });

  if (ImGui::Button("Create Ability")) {
    if (IsEmpty(abilityForm_.id) || IsEmpty(abilityForm_.name)) {
      statusMessage_ = "Ability: ID と Name は必須です。";
      return;
    }

    Shared::Data::AbilityDef def;
    def.id = ToString(abilityForm_.id);
    def.name = ToString(abilityForm_.name);
    def.description = ToString(abilityForm_.description);
    def.type = ToString(abilityForm_.type);
    def.effect.stat_type = ToString(abilityForm_.statType);
    def.effect.value = abilityForm_.value;
    def.effect.is_percentage = abilityForm_.isPercentage;

    if (definitions_->HasAbility(def.id)) {
      statusMessage_ = "同じIDのアビリティが既に存在します。";
      return;
    }

    if (definitions_->RegisterAbility(def)) {
      // Save to file
      std::string base_dir = context_->GetDataPath("definitions/abilities");
      std::filesystem::create_directories(base_dir);
      std::string filepath = base_dir + "/" + def.id + ".json";
      
      if (Shared::Data::AbilityLoader::SaveSingleAbility(filepath, def)) {
        statusMessage_ = "Ability を追加・保存しました: " + def.id;
      } else {
        statusMessage_ = "Ability を追加しましたが、保存に失敗しました。";
      }
    } else {
      statusMessage_ = "Ability の追加に失敗しました。";
    }
  }
}

bool DefinitionEditorWindow::SaveEntityDefinition(const Shared::Data::EntityDef &def) {
  if (!context_) return false;

  try {
    namespace fs = std::filesystem;
    
    // Create or use entity save directory
    std::string base_chars_dir = context_->GetDataPath("entities/characters");
    fs::create_directories(base_chars_dir);

    // Save as individual JSON: {id}.json
    std::string filename = def.id + ".json";
    std::string filepath = base_chars_dir + "/" + filename;

    nlohmann::json j;
    j["id"] = def.id;
    j["name"] = def.name;
    j["description"] = def.description;
    j["rarity"] = def.rarity;
    j["type"] = def.type;
    j["is_enemy"] = def.is_enemy;
    j["cost"] = def.cost;
    j["cooldown"] = def.cooldown;
    j["draw_type"] = def.draw_type;

    j["stats"]["hp"] = def.stats.hp;
    j["stats"]["attack"] = def.stats.attack;
    j["stats"]["attack_speed"] = def.stats.attack_speed;
    j["stats"]["range"] = def.stats.range;
    j["stats"]["move_speed"] = def.stats.move_speed;
    j["stats"]["knockback"] = def.stats.knockback;

    j["display"]["atlas_texture"] = def.display.atlas_texture;
    j["display"]["icon"] = def.display.icon;
    j["display"]["sprite_actions"] = def.display.sprite_actions;
    j["skill_ids"] = def.skill_ids;
    j["ability_ids"] = def.ability_ids;
    j["tags"] = def.tags;

    std::ofstream file(filepath);
    if (!file.is_open()) {
      std::cerr << "[DefinitionEditor] Failed to open file for writing: " << filepath << std::endl;
      return false;
    }

    file << j.dump(2);
    file.close();

    std::cout << "[DefinitionEditor] Saved entity definition: " << filepath << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "[DefinitionEditor] Error saving entity: " << e.what() << std::endl;
    return false;
  }
}

void DefinitionEditorWindow::LoadEntityDefinitions() {
  if (!context_) return;

  try {
    namespace fs = std::filesystem;
    
    std::string base_chars_dir = context_->GetDataPath("entities/characters");
    
    if (!fs::exists(base_chars_dir) || !fs::is_directory(base_chars_dir)) {
      std::cout << "[DefinitionEditor] No character defs directory: " << base_chars_dir << std::endl;
      return;
    }

    int count = 0;
    for (const auto &entry : fs::directory_iterator(base_chars_dir)) {
      if (!entry.is_regular_file() || entry.path().extension() != ".json") {
        continue;
      }

      try {
        std::ifstream file(entry.path());
        if (!file.is_open()) continue;

        nlohmann::json j = nlohmann::json::parse(file);
        file.close();

        // JSON から EntityDef をロード
        if (j.contains("id") && j.contains("name")) {
          if (Shared::Data::EntityLoader::LoadFromJson(entry.path().string(), *definitions_)) {
            count++;
          }
        }
      } catch (const std::exception &e) {
        std::cerr << "[DefinitionEditor] Error loading " << entry.path() << ": " << e.what() << std::endl;
      }
    }

    if (count > 0) {
      std::cout << "[DefinitionEditor] Loaded " << count << " entity definitions from " << base_chars_dir << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "[DefinitionEditor] Error in LoadEntityDefinitions: " << e.what() << std::endl;
  }
}

void DefinitionEditorWindow::DuplicateEntity(const std::string &fromId) {
  const auto *source = definitions_->GetEntity(fromId);
  if (!source) {
    statusMessage_ = "複製元ユニットが見つかりません。";
    return;
  }

  // フォームに複製元データをコピー
  std::string newId = source->id + "_copy";
  std::string newName = source->name + " (Copy)";

  CopyToBuffer(entityForm_.id, newId);
  CopyToBuffer(entityForm_.name, newName);
  CopyToBuffer(entityForm_.description, source->description);
  entityForm_.rarity = source->rarity;
  entityForm_.isEnemy = source->is_enemy;
  entityForm_.cost = source->cost;
  entityForm_.cooldown = source->cooldown;
  entityForm_.hp = source->stats.hp;
  entityForm_.attack = source->stats.attack;
  entityForm_.attackSpeed = source->stats.attack_speed;
  entityForm_.range = source->stats.range;
  entityForm_.moveSpeed = source->stats.move_speed;
  entityForm_.knockback = source->stats.knockback;
  CopyToBuffer(entityForm_.type, source->type);
  CopyToBuffer(entityForm_.atlasTexture, source->display.atlas_texture);
  CopyToBuffer(entityForm_.icon, source->display.icon);
  
  if (!source->display.sprite_actions.empty()) {
    CopyToBuffer(entityForm_.spriteActionFile, source->display.sprite_actions.begin()->second);
  }

  statusMessage_ = "ユニット '" + fromId + "' を複製しました。ID と名前を変更して作成してください。";
}

bool DefinitionEditorWindow::ValidateEntityDef(const Shared::Data::EntityDef &def, std::string &errorMsg) {
  // 基本フィールドの検証
  if (def.id.empty()) {
    errorMsg = "ID は必須です。";
    return false;
  }
  if (def.name.empty()) {
    errorMsg = "Name は必須です。";
    return false;
  }
  if (def.rarity < 1 || def.rarity > 5) {
    errorMsg = "Rarity は 1-5 である必要があります。";
    return false;
  }
  if (def.stats.hp <= 0) {
    errorMsg = "HP は 0 より大きい必要があります。";
    return false;
  }
  if (def.stats.attack < 0) {
    errorMsg = "Attack は 0 以上である必要があります。";
    return false;
  }
  if (def.stats.attack_speed <= 0) {
    errorMsg = "Attack Speed は 0 より大きい必要があります。";
    return false;
  }
  if (def.stats.move_speed < 0) {
    errorMsg = "Move Speed は 0 以上である必要があります。";
    return false;
  }
  
  // JSON スキーマ検証
  try {
    nlohmann::json entity_json = {
      {"id", def.id},
      {"name", def.name},
      {"type", def.type},
      {"rarity", def.rarity},
      {"cost", def.cost},
      {"stats", {
        {"hp", def.stats.hp},
        {"attack", def.stats.attack},
        {"attack_speed", def.stats.attack_speed},
        {"knockback", def.stats.knockback}
      }}
    };
    
    if (!Shared::Data::DataValidator::ValidateEntityAgainstSchema(entity_json)) {
      const auto& errors = Shared::Data::DataValidator::GetErrors();
      if (!errors.empty()) {
        errorMsg = "Schema validation failed:\n";
        for (const auto& err : errors) {
          errorMsg += "  - " + err + "\n";
        }
      }
      Shared::Data::DataValidator::ClearErrors();
      return false;
    }
  } catch (const std::exception& e) {
    errorMsg = std::string("JSON schema validation error: ") + e.what();
    return false;
  }
  
  return true;
}

void DefinitionEditorWindow::RefreshEntityList() {
  entityList_.clear();
  const auto entities = definitions_->GetAllEntities();
  for (const auto *entity : entities) {
    if (entity) {
      entityList_.push_back(entity->id);
    }
  }
}

std::vector<std::string> DefinitionEditorWindow::CheckEntityReferences(const std::string& entityId) {
  std::vector<std::string> references;
  
  if (!definitions_) return references;
  
  // 他のエンティティがこのエンティティを参照しているかチェック
  const auto entities = definitions_->GetAllEntities();
  for (const auto *entity : entities) {
    if (entity && entity->id != entityId) {
      // 能力やスキルで参照されているかチェック（実装例）
      // 将来的には formations や teams でも参照チェック可能
      // 現在はエンティティ定義レベルでの参照チェックのプレースホルダー
    }
  }
  
  return references;
}

void DefinitionEditorWindow::DeleteSelectedEntities() {
  // 既存の DeleteSelectedEntities の前に、ファイル削除を追加
  if (selectedBatchIndices_.empty()) {
    statusMessage_ = "削除対象が選択されていません。";
    return;
  }

  const auto entities = definitions_->GetAllEntities();
  std::vector<std::string> entitiesWithReferences;
  
  // インデックスを逆順でソートして、削除時にインデックスシフトを避ける
  std::vector<int> indicesToDelete = selectedBatchIndices_;
  std::sort(indicesToDelete.rbegin(), indicesToDelete.rend());
  
  // 参照チェック
  for (int idx : indicesToDelete) {
    if (idx >= 0 && idx < static_cast<int>(entities.size())) {
      const auto *entity = entities[idx];
      auto refs = CheckEntityReferences(entity->id);
      if (!refs.empty()) {
        entitiesWithReferences.push_back(entity->id);
      }
    }
  }
  
  // 参照がある場合は警告
  if (!entitiesWithReferences.empty()) {
    statusMessage_ = "警告: 以下のユニットは他で参照されています:\n";
    for (const auto& id : entitiesWithReferences) {
      statusMessage_ += "  - " + id + "\n";
    }
    statusMessage_ += "削除してもよろしいですか？";
    return;
  }
  
  int deleteCount = 0;
  for (int idx : indicesToDelete) {
    if (idx >= 0 && idx < static_cast<int>(entities.size())) {
      const auto *entity = entities[idx];
      
      // ファイルを削除
      std::string entity_file_path = "assets/definitions/entities/characters/" + entity->id + "/entity.json";
      if (std::filesystem::exists(entity_file_path)) {
        try {
          std::filesystem::remove(entity_file_path);
        } catch (const std::exception& e) {
          std::cerr << "Failed to delete file " << entity_file_path << ": " << e.what() << std::endl;
        }
      }
      
      // レジストリから削除
      if (definitions_->RemoveEntity(entity->id)) {
        deleteCount++;
      }
    }
  }
  
  selectedBatchIndices_.clear();
  RefreshEntityList();
  statusMessage_ = "削除完了: " + std::to_string(deleteCount) + " ユニット";
}


void DefinitionEditorWindow::ExportSelectedEntities() {
  if (entityList_.empty()) {
    statusMessage_ = "エクスポート対象がありません。";
    return;
  }

  try {
    namespace fs = std::filesystem;
    std::string export_dir = context_->GetDataPath("export");
    fs::create_directories(export_dir);

    int exportCount = 0;
    for (const auto &id : entityList_) {
      const auto *entity = definitions_->GetEntity(id);
      if (entity && SaveEntityDefinition(*entity)) {
        exportCount++;
      }
    }

    statusMessage_ = "エクスポート完了: " + std::to_string(exportCount) + " ユニット";
  } catch (const std::exception &e) {
    statusMessage_ = "エクスポート失敗: " + std::string(e.what());
  }
}

} // namespace Editor::Windows