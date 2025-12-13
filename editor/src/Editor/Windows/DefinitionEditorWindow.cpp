#include "Editor/Windows/DefinitionEditorWindow.h"

#include <algorithm>
#include <string>
#include <vector>

#include <raylib.h>

#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"

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
} // namespace

void DefinitionEditorWindow::Initialize(Shared::Core::GameContext &context,
                                        Shared::Data::DefinitionRegistry &definitions) {
  context_ = &context;
  definitions_ = &definitions;
  statusMessage_.clear();
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
    if (ImGui::BeginTabItem("Entities")) {
      activeTab_ = DefinitionTab::Entities;
      DrawEntitiesTab();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Skills")) {
      activeTab_ = DefinitionTab::Skills;
      DrawSkillsTab();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Stages")) {
      activeTab_ = DefinitionTab::Stages;
      DrawStagesTab();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Waves")) {
      activeTab_ = DefinitionTab::Waves;
      DrawWavesTab();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Abilities")) {
      activeTab_ = DefinitionTab::Abilities;
      DrawAbilitiesTab();
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
  if (ImGui::BeginTable("entity_list", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
                                                ImGuiTableFlags_BordersInnerV)) {
    ImGui::TableSetupColumn("ID");
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Type");
    ImGui::TableHeadersRow();

    const auto entities = definitions_->GetAllEntities();
    for (const auto *entity : entities) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::TextUnformatted(entity->id.c_str());
      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(entity->name.c_str());
      ImGui::TableSetColumnIndex(2);
      ImGui::TextUnformatted(entity->type.c_str());
    }
    ImGui::EndTable();
  }

  ImGui::Separator();
  ImGui::TextUnformatted("エンティティ作成（簡易）");
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
  WithTwoColumns("entity_form", [this]() {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("ID");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##entity_id", entityForm_.id.data(), entityForm_.id.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Name");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputText("##entity_name", entityForm_.name.data(), entityForm_.name.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Description");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputTextMultiline("##entity_desc", entityForm_.description.data(),
                              entityForm_.description.size(), ImVec2(-FLT_MIN, 80));

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Rarity");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputInt("##entity_rarity", &entityForm_.rarity);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Is Enemy");
    ImGui::TableSetColumnIndex(1);
    ImGui::Checkbox("##entity_enemy", &entityForm_.isEnemy);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Cost");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputInt("##entity_cost", &entityForm_.cost);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Cooldown");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputFloat("##entity_cd", &entityForm_.cooldown);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("HP");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputInt("##entity_hp", &entityForm_.hp);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Attack");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputInt("##entity_atk", &entityForm_.attack);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Attack Speed");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputFloat("##entity_as", &entityForm_.attackSpeed);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Range");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputInt("##entity_range", &entityForm_.range);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Move Speed");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputFloat("##entity_ms", &entityForm_.moveSpeed);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted("Knockback");
    ImGui::TableSetColumnIndex(1);
    ImGui::InputInt("##entity_kb", &entityForm_.knockback);
  });

  if (ImGui::Button("Create Entity")) {
    if (IsEmpty(entityForm_.id) || IsEmpty(entityForm_.name)) {
      statusMessage_ = "Entity: ID と Name は必須です。";
      return;
    }
    Shared::Data::EntityDef def;
    def.id = ToString(entityForm_.id);
    def.name = ToString(entityForm_.name);
    def.description = ToString(entityForm_.description);
    def.rarity = std::max(1, entityForm_.rarity);
    def.is_enemy = entityForm_.isEnemy;
    def.cost = entityForm_.cost;
    def.cooldown = entityForm_.cooldown;
    def.stats.hp = entityForm_.hp;
    def.stats.attack = entityForm_.attack;
    def.stats.attack_speed = entityForm_.attackSpeed;
    def.stats.range = entityForm_.range;
    def.stats.move_speed = entityForm_.moveSpeed;
    def.stats.knockback = entityForm_.knockback;
    def.type = "main";
    def.draw_type = "sprite";

    if (definitions_->HasEntity(def.id)) {
      statusMessage_ = "同じIDのエンティティが既に存在します。";
      return;
    }

    if (definitions_->RegisterEntity(def)) {
      statusMessage_ = "Entity を追加しました。";
    } else {
      statusMessage_ = "Entity の追加に失敗しました。";
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
      statusMessage_ = "Skill を追加しました。";
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
      statusMessage_ = "Stage を追加しました。";
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
      statusMessage_ = "Wave を追加しました。";
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
      statusMessage_ = "Ability を追加しました。";
    } else {
      statusMessage_ = "Ability の追加に失敗しました。";
    }
  }
}

} // namespace Editor::Windows

