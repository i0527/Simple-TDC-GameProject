#include "Editor/Windows/UnitEditorWindow.h"
#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"
#include "Data/Loaders/EntityLoader.h"
#include "imgui.h"
#include <algorithm>
#include <regex>
#include <sstream>

using namespace Shared::Data;

namespace Editor::Windows {

UnitEditorWindow::UnitEditorWindow()
    : context_(nullptr)
    , definitions_(nullptr)
    , isOpen_(false)
    , activeEntityId_("")
    , isEditMode_(false)
    , isDirty_(false)
    , formData_{}
    , searchBuffer_{}
    , filterType_(0)
    , sortMode_(0)
    , showDeleteConfirm_(false)
    , leftPanelWidth_(300.0f)
{
}

UnitEditorWindow::~UnitEditorWindow() {
}

void UnitEditorWindow::Initialize(Shared::Core::GameContext& context, 
                                   DefinitionRegistry& definitions) {
    context_ = &context;
    definitions_ = &definitions;
    ClearForm();
}

void UnitEditorWindow::OnUpdate(float deltaTime) {
    // 更新処理（必要に応じて実装）
}

void UnitEditorWindow::OnDrawUI() {
    if (!isOpen_) return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
    if (ImGui::Begin(GetWindowTitle().c_str(), &isOpen_, flags)) {
        DrawToolbar();
        
        ImGui::BeginChild("MainContent", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
        {
            // 左右分割レイアウト
            DrawLeftPanel();
            ImGui::SameLine();
            DrawRightPanel();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void UnitEditorWindow::DrawToolbar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::MenuItem("新規作成", "Ctrl+N")) {
            CreateNewEntity();
        }
        if (ImGui::MenuItem("複製", "Ctrl+D", false, isEditMode_)) {
            DuplicateEntity();
        }
        if (ImGui::MenuItem("削除", "Delete", false, isEditMode_)) {
            showDeleteConfirm_ = true;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("保存", "Ctrl+S", false, isDirty_)) {
            SaveChanges();
        }
        if (isDirty_) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "●");  // 未保存マーク
        }
        ImGui::EndMenuBar();
    }
    
    // 削除確認ダイアログ
    if (showDeleteConfirm_) {
        ImGui::OpenPopup("削除確認");
        showDeleteConfirm_ = false;
    }
    
    if (ImGui::BeginPopupModal("削除確認", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("エンティティ '%s' を削除しますか？", formData_.id);
        ImGui::Text("この操作は元に戻せません。");
        ImGui::Separator();
        
        if (ImGui::Button("削除", ImVec2(120, 0))) {
            DeleteActiveEntity();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("キャンセル", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void UnitEditorWindow::DrawLeftPanel() {
    ImGui::BeginChild("LeftPanel", ImVec2(leftPanelWidth_, 0), true);
    {
        // 検索ボックス
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##Search", "検索...", searchBuffer_, sizeof(searchBuffer_));
        
        // フィルタ
        ImGui::SetNextItemWidth(-1);
        const char* filterItems[] = { "全て", "味方のみ", "敵のみ", "メインのみ", "サブのみ" };
        ImGui::Combo("##Filter", &filterType_, filterItems, IM_ARRAYSIZE(filterItems));
        
        // ソート
        ImGui::SetNextItemWidth(-1);
        const char* sortItems[] = { "ID順", "名前順", "レアリティ順", "タイプ順" };
        ImGui::Combo("##Sort", &sortMode_, sortItems, IM_ARRAYSIZE(sortItems));
        
        ImGui::Separator();
        
        DrawEntityList();
    }
    ImGui::EndChild();
}

void UnitEditorWindow::DrawEntityList() {
    auto filteredEntities = GetFilteredEntities();
    
    // ソート
    switch (sortMode_) {
        case 0:  // ID順
            std::sort(filteredEntities.begin(), filteredEntities.end(),
                [](const EntityDef* a, const EntityDef* b) { return a->id < b->id; });
            break;
        case 1:  // 名前順
            std::sort(filteredEntities.begin(), filteredEntities.end(),
                [](const EntityDef* a, const EntityDef* b) { return a->name < b->name; });
            break;
        case 2:  // レアリティ順
            std::sort(filteredEntities.begin(), filteredEntities.end(),
                [](const EntityDef* a, const EntityDef* b) { return a->rarity > b->rarity; });
            break;
        case 3:  // タイプ順
            std::sort(filteredEntities.begin(), filteredEntities.end(),
                [](const EntityDef* a, const EntityDef* b) { return a->type < b->type; });
            break;
    }
    
    // テーブル表示
    if (ImGui::BeginTable("EntityList", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupColumn("名前", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("種別", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableHeadersRow();
        
        for (const auto* entity : filteredEntities) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            
            bool isSelected = (activeEntityId_ == entity->id);
            if (ImGui::Selectable(entity->name.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                LoadEntityToForm(entity->id);
            }
            
            // ツールチップにIDを表示
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("ID: %s", entity->id.c_str());
            }
            
            ImGui::TableSetColumnIndex(1);
            const char* typeLabel = entity->is_enemy ? "敵" : "味方";
            ImGui::TextDisabled("%s", typeLabel);
        }
        
        ImGui::EndTable();
    }
    
    ImGui::Text("合計: %zu 件", filteredEntities.size());
}

void UnitEditorWindow::DrawRightPanel() {
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
    {
        if (isEditMode_) {
            ImGui::Text("編集中: %s", formData_.id);
        } else {
            ImGui::Text("新規作成");
        }
        ImGui::Separator();
        
        DrawEditForm();
    }
    ImGui::EndChild();
}

void UnitEditorWindow::DrawEditForm() {
    DrawBasicInfoFields();
    ImGui::Separator();
    
    DrawTeamFields();
    ImGui::Separator();
    
    // 味方の場合のみコストフィールドを表示
    if (!formData_.is_enemy) {
        DrawCostFields();
        ImGui::Separator();
    }
    
    DrawStatsFields();
    ImGui::Separator();
    
    DrawCombatFields();
    ImGui::Separator();
    
    // 説明・タグ
    ImGui::InputTextMultiline("説明", formData_.description, sizeof(formData_.description), ImVec2(-1, 60));
    ImGui::InputText("タグ (カンマ区切り)", formData_.tags, sizeof(formData_.tags));
    
    ImGui::Separator();
    
    // バリデーションエラー表示
    if (!validationErrors_.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
        for (const auto& error : validationErrors_) {
            ImGui::TextWrapped("⚠ %s", error.c_str());
        }
        ImGui::PopStyleColor();
    }
}

void UnitEditorWindow::DrawBasicInfoFields() {
    // ID（編集モードでは読み取り専用）
    if (isEditMode_) {
        ImGui::BeginDisabled();
        ImGui::InputText("ID", formData_.id, sizeof(formData_.id));
        ImGui::EndDisabled();
    } else {
        bool idValid = ValidateId(formData_.id, true);
        if (!idValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.5f, 0, 0, 0.5f));
        
        if (ImGui::InputText("ID", formData_.id, sizeof(formData_.id))) {
            isDirty_ = true;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("英数字とアンダースコアのみ使用可能\n例: char_sub_slime_001");
        }
        
        if (!idValid) ImGui::PopStyleColor();
    }
    
    // 名前
    bool nameValid = ValidateName(formData_.name);
    if (!nameValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.5f, 0, 0, 0.5f));
    
    if (ImGui::InputText("名前", formData_.name, sizeof(formData_.name))) {
        isDirty_ = true;
    }
    
    if (!nameValid) ImGui::PopStyleColor();
    
    // レアリティ
    if (ImGui::SliderInt("レアリティ", &formData_.rarity, 1, 5)) {
        isDirty_ = true;
    }
    
    // タイプ
    const char* typeItems[] = { "main", "sub", "enemy" };
    int typeIndex = 0;
    if (formData_.type[0] != '\0') {
        std::string typeStr = formData_.type;
        if (typeStr == "main") typeIndex = 0;
        else if (typeStr == "sub") typeIndex = 1;
        else if (typeStr == "enemy") typeIndex = 2;
    }
    
    if (ImGui::Combo("タイプ", &typeIndex, typeItems, IM_ARRAYSIZE(typeItems))) {
        strcpy_s(formData_.type, typeItems[typeIndex]);
        isDirty_ = true;
    }
}

void UnitEditorWindow::DrawTeamFields() {
    int teamRadio = formData_.is_enemy ? 1 : 0;
    if (ImGui::RadioButton("味方ユニット", &teamRadio, 0)) {
        formData_.is_enemy = false;
        isDirty_ = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("敵ユニット", &teamRadio, 1)) {
        formData_.is_enemy = true;
        isDirty_ = true;
    }
}

void UnitEditorWindow::DrawCostFields() {
    ImGui::Text("コスト設定（味方のみ）");
    
    if (ImGui::InputInt("コスト", &formData_.cost)) {
        if (formData_.cost < 0) formData_.cost = 0;
        isDirty_ = true;
    }
    
    if (ImGui::InputFloat("クールダウン(秒)", &formData_.cooldown, 0.5f, 1.0f, "%.1f")) {
        if (formData_.cooldown < 0) formData_.cooldown = 0;
        isDirty_ = true;
    }
}

void UnitEditorWindow::DrawStatsFields() {
    ImGui::Text("ステータス");
    
    bool statsValid = ValidateStats();
    if (!statsValid) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.5f, 0, 0, 0.5f));
    
    if (ImGui::InputInt("HP", &formData_.stats.hp)) {
        isDirty_ = true;
    }
    
    if (ImGui::InputInt("攻撃力", &formData_.stats.attack)) {
        isDirty_ = true;
    }
    
    if (ImGui::InputFloat("攻撃速度", &formData_.stats.attack_speed, 0.1f, 0.5f, "%.2f")) {
        isDirty_ = true;
    }
    
    if (ImGui::InputFloat("移動速度", &formData_.stats.move_speed, 1.0f, 10.0f, "%.1f")) {
        isDirty_ = true;
    }
    
    if (ImGui::InputInt("射程", &formData_.stats.range)) {
        isDirty_ = true;
    }
    
    if (ImGui::InputInt("ノックバック", &formData_.stats.knockback)) {
        isDirty_ = true;
    }
    
    if (!statsValid) ImGui::PopStyleColor();
}

void UnitEditorWindow::DrawCombatFields() {
    ImGui::Text("戦闘設定");
    
    if (ImGui::SliderFloat("攻撃ポイント (0.0-1.0)", &formData_.combat.attack_point, 0.0f, 1.0f, "%.2f")) {
        isDirty_ = true;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("アニメーション中の攻撃判定タイミング (0.0=開始, 1.0=終了)");
    }
    
    if (ImGui::InputInt("攻撃フレーム", &formData_.combat.attack_frame)) {
        if (formData_.combat.attack_frame < 0) formData_.combat.attack_frame = 0;
        isDirty_ = true;
    }
    
    ImGui::Text("ヒットボックス");
    ImGui::Indent();
    if (ImGui::InputFloat("幅", &formData_.combat.hitbox.width, 1.0f, 10.0f, "%.1f")) {
        isDirty_ = true;
    }
    if (ImGui::InputFloat("高さ", &formData_.combat.hitbox.height, 1.0f, 10.0f, "%.1f")) {
        isDirty_ = true;
    }
    if (ImGui::InputFloat("X オフセット", &formData_.combat.hitbox.offset_x, 1.0f, 10.0f, "%.1f")) {
        isDirty_ = true;
    }
    if (ImGui::InputFloat("Y オフセット", &formData_.combat.hitbox.offset_y, 1.0f, 10.0f, "%.1f")) {
        isDirty_ = true;
    }
    ImGui::Unindent();
}

void UnitEditorWindow::CreateNewEntity() {
    ClearForm();
    isEditMode_ = false;
    activeEntityId_.clear();
    isDirty_ = true;
}

void UnitEditorWindow::DuplicateEntity() {
    if (!isEditMode_) return;
    
    // IDに "_copy" を追加
    std::string newId = std::string(formData_.id) + "_copy";
    strncpy_s(formData_.id, sizeof(formData_.id), newId.c_str(), _TRUNCATE);
    
    isEditMode_ = false;
    activeEntityId_.clear();
    isDirty_ = true;
}

void UnitEditorWindow::DeleteActiveEntity() {
    if (!isEditMode_ || activeEntityId_.empty()) return;
    
    definitions_->RemoveEntity(activeEntityId_);
    
    // TODO: JSON保存処理
    // EntityLoader::SaveToJson("assets/definitions/entities_custom.json", *definitions_);
    
    ClearForm();
}

void UnitEditorWindow::SaveChanges() {
    validationErrors_.clear();
    
    if (!ValidateForm()) {
        return;  // バリデーションエラーがある場合は保存しない
    }
    
    // EntityDef を作成または更新
    EntityDef entity;
    entity.id = formData_.id;
    entity.name = formData_.name;
    entity.description = formData_.description;
    entity.rarity = formData_.rarity;
    entity.type = formData_.type;
    entity.is_enemy = formData_.is_enemy;
    entity.cost = formData_.cost;
    entity.cooldown = formData_.cooldown;
    
    entity.stats.hp = formData_.stats.hp;
    entity.stats.attack = formData_.stats.attack;
    entity.stats.attack_speed = formData_.stats.attack_speed;
    entity.stats.move_speed = formData_.stats.move_speed;
    entity.stats.range = formData_.stats.range;
    entity.stats.knockback = formData_.stats.knockback;
    
    entity.combat.attack_point = formData_.combat.attack_point;
    entity.combat.attack_frame = formData_.combat.attack_frame;
    entity.combat.hitbox.width = formData_.combat.hitbox.width;
    entity.combat.hitbox.height = formData_.combat.hitbox.height;
    entity.combat.hitbox.offset_x = formData_.combat.hitbox.offset_x;
    entity.combat.hitbox.offset_y = formData_.combat.hitbox.offset_y;
    
    // タグのパース (カンマ区切り)
    std::string tagsStr = formData_.tags;
    std::istringstream ss(tagsStr);
    std::string tag;
    while (std::getline(ss, tag, ',')) {
        // 前後の空白を削除
        tag.erase(0, tag.find_first_not_of(" \t"));
        tag.erase(tag.find_last_not_of(" \t") + 1);
        if (!tag.empty()) {
            entity.tags.push_back(tag);
        }
    }
    
    // DefinitionRegistryに登録
    if (isEditMode_) {
        definitions_->RemoveEntity(activeEntityId_);
    }
    definitions_->RegisterEntity(entity);
    
    // TODO: JSON保存処理
    // EntityLoader::SaveToJson("assets/definitions/entities_custom.json", *definitions_);
    
    isDirty_ = false;
    
    // 編集モードに切り替え
    if (!isEditMode_) {
        activeEntityId_ = entity.id;
        isEditMode_ = true;
    }
}

void UnitEditorWindow::LoadEntityToForm(const std::string& entityId) {
    const auto* entity = definitions_->GetEntity(entityId);
    if (!entity) return;
    
    ClearForm();
    
    strncpy_s(formData_.id, sizeof(formData_.id), entity->id.c_str(), _TRUNCATE);
    strncpy_s(formData_.name, sizeof(formData_.name), entity->name.c_str(), _TRUNCATE);
    strncpy_s(formData_.description, sizeof(formData_.description), entity->description.c_str(), _TRUNCATE);
    formData_.rarity = entity->rarity;
    strncpy_s(formData_.type, sizeof(formData_.type), entity->type.c_str(), _TRUNCATE);
    formData_.is_enemy = entity->is_enemy;
    formData_.cost = entity->cost;
    formData_.cooldown = entity->cooldown;
    
    formData_.stats.hp = entity->stats.hp;
    formData_.stats.attack = entity->stats.attack;
    formData_.stats.attack_speed = entity->stats.attack_speed;
    formData_.stats.move_speed = entity->stats.move_speed;
    formData_.stats.range = entity->stats.range;
    formData_.stats.knockback = entity->stats.knockback;
    
    formData_.combat.attack_point = entity->combat.attack_point;
    formData_.combat.attack_frame = entity->combat.attack_frame;
    formData_.combat.hitbox.width = entity->combat.hitbox.width;
    formData_.combat.hitbox.height = entity->combat.hitbox.height;
    formData_.combat.hitbox.offset_x = entity->combat.hitbox.offset_x;
    formData_.combat.hitbox.offset_y = entity->combat.hitbox.offset_y;
    
    // タグを文字列化
    std::ostringstream tagsStream;
    for (size_t i = 0; i < entity->tags.size(); ++i) {
        if (i > 0) tagsStream << ", ";
        tagsStream << entity->tags[i];
    }
    std::string tagsStr = tagsStream.str();
    strncpy_s(formData_.tags, sizeof(formData_.tags), tagsStr.c_str(), _TRUNCATE);
    
    activeEntityId_ = entityId;
    isEditMode_ = true;
    isDirty_ = false;
}

void UnitEditorWindow::ClearForm() {
    // 各メンバを明示的に初期化（memset は非POD型に使用不可）
    formData_.id[0] = '\0';
    formData_.name[0] = '\0';
    formData_.description[0] = '\0';
    formData_.rarity = 1;
    strncpy_s(formData_.type, sizeof(formData_.type), "sub", _TRUNCATE);
    formData_.is_enemy = false;
    formData_.cost = 0;
    formData_.cooldown = 0.0f;
    
    formData_.stats.hp = 100;
    formData_.stats.attack = 10;
    formData_.stats.attack_speed = 1.0f;
    formData_.stats.move_speed = 50.0f;
    formData_.stats.range = 100;
    formData_.stats.knockback = 0;
    
    formData_.combat.attack_point = 0.5f;
    formData_.combat.attack_frame = 0;
    formData_.combat.hitbox.width = 32.0f;
    formData_.combat.hitbox.height = 32.0f;
    formData_.combat.hitbox.offset_x = 0.0f;
    formData_.combat.hitbox.offset_y = 0.0f;
    
    formData_.tags[0] = '\0';
    
    activeEntityId_.clear();
    isEditMode_ = false;
    isDirty_ = false;
    validationErrors_.clear();
}

void UnitEditorWindow::SetActiveEntity(const std::string& entityId) {
    LoadEntityToForm(entityId);
}

bool UnitEditorWindow::ValidateForm() const {
    validationErrors_.clear();
    
    if (!ValidateId(formData_.id, !isEditMode_)) {
        validationErrors_.push_back("IDは英数字とアンダースコアのみ使用可能で、重複不可です");
    }
    
    if (!ValidateName(formData_.name)) {
        validationErrors_.push_back("名前は必須です");
    }
    
    if (!ValidateStats()) {
        validationErrors_.push_back("ステータスの範囲が不正です");
    }
    
    if (!formData_.is_enemy) {
        if (formData_.cost <= 0) {
            validationErrors_.push_back("味方ユニットはコストが1以上必要です");
        }
        if (formData_.cooldown < 0) {
            validationErrors_.push_back("クールダウンは0以上必要です");
        }
    }
    
    return validationErrors_.empty();
}

bool UnitEditorWindow::ValidateId(const std::string& id, bool isNewEntity) const {
    if (id.empty()) return false;
    
    // 英数字とアンダースコアのみ
    std::regex idPattern("^[a-zA-Z0-9_]+$");
    if (!std::regex_match(id, idPattern)) return false;
    
    // 重複チェック（新規作成時のみ）
    if (isNewEntity) {
        const auto* existing = definitions_->GetEntity(id);
        if (existing) return false;
    }
    
    return true;
}

bool UnitEditorWindow::ValidateName(const std::string& name) const {
    return !name.empty();
}

bool UnitEditorWindow::ValidateStats() const {
    if (formData_.stats.hp <= 0) return false;
    if (formData_.stats.attack < 0) return false;
    if (formData_.stats.attack_speed <= 0) return false;
    if (formData_.stats.move_speed < 0) return false;
    if (formData_.stats.range < 0) return false;
    return true;
}

std::vector<const EntityDef*> UnitEditorWindow::GetFilteredEntities() const {
    std::vector<const EntityDef*> result = definitions_->GetAllEntities();
    
    // フィルタを適用
    result.erase(
        std::remove_if(result.begin(), result.end(),
            [this](const EntityDef* entity) {
                // フィルタ適用
                switch (filterType_) {
                    case 1:  // 味方のみ
                        if (entity->is_enemy) return true;
                        break;
                    case 2:  // 敵のみ
                        if (!entity->is_enemy) return true;
                        break;
                    case 3:  // メインのみ
                        if (entity->type != "main") return true;
                        break;
                    case 4:  // サブのみ
                        if (entity->type != "sub") return true;
                        break;
                }
                
                // 検索クエリ適用
                if (!MatchesSearchQuery(entity)) return true;
                
                return false;
            }
        ),
        result.end()
    );
    
    return result;
}

bool UnitEditorWindow::MatchesSearchQuery(const EntityDef* entity) const {
    if (searchBuffer_[0] == '\0') return true;
    
    std::string query = searchBuffer_;
    std::transform(query.begin(), query.end(), query.begin(), ::tolower);
    
    std::string id = entity->id;
    std::string name = entity->name;
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    
    return (id.find(query) != std::string::npos) || (name.find(query) != std::string::npos);
}

} // namespace Editor::Windows
