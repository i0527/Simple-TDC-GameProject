#include "Editor/Windows/ValidationPanel.h"

#include <imgui.h>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"
#include "Data/Validators/DataValidator.h"

namespace Editor::Windows {

ValidationPanel::ValidationPanel() {
  std::memset(filterBuf_, 0, sizeof(filterBuf_));
}

ValidationPanel::~ValidationPanel() {}

void ValidationPanel::Initialize(Shared::Core::GameContext &context,
                                 Shared::Data::DefinitionRegistry &definitions) {
  context_ = &context;
  definitions_ = &definitions;
}

void ValidationPanel::Shutdown() {}

void ValidationPanel::FocusEntity(const std::string& id) {
  if (unitEditor_ && !id.empty()) {
    unitEditor_->SetOpen(true);
    unitEditor_->SetActiveEntity(id);
  }
}

void ValidationPanel::FocusSkill(const std::string& /*id*/) {}
void ValidationPanel::FocusAbility(const std::string& /*id*/) {}
void ValidationPanel::FocusStage(const std::string& /*id*/) {}
void ValidationPanel::FocusWave(const std::string& /*id*/) {}

void ValidationPanel::OnUpdate(float deltaTime) {
  if (!isOpen_) return;
  if (autoRevalidate_) {
    elapsedSeconds_ += deltaTime;
    if (elapsedSeconds_ >= intervalSeconds_) {
      elapsedSeconds_ = 0.0f;
      RunValidation();
    }
  }
}

void ValidationPanel::RunValidation() {
  if (!definitions_) return;
  Shared::Data::DataValidator::ClearErrors();
  const double t0 = ImGui::GetTime();
  lastOk_ = Shared::Data::DataValidator::Validate(*definitions_);
  lastRunSeconds_ = static_cast<float>(ImGui::GetTime() - t0);
}

void ValidationPanel::SetupFileWatches() {
  if (!context_) return;
  ClearFileWatches();
  try {
    auto &fw = context_->GetFileWatcher();
    auto add = [&](const std::string &abs){
      watchedPaths_.push_back(abs);
      fw.Watch(abs, [this]() { this->RunValidation(); });
    };

    add(context_->GetDataPath("entities/debug.json"));
    add(context_->GetDataPath("abilities/debug.json"));
    add(context_->GetDataPath("stages/debug.json"));
    add(context_->GetDataPath("waves/debug.json"));

    // characters 以下の entity.json を監視
    const std::string charsDir = context_->GetDataPath("entities/characters");
    if (std::filesystem::exists(charsDir) && std::filesystem::is_directory(charsDir)) {
      for (const auto &entry : std::filesystem::recursive_directory_iterator(charsDir)) {
        if (entry.is_regular_file() && entry.path().filename() == "entity.json") {
          add(entry.path().string());
        }
      }
    }
  } catch (...) {
    // 監視設定の失敗は致命的ではないため黙殺
  }
}

void ValidationPanel::ClearFileWatches() {
  if (!context_) return;
  try {
    auto &fw = context_->GetFileWatcher();
    for (const auto &p : watchedPaths_) {
      fw.Unwatch(p);
    }
    watchedPaths_.clear();
  } catch (...) {
  }
}

void ValidationPanel::OnDrawUI() {
  if (!isOpen_) return;

  if (ImGui::Begin("バリデーション", &isOpen_)) {
    if (ImGui::Button("検証を実行")) {
      RunValidation();
    }
    ImGui::SameLine();
    ImGui::Text("前回結果: %s (%.2fs)", lastOk_ ? "OK" : "NG", lastRunSeconds_);
    ImGui::SameLine();
    ImGui::Checkbox("自動再検証", &autoRevalidate_);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    ImGui::SliderFloat("間隔(秒)", &intervalSeconds_, 1.0f, 30.0f, "%.0f");

    ImGui::SameLine();
    if (ImGui::Button("エクスポート")) {
      try {
        std::filesystem::create_directories("validation");
        std::ofstream ofs("validation/last_errors.txt", std::ios::out | std::ios::trunc);
        const auto &errors = Shared::Data::DataValidator::GetErrors();
        for (const auto &e : errors) {
          ofs << e << "\n";
        }
      } catch (...) {
        // 失敗はUI表示のみで無視
      }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("フィルタ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(300.0f);
    ImGui::InputText("##validation_filter", filterBuf_, sizeof(filterBuf_));
    ImGui::SameLine();
    ImGui::Checkbox("テーブル表示", &tableView_);
    ImGui::SameLine();
    if (ImGui::Checkbox("ファイル監視で再検証", &watchFiles_)) {
      if (watchFiles_) SetupFileWatches(); else ClearFileWatches();
    }
    ImGui::SameLine();
    if (ImGui::Button("再スキャン")) {
      if (watchFiles_) {
        SetupFileWatches();
        RunValidation();
      }
    }

    const auto &allErrors = Shared::Data::DataValidator::GetErrors();
    ImGui::Separator();
    ImGui::Text("エラー数: %d", static_cast<int>(allErrors.size()));

    auto isMatch = [&](const std::string &err, const std::string &filter){
      return filter.empty() || err.find(filter) != std::string::npos;
    };

    auto parseTypeId = [](const std::string& e){
      struct R { std::string type; std::string id; } r;
      const char* types[] = {"Entity ", "Skill ", "Stage ", "Wave ", "Ability "};
      for (auto t : types) {
        if (e.rfind(t, 0) == 0) {
          r.type = std::string(t, std::strlen(t) - 1); // remove trailing space
          // find quoted id
          std::string key = std::string(r.type) + " '";
          auto pos = e.find(key);
          if (pos != std::string::npos) {
            auto start = pos + key.size();
            auto end = e.find("'", start);
            if (end != std::string::npos && end > start) r.id = e.substr(start, end - start);
          }
          break;
        }
      }
      return r;
    };

    auto drawList = [&](const std::vector<std::string>& list, bool entityList){
      ImGui::BeginChild("validation_errors_child", ImVec2(0, 0), true);
      if (tableView_) {
        if (ImGui::BeginTable("validation_table", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable)) {
          ImGui::TableSetupColumn("Type");
          ImGui::TableSetupColumn("Target");
          ImGui::TableSetupColumn("Message");
          ImGui::TableHeadersRow();
          for (const auto &err : list) {
            if (!isMatch(err, filterBuf_)) continue;
            auto p = parseTypeId(err);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(p.type.c_str());
            ImGui::TableSetColumnIndex(1);
            if (!p.id.empty()) {
              if (ImGui::Selectable(p.id.c_str(), false, ImGuiSelectableFlags_SpanAllColumns)) {
                // noop
              }
              if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                if (p.type == "Entity") FocusEntity(p.id);
                else if (p.type == "Skill") FocusSkill(p.id);
                else if (p.type == "Ability") FocusAbility(p.id);
                else if (p.type == "Stage") FocusStage(p.id);
                else if (p.type == "Wave") FocusWave(p.id);
              }
            }
            ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(err.c_str());
          }
          ImGui::EndTable();
        }
      } else {
        for (const auto &err : list) {
          if (!isMatch(err, filterBuf_)) continue;
          if (ImGui::Selectable(err.c_str(), false)) {}
          if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            // シンプルなメッセージから Entity のみ抽出 (既存互換)
            if (entityList && unitEditor_) {
              std::string id;
              const std::string key = "Entity '";
              auto pos = err.find(key);
              if (pos != std::string::npos) {
                auto start = pos + key.size();
                auto end = err.find("'", start);
                if (end != std::string::npos && end > start) id = err.substr(start, end - start);
              }
              if (!id.empty()) {
                FocusEntity(id);
              }
            }
          }
        }
      }
      if (list.empty() && lastOk_) {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "エラーはありません。");
      }
      ImGui::EndChild();
    };

    // カテゴリ分け
    std::vector<std::string> ent, skl, stg, wav, abl, oth;
    ent.reserve(allErrors.size());
    for (const auto &e : allErrors) {
      if (e.rfind("Entity ", 0) == 0) ent.push_back(e);
      else if (e.rfind("Skill ", 0) == 0) skl.push_back(e);
      else if (e.rfind("Stage ", 0) == 0) stg.push_back(e);
      else if (e.rfind("Wave ", 0) == 0) wav.push_back(e);
      else if (e.rfind("Ability ", 0) == 0) abl.push_back(e);
      else oth.push_back(e);
    }

    if (ImGui::BeginTabBar("validation_tabs")) {
      if (ImGui::BeginTabItem((std::string("Entities (") + std::to_string(ent.size()) + ")").c_str())) { drawList(ent, true); ImGui::EndTabItem(); }
      if (ImGui::BeginTabItem((std::string("Skills (") + std::to_string(skl.size()) + ")").c_str())) { drawList(skl, false); ImGui::EndTabItem(); }
      if (ImGui::BeginTabItem((std::string("Stages (") + std::to_string(stg.size()) + ")").c_str())) { drawList(stg, false); ImGui::EndTabItem(); }
      if (ImGui::BeginTabItem((std::string("Waves (") + std::to_string(wav.size()) + ")").c_str()))  { drawList(wav, false); ImGui::EndTabItem(); }
      if (ImGui::BeginTabItem((std::string("Abilities (") + std::to_string(abl.size()) + ")").c_str())) { drawList(abl, false); ImGui::EndTabItem(); }
      if (ImGui::BeginTabItem((std::string("Others (") + std::to_string(oth.size()) + ")").c_str())) { drawList(oth, false); ImGui::EndTabItem(); }
      ImGui::EndTabBar();
    }
  }
  ImGui::End();
}

std::string ValidationPanel::GetWindowTitle() const { return "バリデーション"; }

std::string ValidationPanel::GetWindowId() const { return "validation_panel"; }

bool ValidationPanel::IsOpen() const { return isOpen_; }

void ValidationPanel::SetOpen(bool open) { isOpen_ = open; }

} // namespace Editor::Windows
