#include "Game/Systems/NewRenderingSystem.h"

#include <filesystem>
#include <iostream>
#include <fstream>

#include "Data/Definitions/EntityDef.h"
#include "Game/Components/NewCoreComponents.h"
#include "Game/Graphics/SpriteRenderer.h"
#include <nlohmann/json.hpp>

namespace Game::Systems {

NewRenderingSystem::NewRenderingSystem(Shared::Data::DefinitionRegistry& definitions)
    : definitions_(definitions) {
}

NewRenderingSystem::~NewRenderingSystem() {
    // テクスチャキャッシュのクリーンアップ
    for (auto& [path, texture] : texture_cache_) {
        if (texture.id != 0) {
            UnloadTexture(texture);
        }
    }
    texture_cache_.clear();
}

std::string NewRenderingSystem::ResolveCharacterFolder(const Shared::Data::EntityDef* def) const {
    if (!def) return {};
    
    namespace fs = std::filesystem;
    
    // 1. source_pathからフォルダ名を取得
    if (!def->source_path.empty()) {
        fs::path src(def->source_path);
        fs::path folder_name = src.parent_path().filename();
        std::string tier = def->type.empty() ? "main" : def->type;
        fs::path candidate = fs::path("assets/characters") / tier / folder_name;
        if (fs::exists(candidate)) {
            return candidate.lexically_normal().generic_string();
        }
    }
    
    // 2. atlas_textureのパスからフォルダ名を取得（フォールバック）
    if (!def->display.atlas_texture.empty()) {
        fs::path hint(def->display.atlas_texture);
        fs::path folder_name = hint.parent_path().filename();
        std::string tier = def->type.empty() ? "main" : def->type;
        fs::path candidate = fs::path("assets/characters") / tier / folder_name;
        if (fs::exists(candidate)) {
            return candidate.lexically_normal().generic_string();
        }
    }
    
    return {};
}

std::string NewRenderingSystem::GetActionImagePath(const std::string& folder_path, const std::string& action) const {
    if (folder_path.empty()) return {};
    
    namespace fs = std::filesystem;
    
    // アクション名をファイル名に変換
    std::string filename;
    if (action == "idle") {
        filename = "idle.png";
    } else if (action == "walk") {
        filename = "walk.png";
    } else if (action == "attack") {
        filename = "attack.png";
    } else if (action == "death") {
        filename = "die.png";
    } else {
        // デフォルトはidle
        filename = "idle.png";
    }
    
    fs::path image_path = fs::path(folder_path) / filename;
    return image_path.lexically_normal().generic_string();
}

std::string NewRenderingSystem::GetActionJsonPath(const std::string& folder_path, const std::string& action) const {
    if (folder_path.empty()) return {};
    
    namespace fs = std::filesystem;
    
    // アクション名をJSONファイル名に変換
    std::string filename;
    if (action == "idle") {
        filename = "idle.json";
    } else if (action == "walk") {
        filename = "walk.json";
    } else if (action == "attack") {
        filename = "attack.json";
    } else if (action == "death") {
        filename = "die.json";
    } else {
        // デフォルトはidle
        filename = "idle.json";
    }
    
    fs::path json_path = fs::path(folder_path) / filename;
    return json_path.lexically_normal().generic_string();
}

bool NewRenderingSystem::GetFrameRectFromJson(const std::string& jsonPath, const std::string& actionName, int frameIndex, Rectangle& outFrameRect) const {
    namespace fs = std::filesystem;
    
    if (!fs::exists(jsonPath)) {
        return false;
    }
    
    try {
        std::ifstream file(jsonPath);
        if (!file.is_open()) {
            return false;
        }
        
        nlohmann::json j = nlohmann::json::parse(file);
        
        // framesオブジェクトからアクション名に対応するフレームを取得
        if (!j.contains("frames") || !j["frames"].is_object()) {
            return false;
        }
        
        // frameTagsからアクション名に対応するフレーム範囲を取得
        int fromFrame = 0;
        int toFrame = 0;
        if (j.contains("meta") && j["meta"].contains("frameTags")) {
            for (const auto& tag : j["meta"]["frameTags"]) {
                if (tag.contains("name") && tag["name"] == actionName) {
                    fromFrame = tag.value("from", 0);
                    toFrame = tag.value("to", 0);
                    break;
                }
            }
        }
        
        // フレームインデックスを範囲内に制限
        int actualFrameIndex = std::clamp(frameIndex, fromFrame, toFrame);
        
        // フレーム名を構築（例: "walk-0.aseprite"）
        std::string frameName = actionName + "-" + std::to_string(actualFrameIndex) + ".aseprite";
        
        // framesオブジェクトから該当フレームを取得
        if (!j["frames"].contains(frameName)) {
            // フレームが見つからない場合は、最初のフレームを試す
            frameName = actionName + "-0.aseprite";
            if (!j["frames"].contains(frameName)) {
                return false;
            }
        }
        
        const auto& frameJson = j["frames"][frameName];
        if (!frameJson.contains("frame")) {
            return false;
        }
        
        const auto& frameRect = frameJson["frame"];
        outFrameRect.x = frameRect.value("x", 0.0f);
        outFrameRect.y = frameRect.value("y", 0.0f);
        outFrameRect.width = frameRect.value("w", 0.0f);
        outFrameRect.height = frameRect.value("h", 0.0f);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[NewRenderingSystem] JSON parse error: " << e.what() << std::endl;
        return false;
    }
}

Texture2D NewRenderingSystem::LoadOrGetTexture(const std::string& path) const {
    if (path.empty()) {
        Texture2D empty{};
        empty.id = 0;
        return empty;
    }
    
    // キャッシュを確認
    auto it = texture_cache_.find(path);
    if (it != texture_cache_.end()) {
        return it->second;
    }
    
    // ファイルの存在確認
    namespace fs = std::filesystem;
    if (!fs::exists(path)) {
        // キャッシュに空のテクスチャを保存（再試行を防ぐ）
        Texture2D empty{};
        empty.id = 0;
        texture_cache_[path] = empty;
        return empty;
    }
    
    // テクスチャを読み込む
    Texture2D texture = LoadTexture(path.c_str());
    if (texture.id != 0) {
        texture_cache_[path] = texture;
    } else {
        // 読み込み失敗時も空のテクスチャをキャッシュ
        texture_cache_[path] = texture;
    }
    
    return texture;
}

void NewRenderingSystem::DrawEntities(entt::registry& registry) const {
    auto view = registry.view<Game::Components::Transform, Game::Components::Animation, Game::Components::Team, Game::Components::EntityDefId>();

    for (auto entity : view) {
        const auto& transform = view.get<Game::Components::Transform>(entity);
        const auto& anim = view.get<Game::Components::Animation>(entity);
        const auto& team = view.get<Game::Components::Team>(entity);
        const auto& entityId = view.get<Game::Components::EntityDefId>(entity);
        
        // エンティティ定義を取得
        const auto* def = definitions_.GetEntity(entityId.id);
        if (!def) continue;
        
        // キャラクターフォルダパスを解決
        std::string folder_path = ResolveCharacterFolder(def);
        if (folder_path.empty()) {
            // フォルダが解決できない場合は短形で描画
            Color tint = (team.type == Game::Components::Team::Type::Player) ? BLUE : RED;
            constexpr float DEFAULT_SIZE = 40.0f;
            DrawRectangle(static_cast<int>(transform.x - DEFAULT_SIZE * 0.5f),
                         static_cast<int>(transform.y - DEFAULT_SIZE * 0.5f),
                         static_cast<int>(DEFAULT_SIZE),
                         static_cast<int>(DEFAULT_SIZE),
                         tint);
            continue;
        }
        
        // アクションに応じた画像パスを取得
        std::string action = anim.currentClip; // "idle", "walk", "attack", "death"
        std::string image_path = GetActionImagePath(folder_path, action);
        std::string json_path = GetActionJsonPath(folder_path, action);
        
        // テクスチャを読み込む（キャッシュから取得、または新規読み込み）
        Texture2D texture = LoadOrGetTexture(image_path);
        
        if (texture.id != 0) {
            // JSONファイルが存在する場合はフレーム情報を使用
            Rectangle src_rect{0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height)};
            bool useJsonFrame = false;
            
            namespace fs = std::filesystem;
            if (fs::exists(json_path)) {
                Rectangle frameRect;
                // フレームインデックスが範囲外の場合は0に固定
                int safeFrameIndex = std::max(0, anim.frameIndex);
                if (GetFrameRectFromJson(json_path, action, safeFrameIndex, frameRect)) {
                    // フレーム矩形が有効な場合のみ使用
                    if (frameRect.width > 0 && frameRect.height > 0 && 
                        frameRect.x >= 0 && frameRect.y >= 0 &&
                        frameRect.x + frameRect.width <= texture.width &&
                        frameRect.y + frameRect.height <= texture.height) {
                        src_rect = frameRect;
                        useJsonFrame = true;
                    }
                }
            }
            
            // JSONファイルが存在しない場合、またはフレーム情報が取得できない場合は
            // テクスチャ全体を使用（単一画像として扱う）
            if (!useJsonFrame) {
                src_rect = Rectangle{0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height)};
            }
            
            float width = src_rect.width * transform.scaleX;
            float height = src_rect.height * transform.scaleY;
            Rectangle dst_rect{transform.x - width * 0.5f, transform.y - height * 0.5f, width, height};
            Vector2 origin{width * 0.5f, height * 0.5f};
            
            Color tint = (team.type == Game::Components::Team::Type::Player) ? WHITE : Color{255, 150, 150, 255};
            
            // フリップ処理
            // 味方（Player）の場合は常に左右反転
            bool shouldFlipH = transform.flipH;
            if (team.type == Game::Components::Team::Type::Player) {
                shouldFlipH = !shouldFlipH; // 味方は左右反転
            }
            if (shouldFlipH) {
                src_rect.width = -src_rect.width;
            }
            if (transform.flipV) {
                src_rect.height = -src_rect.height;
            }
            
            DrawTexturePro(texture, src_rect, dst_rect, origin, transform.rotation, tint);
        } else {
            // テクスチャが読み込めない場合は短形で描画（フォールバック）
            Color tint = (team.type == Game::Components::Team::Type::Player) ? BLUE : RED;
            constexpr float DEFAULT_SIZE = 40.0f;
            DrawRectangle(static_cast<int>(transform.x - DEFAULT_SIZE * 0.5f),
                         static_cast<int>(transform.y - DEFAULT_SIZE * 0.5f),
                         static_cast<int>(DEFAULT_SIZE),
                         static_cast<int>(DEFAULT_SIZE),
                         tint);
        }
    }
}

} // namespace Game::Systems
