#include "EditorScene.hpp"

// 標準ライブラリ
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cmath>
#include <vector>

// 外部ライブラリ
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <raylib.h>

// プロジェクト内
#include "../api/BaseSystemAPI.hpp"
#include "../api/InputSystemAPI.hpp"
#include "../api/GameplayDataAPI.hpp"
#include "../api/BattleProgressAPI.hpp"
#include "../api/ECSystemAPI.hpp"
#include "../api/SetupAPI.hpp"
#include "../config/SharedContext.hpp"
#include "../config/RenderPrimitives.hpp"
#include "../ecs/defineComponents.hpp"
#include "../ecs/entities/EntityCreationData.hpp"
#include "../ui/OverlayColors.hpp"
#include "../ui/ImGuiSoundHelpers.hpp"
#include "../../utils/Log.h"

namespace game {
namespace core {
namespace states {

namespace {

bool InputTextStdString(const char* label, std::string* value, size_t maxSize = 512) {
    if (!value) {
        return false;
    }
    std::vector<char> buffer(maxSize);
    std::snprintf(buffer.data(), buffer.size(), "%s", value->c_str());
    if (ImGui::InputText(label, buffer.data(), buffer.size())) {
        *value = buffer.data();
        return true;
    }
    return false;
}

bool InputTextMultilineStdString(const char* label, std::string* value,
                                 const ImVec2& size = ImVec2(0, 90.0f),
                                 size_t maxSize = 1024) {
    if (!value) {
        return false;
    }
    std::vector<char> buffer(maxSize);
    std::snprintf(buffer.data(), buffer.size(), "%s", value->c_str());
    if (ImGui::InputTextMultiline(label, buffer.data(), buffer.size(), size)) {
        *value = buffer.data();
        return true;
    }
    return false;
}

std::string ToLowerCopy(const std::string& value) {
    std::string lowered;
    lowered.reserve(value.size());
    for (unsigned char ch : value) {
        lowered.push_back(static_cast<char>(std::tolower(ch)));
    }
    return lowered;
}

bool ContainsIgnoreCase(const std::string& text, const std::string& needle) {
    if (needle.empty()) {
        return true;
    }
    const std::string textLower = ToLowerCopy(text);
    const std::string needleLower = ToLowerCopy(needle);
    return textLower.find(needleLower) != std::string::npos;
}

template <typename MapType>
std::string GenerateUniqueId(const std::string& prefix, const MapType& map) {
    int index = 1;
    while (true) {
        std::string candidate = prefix + std::to_string(index);
        if (map.find(candidate) == map.end()) {
            return candidate;
        }
        ++index;
    }
}

std::string TrimCopy(const std::string& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

std::vector<std::string> SplitCommaSeparated(const std::string& value) {
    std::vector<std::string> result;
    std::string current;
    for (char ch : value) {
        if (ch == ',') {
            const auto trimmed = TrimCopy(current);
            if (!trimmed.empty()) {
                result.push_back(trimmed);
            }
            current.clear();
        } else {
            current.push_back(ch);
        }
    }
    const auto trimmed = TrimCopy(current);
    if (!trimmed.empty()) {
        result.push_back(trimmed);
    }
    return result;
}

} // namespace

EditorScene::EditorScene()
    : systemAPI_(nullptr),
      inputAPI_(nullptr),
      sharedContext_(nullptr),
      isInitialized_(false),
      requestTransition_(false),
      nextState_(GameState::Home),
      requestQuit_(false),
      activeTab_(EditorTab::Characters),
      selectedCharacterIndex_(-1),
      lastCharacterSaveOk_(false),
      hasCharacterSaveResult_(false),
      selectedEquipmentIndex_(-1),
      lastItemSaveOk_(false),
      hasItemSaveResult_(false),
      selectedPassiveIndex_(-1),
      selectedStageIndex_(-1),
      lastStageSaveOk_(false),
      hasStageSaveResult_(false),
      stageUnlockText_(""),
      previewTime_(0.0f),
      previewSpeed_(1.0f),
      previewPaused_(false),
      previewLoopEnabled_(true),
      previewUseMoveSprite_(false),
      previewLoopStart_(0.0f),
      previewLoopEnd_(0.0f),
      showHitMarker_(true),
      showTimeBar_(true),
      showRangeOverlay_(false),
      showStatusOverlay_(false),
      showAttackLog_(false),
      attackLogEnabled_(true),
      spawnCharacterIndex_(0),
      spawnAsEnemy_(false),
      spawnX_(960.0f),
      spawnY_(360.0f),
      spawnLevel_(1),
      moveSimEnabled_(true),
      moveSimTime_(0.0f),
      moveSimSpeed_(1.0f),
      moveSimTargetOffset_(0.0f),
      moveSimStartOffset_(0.0f),
      moveSimLoop_(true),
      moveSimShowPath_(true),
      moveSimShowStop_(true),
      moveSimShowDistance_(true),
      moveSimShowHitbox_(true),
      characterFilter_(""),
      equipmentFilter_(""),
      passiveFilter_(""),
      stageFilter_("") {
}

EditorScene::~EditorScene() {
    Shutdown();
}

bool EditorScene::Initialize(BaseSystemAPI* systemAPI) {
    if (!systemAPI) {
        LOG_ERROR("EditorScene: systemAPI is null");
        return false;
    }
    systemAPI_ = systemAPI;
    isInitialized_ = true;
    requestTransition_ = false;
    requestQuit_ = false;
    activeTab_ = EditorTab::Characters;
    LoadDataFromAPI();
    ResetAttackPreview();
    LOG_INFO("EditorScene initialized");
    return true;
}

void EditorScene::SetSharedContext(SharedContext* ctx) {
    sharedContext_ = ctx;
    inputAPI_ = ctx ? ctx->inputAPI : nullptr;
}

void EditorScene::Update(float deltaTime) {
    if (!isInitialized_) {
        return;
    }
    UpdateAttackPreview(deltaTime);
    UpdateMoveSimulation(deltaTime);
}

void EditorScene::Render() {
    if (!systemAPI_) {
        return;
    }

    // 背景
    systemAPI_->Render().DrawRectangle(0, 0, 1920, 1080,
                                       ToCoreColor(ui::OverlayColors::MAIN_BG));

    // 攻撃アニメのプレビュー
    const auto* ch = GetSelectedCharacter();
    if (!ch) {
        return;
    }

    const float panelX = 1050.0f;
    const float panelY = 120.0f;
    const float panelW = 820.0f;
    const float panelH = 820.0f;

    systemAPI_->Render().DrawRectangle(panelX, panelY, panelW, panelH,
                                       ToCoreColor(ui::OverlayColors::PANEL_BG));

    void* texturePtr = systemAPI_->Resource().GetTexture(ch->attack_sprite.sheet_path);
    if (!texturePtr) {
        systemAPI_->Render().DrawTextDefault(
            "Attack sprite not found", panelX + 20.0f, panelY + 20.0f, 18.0f,
            ToCoreColor(ui::OverlayColors::TEXT_MUTED));
        return;
    }

    auto* texture = static_cast<Texture2D*>(texturePtr);
    if (!texture || texture->id == 0) {
        return;
    }

    const auto& sprite = previewUseMoveSprite_ ? ch->move_sprite : ch->attack_sprite;
    const int frameCount = std::max(1, sprite.frame_count);
    const float fw = static_cast<float>(sprite.frame_width);
    const float fh = static_cast<float>(sprite.frame_height);
    const float frameDuration = std::max(0.01f, sprite.frame_duration);
    const float previewDuration = GetPreviewDuration();
    const float clampedTime = std::min(previewTime_, previewDuration);
    const int frame = std::min(static_cast<int>(clampedTime / frameDuration),
                               frameCount - 1);
    Rectangle src{fw * static_cast<float>(frame), 0.0f, fw, fh};

    const float dstX = panelX + (panelW - fw) * 0.5f;
    const float dstY = panelY + (panelH - fh) * 0.5f;
    Rectangle dst{dstX, dstY, fw, fh};

    systemAPI_->Render().DrawTexturePro(*texture, src, dst, {0.0f, 0.0f}, 0.0f,
                                        WHITE);

    const float elapsed = clampedTime;

    const float attackDuration = GetAttackDuration();
    systemAPI_->Render().DrawTextDefault(
        previewUseMoveSprite_ ? "Move preview" : "Attack preview",
        panelX + 20.0f, panelY + panelH - 80.0f, 18.0f,
        ToCoreColor(ui::OverlayColors::TEXT_PRIMARY));
    systemAPI_->Render().DrawTextDefault(
        "time: " + std::to_string(elapsed) + " / " +
            std::to_string(previewDuration),
        panelX + 20.0f, panelY + panelH - 55.0f, 16.0f,
        ToCoreColor(ui::OverlayColors::TEXT_MUTED));
    if (!previewUseMoveSprite_) {
        systemAPI_->Render().DrawTextDefault(
            "hit_time: " + std::to_string(ch->attack_hit_time),
            panelX + 20.0f, panelY + panelH - 30.0f, 16.0f,
            ToCoreColor(ui::OverlayColors::TEXT_MUTED));
    }

    // 移動シミュレーション可視化
    if (moveSimEnabled_) {
        const float laneY = dstY + fh - 6.0f;
        const float startX = panelX + 80.0f + moveSimStartOffset_;
        const float targetX = panelX + panelW - 80.0f - moveSimTargetOffset_;
        const float range = std::max(0.0f, ch->attack_size.x);
        const float offset = std::max(0.0f, ch->attack_size.y);
        const float stopX = std::max(startX, targetX - (range + offset));
        const float moveSpeed = std::max(0.0f, ch->move_speed) * moveSimSpeed_;
        float posX = std::min(stopX, startX + moveSimTime_ * moveSpeed);
        if (moveSimLoop_ && moveSpeed > 0.0f && posX >= stopX) {
            moveSimTime_ = 0.0f;
            posX = startX;
        }

        const ColorRGBA laneColor{ui::OverlayColors::CARD_BORDER_NORMAL.r,
                                  ui::OverlayColors::CARD_BORDER_NORMAL.g,
                                  ui::OverlayColors::CARD_BORDER_NORMAL.b, 180};
        systemAPI_->Render().DrawLine(static_cast<int>(panelX + 30.0f),
                                      static_cast<int>(laneY),
                                      static_cast<int>(panelX + panelW - 30.0f),
                                      static_cast<int>(laneY), 2.0f, laneColor);

        if (moveSimShowPath_) {
            const ColorRGBA pathColor{ui::OverlayColors::ACCENT_BLUE.r,
                                      ui::OverlayColors::ACCENT_BLUE.g,
                                      ui::OverlayColors::ACCENT_BLUE.b, 120};
            systemAPI_->Render().DrawLine(static_cast<int>(startX),
                                          static_cast<int>(laneY - 8.0f),
                                          static_cast<int>(stopX),
                                          static_cast<int>(laneY - 8.0f), 3.0f, pathColor);
        }
        if (moveSimShowStop_) {
            const ColorRGBA stopColor{ui::OverlayColors::ACCENT_GOLD.r,
                                      ui::OverlayColors::ACCENT_GOLD.g,
                                      ui::OverlayColors::ACCENT_GOLD.b, 200};
            systemAPI_->Render().DrawLine(static_cast<int>(stopX),
                                          static_cast<int>(laneY - 20.0f),
                                          static_cast<int>(stopX),
                                          static_cast<int>(laneY + 4.0f), 2.0f, stopColor);
        }

        const ColorRGBA curColor{ui::OverlayColors::ACCENT_PRIMARY.r,
                                 ui::OverlayColors::ACCENT_PRIMARY.g,
                                 ui::OverlayColors::ACCENT_PRIMARY.b, 220};
        systemAPI_->Render().DrawCircle(posX, laneY - 12.0f, 6.0f, curColor);

        if (moveSimShowHitbox_) {
            // ヒットボックスはプレビューのキャラ位置に追従させる
            const float hitboxX = dstX + fw + offset;
            const float hitboxY = dstY;
            const float hitboxW = range;
            const float hitboxH = fh;
            const ColorRGBA fillColor{ui::OverlayColors::ACCENT_GOLD.r,
                                      ui::OverlayColors::ACCENT_GOLD.g,
                                      ui::OverlayColors::ACCENT_GOLD.b, 45};
            const ColorRGBA lineColor{ui::OverlayColors::ACCENT_GOLD.r,
                                      ui::OverlayColors::ACCENT_GOLD.g,
                                      ui::OverlayColors::ACCENT_GOLD.b, 180};
            systemAPI_->Render().DrawRectangle(hitboxX, hitboxY, hitboxW, hitboxH, fillColor);
            systemAPI_->Render().DrawRectangleLines(hitboxX, hitboxY, hitboxW, hitboxH, 2.0f,
                                                    lineColor);
        }

        if (moveSimShowDistance_) {
            const float dist = std::max(0.0f, targetX - posX);
            systemAPI_->Render().DrawTextDefault(
                "dist: " + std::to_string(dist),
                panelX + 20.0f, panelY + panelH - 130.0f, 16.0f,
                ToCoreColor(ui::OverlayColors::TEXT_MUTED));
        }
    }

    if (showTimeBar_) {
        const float barX = panelX + 20.0f;
        const float barY = panelY + panelH - 110.0f;
        const float barW = panelW - 40.0f;
        const float barH = 10.0f;
        systemAPI_->Render().DrawRectangle(
            barX, barY, barW, barH, ToCoreColor(ui::OverlayColors::PANEL_BG_DARK));
        const float progress = (previewDuration > 0.0f) ? (elapsed / previewDuration) : 0.0f;
        systemAPI_->Render().DrawRectangle(
            barX, barY, barW * std::min(1.0f, progress), barH,
            ToCoreColor(ui::OverlayColors::ACCENT_BLUE));
        systemAPI_->Render().DrawRectangleLines(
            barX, barY, barW, barH, 1.0f, ToCoreColor(ui::OverlayColors::CARD_BORDER_NORMAL));
    }

    if (showHitMarker_ && !previewUseMoveSprite_) {
        const float hitTime = std::min(std::max(0.0f, ch->attack_hit_time), attackDuration);
        const float hitPos = (attackDuration > 0.0f)
                                 ? (panelX + 20.0f + (panelW - 40.0f) * (hitTime / attackDuration))
                                 : panelX + 20.0f;
        const float barTop = panelY + panelH - 125.0f;
        const float barBottom = panelY + panelH - 95.0f;
        const float pulse = 0.5f + 0.5f * std::sin(elapsed * 12.0f);
        ColorRGBA hitColor{ui::OverlayColors::ACCENT_GOLD.r,
                           ui::OverlayColors::ACCENT_GOLD.g,
                           ui::OverlayColors::ACCENT_GOLD.b,
                           static_cast<std::uint8_t>(140 + pulse * 115)};
        systemAPI_->Render().DrawRectangle(hitPos - 1.0f, barTop, 2.0f,
                                           barBottom - barTop, hitColor);
    }

    if (showRangeOverlay_ && !previewUseMoveSprite_) {
        const float range = std::max(0.0f, ch->attack_size.x);
        const float offset = std::max(0.0f, ch->attack_size.y);
        const float hitboxX = dstX + fw + offset;
        const float hitboxY = dstY;
        const float hitboxW = range;
        const float hitboxH = fh;
        const ColorRGBA fillColor{ui::OverlayColors::ACCENT_GOLD.r,
                                  ui::OverlayColors::ACCENT_GOLD.g,
                                  ui::OverlayColors::ACCENT_GOLD.b, 60};
        const ColorRGBA lineColor{ui::OverlayColors::ACCENT_GOLD.r,
                                  ui::OverlayColors::ACCENT_GOLD.g,
                                  ui::OverlayColors::ACCENT_GOLD.b, 200};
        systemAPI_->Render().DrawRectangle(hitboxX, hitboxY, hitboxW, hitboxH, fillColor);
        systemAPI_->Render().DrawRectangleLines(hitboxX, hitboxY, hitboxW, hitboxH, 2.0f, lineColor);
    }
}

void EditorScene::RenderImGui() {
    if (!isInitialized_) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(980.0f, 920.0f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Attack Timing Editor", nullptr)) {
        ImGui::End();
        return;
    }

    if (ui::ImGuiSound::Button(systemAPI_, "Reload All")) {
        LoadDataFromAPI();
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "Save All") && sharedContext_ && sharedContext_->gameplayDataAPI) {
        lastCharacterSaveOk_ =
            sharedContext_->gameplayDataAPI->SaveCharacterMasters(characterEdits_);
        if (lastCharacterSaveOk_) {
            characterOriginal_ = characterEdits_;
        }
        lastItemSaveOk_ =
            sharedContext_->gameplayDataAPI->SaveItemPassiveMasters(passiveEdits_, equipmentEdits_);
        if (lastItemSaveOk_) {
            passiveOriginal_ = passiveEdits_;
            equipmentOriginal_ = equipmentEdits_;
        }
        lastStageSaveOk_ =
            sharedContext_->gameplayDataAPI->SaveStageMasters(stageEdits_);
        if (lastStageSaveOk_) {
            stageOriginal_ = stageEdits_;
        }
        hasCharacterSaveResult_ = true;
        hasItemSaveResult_ = true;
        hasStageSaveResult_ = true;
    }
    ImGui::Separator();

    if (ImGui::BeginTabBar("EditorTabs")) {
        if (ImGui::BeginTabItem("Characters")) {
            activeTab_ = EditorTab::Characters;
            RenderCharacterTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Equipment")) {
            activeTab_ = EditorTab::Equipment;
            RenderEquipmentTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Passives")) {
            activeTab_ = EditorTab::Passives;
            RenderPassiveTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Stages")) {
            activeTab_ = EditorTab::Stages;
            RenderStageTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Battle Debug")) {
            activeTab_ = EditorTab::BattleDebug;
            RenderBattleDebugTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void EditorScene::Shutdown() {
    isInitialized_ = false;
    characterEdits_.clear();
    equipmentEdits_.clear();
    passiveEdits_.clear();
    stageEdits_.clear();
    characterIds_.clear();
    equipmentIds_.clear();
    passiveIds_.clear();
    stageIds_.clear();
}

bool EditorScene::RequestTransition(GameState& nextState) {
    if (requestTransition_) {
        nextState = nextState_;
        requestTransition_ = false;
        return true;
    }
    return false;
}

bool EditorScene::RequestQuit() {
    bool result = requestQuit_;
    requestQuit_ = false;
    return result;
}

void EditorScene::LoadDataFromAPI() {
    if (!sharedContext_ || !sharedContext_->gameplayDataAPI) {
        return;
    }

    characterEdits_ = sharedContext_->gameplayDataAPI->GetAllCharacterMasters();
    characterOriginal_ = characterEdits_;
    characterIds_.clear();
    characterIds_.reserve(characterEdits_.size());
    for (const auto& [id, _] : characterEdits_) {
        characterIds_.push_back(id);
    }
    std::sort(characterIds_.begin(), characterIds_.end());
    selectedCharacterIndex_ = characterIds_.empty() ? -1 : 0;

    equipmentEdits_ = sharedContext_->gameplayDataAPI->GetAllEquipmentMasters();
    equipmentOriginal_ = equipmentEdits_;
    equipmentIds_.clear();
    equipmentIds_.reserve(equipmentEdits_.size());
    for (const auto& [id, _] : equipmentEdits_) {
        equipmentIds_.push_back(id);
    }
    std::sort(equipmentIds_.begin(), equipmentIds_.end());
    selectedEquipmentIndex_ = equipmentIds_.empty() ? -1 : 0;

    passiveEdits_ = sharedContext_->gameplayDataAPI->GetAllPassiveMasters();
    passiveOriginal_ = passiveEdits_;
    passiveIds_.clear();
    passiveIds_.reserve(passiveEdits_.size());
    for (const auto& [id, _] : passiveEdits_) {
        passiveIds_.push_back(id);
    }
    std::sort(passiveIds_.begin(), passiveIds_.end());
    selectedPassiveIndex_ = passiveIds_.empty() ? -1 : 0;

    stageEdits_.clear();
    stageOriginal_.clear();
    stageIds_.clear();
    const auto stages = sharedContext_->gameplayDataAPI->GetAllStageData();
    stageIds_.reserve(stages.size());
    for (const auto& stage : stages) {
        stageEdits_[stage.id] = stage;
        stageOriginal_[stage.id] = stage;
        stageIds_.push_back(stage.id);
    }
    selectedStageIndex_ = stageIds_.empty() ? -1 : 0;
    stageJsonText_.clear();
    stageJsonError_.clear();
    stageUnlockText_.clear();
    if (selectedStageIndex_ >= 0 &&
        selectedStageIndex_ < static_cast<int>(stageIds_.size())) {
        const auto& id = stageIds_[selectedStageIndex_];
        const auto it = stageEdits_.find(id);
        if (it != stageEdits_.end()) {
            stageJsonText_ = it->second.data.dump(2);
            for (size_t i = 0; i < it->second.unlockOnClear.size(); ++i) {
                if (i > 0) {
                    stageUnlockText_ += ", ";
                }
                stageUnlockText_ += it->second.unlockOnClear[i];
            }
        }
    }
    hasStageSaveResult_ = false;

    hasCharacterSaveResult_ = false;
    hasItemSaveResult_ = false;
    ResetAttackPreview();
}

float EditorScene::GetAttackDuration() const {
    const auto* ch = GetSelectedCharacter();
    if (!ch) {
        return 0.01f;
    }
    return std::max(
        0.01f,
        ch->attack_sprite.frame_duration *
            static_cast<float>(std::max(1, ch->attack_sprite.frame_count)));
}

float EditorScene::GetPreviewDuration() const {
    const auto* ch = GetSelectedCharacter();
    if (!ch) {
        return 0.01f;
    }
    const auto& sprite = previewUseMoveSprite_ ? ch->move_sprite : ch->attack_sprite;
    return std::max(0.01f,
                    sprite.frame_duration *
                        static_cast<float>(std::max(1, sprite.frame_count)));
}

void EditorScene::SetPreviewTime(float time) {
    const float duration = GetPreviewDuration();
    previewTime_ = std::min(std::max(0.0f, time), duration);
}

bool EditorScene::IsCharacterModified(const std::string& id) const {
    auto it = characterEdits_.find(id);
    auto itOriginal = characterOriginal_.find(id);
    if (it == characterEdits_.end()) {
        return false;
    }
    if (itOriginal == characterOriginal_.end()) {
        return true;
    }
    const auto& a = it->second;
    const auto& b = itOriginal->second;
    return a.name != b.name ||
           a.description != b.description ||
           a.rarity != b.rarity ||
           a.default_level != b.default_level ||
           a.hp != b.hp ||
           a.attack != b.attack ||
           a.defense != b.defense ||
           a.move_speed != b.move_speed ||
           a.attack_span != b.attack_span ||
           a.attack_type != b.attack_type ||
           a.attack_size.x != b.attack_size.x ||
           a.attack_size.y != b.attack_size.y ||
           a.effect_type != b.effect_type ||
           a.attack_hit_time != b.attack_hit_time ||
           a.icon_path != b.icon_path ||
           a.move_sprite.sheet_path != b.move_sprite.sheet_path ||
           a.move_sprite.frame_width != b.move_sprite.frame_width ||
           a.move_sprite.frame_height != b.move_sprite.frame_height ||
           a.move_sprite.frame_count != b.move_sprite.frame_count ||
           a.move_sprite.frame_duration != b.move_sprite.frame_duration ||
           a.attack_sprite.sheet_path != b.attack_sprite.sheet_path ||
           a.attack_sprite.frame_width != b.attack_sprite.frame_width ||
           a.attack_sprite.frame_height != b.attack_sprite.frame_height ||
           a.attack_sprite.frame_count != b.attack_sprite.frame_count ||
           a.attack_sprite.frame_duration != b.attack_sprite.frame_duration ||
           a.cost != b.cost ||
           a.default_unlocked != b.default_unlocked;
}

bool EditorScene::IsEquipmentModified(const std::string& id) const {
    auto it = equipmentEdits_.find(id);
    auto itOriginal = equipmentOriginal_.find(id);
    if (it == equipmentEdits_.end()) {
        return false;
    }
    if (itOriginal == equipmentOriginal_.end()) {
        return true;
    }
    const auto& a = it->second;
    const auto& b = itOriginal->second;
    return a.name != b.name ||
           a.description != b.description ||
           a.attack_bonus != b.attack_bonus ||
           a.defense_bonus != b.defense_bonus ||
           a.hp_bonus != b.hp_bonus;
}

bool EditorScene::IsPassiveModified(const std::string& id) const {
    auto it = passiveEdits_.find(id);
    auto itOriginal = passiveOriginal_.find(id);
    if (it == passiveEdits_.end()) {
        return false;
    }
    if (itOriginal == passiveOriginal_.end()) {
        return true;
    }
    const auto& a = it->second;
    const auto& b = itOriginal->second;
    return a.name != b.name ||
           a.description != b.description ||
           a.value != b.value ||
           a.effect_type != b.effect_type ||
           a.target_stat != b.target_stat ||
           a.rarity != b.rarity;
}

bool EditorScene::IsStageModified(const std::string& id) const {
    auto it = stageEdits_.find(id);
    auto itOriginal = stageOriginal_.find(id);
    if (it == stageEdits_.end()) {
        return false;
    }
    if (itOriginal == stageOriginal_.end()) {
        return true;
    }
    const auto& a = it->second;
    const auto& b = itOriginal->second;
    return a.stageNumber != b.stageNumber ||
           a.chapter != b.chapter ||
           a.chapterName != b.chapterName ||
           a.stageName != b.stageName ||
           a.difficulty != b.difficulty ||
           a.isBoss != b.isBoss ||
           a.isLocked != b.isLocked ||
           a.rewardGold != b.rewardGold ||
           a.waveCount != b.waveCount ||
           a.recommendedLevel != b.recommendedLevel ||
           a.previewImageId != b.previewImageId ||
           a.unlockOnClear != b.unlockOnClear ||
           a.data != b.data;
}

const entities::Character* EditorScene::GetSelectedCharacter() const {
    if (selectedCharacterIndex_ < 0 ||
        selectedCharacterIndex_ >= static_cast<int>(characterIds_.size())) {
        return nullptr;
    }
    const auto& id = characterIds_[selectedCharacterIndex_];
    auto it = characterEdits_.find(id);
    if (it == characterEdits_.end()) {
        return nullptr;
    }
    return &it->second;
}

entities::Character* EditorScene::GetSelectedCharacterMutable() {
    if (selectedCharacterIndex_ < 0 ||
        selectedCharacterIndex_ >= static_cast<int>(characterIds_.size())) {
        return nullptr;
    }
    const auto& id = characterIds_[selectedCharacterIndex_];
    auto it = characterEdits_.find(id);
    if (it == characterEdits_.end()) {
        return nullptr;
    }
    return &it->second;
}

void EditorScene::ResetAttackPreview() {
    previewTime_ = 0.0f;
    previewSpeed_ = 1.0f;
    previewPaused_ = false;
    previewLoopEnabled_ = true;
    previewLoopStart_ = 0.0f;
    previewLoopEnd_ = GetPreviewDuration();
    moveSimTime_ = 0.0f;
}

void EditorScene::UpdateAttackPreview(float deltaTime) {
    if (activeTab_ != EditorTab::Characters || previewPaused_) {
        return;
    }
    const auto* ch = GetSelectedCharacter();
    if (!ch) {
        return;
    }
    const float duration = GetPreviewDuration();
    if (duration <= 0.0f) {
        return;
    }

    previewTime_ += deltaTime * previewSpeed_;
    const float loopStart = std::min(previewLoopStart_, previewLoopEnd_);
    const float loopEnd = std::max(previewLoopStart_, previewLoopEnd_);
    if (previewLoopEnabled_) {
        if (previewTime_ > loopEnd) {
            previewTime_ = loopStart;
        }
        if (previewTime_ < loopStart) {
            previewTime_ = loopStart;
        }
    } else if (previewTime_ > duration) {
        previewTime_ = duration;
        previewPaused_ = true;
    }
}

void EditorScene::UpdateMoveSimulation(float deltaTime) {
    if (!moveSimEnabled_) {
        return;
    }
    const auto* ch = GetSelectedCharacter();
    if (!ch) {
        return;
    }
    const float moveSpeed = std::max(0.0f, ch->move_speed) * moveSimSpeed_;
    moveSimTime_ += deltaTime;
    if (moveSpeed <= 0.0f) {
        moveSimTime_ = 0.0f;
    }
}

void EditorScene::RenderCharacterTab() {
    if (ui::ImGuiSound::Button(systemAPI_, "Reload##Characters")) {
        LoadDataFromAPI();
    }
    ImGui::SameLine();
    if (sharedContext_ && sharedContext_->gameplayDataAPI) {
        if (ui::ImGuiSound::Button(systemAPI_, "Save##Characters")) {
            lastCharacterSaveOk_ =
                sharedContext_->gameplayDataAPI->SaveCharacterMasters(characterEdits_);
            if (lastCharacterSaveOk_) {
                characterOriginal_ = characterEdits_;
            }
            hasCharacterSaveResult_ = true;
        }
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "New##Characters")) {
        entities::Character ch;
        ch.id = GenerateUniqueId("character_", characterEdits_);
        ch.name = "New Character";
        ch.rarity = 1;
        ch.default_level = 1;
        ch.hp = 100;
        ch.attack = 10;
        ch.defense = 0;
        ch.move_speed = 120.0f;
        ch.attack_span = 1.0f;
        ch.attack_type = entities::AttackType::Single;
        ch.attack_size = {80.0f, 0.0f};
        ch.effect_type = entities::EffectType::Normal;
        ch.attack_hit_time = 0.2f;
        ch.icon_path = "";
        ch.move_sprite = {"", 64, 64, 1, 0.1f};
        ch.attack_sprite = {"", 64, 64, 1, 0.1f};
        ch.description = "";
        ch.cost = 1;
        ch.default_unlocked = false;
        characterEdits_[ch.id] = ch;
        characterIds_.push_back(ch.id);
        std::sort(characterIds_.begin(), characterIds_.end());
        for (int i = 0; i < static_cast<int>(characterIds_.size()); ++i) {
            if (characterIds_[i] == ch.id) {
                selectedCharacterIndex_ = i;
                break;
            }
        }
        ResetAttackPreview();
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "Duplicate##Characters")) {
        auto* ch = GetSelectedCharacterMutable();
        if (ch) {
            entities::Character copy = *ch;
            copy.id = GenerateUniqueId("character_", characterEdits_);
            copy.name = ch->name + " Copy";
            characterEdits_[copy.id] = copy;
            characterIds_.push_back(copy.id);
            std::sort(characterIds_.begin(), characterIds_.end());
            for (int i = 0; i < static_cast<int>(characterIds_.size()); ++i) {
                if (characterIds_[i] == copy.id) {
                    selectedCharacterIndex_ = i;
                    break;
                }
            }
            ResetAttackPreview();
        }
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "Delete##Characters")) {
        if (selectedCharacterIndex_ >= 0 &&
            selectedCharacterIndex_ < static_cast<int>(characterIds_.size())) {
            const auto id = characterIds_[selectedCharacterIndex_];
            characterEdits_.erase(id);
            characterIds_.erase(characterIds_.begin() + selectedCharacterIndex_);
            if (selectedCharacterIndex_ >= static_cast<int>(characterIds_.size())) {
                selectedCharacterIndex_ = static_cast<int>(characterIds_.size()) - 1;
            }
            ResetAttackPreview();
        }
    }
    if (hasCharacterSaveResult_) {
        ImGui::SameLine();
        ImGui::TextUnformatted(lastCharacterSaveOk_ ? "Save: OK" : "Save: FAILED");
    }

    int modifiedCount = 0;
    for (const auto& id : characterIds_) {
        if (IsCharacterModified(id)) {
            ++modifiedCount;
        }
    }
    ImGui::SameLine();
    ImGui::Text("Modified: %d", modifiedCount);
    InputTextStdString("Filter##Characters", &characterFilter_, 128);

    ImGui::Separator();

    ImGui::BeginChild("CharacterList", ImVec2(260, 0), true);
    for (int i = 0; i < static_cast<int>(characterIds_.size()); ++i) {
        const auto& id = characterIds_[i];
        const auto* ch = (characterEdits_.count(id) > 0) ? &characterEdits_.at(id) : nullptr;
        std::string label = id;
        if (ch) {
            label += " - " + ch->name;
        }
        if (!ContainsIgnoreCase(label, characterFilter_)) {
            continue;
        }
        if (IsCharacterModified(id)) {
            label = "* " + label;
        }
        const bool selected = (i == selectedCharacterIndex_);
        if (ui::ImGuiSound::Selectable(systemAPI_, label.c_str(), selected)) {
            selectedCharacterIndex_ = i;
            ResetAttackPreview();
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("CharacterDetail", ImVec2(0, 0), true);
    auto* ch = GetSelectedCharacterMutable();
    if (ch) {
        ImGui::Text("ID: %s", ch->id.c_str());
        InputTextStdString("Name", &ch->name);
        InputTextMultilineStdString("Description", &ch->description);
        ImGui::InputInt("Rarity", &ch->rarity);
        ch->rarity = std::max(1, ch->rarity);
        ImGui::InputInt("DefaultLevel", &ch->default_level);
        ch->default_level = std::max(1, ch->default_level);

        ImGui::SeparatorText("Stats");
        ImGui::InputInt("HP", &ch->hp);
        ch->hp = std::max(1, ch->hp);
        ImGui::InputInt("Attack", &ch->attack);
        ch->attack = std::max(0, ch->attack);
        ImGui::InputInt("Defense", &ch->defense);
        ch->defense = std::max(0, ch->defense);
        ImGui::InputFloat("MoveSpeed", &ch->move_speed, 1.0f, 10.0f, "%.2f");
        ch->move_speed = std::max(0.0f, ch->move_speed);

        ImGui::SeparatorText("Attack Timing");
        ImGui::InputFloat("AttackRange", &ch->attack_size.x, 1.0f, 10.0f, "%.2f");
        ImGui::InputFloat("AttackOffset", &ch->attack_size.y, 1.0f, 10.0f, "%.2f");
        ImGui::InputFloat("attack_span", &ch->attack_span, 0.01f, 0.1f, "%.3f");
        ch->attack_span = std::max(0.01f, ch->attack_span);

        ImGui::InputInt("attack_frame_count", &ch->attack_sprite.frame_count);
        ch->attack_sprite.frame_count = std::max(1, ch->attack_sprite.frame_count);
        ImGui::InputFloat("attack_frame_duration", &ch->attack_sprite.frame_duration, 0.005f, 0.02f, "%.3f");
        ch->attack_sprite.frame_duration = std::max(0.01f, ch->attack_sprite.frame_duration);
        const float attackDuration = std::max(
            0.01f,
            ch->attack_sprite.frame_duration *
                static_cast<float>(std::max(1, ch->attack_sprite.frame_count)));

        ImGui::InputFloat("attack_hit_time", &ch->attack_hit_time, 0.01f, 0.05f, "%.3f");
        ch->attack_hit_time = std::min(std::max(0.0f, ch->attack_hit_time), attackDuration);
        ImGui::Text("attack_duration: %.3f", attackDuration);

        ImGui::SeparatorText("Attack/Effect");
        {
            const char* attackTypes[] = {"Single", "Range", "Line"};
            int attackTypeIndex = static_cast<int>(ch->attack_type);
            if (attackTypeIndex < 0 || attackTypeIndex > 2) {
                attackTypeIndex = 0;
            }
            if (ui::ImGuiSound::Combo(systemAPI_, "AttackType", &attackTypeIndex, attackTypes, 3)) {
                ch->attack_type = static_cast<entities::AttackType>(attackTypeIndex);
            }

            const char* effectTypes[] = {"Normal", "Fire", "Ice", "Lightning", "Heal"};
            int effectIndex = static_cast<int>(ch->effect_type);
            if (effectIndex < 0 || effectIndex > 4) {
                effectIndex = 0;
            }
            if (ui::ImGuiSound::Combo(systemAPI_, "EffectType", &effectIndex, effectTypes, 5)) {
                ch->effect_type = static_cast<entities::EffectType>(effectIndex);
            }
        }

        ImGui::SeparatorText("Sprites");
        InputTextStdString("IconPath", &ch->icon_path);
        InputTextStdString("MoveSheet", &ch->move_sprite.sheet_path);
        ImGui::InputInt("MoveFrameW", &ch->move_sprite.frame_width);
        ImGui::InputInt("MoveFrameH", &ch->move_sprite.frame_height);
        ImGui::InputInt("MoveFrames", &ch->move_sprite.frame_count);
        ImGui::InputFloat("MoveFrameDur", &ch->move_sprite.frame_duration, 0.005f, 0.02f, "%.3f");
        ch->move_sprite.frame_count = std::max(1, ch->move_sprite.frame_count);
        ch->move_sprite.frame_duration = std::max(0.01f, ch->move_sprite.frame_duration);

        InputTextStdString("AttackSheet", &ch->attack_sprite.sheet_path);
        ImGui::InputInt("AttackFrameW", &ch->attack_sprite.frame_width);
        ImGui::InputInt("AttackFrameH", &ch->attack_sprite.frame_height);

        ImGui::SeparatorText("Codex");
        ImGui::InputInt("Cost", &ch->cost);
        ch->cost = std::max(0, ch->cost);
        ui::ImGuiSound::Checkbox(systemAPI_, "DefaultUnlocked", &ch->default_unlocked);

        ImGui::SeparatorText("Preview");
        if (ui::ImGuiSound::Button(systemAPI_, previewPaused_ ? "Play##Preview" : "Pause##Preview")) {
            previewPaused_ = !previewPaused_;
        }
        ImGui::SameLine();
        if (ui::ImGuiSound::Button(systemAPI_, "Reset##Preview")) {
            ResetAttackPreview();
        }
        ImGui::SameLine();
        bool useMove = previewUseMoveSprite_;
        if (ui::ImGuiSound::Checkbox(systemAPI_, "use_move_anim", &useMove)) {
            previewUseMoveSprite_ = useMove;
            ResetAttackPreview();
        }
        ImGui::SameLine();
        if (ui::ImGuiSound::Button(systemAPI_, "Step -1")) {
            const auto& sprite = previewUseMoveSprite_ ? ch->move_sprite : ch->attack_sprite;
            const float frameDuration = std::max(0.01f, sprite.frame_duration);
            SetPreviewTime(previewTime_ - frameDuration);
        }
        ImGui::SameLine();
        if (ui::ImGuiSound::Button(systemAPI_, "Step +1")) {
            const auto& sprite = previewUseMoveSprite_ ? ch->move_sprite : ch->attack_sprite;
            const float frameDuration = std::max(0.01f, sprite.frame_duration);
            SetPreviewTime(previewTime_ + frameDuration);
        }
        ImGui::SliderFloat("speed", &previewSpeed_, 0.1f, 3.0f, "%.2f");
        const float previewDuration = GetPreviewDuration();
        ImGui::SliderFloat("scrub", &previewTime_, 0.0f, previewDuration, "%.3f");
        SetPreviewTime(previewTime_);

        ui::ImGuiSound::Checkbox(systemAPI_, "loop", &previewLoopEnabled_);
        ImGui::SameLine();
        ImGui::InputFloat("loop_start", &previewLoopStart_, 0.01f, 0.05f, "%.3f");
        ImGui::SameLine();
        ImGui::InputFloat("loop_end", &previewLoopEnd_, 0.01f, 0.05f, "%.3f");
        previewLoopStart_ = std::min(std::max(0.0f, previewLoopStart_), previewDuration);
        previewLoopEnd_ = std::min(std::max(0.0f, previewLoopEnd_), previewDuration);

        ImGui::SeparatorText("Visuals");
        ui::ImGuiSound::Checkbox(systemAPI_, "time_bar", &showTimeBar_);
        ImGui::SameLine();
        ui::ImGuiSound::Checkbox(systemAPI_, "hit_marker", &showHitMarker_);
        ImGui::SameLine();
        ui::ImGuiSound::Checkbox(systemAPI_, "range_overlay", &showRangeOverlay_);

        const auto& sprite = previewUseMoveSprite_ ? ch->move_sprite : ch->attack_sprite;
        const float frameDuration = std::max(0.01f, sprite.frame_duration);
        const int frameCount = std::max(1, sprite.frame_count);
        const int currentFrame = std::min(
            static_cast<int>(previewTime_ / frameDuration), frameCount - 1);
        ImGui::SeparatorText("Anim State");
        ImGui::Text("type: %s", previewUseMoveSprite_ ? "Move" : "Attack");
        ImGui::Text("frame: %d / %d", currentFrame, frameCount);
        ImGui::Text("time: %.3f / %.3f", previewTime_, previewDuration);

        ImGui::SeparatorText("Move Simulation");
        ui::ImGuiSound::Checkbox(systemAPI_, "move_sim_enabled", &moveSimEnabled_);
        ImGui::SameLine();
        if (ui::ImGuiSound::Button(systemAPI_, "Reset##MoveSim")) {
            moveSimTime_ = 0.0f;
        }
        ImGui::SliderFloat("move_sim_speed", &moveSimSpeed_, 0.1f, 3.0f, "%.2f");
        ImGui::InputFloat("move_start_offset", &moveSimStartOffset_, 5.0f, 20.0f, "%.1f");
        ImGui::InputFloat("move_target_offset", &moveSimTargetOffset_, 5.0f, 20.0f, "%.1f");
        ui::ImGuiSound::Checkbox(systemAPI_, "move_loop", &moveSimLoop_);
        ui::ImGuiSound::Checkbox(systemAPI_, "move_show_path", &moveSimShowPath_);
        ImGui::SameLine();
        ui::ImGuiSound::Checkbox(systemAPI_, "move_show_stop", &moveSimShowStop_);
        ImGui::SameLine();
        ui::ImGuiSound::Checkbox(systemAPI_, "move_show_distance", &moveSimShowDistance_);
        ImGui::SameLine();
        ui::ImGuiSound::Checkbox(systemAPI_, "move_show_hitbox", &moveSimShowHitbox_);
    } else {
        ImGui::TextDisabled("No character selected");
    }
    ImGui::EndChild();
}

void EditorScene::RenderEquipmentTab() {
    if (ui::ImGuiSound::Button(systemAPI_, "Reload##Equipment")) {
        LoadDataFromAPI();
    }
    ImGui::SameLine();
    if (sharedContext_ && sharedContext_->gameplayDataAPI) {
        if (ui::ImGuiSound::Button(systemAPI_, "Save##Equipment")) {
            lastItemSaveOk_ = sharedContext_->gameplayDataAPI->SaveItemPassiveMasters(
                passiveEdits_, equipmentEdits_);
            if (lastItemSaveOk_) {
                equipmentOriginal_ = equipmentEdits_;
                passiveOriginal_ = passiveEdits_;
            }
            hasItemSaveResult_ = true;
        }
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "New##Equipment")) {
        entities::Equipment eq;
        eq.id = GenerateUniqueId("equipment_", equipmentEdits_);
        eq.name = "New Equipment";
        eq.description = "";
        eq.attack_bonus = 0.0f;
        eq.defense_bonus = 0.0f;
        eq.hp_bonus = 0.0f;
        equipmentEdits_[eq.id] = eq;
        equipmentIds_.push_back(eq.id);
        std::sort(equipmentIds_.begin(), equipmentIds_.end());
        for (int i = 0; i < static_cast<int>(equipmentIds_.size()); ++i) {
            if (equipmentIds_[i] == eq.id) {
                selectedEquipmentIndex_ = i;
                break;
            }
        }
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "Duplicate##Equipment")) {
        if (selectedEquipmentIndex_ >= 0 &&
            selectedEquipmentIndex_ < static_cast<int>(equipmentIds_.size())) {
            const auto& id = equipmentIds_[selectedEquipmentIndex_];
            auto it = equipmentEdits_.find(id);
            if (it != equipmentEdits_.end()) {
                entities::Equipment copy = it->second;
                copy.id = GenerateUniqueId("equipment_", equipmentEdits_);
                copy.name = it->second.name + " Copy";
                equipmentEdits_[copy.id] = copy;
                equipmentIds_.push_back(copy.id);
                std::sort(equipmentIds_.begin(), equipmentIds_.end());
                for (int i = 0; i < static_cast<int>(equipmentIds_.size()); ++i) {
                    if (equipmentIds_[i] == copy.id) {
                        selectedEquipmentIndex_ = i;
                        break;
                    }
                }
            }
        }
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "Delete##Equipment")) {
        if (selectedEquipmentIndex_ >= 0 &&
            selectedEquipmentIndex_ < static_cast<int>(equipmentIds_.size())) {
            const auto id = equipmentIds_[selectedEquipmentIndex_];
            equipmentEdits_.erase(id);
            equipmentIds_.erase(equipmentIds_.begin() + selectedEquipmentIndex_);
            if (selectedEquipmentIndex_ >= static_cast<int>(equipmentIds_.size())) {
                selectedEquipmentIndex_ = static_cast<int>(equipmentIds_.size()) - 1;
            }
        }
    }
    if (hasItemSaveResult_) {
        ImGui::SameLine();
        ImGui::TextUnformatted(lastItemSaveOk_ ? "Save: OK" : "Save: FAILED");
    }
    int modifiedEq = 0;
    for (const auto& id : equipmentIds_) {
        if (IsEquipmentModified(id)) {
            ++modifiedEq;
        }
    }
    ImGui::SameLine();
    ImGui::Text("Modified: %d", modifiedEq);
    InputTextStdString("Filter##Equipment", &equipmentFilter_, 128);

    ImGui::Separator();

    ImGui::BeginChild("EquipmentList", ImVec2(260, 0), true);
    for (int i = 0; i < static_cast<int>(equipmentIds_.size()); ++i) {
        const auto& id = equipmentIds_[i];
        const auto* eq = (equipmentEdits_.count(id) > 0) ? &equipmentEdits_.at(id) : nullptr;
        std::string label = id;
        if (eq) {
            label += " - " + eq->name;
        }
        if (!ContainsIgnoreCase(label, equipmentFilter_)) {
            continue;
        }
        if (IsEquipmentModified(id)) {
            label = "* " + label;
        }
        const bool selected = (i == selectedEquipmentIndex_);
        if (ui::ImGuiSound::Selectable(systemAPI_, label.c_str(), selected)) {
            selectedEquipmentIndex_ = i;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("EquipmentDetail", ImVec2(0, 0), true);
    if (selectedEquipmentIndex_ >= 0 &&
        selectedEquipmentIndex_ < static_cast<int>(equipmentIds_.size())) {
        const auto& id = equipmentIds_[selectedEquipmentIndex_];
        auto it = equipmentEdits_.find(id);
        if (it != equipmentEdits_.end()) {
            auto& eq = it->second;
            ImGui::Text("ID: %s", eq.id.c_str());
            InputTextStdString("Name", &eq.name);
            InputTextMultilineStdString("Description", &eq.description);
            ImGui::InputFloat("attack_bonus", &eq.attack_bonus, 0.1f, 1.0f, "%.2f");
            ImGui::InputFloat("defense_bonus", &eq.defense_bonus, 0.1f, 1.0f, "%.2f");
            ImGui::InputFloat("hp_bonus", &eq.hp_bonus, 0.1f, 1.0f, "%.2f");
        }
    } else {
        ImGui::TextDisabled("No equipment selected");
    }
    ImGui::EndChild();
}

void EditorScene::RenderPassiveTab() {
    if (ui::ImGuiSound::Button(systemAPI_, "Reload##Passives")) {
        LoadDataFromAPI();
    }
    ImGui::SameLine();
    if (sharedContext_ && sharedContext_->gameplayDataAPI) {
        if (ui::ImGuiSound::Button(systemAPI_, "Save##Passives")) {
            lastItemSaveOk_ = sharedContext_->gameplayDataAPI->SaveItemPassiveMasters(
                passiveEdits_, equipmentEdits_);
            if (lastItemSaveOk_) {
                passiveOriginal_ = passiveEdits_;
                equipmentOriginal_ = equipmentEdits_;
            }
            hasItemSaveResult_ = true;
        }
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "New##Passives")) {
        entities::PassiveSkill ps;
        ps.id = GenerateUniqueId("passive_", passiveEdits_);
        ps.name = "New Passive";
        ps.description = "";
        ps.value = 0.1f;
        ps.effect_type = entities::PassiveEffectType::Percentage;
        ps.target_stat = entities::PassiveTargetStat::Attack;
        ps.rarity = 1;
        passiveEdits_[ps.id] = ps;
        passiveIds_.push_back(ps.id);
        std::sort(passiveIds_.begin(), passiveIds_.end());
        for (int i = 0; i < static_cast<int>(passiveIds_.size()); ++i) {
            if (passiveIds_[i] == ps.id) {
                selectedPassiveIndex_ = i;
                break;
            }
        }
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "Duplicate##Passives")) {
        if (selectedPassiveIndex_ >= 0 &&
            selectedPassiveIndex_ < static_cast<int>(passiveIds_.size())) {
            const auto& id = passiveIds_[selectedPassiveIndex_];
            auto it = passiveEdits_.find(id);
            if (it != passiveEdits_.end()) {
                entities::PassiveSkill copy = it->second;
                copy.id = GenerateUniqueId("passive_", passiveEdits_);
                copy.name = it->second.name + " Copy";
                passiveEdits_[copy.id] = copy;
                passiveIds_.push_back(copy.id);
                std::sort(passiveIds_.begin(), passiveIds_.end());
                for (int i = 0; i < static_cast<int>(passiveIds_.size()); ++i) {
                    if (passiveIds_[i] == copy.id) {
                        selectedPassiveIndex_ = i;
                        break;
                    }
                }
            }
        }
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "Delete##Passives")) {
        if (selectedPassiveIndex_ >= 0 &&
            selectedPassiveIndex_ < static_cast<int>(passiveIds_.size())) {
            const auto id = passiveIds_[selectedPassiveIndex_];
            passiveEdits_.erase(id);
            passiveIds_.erase(passiveIds_.begin() + selectedPassiveIndex_);
            if (selectedPassiveIndex_ >= static_cast<int>(passiveIds_.size())) {
                selectedPassiveIndex_ = static_cast<int>(passiveIds_.size()) - 1;
            }
        }
    }
    if (hasItemSaveResult_) {
        ImGui::SameLine();
        ImGui::TextUnformatted(lastItemSaveOk_ ? "Save: OK" : "Save: FAILED");
    }
    int modifiedPassive = 0;
    for (const auto& id : passiveIds_) {
        if (IsPassiveModified(id)) {
            ++modifiedPassive;
        }
    }
    ImGui::SameLine();
    ImGui::Text("Modified: %d", modifiedPassive);
    InputTextStdString("Filter##Passives", &passiveFilter_, 128);

    ImGui::Separator();

    ImGui::BeginChild("PassiveList", ImVec2(260, 0), true);
    for (int i = 0; i < static_cast<int>(passiveIds_.size()); ++i) {
        const auto& id = passiveIds_[i];
        const auto* ps = (passiveEdits_.count(id) > 0) ? &passiveEdits_.at(id) : nullptr;
        std::string label = id;
        if (ps) {
            label += " - " + ps->name;
        }
        if (!ContainsIgnoreCase(label, passiveFilter_)) {
            continue;
        }
        if (IsPassiveModified(id)) {
            label = "* " + label;
        }
        const bool selected = (i == selectedPassiveIndex_);
        if (ui::ImGuiSound::Selectable(systemAPI_, label.c_str(), selected)) {
            selectedPassiveIndex_ = i;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("PassiveDetail", ImVec2(0, 0), true);
    if (selectedPassiveIndex_ >= 0 &&
        selectedPassiveIndex_ < static_cast<int>(passiveIds_.size())) {
        const auto& id = passiveIds_[selectedPassiveIndex_];
        auto it = passiveEdits_.find(id);
        if (it != passiveEdits_.end()) {
            auto& ps = it->second;
            ImGui::Text("ID: %s", ps.id.c_str());
            InputTextStdString("Name", &ps.name);
            InputTextMultilineStdString("Description", &ps.description);
            ImGui::InputFloat("value", &ps.value, 0.01f, 0.05f, "%.3f");
            ImGui::InputInt("rarity", &ps.rarity);
            ps.rarity = std::max(1, ps.rarity);

            const char* effectTypes[] = {"Percentage", "Flat"};
            int effectIndex = (ps.effect_type == entities::PassiveEffectType::Flat) ? 1 : 0;
            if (ui::ImGuiSound::Combo(systemAPI_, "effect_type", &effectIndex, effectTypes, 2)) {
                ps.effect_type = (effectIndex == 1) ? entities::PassiveEffectType::Flat
                                                    : entities::PassiveEffectType::Percentage;
            }

            const char* targetStats[] = {"Attack", "Defense", "Hp", "MoveSpeed", "AttackSpeed",
                                         "Range", "CritChance", "CritDamage", "GoldGain", "ExpGain"};
            int targetIndex = 0;
            switch (ps.target_stat) {
            case entities::PassiveTargetStat::Attack: targetIndex = 0; break;
            case entities::PassiveTargetStat::Defense: targetIndex = 1; break;
            case entities::PassiveTargetStat::Hp: targetIndex = 2; break;
            case entities::PassiveTargetStat::MoveSpeed: targetIndex = 3; break;
            case entities::PassiveTargetStat::AttackSpeed: targetIndex = 4; break;
            case entities::PassiveTargetStat::Range: targetIndex = 5; break;
            case entities::PassiveTargetStat::CritChance: targetIndex = 6; break;
            case entities::PassiveTargetStat::CritDamage: targetIndex = 7; break;
            case entities::PassiveTargetStat::GoldGain: targetIndex = 8; break;
            case entities::PassiveTargetStat::ExpGain: targetIndex = 9; break;
            default: targetIndex = 0; break;
            }
            if (ui::ImGuiSound::Combo(systemAPI_, "target_stat", &targetIndex, targetStats, 10)) {
                switch (targetIndex) {
                case 0: ps.target_stat = entities::PassiveTargetStat::Attack; break;
                case 1: ps.target_stat = entities::PassiveTargetStat::Defense; break;
                case 2: ps.target_stat = entities::PassiveTargetStat::Hp; break;
                case 3: ps.target_stat = entities::PassiveTargetStat::MoveSpeed; break;
                case 4: ps.target_stat = entities::PassiveTargetStat::AttackSpeed; break;
                case 5: ps.target_stat = entities::PassiveTargetStat::Range; break;
                case 6: ps.target_stat = entities::PassiveTargetStat::CritChance; break;
                case 7: ps.target_stat = entities::PassiveTargetStat::CritDamage; break;
                case 8: ps.target_stat = entities::PassiveTargetStat::GoldGain; break;
                case 9: ps.target_stat = entities::PassiveTargetStat::ExpGain; break;
                default: ps.target_stat = entities::PassiveTargetStat::Attack; break;
                }
            }
        }
    } else {
        ImGui::TextDisabled("No passive selected");
    }
    ImGui::EndChild();
}

void EditorScene::RenderStageTab() {
    if (ui::ImGuiSound::Button(systemAPI_, "Reload##Stages")) {
        LoadDataFromAPI();
    }
    ImGui::SameLine();
    if (sharedContext_ && sharedContext_->gameplayDataAPI) {
        if (ui::ImGuiSound::Button(systemAPI_, "Save##Stages")) {
            lastStageSaveOk_ =
                sharedContext_->gameplayDataAPI->SaveStageMasters(stageEdits_);
            if (lastStageSaveOk_) {
                stageOriginal_ = stageEdits_;
            }
            hasStageSaveResult_ = true;
        }
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "New##Stages")) {
        entities::StageData stage;
        stage.id = GenerateUniqueId("stage_", stageEdits_);
        stage.stageName = "New Stage";
        stage.stageNumber = 0;
        stage.chapter = 1;
        stage.chapterName = "Chapter 1";
        stage.difficulty = 1;
        stage.isBoss = false;
        stage.isLocked = true;
        stage.rewardGold = 100;
        stage.waveCount = 1;
        stage.recommendedLevel = 1;
        stage.previewImageId = "";
        stage.data = nlohmann::json::object();
        stageEdits_[stage.id] = stage;
        stageIds_.push_back(stage.id);
        std::sort(stageIds_.begin(), stageIds_.end());
        for (int i = 0; i < static_cast<int>(stageIds_.size()); ++i) {
            if (stageIds_[i] == stage.id) {
                selectedStageIndex_ = i;
                break;
            }
        }
        stageJsonText_ = stage.data.dump(2);
        stageJsonError_.clear();
        stageUnlockText_.clear();
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "Duplicate##Stages")) {
        if (selectedStageIndex_ >= 0 &&
            selectedStageIndex_ < static_cast<int>(stageIds_.size())) {
            const auto& id = stageIds_[selectedStageIndex_];
            auto it = stageEdits_.find(id);
            if (it != stageEdits_.end()) {
                entities::StageData copy = it->second;
                copy.id = GenerateUniqueId("stage_", stageEdits_);
                copy.stageName = it->second.stageName + " Copy";
                copy.stageNumber = 0;
                stageEdits_[copy.id] = copy;
                stageIds_.push_back(copy.id);
                std::sort(stageIds_.begin(), stageIds_.end());
                for (int i = 0; i < static_cast<int>(stageIds_.size()); ++i) {
                    if (stageIds_[i] == copy.id) {
                        selectedStageIndex_ = i;
                        break;
                    }
                }
                stageJsonText_ = copy.data.dump(2);
                stageJsonError_.clear();
                stageUnlockText_.clear();
                for (size_t i = 0; i < copy.unlockOnClear.size(); ++i) {
                    if (i > 0) {
                        stageUnlockText_ += ", ";
                    }
                    stageUnlockText_ += copy.unlockOnClear[i];
                }
            }
        }
    }
    ImGui::SameLine();
    if (ui::ImGuiSound::Button(systemAPI_, "Delete##Stages")) {
        if (selectedStageIndex_ >= 0 &&
            selectedStageIndex_ < static_cast<int>(stageIds_.size())) {
            const auto id = stageIds_[selectedStageIndex_];
            stageEdits_.erase(id);
            stageIds_.erase(stageIds_.begin() + selectedStageIndex_);
            if (selectedStageIndex_ >= static_cast<int>(stageIds_.size())) {
                selectedStageIndex_ = static_cast<int>(stageIds_.size()) - 1;
            }
            stageJsonText_.clear();
            stageJsonError_.clear();
            stageUnlockText_.clear();
        }
    }
    if (hasStageSaveResult_) {
        ImGui::SameLine();
        ImGui::TextUnformatted(lastStageSaveOk_ ? "Save: OK" : "Save: FAILED");
    }

    int modifiedStages = 0;
    for (const auto& id : stageIds_) {
        if (IsStageModified(id)) {
            ++modifiedStages;
        }
    }
    ImGui::SameLine();
    ImGui::Text("Modified: %d", modifiedStages);
    InputTextStdString("Filter##Stages", &stageFilter_, 128);

    ImGui::Separator();

    ImGui::BeginChild("StageList", ImVec2(260, 0), true);
    for (int i = 0; i < static_cast<int>(stageIds_.size()); ++i) {
        const auto& id = stageIds_[i];
        auto it = stageEdits_.find(id);
        if (it == stageEdits_.end()) {
            continue;
        }
        std::string label = id + " - " + it->second.stageName;
        if (!ContainsIgnoreCase(label, stageFilter_)) {
            continue;
        }
        if (IsStageModified(id)) {
            label = "* " + label;
        }
        const bool selected = (i == selectedStageIndex_);
        if (ui::ImGuiSound::Selectable(systemAPI_, label.c_str(), selected)) {
            selectedStageIndex_ = i;
            stageJsonText_ = it->second.data.dump(2);
            stageJsonError_.clear();
            stageUnlockText_.clear();
            for (size_t idx = 0; idx < it->second.unlockOnClear.size(); ++idx) {
                if (idx > 0) {
                    stageUnlockText_ += ", ";
                }
                stageUnlockText_ += it->second.unlockOnClear[idx];
            }
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("StageDetail", ImVec2(0, 0), true);
    if (selectedStageIndex_ >= 0 &&
        selectedStageIndex_ < static_cast<int>(stageIds_.size())) {
        const auto& id = stageIds_[selectedStageIndex_];
        auto it = stageEdits_.find(id);
        if (it != stageEdits_.end()) {
            auto& stage = it->second;
            ImGui::Text("ID: %s", stage.id.c_str());

            ImGui::InputInt("StageNumber", &stage.stageNumber);
            stage.stageNumber = std::max(0, stage.stageNumber);
            ImGui::InputInt("Chapter", &stage.chapter);
            stage.chapter = std::max(0, stage.chapter);
            InputTextStdString("ChapterName", &stage.chapterName);
            InputTextStdString("StageName", &stage.stageName);
            ImGui::InputInt("Difficulty", &stage.difficulty);
            stage.difficulty = std::max(1, stage.difficulty);
            ui::ImGuiSound::Checkbox(systemAPI_, "IsBoss", &stage.isBoss);
            ui::ImGuiSound::Checkbox(systemAPI_, "IsLocked", &stage.isLocked);
            ImGui::InputInt("RewardGold", &stage.rewardGold);
            stage.rewardGold = std::max(0, stage.rewardGold);
            ImGui::InputInt("WaveCount", &stage.waveCount);
            stage.waveCount = std::max(0, stage.waveCount);
            ImGui::InputInt("RecommendedLevel", &stage.recommendedLevel);
            stage.recommendedLevel = std::max(1, stage.recommendedLevel);
            InputTextStdString("PreviewImageId", &stage.previewImageId);
            if (InputTextStdString("UnlockOnClear", &stageUnlockText_, 256)) {
                stage.unlockOnClear = SplitCommaSeparated(stageUnlockText_);
            }

            ImGui::SeparatorText("Stage JSON");
            if (ui::ImGuiSound::Button(systemAPI_, "Apply JSON")) {
                try {
                    stage.data = nlohmann::json::parse(stageJsonText_);
                    stageJsonError_.clear();
                } catch (const nlohmann::json::parse_error& e) {
                    stageJsonError_ = e.what();
                } catch (const std::exception& e) {
                    stageJsonError_ = e.what();
                }
            }
            ImGui::SameLine();
            if (ui::ImGuiSound::Button(systemAPI_, "Refresh JSON")) {
                stageJsonText_ = stage.data.dump(2);
                stageJsonError_.clear();
            }
            if (!stageJsonError_.empty()) {
                ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f),
                                   "JSON Error: %s", stageJsonError_.c_str());
            }
            InputTextMultilineStdString("JSON##Stage", &stageJsonText_, ImVec2(0, 220), 8192);
        }
    } else {
        ImGui::TextDisabled("No stage selected");
    }
    ImGui::EndChild();
}

void EditorScene::RenderBattleDebugTab() {
    if (sharedContext_ && sharedContext_->battleProgressAPI) {
        auto* battle = sharedContext_->battleProgressAPI;
        attackLogEnabled_ = battle->IsAttackLogEnabled();
        float speed = battle->GetGameSpeed();
        bool paused = battle->IsPaused();
        if (ImGui::SliderFloat("GameSpeed", &speed, 0.1f, 3.0f, "%.2f")) {
            battle->SetGameSpeed(speed);
        }
        if (ui::ImGuiSound::Checkbox(systemAPI_, "Paused", &paused)) {
            battle->SetPaused(paused);
        }
        ImGui::SameLine();
        if (ui::ImGuiSound::Button(systemAPI_, "Step 1/60")) {
            battle->Update(1.0f / 60.0f);
        }

        ImGui::Separator();
        if (ui::ImGuiSound::Checkbox(systemAPI_, "AttackLogEnabled", &attackLogEnabled_)) {
            battle->SetAttackLogEnabled(attackLogEnabled_);
        }
        ImGui::SameLine();
        if (ui::ImGuiSound::Button(systemAPI_, "ClearLog")) {
            battle->ClearAttackLog();
        }
        ui::ImGuiSound::Checkbox(systemAPI_, "ShowAttackLog", &showAttackLog_);
        if (showAttackLog_) {
            ImGui::BeginChild("AttackLog", ImVec2(0, 160), true);
            for (const auto& entry : battle->GetAttackLog()) {
                ImGui::Text("[%.2f] %s -> %s dmg=%d %s",
                            entry.time,
                            entry.attackerId.c_str(),
                            entry.targetId.c_str(),
                            entry.damage,
                            entry.hit ? "hit" : "miss");
            }
            ImGui::EndChild();
        }

        ui::ImGuiSound::Checkbox(systemAPI_, "ShowStatusOverlay", &showStatusOverlay_);
        if (showStatusOverlay_ && sharedContext_->ecsAPI) {
            auto view = sharedContext_->ecsAPI->View<ecs::components::Position,
                                                    ecs::components::Health,
                                                    ecs::components::Stats,
                                                    ecs::components::Movement,
                                                    ecs::components::Team>();
            ImGui::BeginChild("StatusOverlay", ImVec2(0, 220), true);
            ImGui::Text("Entities: %d", static_cast<int>(view.size_hint()));
            for (auto e : view) {
                const auto& pos = view.get<ecs::components::Position>(e);
                const auto& hp = view.get<ecs::components::Health>(e);
                const auto& stats = view.get<ecs::components::Stats>(e);
                const auto& move = view.get<ecs::components::Movement>(e);
                const auto* cid = sharedContext_->ecsAPI->Try<ecs::components::CharacterId>(e);
                ImGui::Text("E%u %s HP:%d/%d ATK:%d DEF:%d SPD:%.1f Pos:(%.1f,%.1f)",
                            static_cast<unsigned int>(e),
                            cid ? cid->id.c_str() : "unknown",
                            hp.current, hp.max,
                            stats.attack, stats.defense,
                            move.speed,
                            pos.x, pos.y);
            }
            ImGui::EndChild();
        }

        ImGui::SeparatorText("Test Spawn");
        if (characterIds_.empty()) {
            ImGui::TextDisabled("No characters loaded");
        } else {
            if (spawnCharacterIndex_ < 0 ||
                spawnCharacterIndex_ >= static_cast<int>(characterIds_.size())) {
                spawnCharacterIndex_ = 0;
            }
            const char* preview = characterIds_[spawnCharacterIndex_].c_str();
            if (ImGui::BeginCombo("CharacterId", preview)) {
                for (int i = 0; i < static_cast<int>(characterIds_.size()); ++i) {
                    const bool selected = (i == spawnCharacterIndex_);
                    if (ui::ImGuiSound::Selectable(systemAPI_, characterIds_[i].c_str(), selected)) {
                        spawnCharacterIndex_ = i;
                    }
                }
                ImGui::EndCombo();
            }

            ui::ImGuiSound::Checkbox(systemAPI_, "SpawnAsEnemy", &spawnAsEnemy_);
            ImGui::InputFloat("SpawnX", &spawnX_, 10.0f, 50.0f, "%.1f");
            ImGui::InputFloat("SpawnY", &spawnY_, 10.0f, 50.0f, "%.1f");
            ImGui::InputInt("SpawnLevel", &spawnLevel_);
            spawnLevel_ = std::max(1, spawnLevel_);

            if (ui::ImGuiSound::Button(systemAPI_, "Spawn##Debug")) {
                if (sharedContext_->setupAPI && sharedContext_->ecsAPI &&
                    sharedContext_->gameplayDataAPI) {
                    const auto& charId = characterIds_[spawnCharacterIndex_];
                    auto character = sharedContext_->gameplayDataAPI->GetCharacterTemplate(charId);
                    if (character) {
                        entities::EntityCreationData creationData;
                        creationData.character_id = character->id;
                        creationData.position = {spawnX_, spawnY_};
                        creationData.level = spawnLevel_;
                        const auto faction = spawnAsEnemy_
                                                 ? ecs::components::Faction::Enemy
                                                 : ecs::components::Faction::Player;
                        sharedContext_->setupAPI->CreateBattleEntityFromCharacter(
                            *character, creationData, faction, nullptr);
                    }
                }
            }
        }
    } else {
        ImGui::TextDisabled("battleProgressAPI not available");
    }
}

} // namespace states
} // namespace core
} // namespace game
