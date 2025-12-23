#include "Editor/Windows/PreviewWindow.h"

#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"
#include "Game/Components/NewCoreComponents.h"
#include <imgui.h>
#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace Editor::Windows {

namespace {
std::string ResolveIconPathImpl(const Shared::Data::EntityDef& def) {
  namespace fs = std::filesystem;

  auto exists_path = [](const fs::path& p) {
    return !p.empty() && fs::exists(p);
  };

  if (exists_path(def.display.icon)) {
    return fs::path(def.display.icon).lexically_normal().generic_string();
  }

  fs::path hint = def.display.icon.empty() ? fs::path(def.display.atlas_texture) : fs::path(def.display.icon);
  if (!hint.empty()) {
    fs::path folder = hint.parent_path().filename();
    std::string tier = def.type.empty() ? "main" : def.type; // type: main/sub
    fs::path candidate = fs::path("assets/textures/icons/characters") / tier / folder / "icon.png";
    if (exists_path(candidate)) {
      return candidate.lexically_normal().generic_string();
    }
  }

  // Fallback: resolve by definition file location
  if (!def.source_path.empty()) {
    fs::path src(def.source_path);
    fs::path folder = src.parent_path().filename();
    std::string tier = def.type.empty() ? "main" : def.type;
    fs::path candidate = fs::path("assets/textures/icons/characters") / tier / folder / "icon.png";
    if (exists_path(candidate)) {
      return candidate.lexically_normal().generic_string();
    }
  }

  return {};
}
} // namespace

PreviewWindow::PreviewWindow() {
  preview_texture_initialized_ = false;
  preview_texture_ = {};
}

PreviewWindow::~PreviewWindow() {
  Shutdown();
}

void PreviewWindow::Initialize(Shared::Core::GameContext &context,
                               Shared::Data::DefinitionRegistry &definitions) {
  context_ = &context;
  definitions_ = &definitions;
  simulation_ = &context.GetSimulation();
  entity_reload_handle_ = definitions.OnEntityDefinitionReloaded.Connect(
      [this](const std::string &id) {
        if (id == current_entity_id_) {
          LoadEntity(id);
        }
      });
}

void PreviewWindow::Shutdown() {
  if (definitions_ && entity_reload_handle_ != 0) {
    definitions_->OnEntityDefinitionReloaded.Disconnect(entity_reload_handle_);
    entity_reload_handle_ = 0;
  }
  UnloadPreviewAssets();
  if (preview_texture_initialized_ && preview_texture_.texture.id != 0) {
    UnloadRenderTexture(preview_texture_);
    preview_texture_initialized_ = false;
  }
  Clear();
}

void PreviewWindow::OnUpdate(float deltaTime) {
  if (!is_open_ || !simulation_)
    return;
  
  // アニメーション更新（再生中の場合のみ）
  if (is_playing_ && preview_entity_ != entt::null) {
    UpdateAnimation(deltaTime * animation_speed_);
    // フレームアニメ進行（プレビュー矩形更新）
    if (!current_frames_.empty()) {
      current_frame_timer_ += deltaTime * animation_speed_;
      float baseDur = std::max(0.001f, current_frames_[current_frame_index_].duration);
      float dur = override_frame_duration_ ? std::max(0.001f, frame_duration_override_) : baseDur;
      while (current_frame_timer_ >= dur) {
        current_frame_timer_ -= dur;
        current_frame_index_ = (current_frame_index_ + 1) % static_cast<int>(current_frames_.size());
        baseDur = std::max(0.001f, current_frames_[current_frame_index_].duration);
        dur = override_frame_duration_ ? std::max(0.001f, frame_duration_override_) : baseDur;
      }
      preview_frame_ = current_frames_[current_frame_index_].rect;
      preview_frame_valid_ = atlas_width_ > 0 && atlas_height_ > 0 && preview_frame_.width > 0 && preview_frame_.height > 0;
    }
  }
  // ステージ動作（簡易）
  if (preview_entity_ != entt::null && simulate_movement_) {
    auto& registry = simulation_->GetRegistry();
    if (registry.all_of<Game::Components::Transform>(preview_entity_)) {
      auto& tf = registry.get<Game::Components::Transform>(preview_entity_);
      patrol_phase_ += deltaTime * move_speed_;
      float half = patrol_width_ * 0.5f;
      tf.x = std::sin(patrol_phase_ / std::max(1.0f, half)) * half;
      tf.y = ground_y_;
    }
  }
  
  simulation_->Update(deltaTime);
}

void PreviewWindow::OnDrawUI() {
  if (!is_open_)
    return;

  ImGui::Begin(GetWindowTitle().c_str(), &is_open_);
  
  // エンティティ選択
  ImGui::Text("Preview Entity: %s",
              current_entity_id_.empty() ? "(none)" : current_entity_id_.c_str());
  
  // プレビューエリア
  DrawPreviewArea();

  // ステージ動作設定（上部コントロール）
  ImGui::Separator();
  ImGui::Text("ステージ動作設定");
  ImGui::Checkbox("移動プレビュー", &simulate_movement_);
  ImGui::SameLine();
  ImGui::SliderFloat("速度", &move_speed_, 0.0f, 240.0f, "%.0f");
  ImGui::SliderFloat("巡回幅", &patrol_width_, 50.0f, 600.0f, "%.0f");
  ImGui::SliderFloat("地面Y", &ground_y_, -200.0f, 200.0f, "%.0f");

  // アクション選択
  if (definitions_ && !current_entity_id_.empty()) {
    const auto* def = definitions_->GetEntity(current_entity_id_);
    if (def) {
      // アクション一覧を作成
      std::vector<std::string> actions;
      actions.reserve(def->display.sprite_actions.size());
      for (const auto& kv : def->display.sprite_actions) actions.push_back(kv.first);
      int currentIndex = 0;
      for (size_t i = 0; i < actions.size(); ++i) if (actions[i] == current_action_) currentIndex = (int)i;
      if (!actions.empty()) {
        ImGui::Text("アニメーションアクション");
        if (ImGui::BeginCombo("##action", actions[currentIndex].c_str())) {
          for (size_t i = 0; i < actions.size(); ++i) {
            bool selected = (int)i == currentIndex;
            if (ImGui::Selectable(actions[i].c_str(), selected)) {
              current_action_ = actions[i];
              // 選択したアクションの全フレームを読み込む
              auto it = def->display.sprite_actions.find(current_action_);
              if (it != def->display.sprite_actions.end()) {
                std::string path = it->second;
                namespace fs = std::filesystem;
                fs::path p = path;
                if (!fs::exists(p) && !def->source_path.empty()) {
                  p = fs::path(def->source_path).parent_path() / path;
                }
                if (fs::exists(p)) {
                  LoadActionFrames(p.lexically_normal().generic_string());
                }
              }
              ApplyPreviewSettings();
            }
            if (selected) ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
      }
    }
  }

  ImGui::Separator();
  DrawFormationPreview();
  
  ImGui::Separator();
  
  // アニメーション制御
  if (preview_entity_ != entt::null) {
    if (ImGui::Button(is_playing_ ? "Stop" : "Play")) {
      is_playing_ = !is_playing_;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
      if (preview_entity_ != entt::null && simulation_) {
        auto& registry = simulation_->GetRegistry();
        if (registry.all_of<Game::Components::Animation>(preview_entity_)) {
          auto& anim = registry.get<Game::Components::Animation>(preview_entity_);
          anim.current_frame = 0;
          anim.frame_timer = 0.0f;
          anim.elapsedTime = 0.0f;
          anim.frameIndex = 0;
        }
      }
    }
    
    ImGui::Text("Speed: %.2fx", animation_speed_);
    ImGui::SliderFloat("##speed", &animation_speed_, 0.0f, 3.0f, "%.2f");
  }
  
  ImGui::Separator();
  
  // 表示オプション
  ImGui::Text("表示設定:");
  ImGui::Checkbox("ヒットボックス", &show_hitbox_);
  ImGui::SameLine();
  ImGui::Checkbox("攻撃ポイント", &show_attack_point_);
  // ミラー/フレーム時間上書き
  ImGui::Checkbox("左右反転", &mirror_h_); ImGui::SameLine(); ImGui::Checkbox("上下反転", &mirror_v_);
  ImGui::Checkbox("フレーム時間を上書き", &override_frame_duration_);
  if (override_frame_duration_) {
    ImGui::SliderFloat("上書き秒", &frame_duration_override_, 0.02f, 0.50f, "%.3f");
  }
  
  ImGui::Separator();
  
  // 操作ボタン
  if (ImGui::Button("Reload") && !current_entity_id_.empty()) {
    LoadEntity(current_entity_id_);
  }
  ImGui::SameLine();
  if (ImGui::Button("Clear")) {
    Clear();
  }
  
  ImGui::End();
}

void PreviewWindow::LoadEntity(const std::string &definition_id) {
  if (!simulation_)
    return;

  Clear();
  preview_entity_ = simulation_->SpawnEntity(definition_id, {0.0f, 0.0f});
  if (preview_entity_ != entt::null) {
    current_entity_id_ = definition_id;
    if (definitions_) {
      const auto* def = definitions_->GetEntity(definition_id);
      if (def) {
        LoadPreviewAssets(*def);
        // デフォルトアクションを設定
        if (def->display.sprite_actions.find("idle") != def->display.sprite_actions.end()) {
          current_action_ = "idle";
        } else if (!def->display.sprite_actions.empty()) {
          current_action_ = def->display.sprite_actions.begin()->first;
        } else {
          current_action_.clear();
        }
        // フレームロード
        if (!current_action_.empty()) {
          auto it = def->display.sprite_actions.find(current_action_);
          if (it != def->display.sprite_actions.end()) {
            namespace fs = std::filesystem;
            fs::path p = it->second;
            if (!fs::exists(p) && !def->source_path.empty()) {
              p = fs::path(def->source_path).parent_path() / it->second;
            }
            if (fs::exists(p)) {
              LoadActionFrames(p.lexically_normal().generic_string());
            }
          }
        }
        ApplyPreviewSettings();
      }
    }
  }
}

void PreviewWindow::Clear() {
  if (simulation_ && preview_entity_ != entt::null) {
    simulation_->DestroyEntity(preview_entity_);
  }
  if (simulation_ && formation_preview_entity_ != entt::null) {
    simulation_->GetRegistry().destroy(formation_preview_entity_);
    formation_preview_entity_ = entt::null;
  }
  formation_anim_time_ = 0.0f;
  UnloadPreviewAssets();
  preview_entity_ = entt::null;
  current_entity_id_.clear();
  is_playing_ = true;
  animation_speed_ = 1.0f;
  forced_action_.clear();
}

void PreviewWindow::SetCurrentAction(const std::string &action_name) {
  forced_action_ = action_name;
  current_action_ = action_name;
  
  // アクション変更時にフレームをリロード
  if (definitions_ && !current_entity_id_.empty()) {
    const auto* def = definitions_->GetEntity(current_entity_id_);
    if (def) {
      auto it = def->display.sprite_actions.find(current_action_);
      if (it != def->display.sprite_actions.end()) {
        namespace fs = std::filesystem;
        fs::path p = it->second;
        if (!fs::exists(p) && !def->source_path.empty()) {
          p = fs::path(def->source_path).parent_path() / it->second;
        }
        if (fs::exists(p)) {
          LoadActionFrames(p.lexically_normal().generic_string());
        }
      }
    }
  }
}

void PreviewWindow::UnloadPreviewAssets() {
  if (preview_atlas_texture_.id != 0) {
    UnloadTexture(preview_atlas_texture_);
    preview_atlas_texture_ = {};
  }
  if (preview_icon_texture_.id != 0) {
    UnloadTexture(preview_icon_texture_);
    preview_icon_texture_ = {};
  }
  preview_atlas_path_.clear();
  preview_icon_path_.clear();
  preview_idle_path_.clear();
  preview_frame_ = {};
  preview_frame_valid_ = false;
  atlas_width_ = 0;
  atlas_height_ = 0;
}

void PreviewWindow::LoadPreviewAssets(const Shared::Data::EntityDef& def) {
  namespace fs = std::filesystem;
  UnloadPreviewAssets();

  const std::string icon_path = ResolveIconPath(def);
  if (!icon_path.empty() && fs::exists(icon_path)) {
    preview_icon_texture_ = LoadTexture(icon_path.c_str());
    preview_icon_path_ = icon_path;
  }

  if (!def.display.atlas_texture.empty() && fs::exists(def.display.atlas_texture)) {
    preview_atlas_texture_ = LoadTexture(def.display.atlas_texture.c_str());
    preview_atlas_path_ = def.display.atlas_texture;
    atlas_width_ = preview_atlas_texture_.width;
    atlas_height_ = preview_atlas_texture_.height;
  }

  // アイドルがあれば優先してフレーム情報を読み込む
  std::string idle_file;
  if (def.display.sprite_actions.find("idle") != def.display.sprite_actions.end()) {
    idle_file = def.display.sprite_actions.at("idle");
  } else if (!def.display.sprite_actions.empty()) {
    idle_file = def.display.sprite_actions.begin()->second;
  }

  if (!idle_file.empty()) {
    fs::path p = idle_file;
    if (!fs::exists(p) && !def.source_path.empty()) {
      p = fs::path(def.source_path).parent_path() / idle_file;
    }
    if (fs::exists(p)) {
      LoadIdleFrame(p.lexically_normal().generic_string());
    }
  }
}

bool PreviewWindow::LoadIdleFrame(const std::string& animation_path) {
  try {
    std::ifstream in(animation_path, std::ios::binary);
    if (!in.is_open()) {
      return false;
    }
    auto json_data = nlohmann::json::parse(in);
    if (!json_data.contains("frames") || !json_data["frames"].is_array() || json_data["frames"].empty()) {
      return false;
    }

    const auto& first_frame = json_data["frames"].front();
    if (!first_frame.contains("frame")) {
      return false;
    }
    const auto& frame_data = first_frame["frame"];

    preview_frame_.x = frame_data.value("x", 0.0f);
    preview_frame_.y = frame_data.value("y", 0.0f);
    preview_frame_.width = frame_data.value("w", 0.0f);
    preview_frame_.height = frame_data.value("h", 0.0f);
    preview_idle_path_ = animation_path;

    preview_frame_valid_ =
        preview_frame_.width > 0.0f && preview_frame_.height > 0.0f &&
        atlas_width_ > 0 && atlas_height_ > 0;
    return preview_frame_valid_;
  } catch (const std::exception& e) {
    std::cerr << "Preview idle load error: " << e.what() << std::endl;
    return false;
  }
}

bool PreviewWindow::LoadActionFrames(const std::string& animation_path) {
  current_frames_.clear();
  current_frame_index_ = 0;
  current_frame_timer_ = 0.0f;
  bool ok = false;
  try {
    std::ifstream in(animation_path, std::ios::binary);
    if (!in.is_open()) return false;
    auto json_data = nlohmann::json::parse(in);
    // まずmetaから生成パラメータがあるか確認（sprite_for4animation向け）
    bool usedMetaParams = false;
    if (json_data.contains("meta")) {
      const auto& m = json_data["meta"];
      int frameW = m.value("frameW", 0);
      int frameH = m.value("frameH", 0);
      int frames = m.value("frames", 0);
      int columns = m.value("columns", 0);
      int rows = m.value("rows", 0);
      int yOffset = m.value("yOffset", 0);
      // メタのミラー設定（あれば既定に反映）
      if (m.contains("mirror")) {
        try {
          const auto& mir = m["mirror"];
          mirror_h_ = mir.value("horizontal", false);
          mirror_v_ = mir.value("vertical", false);
        } catch (...) {}
      }
      // アトラス切替
      try {
        if (m.contains("image")) {
          std::string imageFile = m["image"].get<std::string>();
          if (!imageFile.empty()) {
            namespace fs = std::filesystem;
            fs::path base = fs::path(animation_path).parent_path();
            fs::path imagePath = base / imageFile;
            if (fs::exists(imagePath)) {
              if (preview_atlas_texture_.id != 0) { UnloadTexture(preview_atlas_texture_); preview_atlas_texture_ = {}; }
              preview_atlas_texture_ = LoadTexture(imagePath.lexically_normal().generic_string().c_str());
              preview_atlas_path_ = imagePath.lexically_normal().generic_string();
              atlas_width_ = preview_atlas_texture_.width;
              atlas_height_ = preview_atlas_texture_.height;
            }
          }
        }
      } catch (...) {}

      if (frameW > 0 && frameH > 0 && frames > 0) {
        // メタのdurationMsがあれば使用（ms→秒）
        float metaDurSec = 0.10f;
        try {
          if (m.contains("durationMs")) {
            int ms = m["durationMs"].get<int>();
            metaDurSec = std::max(1, ms) / 1000.0f;
          }
        } catch (...) {}
        int cols = (columns > 0) ? columns : frames;
        int rws = (rows > 0) ? rows : ((frames + cols - 1) / cols);
        current_frames_.reserve(frames);
        for (int i = 0; i < frames; ++i) {
          int col = i % cols;
          int row = i / cols;
          FrameInfo info{};
          info.rect.x = static_cast<float>(col * frameW);
          info.rect.y = static_cast<float>(row * frameH + yOffset);
          info.rect.width = static_cast<float>(frameW);
          info.rect.height = static_cast<float>(frameH);
          // durationはメタのdurationMsがあれば使用、無ければ100ms
          info.duration = std::max(0.001f, metaDurSec);
          current_frames_.push_back(info);
        }
        usedMetaParams = true;
      }
    }

    // メタパラメータが無い場合は従来通りframes配列を使用
    if (!usedMetaParams) {
      if (!json_data.contains("frames") || !json_data["frames"].is_array() || json_data["frames"].empty()) {
        return false;
      }
      for (const auto& f : json_data["frames"]) {
        if (!f.contains("frame")) continue;
        const auto& rect = f["frame"];
        FrameInfo info{};
        info.rect.x = rect.value("x", 0.0f);
        info.rect.y = rect.value("y", 0.0f);
        info.rect.width = rect.value("w", 0.0f);
        info.rect.height = rect.value("h", 0.0f);
        float ms = 100.0f;
        if (f.contains("duration")) { try { ms = static_cast<float>(f["duration"].get<int>()); } catch (...) {} }
        info.duration = std::max(0.001f, ms / 1000.0f);
        if (info.rect.width > 0 && info.rect.height > 0) current_frames_.push_back(info);
      }
    }
    if (!current_frames_.empty()) {
      preview_frame_ = current_frames_[0].rect;
      preview_frame_valid_ = atlas_width_ > 0 && atlas_height_ > 0 && preview_frame_.width > 0 && preview_frame_.height > 0;
      ok = true;
    }
  } catch (const std::exception& e) {
    std::cerr << "Preview frames load error: " << e.what() << std::endl;
    ok = false;
  }
  return ok;
}

std::string PreviewWindow::ResolveIconPath(const Shared::Data::EntityDef& def) const {
  return ResolveIconPathImpl(def);
}

void PreviewWindow::DrawFormationPreview() {
  ImGui::Text("編成プレビュー");
  
  // アニメーション選択UI
  const char* actions[] = { "idle", "walk", "attack" };
  int currentActionIdx = 0;
  for (int i = 0; i < 3; ++i) {
    if (formation_selected_action_ == actions[i]) {
      currentActionIdx = i;
      break;
    }
  }
  
  ImGui::Text("アニメーション:");
  ImGui::SameLine();
  if (ImGui::BeginCombo("##formation_action", actions[currentActionIdx])) {
    for (int i = 0; i < 3; ++i) {
      bool selected = (i == currentActionIdx);
      if (ImGui::Selectable(actions[i], selected)) {
        formation_selected_action_ = actions[i];
        formation_anim_time_ = 0.0f;
        // プレビューエンティティを再生成
        if (formation_preview_entity_ != entt::null && simulation_) {
          simulation_->GetRegistry().destroy(formation_preview_entity_);
          formation_preview_entity_ = entt::null;
        }
        if (!current_entity_id_.empty() && simulation_) {
          formation_preview_entity_ = simulation_->SpawnEntity(current_entity_id_, {0.0f, 0.0f});
          if (formation_preview_entity_ != entt::null) {
            auto& registry = simulation_->GetRegistry();
            if (registry.all_of<Game::Components::Animation>(formation_preview_entity_)) {
              auto& anim = registry.get<Game::Components::Animation>(formation_preview_entity_);
              anim.currentAction = formation_selected_action_;
              anim.currentClip = formation_selected_action_;
            }
          }
        }
      }
      if (selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  
  if (!definitions_ || current_entity_id_.empty()) {
    ImGui::TextDisabled("エンティティ未選択");
    return;
  }

  const auto* def = definitions_->GetEntity(current_entity_id_);
  if (!def) {
    ImGui::TextDisabled("エンティティ未選択");
    return;
  }

  ImVec2 base = ImGui::GetCursorScreenPos();
  const float slot = 110.0f;
  const float pad = 20.0f;
  ImDrawList* dl = ImGui::GetWindowDrawList();

  auto drawSlot = [&](ImVec2 pos, const char* label, bool filled) {
    ImVec2 p0{pos.x, pos.y};
    ImVec2 p1{pos.x + slot, pos.y + slot + 20.0f};
    dl->AddRectFilled(p0, p1, IM_COL32(45, 45, 45, 255), 6.0f);
    dl->AddRect(p0, p1, IM_COL32(120, 120, 120, 255), 6.0f, 0, 2.0f);

    // Label
    ImVec2 labelSize = ImGui::CalcTextSize(label);
    dl->AddText(ImVec2(pos.x + (slot - labelSize.x) * 0.5f, pos.y + 4.0f), IM_COL32(220, 220, 220, 255), label);

    // Icon area
    ImVec2 iconMin{pos.x + 10.0f, pos.y + 24.0f};
    ImVec2 iconMax{pos.x + slot - 10.0f, pos.y + slot - 10.0f};
    dl->AddRect(iconMin, iconMax, IM_COL32(90, 90, 90, 255), 4.0f);

    if (filled && preview_icon_texture_.id != 0) {
      ImVec2 texSize{static_cast<float>(preview_icon_texture_.width), static_cast<float>(preview_icon_texture_.height)};
      float scale = std::min((iconMax.x - iconMin.x) / texSize.x, (iconMax.y - iconMin.y) / texSize.y);
      ImVec2 size{texSize.x * scale, texSize.y * scale};
      ImVec2 center{(iconMin.x + iconMax.x) * 0.5f, (iconMin.y + iconMax.y) * 0.5f};
      ImVec2 a{center.x - size.x * 0.5f, center.y - size.y * 0.5f};
      ImVec2 b{center.x + size.x * 0.5f, center.y + size.y * 0.5f};
      dl->AddImage((void*)(intptr_t)preview_icon_texture_.id, a, b, ImVec2(0, 0), ImVec2(1, 1));
    }
  };

  drawSlot(base, "候補", true);
  
  // 編成スロット：アニメーション描画
  ImVec2 formationPos = ImVec2(base.x + slot + pad, base.y);
  ImVec2 p0{formationPos.x, formationPos.y};
  ImVec2 p1{formationPos.x + slot, formationPos.y + slot + 20.0f};
  dl->AddRectFilled(p0, p1, IM_COL32(45, 45, 45, 255), 6.0f);
  dl->AddRect(p0, p1, IM_COL32(120, 120, 120, 255), 6.0f, 0, 2.0f);
  
  // Label
  const char* label = "編成";
  ImVec2 labelSize = ImGui::CalcTextSize(label);
  dl->AddText(ImVec2(formationPos.x + (slot - labelSize.x) * 0.5f, formationPos.y + 4.0f), IM_COL32(220, 220, 220, 255), label);
  
  // Animation area
  ImVec2 animMin{formationPos.x + 10.0f, formationPos.y + 24.0f};
  ImVec2 animMax{formationPos.x + slot - 10.0f, formationPos.y + slot - 10.0f};
  dl->AddRect(animMin, animMax, IM_COL32(90, 90, 90, 255), 4.0f);
  
  // 編成プレビューエンティティの描画
  if (formation_preview_entity_ == entt::null && simulation_ && !current_entity_id_.empty()) {
    formation_preview_entity_ = simulation_->SpawnEntity(current_entity_id_, {0.0f, 0.0f});
    if (formation_preview_entity_ != entt::null) {
      auto& registry = simulation_->GetRegistry();
      if (registry.all_of<Game::Components::Animation>(formation_preview_entity_)) {
        auto& anim = registry.get<Game::Components::Animation>(formation_preview_entity_);
        anim.currentAction = formation_selected_action_;
        anim.currentClip = formation_selected_action_;
      }
    }
  }
  
  if (formation_preview_entity_ != entt::null && simulation_) {
    auto& registry = simulation_->GetRegistry();
    if (registry.all_of<Game::Components::Sprite, Game::Components::Animation>(formation_preview_entity_)) {
      auto& sprite = registry.get<Game::Components::Sprite>(formation_preview_entity_);
      auto& anim = registry.get<Game::Components::Animation>(formation_preview_entity_);
      
      // アニメーション更新
      if (is_playing_) {
        formation_anim_time_ += ImGui::GetIO().DeltaTime * animation_speed_;
        anim.elapsedTime = formation_anim_time_;
      }
      
      // フレーム取得
      if (sprite.provider) {
        std::string clipName = formation_selected_action_;
        if (!sprite.provider->HasClip(clipName)) {
          clipName = anim.currentAction.empty() ? "idle" : anim.currentAction;
        }
        
        int frameCount = sprite.provider->GetFrameCount(clipName);
        if (frameCount > 0) {
          float fps = sprite.provider->GetClipFps(clipName);
          if (fps <= 0.0f) fps = 12.0f;
          int frameIdx = static_cast<int>(formation_anim_time_ * fps) % frameCount;
          auto frameRef = sprite.provider->GetFrame(clipName, frameIdx);
          
          if (frameRef.valid && frameRef.texture) {
            // 描画領域の中心に配置
            float animWidth = animMax.x - animMin.x;
            float animHeight = animMax.y - animMin.y;
            float scale = std::min(animWidth / frameRef.src.width, animHeight / frameRef.src.height) * 0.8f;
            
            ImVec2 spriteSize{frameRef.src.width * scale, frameRef.src.height * scale};
            ImVec2 center{(animMin.x + animMax.x) * 0.5f, (animMin.y + animMax.y) * 0.5f};
            ImVec2 a{center.x - spriteSize.x * 0.5f, center.y - spriteSize.y * 0.5f};
            ImVec2 b{center.x + spriteSize.x * 0.5f, center.y + spriteSize.y * 0.5f};
            
            // テクスチャ座標
            float u0 = frameRef.src.x / frameRef.texture->width;
            float v0 = frameRef.src.y / frameRef.texture->height;
            float u1 = (frameRef.src.x + frameRef.src.width) / frameRef.texture->width;
            float v1 = (frameRef.src.y + frameRef.src.height) / frameRef.texture->height;
            
            dl->AddImage(
              (void*)(intptr_t)frameRef.texture->id,
              a, b,
              ImVec2(u0, v0), ImVec2(u1, v1)
            );
          }
        }
      }
    }
  }

  ImGui::Dummy(ImVec2(slot * 2 + pad, slot + 30.0f));
}

void PreviewWindow::UpdateAnimation(float deltaTime) {
  if (!simulation_ || preview_entity_ == entt::null)
    return;
  
  auto& registry = simulation_->GetRegistry();
  if (!registry.all_of<Game::Components::Animation>(preview_entity_))
    return;
  
  auto& anim = registry.get<Game::Components::Animation>(preview_entity_);
  
  // 新しいアニメーションシステム（クリップベース）
  if (!anim.currentClip.empty()) {
    anim.elapsedTime += deltaTime;
    // フレーム更新は実際のアニメーションシステムに任せる
  } else {
    // 古いアニメーションシステム（後方互換性）
    anim.frame_timer += deltaTime;
    if (anim.frame_timer >= anim.frame_duration) {
      anim.frame_timer = 0.0f;
      anim.current_frame = (anim.current_frame + 1) % std::max(1, anim.frames_per_state);
    }
  }
}

void PreviewWindow::ApplyPreviewSettings() {
  if (!simulation_ || preview_entity_ == entt::null) return;
  auto& registry = simulation_->GetRegistry();
  if (registry.all_of<Game::Components::Animation>(preview_entity_)) {
    auto& anim = registry.get<Game::Components::Animation>(preview_entity_);
    if (!current_action_.empty()) {
      anim.currentClip = current_action_;
    }
  }
}

void PreviewWindow::DrawPreviewArea() {
  // RenderTextureの初期化
  if (!preview_texture_initialized_) {
    preview_texture_ = LoadRenderTexture(static_cast<int>(preview_size_), static_cast<int>(preview_size_));
    preview_texture_initialized_ = true;
  }

  ImVec2 canvas_size = ImVec2(preview_size_, preview_size_);
  ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
  ImGui::InvisibleButton("canvas", canvas_size);

  // プレビュー描画領域をRaylibでレンダリング
  float render_scale = 1.0f;
  Rectangle dest_rect{preview_size_ * 0.5f, preview_size_ * 0.5f, preview_frame_.width, preview_frame_.height};
  if (preview_frame_valid_) {
    float avail_w = preview_size_ - 40.0f;
    float avail_h = preview_size_ - 40.0f;
    render_scale = std::clamp(std::min(avail_w / preview_frame_.width, avail_h / preview_frame_.height), 0.2f, 6.0f);
    dest_rect.width = preview_frame_.width * render_scale;
    dest_rect.height = preview_frame_.height * render_scale;
    if (mirror_h_) dest_rect.width = -std::fabs(dest_rect.width);
    if (mirror_v_) dest_rect.height = -std::fabs(dest_rect.height);
  }

  if (preview_texture_initialized_) {
    BeginTextureMode(preview_texture_);
    ClearBackground(Color{40, 40, 40, 255});

    if (preview_frame_valid_ && preview_atlas_texture_.id != 0) {
      Rectangle source = preview_frame_;
      float ow = std::fabs(dest_rect.width) * 0.5f;
      float oh = std::fabs(dest_rect.height) * 0.5f;
      Vector2 origin{ow, oh};
      DrawTexturePro(preview_atlas_texture_, source, dest_rect, origin, 0.0f, WHITE);
    }
    EndTextureMode();
  }

  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImVec2 canvas_max = ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y);
  draw_list->AddRectFilled(canvas_pos, canvas_max, IM_COL32(50, 50, 50, 255));
  draw_list->AddRect(canvas_pos, canvas_max, IM_COL32(200, 200, 200, 255));

  // ImGui側にRenderTextureを貼る（Y軸反転に注意）
  if (preview_texture_initialized_) {
    draw_list->AddImage((void*)(intptr_t)preview_texture_.texture.id, canvas_pos, canvas_max, ImVec2(0, 1), ImVec2(1, 0));
  }

  ImVec2 center = ImVec2(canvas_pos.x + canvas_size.x * 0.5f, canvas_pos.y + canvas_size.y * 0.5f);

  // オーバーレイ（ヒットボックス等）
  if (preview_entity_ != entt::null && definitions_) {
    const auto* def = definitions_->GetEntity(current_entity_id_);
    if (def) {
      if (show_hitbox_) {
        float half_w = def->combat.hitbox.width * 0.5f * render_scale;
        float half_h = def->combat.hitbox.height * 0.5f * render_scale;
        float off_x = def->combat.hitbox.offset_x * render_scale;
        float off_y = def->combat.hitbox.offset_y * render_scale;

        ImVec2 p_min = ImVec2(center.x + off_x - half_w, center.y + off_y - half_h);
        ImVec2 p_max = ImVec2(center.x + off_x + half_w, center.y + off_y + half_h);

        draw_list->AddRect(p_min, p_max, IM_COL32(0, 255, 0, 200), 0.0f, 0, 2.0f);
      }

      if (show_attack_point_ && def->combat.attack_point >= 0.0f) {
        float attack_x = center.x + def->combat.hitbox.width * def->combat.attack_point * render_scale;
        float attack_y = center.y;

        draw_list->AddCircleFilled(ImVec2(attack_x, attack_y), 5.0f, IM_COL32(255, 0, 0, 255));
        draw_list->AddCircle(ImVec2(attack_x, attack_y), 5.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);
      }

      draw_list->AddLine(ImVec2(center.x - 10, center.y), ImVec2(center.x + 10, center.y), IM_COL32(128, 128, 128, 200));
      draw_list->AddLine(ImVec2(center.x, center.y - 10), ImVec2(center.x, center.y + 10), IM_COL32(128, 128, 128, 200));
    }
  }

  // エンティティ情報表示
  if (preview_entity_ != entt::null) {
    ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x + 10, canvas_pos.y + 10));
    ImGui::Text("Entity: %u", static_cast<uint32_t>(preview_entity_));

    if (simulation_) {
      auto& registry = simulation_->GetRegistry();
      if (registry.all_of<Game::Components::Transform>(preview_entity_)) {
        const auto& tf = registry.get<Game::Components::Transform>(preview_entity_);
        ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x + 10, canvas_pos.y + 30));
        ImGui::Text("Pos: (%.1f, %.1f)", tf.x, tf.y);
      }
    }
  } else {
    ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x + canvas_size.x * 0.5f - 50, canvas_pos.y + canvas_size.y * 0.5f));
    ImGui::Text("No entity loaded");
  }

  ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x, canvas_pos.y + canvas_size.y + 10));
}

void PreviewWindow::PreviewEntity(const std::string &entity_id) {
  if (!definitions_) return;
  
  const auto *def = definitions_->GetEntity(entity_id);
  if (!def) return;
  
  current_entity_id_ = entity_id;
  
  // エディタ向けUI表示
  ImGui::Text("Entity Preview: %s", entity_id.c_str());
  ImGui::Text("Type: %s | Rarity: %d | Cost: %d", def->type.c_str(), def->rarity, def->cost);
  ImGui::Text("Stats: HP=%d, ATK=%d, SPD=%.1f", def->stats.hp, def->stats.attack, def->stats.attack_speed);
  
  ImGui::Separator();
  
  // アイコン表示
  if (!def->display.icon.empty() && std::filesystem::exists(def->display.icon)) {
    ImGui::TextUnformatted("Icon:");
    auto icon_texture = LoadTexture(def->display.icon.c_str());
    if (icon_texture.id != 0) {
      ImGui::Image((void*)(intptr_t)icon_texture.id, ImVec2(64, 64));
    }
  }
  
  // スプライト描画プレビュー
  if (!def->display.atlas_texture.empty() && !def->display.sprite_actions.empty()) {
    ImGui::TextUnformatted("Sprite Preview:");
    ImGui::Separator();
    
    // アイドルアニメーションを優先的に表示
    std::string idle_file;
    if (def->display.sprite_actions.find("idle") != def->display.sprite_actions.end()) {
      idle_file = def->display.sprite_actions.at("idle");
    } else if (!def->display.sprite_actions.empty()) {
      idle_file = def->display.sprite_actions.begin()->second;
    }
    
    if (!idle_file.empty() && std::filesystem::exists(idle_file)) {
      try {
        auto json_data = nlohmann::json::parse(
            std::ifstream(idle_file, std::ios::binary));
        
        if (json_data.contains("frames") && json_data.contains("meta")) {
          // アニメーション読み込み
          auto atlas_path = def->display.atlas_texture;
          if (std::filesystem::exists(atlas_path)) {
            auto atlas_texture = LoadTexture(atlas_path.c_str());
            if (atlas_texture.id != 0) {
              // 最初のフレームを表示
              if (json_data["frames"].is_array() && !json_data["frames"].empty()) {
                const auto& first_frame = json_data["frames"][0];
                if (first_frame.contains("frame")) {
                  auto frame_data = first_frame["frame"];
                  float frame_x = frame_data["x"].get<float>();
                  float frame_y = frame_data["y"].get<float>();
                  float frame_w = frame_data["w"].get<float>();
                  float frame_h = frame_data["h"].get<float>();
                  
                  // ImGuiプレビュー領域
                  ImVec2 preview_size(128, 128);
                  ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
                  ImDrawList* draw_list = ImGui::GetWindowDrawList();
                  
                  // 背景
                  draw_list->AddRectFilled(canvas_pos, 
                      ImVec2(canvas_pos.x + preview_size.x, canvas_pos.y + preview_size.y),
                      IM_COL32(30, 30, 30, 255));
                  
                  // スプライト描画用レイアウト調整
                  ImGui::InvisibleButton("sprite_preview", preview_size);
                  
                  // raylib でテクスチャ描画（オフスクリーン）
                  if (preview_texture_initialized_) {
                    BeginTextureMode(preview_texture_);
                    ClearBackground(Color{30, 30, 30, 255});
                    
                    Rectangle source = {frame_x, frame_y, frame_w, frame_h};
                    Rectangle dest = {
                        64 - frame_w / 2,
                        64 - frame_h / 2,
                        frame_w,
                        frame_h
                    };
                    DrawTexturePro(atlas_texture, source, dest, {0, 0}, 0, WHITE);
                    EndTextureMode();
                  }
                  
                  UnloadTexture(atlas_texture);
                }
              }
            }
          }
        }
      } catch (const std::exception& e) {
        ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "Sprite load error: %s", e.what());
      }
    }
  }
  
  ImGui::Separator();
  
  // スプライト情報表示
  if (!def->display.atlas_texture.empty()) {
    ImGui::TextUnformatted("Atlas Texture:");
    ImGui::Text("%s", def->display.atlas_texture.c_str());
  }
  
  // アニメーション情報
  if (!def->display.sprite_actions.empty()) {
    ImGui::TextUnformatted("Sprite Actions:");
    for (const auto &[action, file] : def->display.sprite_actions) {
      ImGui::BulletText("%s: %s", action.c_str(), file.c_str());
    }
  }
}

} // namespace Editor::Windows

