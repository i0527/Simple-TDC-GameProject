#include "Editor/Windows/SpriteEditorWindow.h"
#include "Editor/Windows/PreviewWindow.h"
#include "Core/GameContext.h"
#include "Data/DefinitionRegistry.h"
#include "imgui.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace {
std::string NormalizeAssetPath(const std::string& path) {
    try {
        std::filesystem::path p(path);
        if (p.is_absolute()) {
            p = std::filesystem::relative(p, std::filesystem::current_path());
        }
        return p.lexically_normal().generic_string();
    } catch (...) {
        return path;
    }
}
}

using namespace Shared::Data;

namespace Editor::Windows {

SpriteEditorWindow::SpriteEditorWindow()
    : context_(nullptr)
    , definitions_(nullptr)
    , isOpen_(false)
    , activeEntityId_("")
    , isDirty_(false)
    , formData_{}
    , showDrawTypeDialog_(false)
    , pendingDrawType_{}
    , newActionName_{}
    , newActionPath_{}
{
}

SpriteEditorWindow::~SpriteEditorWindow() {
}

void SpriteEditorWindow::Initialize(Shared::Core::GameContext& context, 
                                     DefinitionRegistry& definitions) {
    context_ = &context;
    definitions_ = &definitions;
}

void SpriteEditorWindow::OnUpdate(float deltaTime) {
    // 成功メッセージタイマー更新
    if (successMessageTimer_ > 0.0f) {
        successMessageTimer_ -= deltaTime;
        if (successMessageTimer_ <= 0.0f) {
            successMessage_.clear();
        }
    }
    
    // エラーメッセージタイマー更新
    if (errorMessageTimer_ > 0.0f) {
        errorMessageTimer_ -= deltaTime;
        if (errorMessageTimer_ <= 0.0f) {
            errorMessage_.clear();
        }
    }
}

void SpriteEditorWindow::OnDrawUI() {
    if (!isOpen_) return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
    if (ImGui::Begin(GetWindowTitle().c_str(), &isOpen_, flags)) {
        DrawToolbar();
        
        // エラーメッセージ表示
        if (!errorMessage_.empty() && errorMessageTimer_ > 0.0f) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            ImGui::TextWrapped("✖ %s", errorMessage_.c_str());
            ImGui::PopStyleColor();
            ImGui::Separator();
        }
        
        // 成功メッセージ表示
        if (!successMessage_.empty() && successMessageTimer_ > 0.0f) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.3f, 1.0f));
            ImGui::TextWrapped("✔ %s", successMessage_.c_str());
            ImGui::PopStyleColor();
            ImGui::Separator();
        }
        
        if (activeEntityId_.empty()) {
            ImGui::TextDisabled("エンティティが選択されていません");
            ImGui::Text("ユニットエディタからエンティティを選択してください");
        } else {
            DrawEditForm();
        }
    }
    ImGui::End();
    
    // draw_type切替確認ダイアログ
    if (showDrawTypeDialog_) {
        ShowDrawTypeChangeDialog();
    }
}

void SpriteEditorWindow::DrawToolbar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::MenuItem("保存", "Ctrl+S", false, isDirty_ && !activeEntityId_.empty())) {
            SaveChanges();
        }
        if (isDirty_) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "●");  // 未保存マーク
        }
        ImGui::EndMenuBar();
    }
}

void SpriteEditorWindow::DrawEditForm() {
    const auto* entity = definitions_->GetEntity(activeEntityId_);
    if (!entity) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "エラー: エンティティが見つかりません");
        return;
    }
    
    ImGui::Text("編集中: %s (%s)", entity->name.c_str(), entity->id.c_str());
    ImGui::Separator();

    // アイコン画像を上部に配置
    DrawIconSettings();
    ImGui::Separator();
    
    DrawDrawTypeSelector();
    ImGui::Separator();
    
    // draw_typeに応じて表示を切り替え
    std::string drawType = formData_.draw_type;
    
    if (drawType == "sprite") {
        DrawTextureSettings();
        ImGui::Separator();
        DrawSpriteActionsEditor();
    } else if (drawType == "sprite_for4animation") {
        ImGui::TextDisabled("4アニメーション別スプライトシート設定");
        DrawFourAnimSettings();
    } else {
        ImGui::TextDisabled("サポート外の描画タイプです。'sprite' または 'sprite_for4animation' を選択してください。");
    }
    
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

void SpriteEditorWindow::DrawDrawTypeSelector() {
    ImGui::Text("描画タイプ");
    
    const char* drawTypes[] = { "sprite", "sprite_for4animation" };
    int currentIndex = 0;
    std::string currentType = formData_.draw_type;
    
    if (currentType == "sprite") currentIndex = 0;
    else if (currentType == "sprite_for4animation") currentIndex = 1;
    else currentIndex = 0;
    
    if (ImGui::Combo("##DrawType", &currentIndex, drawTypes, IM_ARRAYSIZE(drawTypes))) {
        const char* newType = (currentIndex == 0) ? "sprite" : "sprite_for4animation";
        
        // 既存データがある場合は警告ダイアログを表示
        bool hasExistingData = false;
        if (currentType == "sprite" && !formData_.sprite_actions.empty()) {
            hasExistingData = true;
        } else if (currentType == "sprite_for4animation" && formData_.sprite_actions.empty() == false) {
            hasExistingData = true;
        }
        
        if (hasExistingData) {
            strncpy_s(pendingDrawType_, sizeof(pendingDrawType_), newType, _TRUNCATE);
            showDrawTypeDialog_ = true;
        } else {
            strncpy_s(formData_.draw_type, sizeof(formData_.draw_type), newType, _TRUNCATE);
            isDirty_ = true;
        }
    }
    
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "sprite: Asepriteアトラス形式（推奨）\n"
            "sprite_for4animation: テスト・開発用の4分割アニメスプライト"
        );
    }
}

void SpriteEditorWindow::DrawFourAnimSettings() {
    const auto* entity = definitions_->GetEntity(activeEntityId_);
    auto existsPath = [&](const char* path) {
        if (!path || path[0] == '\0') return false;
        std::filesystem::path p(path);
        if (std::filesystem::exists(p)) return true;
        if (entity && !entity->source_path.empty()) {
            p = std::filesystem::path(entity->source_path).parent_path() / path;
            return std::filesystem::exists(p);
        }
        return false;
    };

    auto drawPathInput = [&](const char* label, const char* idSuffix, char* buffer, size_t bufSize) {
        ImGui::TextUnformatted(label);
        std::string id = std::string("##").append(label).append("_").append(idSuffix ? idSuffix : "");
        if (ImGui::InputText(id.c_str(), buffer, bufSize)) {
            isDirty_ = true;
        }
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
                std::string dropped = NormalizeAssetPath(std::string(static_cast<const char*>(payload->Data), payload->DataSize - 1));
                strncpy_s(buffer, bufSize, dropped.c_str(), _TRUNCATE);
                isDirty_ = true;
            }
            ImGui::EndDragDropTarget();
        }
        bool ok = existsPath(buffer);
        ImVec4 col = ok ? ImVec4(0.2f, 0.8f, 0.3f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
        ImGui::TextColored(col, ok ? "✔ パス有効" : "✖ パス不明");
    };

    auto drawActionSection = [&](const char* action,
                                 char* jsonBuf, size_t jsonSize,
                                 char* pngBuf, size_t pngSize,
                                 GenParams& gen) {
        ImGui::Separator();
        ImGui::Text("%s", action);
        ImGui::TextDisabled("JSON と PNG を設定できます");
        drawPathInput("JSON", action, jsonBuf, jsonSize);
        drawPathInput("PNG", action, pngBuf, pngSize);
        ImGui::Text("生成パラメータ");
        ImGui::InputInt(std::string("frameW##").append(action).c_str(), &gen.frameW); ImGui::SameLine(); ImGui::InputInt(std::string("frameH##").append(action).c_str(), &gen.frameH);
        ImGui::InputInt(std::string("frames##").append(action).c_str(), &gen.frames); ImGui::SameLine(); ImGui::InputInt(std::string("duration(ms)##").append(action).c_str(), &gen.durationMs);
        ImGui::InputInt(std::string("columns##").append(action).c_str(), &gen.columns); ImGui::SameLine(); ImGui::InputInt(std::string("rows##").append(action).c_str(), &gen.rows);
        ImGui::InputInt(std::string("yOffset##").append(action).c_str(), &gen.yOffset);
        ImGui::SliderFloat(std::string("pivotX##").append(action).c_str(), &gen.pivotX, 0.0f, 1.0f, "%.2f"); ImGui::SameLine(); ImGui::SliderFloat(std::string("pivotY##").append(action).c_str(), &gen.pivotY, 0.0f, 1.0f, "%.2f");
        ImGui::Checkbox(std::string("mirrorH##").append(action).c_str(), &gen.mirrorH); ImGui::SameLine(); ImGui::Checkbox(std::string("mirrorV##").append(action).c_str(), &gen.mirrorV);
        if (ImGui::Button(std::string("生成(" + std::string(action) + ")").c_str())) {
            // 生成処理
            namespace fs = std::filesystem;
            
            // バリデーション
            std::string errorReason;
            if (pngBuf[0] == '\0') {
                errorReason = "PNG画像ファイルが指定されていません";
            } else if (jsonBuf[0] == '\0') {
                errorReason = "JSON出力ファイル名が指定されていません";
            } else if (gen.frameW <= 0) {
                errorReason = "フレーム幅(frameW)が0以下です";
            } else if (gen.frameH <= 0) {
                errorReason = "フレーム高さ(frameH)が0以下です";
            } else if (gen.frames <= 0) {
                errorReason = "フレーム数(frames)が0以下です";
            } else if (gen.durationMs <= 0) {
                errorReason = "フレーム時間(duration)が0以下です";
            }
            
            if (!errorReason.empty()) {
                errorMessage_ = errorReason;
                errorMessageTimer_ = 3.0f;
            } else {
                // ファイル存在確認
                fs::path pngPath = pngBuf;
                fs::path pngAbsolute = pngPath;
                if (!fs::exists(pngAbsolute) && entity && !entity->source_path.empty()) {
                    pngAbsolute = fs::path(entity->source_path).parent_path() / pngPath;
                }
                if (!fs::exists(pngAbsolute)) {
                    errorMessage_ = std::string("PNG画像が見つかりません: ") + pngBuf;
                    errorMessageTimer_ = 3.0f;
                } else {
                    try {
                        // グリッド配置計算
                        int cols = (gen.columns > 0) ? gen.columns : gen.frames;
                        int rows = (gen.rows > 0) ? gen.rows : ((gen.frames + cols - 1) / cols);
                        
                        // JSON生成（Aseprite互換）
                        nlohmann::json j;
                        j["frames"] = nlohmann::json::array();
                        for (int i = 0; i < gen.frames; ++i) {
                            int col = i % cols;
                            int row = i / cols;
                            int x = col * gen.frameW;
                            int y = row * gen.frameH + gen.yOffset;
                            
                            nlohmann::json f;
                            f["filename"] = std::to_string(i);
                            f["frame"] = { {"x", x}, {"y", y}, {"w", gen.frameW}, {"h", gen.frameH} };
                            f["rotated"] = false;
                            f["trimmed"] = false;
                            
                            // 原点を考慮したspriteSourceSize
                            int sprX = static_cast<int>(-gen.frameW * gen.pivotX);
                            int sprY = static_cast<int>(-gen.frameH * gen.pivotY);
                            f["spriteSourceSize"] = { {"x", sprX}, {"y", sprY}, {"w", gen.frameW}, {"h", gen.frameH} };
                            f["sourceSize"] = { {"w", gen.frameW}, {"h", gen.frameH} };
                            f["duration"] = gen.durationMs;
                            j["frames"].push_back(f);
                        }
                        
                        // meta.image はファイル名のみ保存（プレビュー側で相対解決）
                        j["meta"] = nlohmann::json::object();
                        j["meta"]["app"] = "SimpleTDC Editor";
                        j["meta"]["version"] = "1.0";
                        j["meta"]["image"] = fs::path(pngBuf).filename().string();
                        
                        // 生成パラメータも保存（エディタがパラメータ読込に使用）
                        j["meta"]["frameW"] = gen.frameW;
                        j["meta"]["frameH"] = gen.frameH;
                        j["meta"]["frames"] = gen.frames;
                        j["meta"]["columns"] = gen.columns;
                        j["meta"]["rows"] = gen.rows;
                        j["meta"]["yOffset"] = gen.yOffset;
                        j["meta"]["pivotX"] = gen.pivotX;
                        j["meta"]["pivotY"] = gen.pivotY;
                        if (gen.mirrorH || gen.mirrorV) {
                            j["meta"]["mirror"] = nlohmann::json::object();
                            if (gen.mirrorH) j["meta"]["mirror"]["horizontal"] = true;
                            if (gen.mirrorV) j["meta"]["mirror"]["vertical"] = true;
                        }

                        // 保存先決定: entity.jsonの隣にanimations/<jsonファイル名>
                        fs::path baseDir;
                        if (entity && !entity->source_path.empty()) {
                            baseDir = fs::path(entity->source_path).parent_path();
                        } else {
                            baseDir = fs::current_path();
                        }
                        fs::path animDir = baseDir / "animations";
                        std::error_code ec;
                        if (!fs::create_directories(animDir, ec) && ec) {
                            errorMessage_ = std::string("animationsディレクトリ作成失敗: ") + ec.message();
                            errorMessageTimer_ = 3.0f;
                        } else {
                            fs::path outPath = animDir / fs::path(jsonBuf).filename();
                            std::ofstream out(outPath, std::ios::binary);
                            if (!out) {
                                errorMessage_ = std::string("JSONファイル書き込み失敗: ") + outPath.generic_string();
                                errorMessageTimer_ = 3.0f;
                            } else {
                                out << j.dump(2);
                                out.close();

                                // フォームへ相対パスで設定
                                fs::path rel = fs::relative(outPath, fs::current_path());
                                std::string relPath = rel.lexically_normal().generic_string();
                                strncpy_s(jsonBuf, jsonSize, relPath.c_str(), _TRUNCATE);
                                isDirty_ = true;
                                
                                // 成功メッセージ設定
                                successMessage_ = action + std::string(".json を保存しました: ") + relPath;
                                successMessageTimer_ = 3.0f;  // 3秒間表示
                                
                                // PreviewWindowへ通知（アクション切り替え）
                                if (previewWindow_) {
                                    previewWindow_->LoadEntity(activeEntityId_);
                                    previewWindow_->SetCurrentAction(action);
                                }
                            }
                        }
                    } catch (const std::exception& e) {
                        errorMessage_ = std::string("JSON生成エラー: ") + e.what();
                        errorMessageTimer_ = 5.0f;
                    }
                }
            }
        }
    };

    drawActionSection("idle", formData_.idle_animation, sizeof(formData_.idle_animation), formData_.idle_image, sizeof(formData_.idle_image), genIdle_);
    drawActionSection("walk", formData_.walk_animation, sizeof(formData_.walk_animation), formData_.walk_image, sizeof(formData_.walk_image), genWalk_);
    drawActionSection("attack", formData_.attack_animation, sizeof(formData_.attack_animation), formData_.attack_image, sizeof(formData_.attack_image), genAttack_);
    drawActionSection("death", formData_.death_animation, sizeof(formData_.death_animation), formData_.death_image, sizeof(formData_.death_image), genDeath_);
}

void SpriteEditorWindow::DrawTextureSettings() {
    ImGui::Text("Asepriteアトラス設定");
    
    if (ImGui::InputText("アトラス画像", formData_.atlas_texture, sizeof(formData_.atlas_texture))) {
        isDirty_ = true;
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
            std::string dropped = NormalizeAssetPath(std::string(static_cast<const char*>(payload->Data), payload->DataSize - 1));
            strncpy_s(formData_.atlas_texture, sizeof(formData_.atlas_texture), dropped.c_str(), _TRUNCATE);
            isDirty_ = true;
        }
        ImGui::EndDragDropTarget();
    }

    // 素材存在チェック（相対参照も考慮）
    const auto* entity = definitions_->GetEntity(activeEntityId_);
    auto existsPath = [&](const char* path) {
        if (!path || path[0] == '\0') return false;
        std::filesystem::path p(path);
        if (std::filesystem::exists(p)) return true;
        if (entity && !entity->source_path.empty()) {
            p = std::filesystem::path(entity->source_path).parent_path() / path;
            return std::filesystem::exists(p);
        }
        return false;
    };
    bool atlasOk = existsPath(formData_.atlas_texture);
    ImVec4 col = atlasOk ? ImVec4(0.2f, 0.8f, 0.3f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
    ImGui::TextColored(col, atlasOk ? "✔ アトラス画像: 見つかりました" : "✖ アトラス画像: 見つかりません");

    // 既定ミラー（ゲーム側描画のデフォルト反転）
    if (ImGui::Checkbox("既定: 左右反転", &formData_.mirror_h)) { isDirty_ = true; }
    ImGui::SameLine();
    if (ImGui::Checkbox("既定: 上下反転", &formData_.mirror_v)) { isDirty_ = true; }
}

void SpriteEditorWindow::DrawSpriteActionsEditor() {
    ImGui::Text("アニメーション設定 (sprite_actions)");
    
    ImGui::Text("（簡略版 - 詳細なエディタは将来実装予定）");

    const auto* entity = definitions_->GetEntity(activeEntityId_);
    auto resolveRel = [&](const std::string& path) {
        std::filesystem::path p(path);
        if (std::filesystem::exists(p)) return p.lexically_normal().generic_string();
        if (entity && !entity->source_path.empty()) {
            p = std::filesystem::path(entity->source_path).parent_path() / path;
            if (std::filesystem::exists(p)) return p.lexically_normal().generic_string();
        }
        return std::string{};
    };

    if (entity) {
        for (const auto& [action, path] : entity->display.sprite_actions) {
            std::string resolved = resolveRel(path);
            bool ok = !resolved.empty();
            ImVec4 col = ok ? ImVec4(0.2f, 0.8f, 0.3f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
            ImGui::TextColored(col, "%s: %s", action.c_str(), ok ? resolved.c_str() : path.c_str());
        }
    }
}

void SpriteEditorWindow::DrawIconSettings() {
    ImGui::Text("アイコン設定");
    
    if (ImGui::InputText("アイコン画像", formData_.icon, sizeof(formData_.icon))) {
        isDirty_ = true;
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH")) {
            std::string dropped = NormalizeAssetPath(std::string(static_cast<const char*>(payload->Data), payload->DataSize - 1));
            strncpy_s(formData_.icon, sizeof(formData_.icon), dropped.c_str(), _TRUNCATE);
            isDirty_ = true;
        }
        ImGui::EndDragDropTarget();
    }

    const auto* entity = definitions_->GetEntity(activeEntityId_);
    auto existsPath = [&](const char* path) {
        if (!path || path[0] == '\0') return false;
        std::filesystem::path p(path);
        if (std::filesystem::exists(p)) return true;
        if (entity && !entity->source_path.empty()) {
            p = std::filesystem::path(entity->source_path).parent_path() / path;
            return std::filesystem::exists(p);
        }
        return false;
    };
    bool iconOk = existsPath(formData_.icon);
    ImVec4 col = iconOk ? ImVec4(0.2f, 0.8f, 0.3f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
    ImGui::TextColored(col, iconOk ? "✔ アイコン画像: 見つかりました" : "✖ アイコン画像: 見つかりません");
}

void SpriteEditorWindow::DrawAnimationSettings() {
    ImGui::Text("GridSheetアニメーション設定（レガシー）");
}

void SpriteEditorWindow::ShowDrawTypeChangeDialog() {
    ImGui::OpenPopup("draw_type切替確認");
    
    if (ImGui::BeginPopupModal("draw_type切替確認", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("描画タイプを変更すると、既存の設定が失われる可能性があります。");
        ImGui::Text("続行しますか？");
        ImGui::Separator();
        
        if (ImGui::Button("変更する", ImVec2(120, 0))) {
            ApplyDrawTypeChange();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("キャンセル", ImVec2(120, 0))) {
            CancelDrawTypeChange();
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void SpriteEditorWindow::ApplyDrawTypeChange() {
    strncpy_s(formData_.draw_type, sizeof(formData_.draw_type), pendingDrawType_, _TRUNCATE);
    isDirty_ = true;
    showDrawTypeDialog_ = false;
}

void SpriteEditorWindow::CancelDrawTypeChange() {
    showDrawTypeDialog_ = false;
}

void SpriteEditorWindow::AddSpriteAction() {
    // 簡略版 - 将来実装
}

void SpriteEditorWindow::RemoveSpriteAction(const std::string& actionName) {
    // 簡略版 - 将来実装
}

void SpriteEditorWindow::SetActiveEntity(const std::string& entityId) {
    LoadEntityToForm(entityId);
}

void SpriteEditorWindow::LoadEntityToForm(const std::string& entityId) {
    const auto* entity = definitions_->GetEntity(entityId);
    if (!entity) return;
    
    // フォームデータをクリア（memset は std::unordered_map を破壊するため使用不可）
    formData_.draw_type[0] = '\0';
    formData_.atlas_texture[0] = '\0';
    formData_.sprite_actions.clear();
    formData_.sprite_sheet[0] = '\0';
    formData_.idle_animation[0] = '\0';
    formData_.idle_image[0] = '\0';
    formData_.walk_animation[0] = '\0';
    formData_.walk_image[0] = '\0';
    formData_.attack_animation[0] = '\0';
    formData_.attack_image[0] = '\0';
    formData_.death_animation[0] = '\0';
    formData_.death_image[0] = '\0';
    formData_.icon[0] = '\0';
    
    // draw_type
    strncpy_s(formData_.draw_type, sizeof(formData_.draw_type), entity->draw_type.c_str(), _TRUNCATE);
    
    // display設定をコピー
    strncpy_s(formData_.atlas_texture, sizeof(formData_.atlas_texture), entity->display.atlas_texture.c_str(), _TRUNCATE);
    strncpy_s(formData_.icon, sizeof(formData_.icon), entity->display.icon.c_str(), _TRUNCATE);
    formData_.mirror_h = entity->display.mirror_h;
    formData_.mirror_v = entity->display.mirror_v;

    // sprite_actionsから4アニメ用フィールドへ反映
    auto itIdle = entity->display.sprite_actions.find("idle");
    if (itIdle != entity->display.sprite_actions.end()) {
        strncpy_s(formData_.idle_animation, sizeof(formData_.idle_animation), itIdle->second.c_str(), _TRUNCATE);
        try {
            std::ifstream in(itIdle->second, std::ios::binary);
            if (in.is_open()) {
                auto j = nlohmann::json::parse(in);
                if (j.contains("meta")) {
                    auto& m = j["meta"];
                    if (m.contains("image")) {
                        std::string img = m["image"].get<std::string>();
                        std::filesystem::path jp = itIdle->second; std::filesystem::path ip = jp.parent_path() / img;
                        strncpy_s(formData_.idle_image, sizeof(formData_.idle_image), NormalizeAssetPath(ip.lexically_normal().generic_string()).c_str(), _TRUNCATE);
                    }
                    formData_.idle_image[0] = formData_.idle_image[0];
                    genIdle_.frameW = m.value("frameW", genIdle_.frameW);
                    genIdle_.frameH = m.value("frameH", genIdle_.frameH);
                    genIdle_.frames = m.value("frames", genIdle_.frames);
                    genIdle_.columns = m.value("columns", genIdle_.columns);
                    genIdle_.rows = m.value("rows", genIdle_.rows);
                    genIdle_.yOffset = m.value("yOffset", genIdle_.yOffset);
                    genIdle_.pivotX = m.value("pivotX", genIdle_.pivotX);
                    genIdle_.pivotY = m.value("pivotY", genIdle_.pivotY);
                    if (m.contains("mirror") && m["mirror"].is_object()) {
                        auto& mir = m["mirror"];
                        genIdle_.mirrorH = mir.value("horizontal", genIdle_.mirrorH);
                        genIdle_.mirrorV = mir.value("vertical", genIdle_.mirrorV);
                    }
                }
            }
        } catch (...) {}
    }
    auto itWalk = entity->display.sprite_actions.find("walk");
    if (itWalk != entity->display.sprite_actions.end()) {
        strncpy_s(formData_.walk_animation, sizeof(formData_.walk_animation), itWalk->second.c_str(), _TRUNCATE);
        try {
            std::ifstream in(itWalk->second, std::ios::binary);
            if (in.is_open()) {
                auto j = nlohmann::json::parse(in);
                if (j.contains("meta")) {
                    auto& m = j["meta"];
                    if (m.contains("image")) {
                        std::string img = m["image"].get<std::string>();
                        std::filesystem::path jp = itWalk->second; std::filesystem::path ip = jp.parent_path() / img;
                        strncpy_s(formData_.walk_image, sizeof(formData_.walk_image), NormalizeAssetPath(ip.lexically_normal().generic_string()).c_str(), _TRUNCATE);
                    }
                    genWalk_.frameW = m.value("frameW", genWalk_.frameW);
                    genWalk_.frameH = m.value("frameH", genWalk_.frameH);
                    genWalk_.frames = m.value("frames", genWalk_.frames);
                    genWalk_.columns = m.value("columns", genWalk_.columns);
                    genWalk_.rows = m.value("rows", genWalk_.rows);
                    genWalk_.yOffset = m.value("yOffset", genWalk_.yOffset);
                    genWalk_.pivotX = m.value("pivotX", genWalk_.pivotX);
                    genWalk_.pivotY = m.value("pivotY", genWalk_.pivotY);
                    if (m.contains("mirror") && m["mirror"].is_object()) {
                        auto& mir = m["mirror"];
                        genWalk_.mirrorH = mir.value("horizontal", genWalk_.mirrorH);
                        genWalk_.mirrorV = mir.value("vertical", genWalk_.mirrorV);
                    }
                }
            }
        } catch (...) {}
    }
    auto itAttack = entity->display.sprite_actions.find("attack");
    if (itAttack != entity->display.sprite_actions.end()) {
        strncpy_s(formData_.attack_animation, sizeof(formData_.attack_animation), itAttack->second.c_str(), _TRUNCATE);
        try {
            std::ifstream in(itAttack->second, std::ios::binary);
            if (in.is_open()) {
                auto j = nlohmann::json::parse(in);
                if (j.contains("meta")) {
                    auto& m = j["meta"];
                    if (m.contains("image")) {
                        std::string img = m["image"].get<std::string>();
                        std::filesystem::path jp = itAttack->second; std::filesystem::path ip = jp.parent_path() / img;
                        strncpy_s(formData_.attack_image, sizeof(formData_.attack_image), NormalizeAssetPath(ip.lexically_normal().generic_string()).c_str(), _TRUNCATE);
                    }
                    genAttack_.frameW = m.value("frameW", genAttack_.frameW);
                    genAttack_.frameH = m.value("frameH", genAttack_.frameH);
                    genAttack_.frames = m.value("frames", genAttack_.frames);
                    genAttack_.columns = m.value("columns", genAttack_.columns);
                    genAttack_.rows = m.value("rows", genAttack_.rows);
                    genAttack_.yOffset = m.value("yOffset", genAttack_.yOffset);
                    genAttack_.pivotX = m.value("pivotX", genAttack_.pivotX);
                    genAttack_.pivotY = m.value("pivotY", genAttack_.pivotY);
                    if (m.contains("mirror") && m["mirror"].is_object()) {
                        auto& mir = m["mirror"];
                        genAttack_.mirrorH = mir.value("horizontal", genAttack_.mirrorH);
                        genAttack_.mirrorV = mir.value("vertical", genAttack_.mirrorV);
                    }
                }
            }
        } catch (...) {}
    }
    auto itDeath = entity->display.sprite_actions.find("death");
    if (itDeath != entity->display.sprite_actions.end()) {
        strncpy_s(formData_.death_animation, sizeof(formData_.death_animation), itDeath->second.c_str(), _TRUNCATE);
        try {
            std::ifstream in(itDeath->second, std::ios::binary);
            if (in.is_open()) {
                auto j = nlohmann::json::parse(in);
                if (j.contains("meta")) {
                    auto& m = j["meta"];
                    if (m.contains("image")) {
                        std::string img = m["image"].get<std::string>();
                        std::filesystem::path jp = itDeath->second; std::filesystem::path ip = jp.parent_path() / img;
                        strncpy_s(formData_.death_image, sizeof(formData_.death_image), NormalizeAssetPath(ip.lexically_normal().generic_string()).c_str(), _TRUNCATE);
                    }
                    genDeath_.frameW = m.value("frameW", genDeath_.frameW);
                    genDeath_.frameH = m.value("frameH", genDeath_.frameH);
                    genDeath_.frames = m.value("frames", genDeath_.frames);
                    genDeath_.columns = m.value("columns", genDeath_.columns);
                    genDeath_.rows = m.value("rows", genDeath_.rows);
                    genDeath_.yOffset = m.value("yOffset", genDeath_.yOffset);
                    genDeath_.pivotX = m.value("pivotX", genDeath_.pivotX);
                    genDeath_.pivotY = m.value("pivotY", genDeath_.pivotY);
                    if (m.contains("mirror") && m["mirror"].is_object()) {
                        auto& mir = m["mirror"];
                        genDeath_.mirrorH = mir.value("horizontal", genDeath_.mirrorH);
                        genDeath_.mirrorV = mir.value("vertical", genDeath_.mirrorV);
                    }
                }
            }
        } catch (...) {}
    }
    
    activeEntityId_ = entityId;
    isDirty_ = false;
    validationErrors_.clear();
}

void SpriteEditorWindow::SaveChanges() {
    validationErrors_.clear();
    
    if (!ValidateForm()) {
        return;
    }
    
    // EntityDefを更新
    auto* entity = const_cast<EntityDef*>(definitions_->GetEntity(activeEntityId_));
    if (!entity) return;
    
    entity->draw_type = formData_.draw_type;
    entity->display.atlas_texture = formData_.atlas_texture;
    entity->display.icon = formData_.icon;
    entity->display.mirror_h = formData_.mirror_h;
    entity->display.mirror_v = formData_.mirror_v;

    // sprite / sprite_for4animation のアクションパス反映
    if (std::string(entity->draw_type) == "sprite") {
        // 既存のsprite_actionsを維持（本UIは一覧表示のみ）
    } else if (std::string(entity->draw_type) == "sprite_for4animation") {
        // 4つのアクションを設定
        if (formData_.idle_animation[0] != '\0') entity->display.sprite_actions["idle"] = formData_.idle_animation;
        if (formData_.walk_animation[0] != '\0') entity->display.sprite_actions["walk"] = formData_.walk_animation;
        if (formData_.attack_animation[0] != '\0') entity->display.sprite_actions["attack"] = formData_.attack_animation;
        if (formData_.death_animation[0] != '\0') entity->display.sprite_actions["death"] = formData_.death_animation;
        // アクション別ミラー既定も反映
        entity->display.action_mirror_h["idle"] = genIdle_.mirrorH;
        entity->display.action_mirror_v["idle"] = genIdle_.mirrorV;
        entity->display.action_mirror_h["walk"] = genWalk_.mirrorH;
        entity->display.action_mirror_v["walk"] = genWalk_.mirrorV;
        entity->display.action_mirror_h["attack"] = genAttack_.mirrorH;
        entity->display.action_mirror_v["attack"] = genAttack_.mirrorV;
        entity->display.action_mirror_h["death"] = genDeath_.mirrorH;
        entity->display.action_mirror_v["death"] = genDeath_.mirrorV;
    }
    
    // JSONファイルへ保存
    if (!SaveEntityDefToJson(activeEntityId_)) {
        validationErrors_.push_back("JSON保存に失敗しました");
        return;
    }
    
    isDirty_ = false;
}

bool SpriteEditorWindow::ValidateForm() const {
    validationErrors_.clear();
    
    std::string drawType = formData_.draw_type;
    
    if (drawType == "sprite") {
        if (formData_.atlas_texture[0] == '\0') {
            validationErrors_.push_back("アトラス画像パスは必須です");
        }
    } else if (drawType == "sprite_for4animation") {
        // 4つのアクションのうち最低1つは必要
        if (formData_.idle_animation[0] == '\0' && formData_.walk_animation[0] == '\0' &&
            formData_.attack_animation[0] == '\0' && formData_.death_animation[0] == '\0') {
            validationErrors_.push_back("少なくとも1つのアニメーションシートを設定してください");
        }
    }
    
    return validationErrors_.empty();
}

bool SpriteEditorWindow::ValidateTexturePath(const std::string& path) const {
    if (path.empty()) return true;
    std::filesystem::path filePath(path);
    return std::filesystem::exists(filePath);
}

bool SpriteEditorWindow::ValidateJsonPath(const std::string& path) const {
    if (path.empty()) return true;
    std::filesystem::path filePath(path);
    return std::filesystem::exists(filePath);
}

bool SpriteEditorWindow::SaveEntityDefToJson(const std::string& entityId) {
    const auto* entity = definitions_->GetEntity(entityId);
    if (!entity || entity->source_path.empty()) {
        return false;
    }
    
    try {
        // EntityDefをJSONに変換
        nlohmann::json j;
        j["id"] = entity->id;
        j["name"] = entity->name;
        j["type"] = entity->type;
        j["rarity"] = entity->rarity;
        j["cost"] = entity->cost;
        j["draw_type"] = entity->draw_type;
        
        // stats
        j["stats"] = nlohmann::json::object();
        j["stats"]["hp"] = entity->stats.hp;
        j["stats"]["attack"] = entity->stats.attack;
        j["stats"]["attack_speed"] = entity->stats.attack_speed;
        j["stats"]["range"] = entity->stats.range;
        j["stats"]["move_speed"] = entity->stats.move_speed;
        j["stats"]["knockback"] = entity->stats.knockback;
        
        // combat
        j["combat"] = nlohmann::json::object();
        j["combat"]["attack_point"] = entity->combat.attack_point;
        j["combat"]["attack_frame"] = entity->combat.attack_frame;
        j["combat"]["hitbox"] = nlohmann::json::object();
        j["combat"]["hitbox"]["width"] = entity->combat.hitbox.width;
        j["combat"]["hitbox"]["height"] = entity->combat.hitbox.height;
        j["combat"]["hitbox"]["offset_x"] = entity->combat.hitbox.offset_x;
        j["combat"]["hitbox"]["offset_y"] = entity->combat.hitbox.offset_y;
        
        // display
        j["display"] = nlohmann::json::object();
        j["display"]["icon"] = entity->display.icon;
        j["display"]["atlas_texture"] = entity->display.atlas_texture;
        j["display"]["mirror_h"] = entity->display.mirror_h;
        j["display"]["mirror_v"] = entity->display.mirror_v;
        j["display"]["sprite_actions"] = nlohmann::json::object();
        for (const auto& [action, path] : entity->display.sprite_actions) {
            j["display"]["sprite_actions"][action] = path;
        }
        
        // ability_ids
        if (!entity->ability_ids.empty()) {
            j["ability_ids"] = nlohmann::json::array();
            for (const auto& ability : entity->ability_ids) {
                j["ability_ids"].push_back(ability);
            }
        }
        
        // skill_ids
        if (!entity->skill_ids.empty()) {
            j["skill_ids"] = nlohmann::json::array();
            for (const auto& skill : entity->skill_ids) {
                j["skill_ids"].push_back(skill);
            }
        }
        
        // tags
        if (!entity->tags.empty()) {
            j["tags"] = nlohmann::json::array();
            for (const auto& tag : entity->tags) {
                j["tags"].push_back(tag);
            }
        }
        
        // ファイルへ書き込み
        std::ofstream out(entity->source_path, std::ios::binary);
        if (!out.is_open()) {
            return false;
        }
        out << j.dump(2);
        out.close();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Entity save error: " << e.what() << std::endl;
        return false;
    }
}

} // namespace Editor::Windows
