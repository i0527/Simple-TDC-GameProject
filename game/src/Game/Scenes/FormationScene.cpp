#include "Game/Scenes/FormationScene.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <iostream>

namespace {
constexpr float TITLE_Y = 90.0f;
constexpr float PANEL_MARGIN = 60.0f;
constexpr float LIST_ITEM_H = 72.0f;
constexpr float LIST_ITEM_GAP = 8.0f;
constexpr float SLOT_W = 140.0f; // 横幅を少し圧縮（高さは維持）
constexpr float SLOT_H = 160.0f;
constexpr float SLOT_GAP = 10.0f;
constexpr float ICON_SIZE = 110.0f;
constexpr float STATUS_PANEL_MIN_H = 260.0f;
constexpr float STATUS_PANEL_MIN_W = 520.0f; // 横幅を広げる
constexpr float STATUS_PANEL_TARGET_W =
    820.0f; // プレビューをさらに細くする目安
constexpr float PREVIEW_PANEL_MIN_W = 360.0f;
constexpr float PREVIEW_PANEL_MIN_H = 60.0f;
constexpr float PREVIEW_PANEL_BASE_H = 320.0f;
constexpr float PREVIEW_PANEL_MAX_W = 880.0f; // 横幅を持たせすぎない上限
constexpr float ENHANCE_PANEL_H = 160.0f;
constexpr float CAND_CARD_W = 200.0f;
constexpr float CAND_CARD_H = 180.0f; // スロットと揃えた高さ
constexpr float CAND_CARD_GAP = 12.0f;
constexpr int CAND_VISIBLE_ROWS = 1; // 横スクロールの1行表示
constexpr float SCROLL_SPEED = 60.0f;
constexpr float SLOT_HEADER_H = 40.0f; // ヘッダー余白を圧縮（タイトル＋ボタン）
} // namespace

namespace Game::Scenes {

FormationScene::FormationScene(
    Font font, int screen_width, int screen_height,
    Shared::Data::DefinitionRegistry &definitions,
    Game::Managers::FormationManager &formation_manager)
    : font_(font), screen_width_(screen_width), screen_height_(screen_height),
      definitions_(definitions), formation_manager_(formation_manager) {
  RefreshData();
}

FormationScene::~FormationScene() {
  for (auto &[entity_id, tex] : icon_cache_) {
    if (tex.id != 0) {
      UnloadTexture(tex);
    }
  }
}

const Shared::Data::EntityDef *FormationScene::GetSelectedDef() const {
  // 優先: 候補選択。未選択ならスロット選択中のユニットを参照。
  if (selected_candidate_ >= 0 &&
      selected_candidate_ < static_cast<int>(candidates_.size())) {
    return definitions_.GetEntity(candidates_[selected_candidate_]);
  }
  if (selected_slot_ >= 0 && selected_slot_ < static_cast<int>(slots_.size())) {
    const auto &id = slots_[selected_slot_];
    if (!id.empty()) {
      return definitions_.GetEntity(id);
    }
  }
  return nullptr;
}

const Game::Graphics::AsepriteJsonAtlasProvider *FormationScene::GetProvider(const std::string &entity_id) const {
  auto it = provider_cache_.find(entity_id);
  if (it != provider_cache_.end()) {
    return it->second.get();
  }

  const auto *def = definitions_.GetEntity(entity_id);
  if (!def) {
    std::cerr << "[FormationScene] Entity not found: " << entity_id << std::endl;
    return nullptr;
  }
  
  if (def->display.atlas_texture.empty()) {
    std::cerr << "[FormationScene] Atlas texture path is empty for entity: " << entity_id << std::endl;
    return nullptr;
  }

  std::string pngPath = def->display.atlas_texture;
  if (!std::filesystem::exists(pngPath)) {
    std::cerr << "[FormationScene] Atlas texture file not found: " << pngPath << std::endl;
    return nullptr;
  }

  Texture2D tex = LoadTexture(pngPath.c_str());
  if (tex.id == 0) {
    std::cerr << "[FormationScene] Failed to load texture: " << pngPath << std::endl;
    return nullptr;
  }

  // sprite_actions から最初のJSONをロード
  nlohmann::json atlasJson;
  bool loaded = false;
  std::string loadedJsonPath;
  for (const auto &[action, jsonPath] : def->display.sprite_actions) {
    std::filesystem::path fullJsonPath = std::filesystem::path(pngPath).parent_path() / jsonPath;
    if (std::filesystem::exists(fullJsonPath)) {
      std::ifstream file(fullJsonPath.string());
      if (file) {
        try {
          file >> atlasJson;
          loaded = true;
          loadedJsonPath = fullJsonPath.string();
          break;
        } catch (const nlohmann::json::parse_error& e) {
          std::cerr << "[FormationScene] JSON parse error in " << fullJsonPath << ": " << e.what() << std::endl;
        } catch (const std::exception& e) {
          std::cerr << "[FormationScene] Error reading JSON file " << fullJsonPath << ": " << e.what() << std::endl;
        }
      } else {
        std::cerr << "[FormationScene] Failed to open JSON file: " << fullJsonPath << std::endl;
      }
    }
  }
  if (!loaded) {
    std::cerr << "[FormationScene] No valid JSON file found for entity: " << entity_id << std::endl;
    UnloadTexture(tex);
    return nullptr;
  }

  try {
    auto provider = std::make_unique<Game::Graphics::AsepriteJsonAtlasProvider>(tex, atlasJson);
    auto [inserted, _] = provider_cache_.emplace(entity_id, std::move(provider));
    std::cout << "[FormationScene] Successfully loaded provider for " << entity_id 
              << " from " << loadedJsonPath << std::endl;
    return inserted->second.get();
  } catch (const std::exception& e) {
    std::cerr << "[FormationScene] Failed to create AsepriteJsonAtlasProvider for " << entity_id 
              << ": " << e.what() << std::endl;
    UnloadTexture(tex);
    return nullptr;
  }
}

void FormationScene::DrawIcon(const Rectangle &rect,
                              const std::string &entity_id) const {
  const auto *def = definitions_.GetEntity(entity_id);

  auto resolveIconPath = [](const Shared::Data::EntityDef* d) -> std::string {
    if (!d) return {};
    namespace fs = std::filesystem;

    auto exists_path = [](const fs::path& p) { return !p.empty() && fs::exists(p); };

    // 1. display.iconが存在する場合はそれを使用
    if (exists_path(d->display.icon)) {
      return fs::path(d->display.icon).lexically_normal().generic_string();
    }

    // 2. atlas_textureのパスからフォルダ名を取得
    fs::path hint = d->display.icon.empty() ? fs::path(d->display.atlas_texture) : fs::path(d->display.icon);
    if (!hint.empty()) {
      fs::path folder = hint.parent_path().filename();
      std::string tier = d->type.empty() ? "main" : d->type; // main / sub
      fs::path candidate = fs::path("assets/textures/icons/characters") / tier / folder / "icon.png";
      if (exists_path(candidate)) {
        return candidate.lexically_normal().generic_string();
      }
    }

    // 3. source_pathからフォルダ名を取得（フォールバック）
    if (!d->source_path.empty()) {
      fs::path src(d->source_path);
      fs::path folder = src.parent_path().filename();
      std::string tier = d->type.empty() ? "main" : d->type;
      fs::path candidate = fs::path("assets/textures/icons/characters") / tier / folder / "icon.png";
      if (exists_path(candidate)) {
        return candidate.lexically_normal().generic_string();
      }
    }

    return {};
  };
  const std::string icon_path = resolveIconPath(def);

  // Try to draw icon first
  if (def && !icon_path.empty()) {
    auto it = icon_cache_.find(entity_id);
    if (it == icon_cache_.end()) {
      Texture2D tex = LoadTexture(icon_path.c_str());
      if (tex.id != 0) {
        icon_cache_[entity_id] = tex;
        it = icon_cache_.find(entity_id);
      }
    }
    if (it != icon_cache_.end()) {
      // アスペクト比を保持してスケーリング
      float iconAspect = static_cast<float>(it->second.width) / static_cast<float>(it->second.height);
      float rectAspect = rect.width / rect.height;
      
      float scaledWidth, scaledHeight;
      if (iconAspect > rectAspect) {
        // 幅に合わせる
        scaledWidth = rect.width;
        scaledHeight = rect.width / iconAspect;
      } else {
        // 高さに合わせる
        scaledWidth = rect.height * iconAspect;
        scaledHeight = rect.height;
      }
      
      // 中央配置
      float offsetX = (rect.width - scaledWidth) * 0.5f;
      float offsetY = (rect.height - scaledHeight) * 0.5f;
      
      Rectangle destRect = {
        rect.x + offsetX,
        rect.y + offsetY,
        scaledWidth,
        scaledHeight
      };
      
      // originはdestRectの中心（または足元）を指定
      // アイコンの場合は中心に配置
      Vector2 origin = {scaledWidth * 0.5f, scaledHeight * 0.5f};
      
      // 味方（is_enemy == false）の場合は左右反転
      Rectangle src_rect{0.0f, 0.0f, (float)it->second.width, (float)it->second.height};
      if (def && !def->is_enemy) {
        src_rect.width = -src_rect.width; // 左右反転
        destRect.width = -destRect.width;
      }
      
      DrawTexturePro(it->second, 
                     src_rect, 
                     destRect, 
                     origin, 
                     0.0f, 
                     WHITE);
      return;
    }
  }
  
  // Fallback: draw sprite
  auto *provider = const_cast<Game::Graphics::AsepriteJsonAtlasProvider*>(GetProvider(entity_id));
  if (provider) {
    // SharedレイヤーのAnimationRendererを使用
    Shared::Data::Graphics::AnimationRenderer::DrawAnimationInArea(
        *provider, "idle", 0, rect, WHITE);
  } else {
    DrawRectangleRec(rect, Color{60, 100, 200, 255});
    DrawRectangleLinesEx(rect, 2.0f, Color{120, 170, 240, 255});
  }
}

void FormationScene::DrawSpriteAnim(const Rectangle &area,
                                    Shared::Data::Graphics::IFrameProvider &provider,
                                    const std::string &clipName, float /*elapsed*/,
                                    int current_frame) const {
  // フレームを取得
  auto frame = provider.GetFrame(clipName, current_frame);
  if (!frame.valid || !frame.texture) {
    return;
  }
  
  // エリア内に収まるようにスケールを計算（アスペクト比を保持）
  if (frame.src.width <= 0.0f || frame.src.height <= 0.0f) {
    return;
  }
  
  float scaleX = area.width / frame.src.width;
  float scaleY = area.height / frame.src.height;
  float scale = std::min(scaleX, scaleY) * 0.9f; // 90%に縮小して余白を確保
  
  // エリアの中心位置
  Vector2 centerPosition = {
    area.x + area.width * 0.5f,
    area.y + area.height * 0.5f
  };
  
  // 足元位置を計算（中央に配置するため）
  // DrawAnimationの動作：
  // - positionは足元位置として扱われる
  // - destPosition = position + (frame.offset * scale)
  // - destRectはdestPositionから始まり、frame.originが回転中心
  // スプライトの中心をエリアの中心に配置するには：
  // - スプライトの中心Y = destPosition.y + frame.origin.y * scale
  // - これをエリアの中心Yに合わせる：destPosition.y = centerY - frame.origin.y * scale
  // - position.y = destPosition.y - frame.offset.y * scale
  // - position.y = centerY - frame.origin.y * scale - frame.offset.y * scale
  Vector2 footPosition = {
    centerPosition.x,
    centerPosition.y - (frame.origin.y * scale) - (frame.offset.y * scale)
  };
  
  // ミラーリング対応で描画
  Shared::Data::Graphics::AnimationRenderer::DrawAnimation(
      provider, clipName, current_frame, footPosition, {scale, scale}, 0.0f, WHITE, preview_.mirrorH, preview_.mirrorV);
}

void FormationScene::UpdatePreview(float delta_time) {
  const auto *def = GetSelectedDef();
  if (!def) {
    preview_.animTimer = 0.0f;
    preview_.currentFrame = 0;
    preview_.playingAttack = false;
    return;
  }

  std::string entity_id = def->id;
  auto *provider = const_cast<Game::Graphics::AsepriteJsonAtlasProvider*>(GetProvider(entity_id));
  if (!provider) {
    // Providerが取得できない場合でも、タイマーは進める（アイコン表示時など）
    preview_.animTimer += delta_time;
    preview_.currentFrame = 0;
    return;
  }

  // ミラーリング設定を更新
  if (def) {
    preview_.mirrorH = def->display.mirror_h;
    preview_.mirrorV = def->display.mirror_v;
    
    // 選択されたアニメーションを使用（playingAttackは非推奨だが互換性のため残す）
    std::string clipName = preview_.playingAttack ? "attack" : preview_.selectedAnimation;
    
    // アクション別ミラー設定を確認
    auto itH = def->display.action_mirror_h.find(clipName);
    auto itV = def->display.action_mirror_v.find(clipName);
    if (itH != def->display.action_mirror_h.end()) {
      preview_.mirrorH = itH->second;
    }
    if (itV != def->display.action_mirror_v.end()) {
      preview_.mirrorV = itV->second;
    }
  }

  // 選択されたアニメーションを使用（playingAttackは非推奨だが互換性のため残す）
  std::string clipName = preview_.playingAttack ? "attack" : preview_.selectedAnimation;
  if (!provider->HasClip(clipName)) {
    // フォールバック: walk -> idle -> attack -> death の順で試す
    if (provider->HasClip("walk")) {
      clipName = "walk";
      preview_.selectedAnimation = "walk";
    } else if (provider->HasClip("idle")) {
      clipName = "idle";
      preview_.selectedAnimation = "idle";
    } else if (provider->HasClip("attack")) {
      clipName = "attack";
      preview_.selectedAnimation = "attack";
    } else if (provider->HasClip("death")) {
      clipName = "death";
      preview_.selectedAnimation = "death";
    } else {
      return;
    }
  }

  // フレームごとの個別durationを考慮したアニメーション更新
  int frameCount = provider->GetFrameCount(clipName);
  if (frameCount <= 0) return;

  // 現在のフレームのdurationを取得
  auto currentFrame = provider->GetFrame(clipName, preview_.currentFrame);
  float frameDuration = 0.0f;
  if (currentFrame.valid && currentFrame.durationSec > 0.0f) {
    frameDuration = currentFrame.durationSec;
  } else {
    // フォールバック: FPSから計算
    float fps = provider->GetClipFps(clipName);
    if (fps <= 0.0f) fps = 12.0f; // デフォルトFPS
    frameDuration = 1.0f / fps;
  }

  preview_.animTimer += delta_time;
  while (preview_.animTimer >= frameDuration && frameDuration > 0.0f) {
    preview_.animTimer -= frameDuration;
    preview_.currentFrame++;
    
    // ループ処理
    if (preview_.currentFrame >= frameCount) {
      if (provider->IsLooping(clipName)) {
        preview_.currentFrame = 0;
      } else {
        preview_.currentFrame = frameCount - 1;
        preview_.animTimer = 0.0f;
        break;
      }
    }
    
    // 次のフレームのdurationを取得
    if (preview_.currentFrame < frameCount) {
      auto nextFrame = provider->GetFrame(clipName, preview_.currentFrame);
      if (nextFrame.valid && nextFrame.durationSec > 0.0f) {
        frameDuration = nextFrame.durationSec;
      } else {
        float fps = provider->GetClipFps(clipName);
        if (fps <= 0.0f) fps = 12.0f;
        frameDuration = 1.0f / fps;
      }
    }
  }

  if (preview_.playingAttack) {
    // revert to selected animation after short burst
    if (preview_.animTimer > 0.6f) {
      preview_.playingAttack = false;
      preview_.animTimer = 0.0f;
      preview_.currentFrame = 0;
    }
  }
}


void FormationScene::StartAttackPreview() {
  preview_.playingAttack = true;
  preview_.animTimer = 0.0f;
  preview_.currentFrame = 0;
}

std::string FormationScene::GetAnimationClipName(const std::string &displayName) const {
  // 表示名からクリップ名に変換
  if (displayName == u8"移動") return "walk";
  if (displayName == u8"攻撃") return "attack";
  if (displayName == u8"待機") return "idle";
  if (displayName == u8"死亡") return "death";
  return displayName; // そのまま返す（既にクリップ名の場合）
}

void FormationScene::DrawAnimationDropdown(const Rectangle &area) {
  const std::vector<std::string> allAnimationNames = {u8"移動", u8"攻撃", u8"待機", u8"死亡"};
  const std::vector<std::string> allClipNames = {"walk", "attack", "idle", "death"};
  
  // 利用可能なアニメーションを確認
  std::vector<std::string> availableNames;
  std::vector<std::string> availableClips;
  std::vector<int> originalIndices;
  
  const auto *def = GetSelectedDef();
  if (def) {
    auto *provider = const_cast<Game::Graphics::AsepriteJsonAtlasProvider*>(GetProvider(def->id));
    if (provider) {
      for (size_t i = 0; i < allClipNames.size(); ++i) {
        if (provider->HasClip(allClipNames[i])) {
          availableNames.push_back(allAnimationNames[i]);
          availableClips.push_back(allClipNames[i]);
          originalIndices.push_back(static_cast<int>(i));
        }
      }
    }
  }
  
  // 利用可能なアニメーションがない場合はデフォルトリストを使用
  if (availableNames.empty()) {
    availableNames = allAnimationNames;
    availableClips = allClipNames;
    for (size_t i = 0; i < allClipNames.size(); ++i) {
      originalIndices.push_back(static_cast<int>(i));
    }
  }
  
  // 現在選択されているアニメーションの表示名を取得
  std::string currentDisplayName = availableNames.empty() ? u8"移動" : availableNames[0];
  int currentIndex = 0;
  for (size_t i = 0; i < availableClips.size(); ++i) {
    if (preview_.selectedAnimation == availableClips[i]) {
      currentDisplayName = availableNames[i];
      currentIndex = static_cast<int>(i);
      break;
    }
  }

  // ドロップダウンボタン
  Color btnBg = preview_.dropdownOpen ? Color{100, 120, 160, 255} : Color{70, 90, 130, 255};
  DrawRectangleRounded(area, 0.08f, 4, btnBg);
  DrawRectangleLinesEx(area, 2.0f, Color{180, 220, 255, 255});

  // 選択されたアニメーション名を表示
  Vector2 textSize = MeasureTextEx(font_, currentDisplayName.c_str(), 18.0f, 2.0f);
  DrawTextEx(font_, currentDisplayName.c_str(),
             {area.x + 8.0f, area.y + (area.height - textSize.y) * 0.5f},
             18.0f, 2.0f, RAYWHITE);

  // 下矢印アイコン（簡易版: "▼"）
  const char *arrow = u8"▼";
  Vector2 arrowSize = MeasureTextEx(font_, arrow, 14.0f, 2.0f);
  DrawTextEx(font_, arrow,
             {area.x + area.width - arrowSize.x - 8.0f, area.y + (area.height - arrowSize.y) * 0.5f},
             14.0f, 2.0f, RAYWHITE);

  // ドロップダウンリスト（開いている場合）
  if (preview_.dropdownOpen && !availableNames.empty()) {
    const float itemHeight = 32.0f;
    const float listHeight = static_cast<float>(availableNames.size()) * itemHeight;
    Rectangle listArea{area.x, area.y + area.height, area.width, listHeight};

    // 背景
    DrawRectangleRounded(listArea, 0.08f, 4, Color{50, 60, 80, 255});
    DrawRectangleLinesEx(listArea, 2.0f, Color{180, 220, 255, 255});

    // 各項目を描画
    for (size_t i = 0; i < availableNames.size(); ++i) {
      Rectangle itemRect{listArea.x, listArea.y + static_cast<float>(i) * itemHeight,
                         listArea.width, itemHeight};
      
      // ホバー効果（マウス位置で判定）
      Vector2 mouse = GetMousePosition();
      bool hovered = CheckCollisionPointRec(mouse, itemRect);
      Color itemBg = (i == static_cast<size_t>(currentIndex))
                         ? Color{70, 90, 130, 240}
                         : (hovered ? Color{50, 70, 100, 240} : Color{40, 50, 70, 240});
      
      DrawRectangleRounded(itemRect, 0.06f, 2, itemBg);
      if (i == static_cast<size_t>(currentIndex)) {
        DrawRectangleLinesEx(itemRect, 1.5f, Color{120, 180, 255, 255});
      }

      // テキスト
      Vector2 itemTextSize = MeasureTextEx(font_, availableNames[i].c_str(), 18.0f, 2.0f);
      DrawTextEx(font_, availableNames[i].c_str(),
                 {itemRect.x + 8.0f, itemRect.y + (itemRect.height - itemTextSize.y) * 0.5f},
                 18.0f, 2.0f, RAYWHITE);
    }
  }
}

bool FormationScene::HasPreviewAnimation() const {
  const auto *def = GetSelectedDef();
  if (!def) return false;
  const auto *provider = GetProvider(def->id);
  return provider != nullptr;
}

bool FormationScene::ConsumeReturnHome() {
  bool v = return_home_requested_ || apply_requested_;
  return_home_requested_ = false;
  apply_requested_ = false;
  return v;
}

void FormationScene::RefreshData() {
  candidates_ = formation_manager_.GetCandidates();
  slots_ = formation_manager_.GetSlots();
  candidate_scroll_ = 0.0f;

  if (candidates_.empty()) {
    selected_candidate_ = -1;
  } else {
    selected_candidate_ = std::clamp(selected_candidate_, 0,
                                     static_cast<int>(candidates_.size() - 1));
  }

  if (slots_.empty()) {
    selected_slot_ = -1;
  } else {
    selected_slot_ =
        std::clamp(selected_slot_, 0, static_cast<int>(slots_.size() - 1));
  }

  preview_.animTimer = 0.0f;
  preview_.currentFrame = 0;
  preview_.playingAttack = false;
  preview_.selectedAnimation = "walk"; // デフォルトは移動
  preview_.dropdownOpen = false;
  preview_.mirrorH = false;
  preview_.mirrorV = false;
  preview_.showDebug = false;
}

FormationScene::LayoutInfo FormationScene::BuildLayout() const {
  LayoutInfo layout{};

  const int cand_count = static_cast<int>(candidates_.size());
  const int slot_count = static_cast<int>(slots_.size());
  const float cand_panel_width =
      static_cast<float>(screen_width_) - PANEL_MARGIN * 2.0f;
  int per_row = cand_count; // 全件を横に並べる
  int rows = 1;

  float cand_panel_height = 44.0f + CAND_CARD_H + 24.0f;
  float cand_panel_y =
      static_cast<float>(screen_height_) - cand_panel_height - 28.0f;

  layout.candidatePanel = {PANEL_MARGIN, cand_panel_y, cand_panel_width,
                           cand_panel_height};
  layout.candidatesPerRow = per_row;
  layout.candidateRows = rows;

  const float top_y = TITLE_Y + 60.0f;
  float available_h = cand_panel_y - top_y - 12.0f;
  available_h = std::max(360.0f, available_h);

  const float content_w =
      static_cast<float>(screen_width_) - PANEL_MARGIN * 2.0f;
  float max_status_w =
      std::max(STATUS_PANEL_MIN_W, content_w - PREVIEW_PANEL_MIN_W - 16.0f);
  float status_w =
      std::clamp(STATUS_PANEL_TARGET_W, STATUS_PANEL_MIN_W, max_status_w);
  float preview_w = content_w - status_w - 16.0f;
  if (preview_w > PREVIEW_PANEL_MAX_W) {
    preview_w = PREVIEW_PANEL_MAX_W;
    status_w = std::max(STATUS_PANEL_MIN_W, content_w - preview_w - 16.0f);
  }
  if (preview_w < PREVIEW_PANEL_MIN_W) {
    preview_w = PREVIEW_PANEL_MIN_W;
    status_w = std::max(STATUS_PANEL_MIN_W, content_w - preview_w - 16.0f);
  }

  // ドロップダウン（32px）+ 余白（8px）+ アニメーションエリア + キャラクター名（40px）を考慮
  float preview_h = PREVIEW_PANEL_BASE_H;
  // ドロップダウン分の高さを確保（最低でもドロップダウン + アニメーションエリア + 名前表示）
  const float min_preview_h = 32.0f + 8.0f + 200.0f + 40.0f; // ドロップダウン + 余白 + アニメーション + 名前
  preview_h = std::max(preview_h, min_preview_h);
  
  int slot_cols = 5; // 固定で 2 x 5
  int slot_rows = slot_count > 0 ? (slot_count + slot_cols - 1) / slot_cols : 1;
  slot_rows = std::min(slot_rows, 2);
  float slot_panel_h =
      SLOT_HEADER_H + static_cast<float>(slot_rows) * SLOT_H +
      static_cast<float>(std::max(0, slot_rows - 1)) * SLOT_GAP + 8.0f;

  float preview_and_slots_h = preview_h + 12.0f + slot_panel_h;
  if (preview_and_slots_h > available_h) {
    float diff = preview_and_slots_h - available_h;
    preview_h = std::max(min_preview_h, preview_h - diff);
    preview_and_slots_h = preview_h + 12.0f + slot_panel_h;
  }
  float status_h = std::max(STATUS_PANEL_MIN_H, preview_and_slots_h);

  layout.statusPanel = {PANEL_MARGIN, top_y, status_w, status_h};
  layout.previewPanel = {layout.statusPanel.x + layout.statusPanel.width +
                             16.0f,
                         top_y, preview_w, preview_h};

  layout.slotPanel = {layout.previewPanel.x,
                      layout.previewPanel.y + layout.previewPanel.height +
                          12.0f,
                      layout.previewPanel.width, slot_panel_h};
  layout.slotCols = slot_cols;
  layout.slotRows = slot_rows;

  return layout;
}

void FormationScene::AssignSelectedToSlot(int slot_index) {
  if (slot_index < 0 || slot_index >= static_cast<int>(slots_.size())) {
    return;
  }
  if (selected_candidate_ < 0 ||
      selected_candidate_ >= static_cast<int>(candidates_.size())) {
    return;
  }
  if (formation_manager_.AssignToSlot(slot_index,
                                      candidates_[selected_candidate_])) {
    RefreshData();
  }
}

void FormationScene::HandleInput() {
  Vector2 mouse = GetMousePosition();
  bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  bool release = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

  LayoutInfo layout = BuildLayout();

  if (IsKeyPressed(KEY_ESCAPE)) {
    return_home_requested_ = true;
    return;
  }

  // 左上ボタン（戻る）＋スロットエリア内ボタン（リセット/外す）
  Rectangle back_btn{24.0f, 20.0f, 180.0f, 44.0f};
  float slot_btn_y = layout.slotPanel.y + 6.0f;
  float slot_btn_h = 30.0f;
  float margin = 10.0f;
  float remove_w = 170.0f;
  float reset_w = 150.0f;
  float remove_x =
      layout.slotPanel.x + layout.slotPanel.width - margin - remove_w;
  float reset_x = remove_x - 8.0f - reset_w;
  Rectangle reset_btn{reset_x, slot_btn_y, reset_w, slot_btn_h};
  Rectangle remove_btn{remove_x, slot_btn_y, remove_w, slot_btn_h};
  if (click) {
    if (CheckCollisionPointRec(mouse, back_btn)) {
      apply_requested_ = true;
      return_home_requested_ = true;
      return;
    }
    if (CheckCollisionPointRec(mouse, reset_btn)) {
      formation_manager_.SetSlots(
          std::vector<std::string>(formation_manager_.GetMaxSlots()));
      RefreshData();
      drag_.active = false;
      return;
    }
    if (CheckCollisionPointRec(mouse, remove_btn)) {
      if (selected_slot_ >= 0 &&
          selected_slot_ < static_cast<int>(formation_manager_.GetMaxSlots())) {
        formation_manager_.ClearSlot(selected_slot_);
        RefreshData();
        drag_.active = false;
      }
      return;
    }
  }

  if (IsKeyPressed(KEY_ENTER)) {
    apply_requested_ = true;
    return_home_requested_ = true;
    return;
  }

  const int cand_count = static_cast<int>(candidates_.size());
  const int slot_count = static_cast<int>(slots_.size());
  auto sync_candidate_from_slot = [&](int slot_index) {
    if (slot_index < 0 || slot_index >= slot_count) {
      return;
    }
    const std::string &slot_id = slots_[slot_index];
    if (slot_id.empty()) {
      return;
    }
    auto it = std::find(candidates_.begin(), candidates_.end(), slot_id);
    if (it != candidates_.end()) {
      selected_candidate_ =
          static_cast<int>(std::distance(candidates_.begin(), it));
    }
  };

  if (cand_count > 0) {
    if (IsKeyPressed(KEY_DOWN)) {
      selected_candidate_ = (selected_candidate_ + 1 + cand_count) % cand_count;
    }
    if (IsKeyPressed(KEY_UP)) {
      selected_candidate_ = (selected_candidate_ - 1 + cand_count) % cand_count;
    }
  }

  if (slot_count > 0) {
    if (IsKeyPressed(KEY_RIGHT)) {
      selected_slot_ = (selected_slot_ + 1 + slot_count) % slot_count;
      sync_candidate_from_slot(selected_slot_);
    }
    if (IsKeyPressed(KEY_LEFT)) {
      selected_slot_ = (selected_slot_ - 1 + slot_count) % slot_count;
      sync_candidate_from_slot(selected_slot_);
    }
  }

  // スロット操作はドラッグ＆ドロップのみに制限（ホットキー/クリック割当は無効化）

  // Candidate scroll (横スクロール)
  if (CheckCollisionPointRec(mouse, layout.candidatePanel)) {
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
      float content_w =
          static_cast<float>(layout.candidatesPerRow) * CAND_CARD_W +
          static_cast<float>(std::max(0, layout.candidatesPerRow - 1)) *
              CAND_CARD_GAP;
      float visible_w =
          layout.candidatePanel.width - 24.0f; // 左右余白分差し引き
      float max_scroll = std::max(0.0f, content_w - visible_w);
      candidate_scroll_ += wheel * SCROLL_SPEED;
      candidate_scroll_ =
          std::min(0.0f, std::max(-max_scroll, candidate_scroll_));
    }
  }

  // Candidate list interaction
  float start_x = layout.candidatePanel.x + 12.0f + candidate_scroll_;
  float start_y = layout.candidatePanel.y + 44.0f;
  int per_row = std::max(1, layout.candidatesPerRow);
  for (int i = 0; i < cand_count; ++i) {
    int row = i / per_row;
    int col = i % per_row;
    Rectangle r{start_x + col * (CAND_CARD_W + CAND_CARD_GAP),
                start_y + row * (CAND_CARD_H + CAND_CARD_GAP), CAND_CARD_W,
                CAND_CARD_H};
    if (CheckCollisionRecs(r, layout.candidatePanel) &&
        CheckCollisionPointRec(mouse, r)) {
      if (click) {
        selected_candidate_ = i;
        preview_.animTimer = 0.0f;
        preview_.currentFrame = 0;
        preview_.playingAttack = false;
        // 選択されたキャラクターが利用可能なアニメーションを確認してデフォルトを設定
        const auto *def = definitions_.GetEntity(candidates_[i]);
        if (def) {
          // ミラーリング設定を更新
          preview_.mirrorH = def->display.mirror_h;
          preview_.mirrorV = def->display.mirror_v;
          
          auto *provider = const_cast<Game::Graphics::AsepriteJsonAtlasProvider*>(GetProvider(def->id));
          if (provider) {
            // 利用可能なアニメーションを優先順位で確認
            if (provider->HasClip("walk")) {
              preview_.selectedAnimation = "walk";
            } else if (provider->HasClip("idle")) {
              preview_.selectedAnimation = "idle";
            } else if (provider->HasClip("attack")) {
              preview_.selectedAnimation = "attack";
            } else if (provider->HasClip("death")) {
              preview_.selectedAnimation = "death";
            }
            
            // 選択されたアニメーションのアクション別ミラー設定を確認
            auto itH = def->display.action_mirror_h.find(preview_.selectedAnimation);
            auto itV = def->display.action_mirror_v.find(preview_.selectedAnimation);
            if (itH != def->display.action_mirror_h.end()) {
              preview_.mirrorH = itH->second;
            }
            if (itV != def->display.action_mirror_v.end()) {
              preview_.mirrorV = itV->second;
            }
          }
        }
        drag_.active = true;
        drag_.candidateIndex = i;
        drag_.startPos = mouse;
        drag_.currentPos = mouse;
      }
    }
  }

  if (drag_.active) {
    drag_.currentPos = mouse;
  }

  // Slots interaction
  float slots_w =
      static_cast<float>(layout.slotCols) * SLOT_W +
      static_cast<float>(std::max(0, layout.slotCols - 1)) * SLOT_GAP;
  float slot_start_x =
      layout.slotPanel.x +
      std::max(8.0f, (layout.slotPanel.width - slots_w) * 0.5f);
  float slot_start_y = layout.slotPanel.y + SLOT_HEADER_H;
  const int cols = layout.slotCols;
  for (int i = 0; i < slot_count; ++i) {
    int row = i / cols;
    int col = i % cols;
    Rectangle r{slot_start_x + col * (SLOT_W + SLOT_GAP),
                slot_start_y + row * (SLOT_H + SLOT_GAP), SLOT_W, SLOT_H};
    if (CheckCollisionPointRec(mouse, r)) {
      if (click && !drag_.active) {
        // クリックは選択のみ（割り当てはドラッグ＆ドロップ限定）
        selected_slot_ = i;
        sync_candidate_from_slot(i);
      }
      if (release && drag_.active && drag_.candidateIndex >= 0) {
        selected_slot_ = i;
        selected_candidate_ = drag_.candidateIndex;
        AssignSelectedToSlot(i);
      }
    }
  }

  if (release && drag_.active) {
    drag_.active = false;
    drag_.candidateIndex = -1;
  }

  // ドロップダウンのクリック処理
  const float dropdown_y = layout.previewPanel.y + 44.0f;
  const float dropdown_h = 32.0f;
  const float dropdown_w = 200.0f;
  const float dropdown_x = layout.previewPanel.x + layout.previewPanel.width - dropdown_w - 20.0f;
  Rectangle dropdown_area{dropdown_x, dropdown_y, dropdown_w, dropdown_h};
  const float itemHeight = 32.0f;
  
  // 利用可能なアニメーションを取得
  const std::vector<std::string> allClipNames = {"walk", "attack", "idle", "death"};
  const std::vector<std::string> allAnimationNames = {u8"移動", u8"攻撃", u8"待機", u8"死亡"};
  std::vector<std::string> availableClips;
  std::vector<std::string> availableNames;
  
  const auto *def = GetSelectedDef();
  if (def) {
    auto *provider = const_cast<Game::Graphics::AsepriteJsonAtlasProvider*>(GetProvider(def->id));
    if (provider) {
      for (size_t i = 0; i < allClipNames.size(); ++i) {
        if (provider->HasClip(allClipNames[i])) {
          availableClips.push_back(allClipNames[i]);
          availableNames.push_back(allAnimationNames[i]);
        }
      }
    }
  }
  
  // 利用可能なアニメーションがない場合はデフォルトリストを使用
  if (availableClips.empty()) {
    availableClips = allClipNames;
    availableNames = allAnimationNames;
  }

  if (click) {
    // ドロップダウンボタンのクリック
    if (CheckCollisionPointRec(mouse, dropdown_area)) {
      preview_.dropdownOpen = !preview_.dropdownOpen;
    } else if (preview_.dropdownOpen) {
      // ドロップダウンリスト内の項目クリック
      Rectangle listArea{dropdown_area.x, dropdown_area.y + dropdown_area.height,
                         dropdown_area.width, static_cast<float>(availableNames.size()) * itemHeight};
      if (CheckCollisionPointRec(mouse, listArea)) {
        for (size_t i = 0; i < availableNames.size(); ++i) {
          Rectangle itemRect{listArea.x, listArea.y + static_cast<float>(i) * itemHeight,
                             listArea.width, itemHeight};
          if (CheckCollisionPointRec(mouse, itemRect)) {
            preview_.selectedAnimation = availableClips[i];
            preview_.animTimer = 0.0f;
            preview_.currentFrame = 0;
            preview_.dropdownOpen = false;
            
            // 選択されたアニメーションのアクション別ミラー設定を更新
            const auto *def = GetSelectedDef();
            if (def) {
              preview_.mirrorH = def->display.mirror_h;
              preview_.mirrorV = def->display.mirror_v;
              
              auto itH = def->display.action_mirror_h.find(availableClips[i]);
              auto itV = def->display.action_mirror_v.find(availableClips[i]);
              if (itH != def->display.action_mirror_h.end()) {
                preview_.mirrorH = itH->second;
              }
              if (itV != def->display.action_mirror_v.end()) {
                preview_.mirrorV = itV->second;
              }
            }
            break;
          }
        }
      } else {
        // リスト外をクリックした場合は閉じる
        preview_.dropdownOpen = false;
      }
    } else {
      // ドロップダウン外をクリックした場合は閉じる
      preview_.dropdownOpen = false;
    }
  }

  // Preview click triggers attack animation (only if animation exists)
  // ドロップダウンエリアを除外
  Rectangle anim_click_area{layout.previewPanel.x + 20.0f,
                             dropdown_y + dropdown_h + 8.0f,
                             layout.previewPanel.width - 40.0f,
                             layout.previewPanel.height - (dropdown_y + dropdown_h + 8.0f - layout.previewPanel.y) - 64.0f};
  if (click && CheckCollisionPointRec(mouse, anim_click_area) &&
      HasPreviewAnimation() && !preview_.dropdownOpen) {
    StartAttackPreview();
  }

  // Enhancement area click (mock)
  if (click &&
      CheckCollisionPointRec(mouse, GetEnhancementArea(layout.statusPanel))) {
    TryMockUpgrade();
  }
}

void FormationScene::Update(float delta_time) {
  HandleInput();
  UpdatePreview(delta_time);
}

void FormationScene::DrawCandidates(const Rectangle &panel,
                                    int candidates_per_row,
                                    int candidate_rows) const {
  (void)candidate_rows;
  DrawRectangleRounded(panel, 0.12f, 6, Color{38, 52, 72, 230});
  DrawRectangleLinesEx(panel, 2.0f, Color{140, 190, 240, 230});

  const float title_size = 28.0f;
  const char *title = u8"候補ユニット";
  Vector2 ts = MeasureTextEx(font_, title, title_size, 2.0f);
  DrawTextEx(font_, title, {panel.x + 16.0f, panel.y + 8.0f}, title_size, 2.0f,
             RAYWHITE);

  if (candidates_.empty()) {
    const char *empty = u8"候補がありません";
    Vector2 es = MeasureTextEx(font_, empty, 22.0f, 2.0f);
    DrawTextEx(font_, empty,
               {panel.x + (panel.width - es.x) * 0.5f,
                panel.y + panel.height * 0.5f - es.y * 0.5f},
               22.0f, 2.0f, GRAY);
    return;
  }

  float start_x = panel.x + 12.0f + candidate_scroll_;
  float start_y = panel.y + 44.0f;
  int per_row = std::max(1, candidates_per_row);

  BeginScissorMode(static_cast<int>(panel.x), static_cast<int>(panel.y),
                   static_cast<int>(panel.width),
                   static_cast<int>(panel.height));

  for (int i = 0; i < static_cast<int>(candidates_.size()); ++i) {
    int col = i % per_row;
    Rectangle card{start_x + col * (CAND_CARD_W + CAND_CARD_GAP), start_y,
                   CAND_CARD_W, CAND_CARD_H};

    bool selected = (i == selected_candidate_);

    Color frame =
        selected ? Color{120, 190, 255, 255} : Color{160, 200, 255, 200};
    Color bg = Color{28, 40, 60, 230};
    DrawRectangleRounded(card, 0.12f, 4, bg);
    DrawRectangleLinesEx(card, 3.0f, frame);

    float padding = 8.0f;
    Rectangle img_rect{card.x + padding, card.y + padding,
                       card.width - padding * 2.0f,
                       card.height - padding * 2.0f};
    DrawIcon(img_rect, candidates_[i]);

    // 下部バー
    float bar_h = 26.0f;
    Rectangle bar{card.x + padding, card.y + card.height - padding - bar_h,
                  card.width - padding * 2.0f, bar_h};
    DrawRectangleRounded(bar, 0.12f, 4, Color{0, 0, 0, 140});

    const auto *def = definitions_.GetEntity(candidates_[i]);
    std::string name = candidates_[i];
    if (def && !def->name.empty()) {
      name = def->name;
    }
    Vector2 ns = MeasureTextEx(font_, name.c_str(), 16.0f, 2.0f);
    DrawTextEx(font_, name.c_str(),
               {bar.x + 8.0f, bar.y + (bar.height - ns.y) * 0.5f}, 16.0f, 2.0f,
               RAYWHITE);

    // タグ（左上）
    std::string tag = def && def->is_enemy ? u8"ENEMY" : u8"ALLY";
    Color tag_bg = def && def->is_enemy ? Color{160, 60, 60, 220}
                                        : Color{60, 120, 180, 220};
    Rectangle tag_rect{card.x + 6.0f, card.y + 6.0f, 48.0f, 16.0f};
    DrawRectangleRounded(tag_rect, 0.18f, 4, tag_bg);
    Vector2 tg = MeasureTextEx(font_, tag.c_str(), 12.0f, 2.0f);
    DrawTextEx(font_, tag.c_str(),
               {tag_rect.x + (tag_rect.width - tg.x) * 0.5f,
                tag_rect.y + (tag_rect.height - tg.y) * 0.5f},
               12.0f, 2.0f, RAYWHITE);
  }

  EndScissorMode();
}

void FormationScene::DrawSlots(const Rectangle &panel, int slot_cols) const {
  DrawRectangleRounded(panel, 0.12f, 6, Color{38, 52, 72, 230});
  DrawRectangleLinesEx(panel, 2.0f, Color{140, 190, 240, 230});

  const float title_size = 22.0f;
  const char *title = u8"編成スロット";
  Vector2 ts = MeasureTextEx(font_, title, title_size, 2.0f);
  DrawTextEx(font_, title, {panel.x + 12.0f, panel.y + 6.0f}, title_size, 2.0f,
             RAYWHITE);

  if (slots_.empty()) {
    const char *empty = u8"スロットがありません";
    Vector2 es = MeasureTextEx(font_, empty, 22.0f, 2.0f);
    DrawTextEx(font_, empty,
               {panel.x + (panel.width - es.x) * 0.5f,
                panel.y + panel.height * 0.5f - es.y * 0.5f},
               22.0f, 2.0f, GRAY);
    return;
  }

  const int cols = std::max(1, slot_cols);
  float slots_w = static_cast<float>(cols) * SLOT_W +
                  static_cast<float>(std::max(0, cols - 1)) * SLOT_GAP;
  float start_x = panel.x + std::max(8.0f, (panel.width - slots_w) * 0.5f);
  float start_y = panel.y + SLOT_HEADER_H;

  for (int i = 0; i < static_cast<int>(slots_.size()); ++i) {
    int row = i / cols;
    int col = i % cols;
    Rectangle cell{start_x + col * (SLOT_W + SLOT_GAP),
                   start_y + row * (SLOT_H + SLOT_GAP), SLOT_W, SLOT_H};
    bool selected = (i == selected_slot_);

    bool candidate_match = false;
    if (selected_candidate_ >= 0 &&
        selected_candidate_ < static_cast<int>(candidates_.size())) {
      candidate_match = (slots_[i] == candidates_[selected_candidate_]);
    }

    Color frame = selected ? Color{120, 190, 255, 255}
                           : (candidate_match ? Color{120, 200, 150, 240}
                                              : Color{160, 200, 255, 200});
    Color bg = Color{28, 40, 60, 230};
    DrawRectangleRounded(cell, 0.12f, 4, bg);
    DrawRectangleLinesEx(cell, 3.0f, frame);

    const std::string &id = slots_[i];
    const auto *def = definitions_.GetEntity(id);

    // 画像エリア（ほぼ全面）
    float padding = 8.0f;
    Rectangle img_rect{cell.x + padding, cell.y + padding,
                       cell.width - padding * 2.0f,
                       cell.height - padding * 2.0f};
    DrawIcon(img_rect, id);

    // 下部の半透明帯
    float bar_h = 26.0f;
    Rectangle bar{cell.x + padding, cell.y + cell.height - padding - bar_h,
                  cell.width - padding * 2.0f, bar_h};
    DrawRectangleRounded(bar, 0.12f, 4, Color{0, 0, 0, 140});

    std::string name =
        id.empty() ? u8"未設定" : (def && !def->name.empty() ? def->name : id);
    Vector2 ns = MeasureTextEx(font_, name.c_str(), 16.0f, 2.0f);
    DrawTextEx(font_, name.c_str(),
               {bar.x + 8.0f, bar.y + (bar.height - ns.y) * 0.5f}, 16.0f, 2.0f,
               RAYWHITE);

    // タグ表示（左上小さく）
    std::string tag =
        def && def->is_enemy ? u8"ENEMY" : (id.empty() ? u8"EMPTY" : u8"ALLY");
    Color tag_bg =
        def && def->is_enemy
            ? Color{160, 60, 60, 220}
            : (id.empty() ? Color{90, 90, 90, 180} : Color{60, 120, 180, 220});
    Rectangle tag_rect{cell.x + 6.0f, cell.y + 6.0f, 64.0f, 18.0f};
    DrawRectangleRounded(tag_rect, 0.18f, 4, tag_bg);
    Vector2 tg = MeasureTextEx(font_, tag.c_str(), 12.0f, 2.0f);
    DrawTextEx(font_, tag.c_str(),
               {tag_rect.x + (tag_rect.width - tg.x) * 0.5f,
                tag_rect.y + (tag_rect.height - tg.y) * 0.5f},
               12.0f, 2.0f, RAYWHITE);
  }
}

Rectangle
FormationScene::GetEnhancementArea(const Rectangle &status_panel) const {
  float enhance_h = ENHANCE_PANEL_H;
  return {status_panel.x + 12.0f,
          status_panel.y + status_panel.height - enhance_h - 12.0f,
          status_panel.width - 24.0f, enhance_h};
}

void FormationScene::DrawStatusPanel(const Rectangle &panel,
                                     const Shared::Data::EntityDef *def) {
  DrawRectangleRounded(panel, 0.12f, 6, Color{34, 46, 64, 230});
  DrawRectangleLinesEx(panel, 2.0f, Color{140, 190, 240, 230});

  const float title_size = 24.0f;
  const char *title = u8"強化 / ステータス";
  Vector2 ts = MeasureTextEx(font_, title, title_size, 2.0f);
  DrawTextEx(font_, title, {panel.x + 12.0f, panel.y + 8.0f}, title_size, 2.0f,
             RAYWHITE);

  Rectangle enhance_area = GetEnhancementArea(panel);
  Rectangle stats_area{panel.x + 12.0f, panel.y + 36.0f, panel.width - 24.0f,
                       enhance_area.y - (panel.y + 36.0f) - 8.0f};

  DrawRectangleRounded(stats_area, 0.08f, 4, Color{28, 36, 50, 230});
  DrawRectangleLinesEx(stats_area, 1.5f, Color{120, 170, 220, 220});

  if (!def) {
    const char *no_sel = u8"候補を選択してください";
    Vector2 ns = MeasureTextEx(font_, no_sel, 20.0f, 2.0f);
    DrawTextEx(font_, no_sel,
               {stats_area.x + (stats_area.width - ns.x) * 0.5f,
                stats_area.y + (stats_area.height - ns.y) * 0.5f},
               20.0f, 2.0f, GRAY);
    return;
  }

  std::string header = def->name.empty() ? def->id : def->name;
  Vector2 hs = MeasureTextEx(font_, header.c_str(), 22.0f, 2.0f);
  DrawTextEx(font_, header.c_str(), {stats_area.x + 12.0f, stats_area.y + 8.0f},
             22.0f, 2.0f, RAYWHITE);

  float y = stats_area.y + 40.0f;
  const float line_gap = 24.0f;
  auto draw_stat = [&](const std::string &label, const std::string &value) {
    Vector2 ls = MeasureTextEx(font_, label.c_str(), 18.0f, 2.0f);
    Vector2 vs = MeasureTextEx(font_, value.c_str(), 18.0f, 2.0f);
    DrawTextEx(font_, label.c_str(), {stats_area.x + 12.0f, y}, 18.0f, 2.0f,
               LIGHTGRAY);
    DrawTextEx(font_, value.c_str(),
               {stats_area.x + stats_area.width - vs.x - 12.0f, y}, 18.0f, 2.0f,
               RAYWHITE);
    y += line_gap;
  };

  auto fmt2 = [](float v) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << v;
    return oss.str();
  };

  draw_stat("HP", std::to_string(def->stats.hp));
  draw_stat("ATK", std::to_string(def->stats.attack));
  draw_stat("ATK SPD", fmt2(def->stats.attack_speed));
  draw_stat("RANGE", fmt2(static_cast<float>(def->stats.range)));
  draw_stat("MOVE", fmt2(def->stats.move_speed));
  draw_stat("KNOCKBACK", fmt2(static_cast<float>(def->stats.knockback)));
  draw_stat("COST", std::to_string(def->cost));
  draw_stat("COOLDOWN", fmt2(def->cooldown));

  auto wrap_lines = [&](const std::string &text, float max_width) {
    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string word;
    std::string line;
    while (iss >> word) {
      std::string candidate = line.empty() ? word : line + " " + word;
      Vector2 sz = MeasureTextEx(font_, candidate.c_str(), 18.0f, 2.0f);
      if (sz.x > max_width && !line.empty()) {
        lines.push_back(line);
        line = word;
      } else {
        line = candidate;
      }
    }
    if (!line.empty()) {
      lines.push_back(line);
    }
    if (lines.empty()) {
      lines.push_back("");
    }
    return lines;
  };

  float detail_y = y + 6.0f;
  const float detail_max_h = stats_area.y + stats_area.height - detail_y - 4.0f;
  if (detail_max_h > 40.0f) {
    const char *detail_title = u8"詳細";
    Vector2 ds = MeasureTextEx(font_, detail_title, 18.0f, 2.0f);
    DrawTextEx(font_, detail_title, {stats_area.x + 12.0f, detail_y}, 18.0f,
               2.0f, LIGHTGRAY);
    detail_y += ds.y + 6.0f;

    float text_width = stats_area.width - 24.0f;
    std::vector<std::string> desc_lines = wrap_lines(
        def->description.empty() ? u8"説明がありません" : def->description,
        text_width);

    for (const auto &ln : desc_lines) {
      if (detail_y + 20.0f > stats_area.y + stats_area.height) {
        break;
      }
      DrawTextEx(font_, ln.c_str(), {stats_area.x + 12.0f, detail_y}, 18.0f,
                 2.0f, RAYWHITE);
      detail_y += 22.0f;
    }

    if (!def->tags.empty() &&
        detail_y + 22.0f < stats_area.y + stats_area.height) {
      std::string tags = "Tags: ";
      for (size_t i = 0; i < def->tags.size(); ++i) {
        if (i > 0)
          tags += ", ";
        tags += def->tags[i];
      }
      auto tag_lines = wrap_lines(tags, text_width);
      for (const auto &ln : tag_lines) {
        if (detail_y + 20.0f > stats_area.y + stats_area.height) {
          break;
        }
        DrawTextEx(font_, ln.c_str(), {stats_area.x + 12.0f, detail_y}, 18.0f,
                   2.0f, LIGHTGRAY);
        detail_y += 20.0f;
      }
    }
  }

  // Enhancement (mock)
  DrawRectangleRounded(enhance_area, 0.08f, 4, Color{22, 32, 48, 230});
  DrawRectangleLinesEx(enhance_area, 1.5f, Color{110, 160, 210, 220});

  const char *enh_title = u8"強化プレビュー (仮)";
  Vector2 ets = MeasureTextEx(font_, enh_title, 20.0f, 2.0f);
  DrawTextEx(font_, enh_title, {enhance_area.x + 10.0f, enhance_area.y + 8.0f},
             20.0f, 2.0f, RAYWHITE);

  std::string level = "Lv." + std::to_string(enhancement_.level) + " / " +
                      std::to_string(enhancement_.maxLevel);
  std::string currency = "資源: " + std::to_string(enhancement_.currency);
  std::string cost = "必要: " + std::to_string(enhancement_.upgradeCost);
  DrawTextEx(font_, level.c_str(),
             {enhance_area.x + 10.0f, enhance_area.y + 36.0f}, 18.0f, 2.0f,
             LIGHTGRAY);
  DrawTextEx(font_, currency.c_str(),
             {enhance_area.x + 10.0f, enhance_area.y + 62.0f}, 18.0f, 2.0f,
             LIGHTGRAY);
  DrawTextEx(font_, cost.c_str(),
             {enhance_area.x + 10.0f, enhance_area.y + 88.0f}, 18.0f, 2.0f,
             LIGHTGRAY);

  Rectangle btn{enhance_area.x + enhance_area.width - 180.0f,
                enhance_area.y + enhance_area.height - 46.0f, 160.0f, 36.0f};
  Color btn_bg = (enhancement_.currency >= enhancement_.upgradeCost &&
                  enhancement_.level < enhancement_.maxLevel)
                     ? Color{60, 120, 180, 240}
                     : Color{50, 70, 100, 200};
  DrawRectangleRounded(btn, 0.12f, 4, btn_bg);
  DrawRectangleLinesEx(btn, 1.5f, Color{160, 200, 255, 220});
  const char *btn_label = u8"強化する";
  Vector2 bs = MeasureTextEx(font_, btn_label, 18.0f, 2.0f);
  DrawTextEx(
      font_, btn_label,
      {btn.x + (btn.width - bs.x) * 0.5f, btn.y + (btn.height - bs.y) * 0.5f},
      18.0f, 2.0f, RAYWHITE);

  Vector2 ms = MeasureTextEx(font_, enhancement_.message.c_str(), 16.0f, 2.0f);
  DrawTextEx(
      font_, enhancement_.message.c_str(),
      {enhance_area.x + 10.0f, enhance_area.y + enhance_area.height - 24.0f},
      16.0f, 2.0f, LIGHTGRAY);
}

void FormationScene::DrawCharacterPreview(const Rectangle &panel,
                                          const Shared::Data::EntityDef *def) {
  DrawRectangleRounded(panel, 0.12f, 6, Color{30, 40, 58, 230});
  DrawRectangleLinesEx(panel, 2.0f, Color{140, 190, 240, 230});

  const float title_size = 24.0f;
  const char *title = u8"キャラクタプレビュー";
  Vector2 ts = MeasureTextEx(font_, title, title_size, 2.0f);
  DrawTextEx(font_, title, {panel.x + 12.0f, panel.y + 8.0f}, title_size, 2.0f,
             RAYWHITE);

  // ドロップダウンエリア（タイトルの下、アニメーションエリアの上）
  const float title_height = ts.y + 16.0f; // タイトル高さ + 余白
  const float dropdown_y = panel.y + title_height;
  const float dropdown_h = 32.0f;
  const float dropdown_w = 200.0f;
  // 右側に配置（余白20px）
  const float dropdown_x = panel.x + panel.width - dropdown_w - 20.0f;
  Rectangle dropdown_area{dropdown_x, dropdown_y, dropdown_w, dropdown_h};
  
  // ドロップダウンがパネル内に収まるか確認
  if (dropdown_area.x < panel.x + 10.0f) {
    dropdown_area.x = panel.x + 10.0f;
    dropdown_area.width = panel.width - 20.0f; // 幅を調整
  }
  if (dropdown_area.x + dropdown_area.width > panel.x + panel.width - 10.0f) {
    dropdown_area.width = panel.x + panel.width - dropdown_area.x - 10.0f;
  }
  // 高さが足りない場合は上に移動
  if (dropdown_area.y + dropdown_area.height > panel.y + panel.height - 50.0f) {
    dropdown_area.y = panel.y + panel.height - dropdown_area.height - 50.0f;
  }
  
  // アニメーションエリア（ドロップダウンの下）
  const float anim_area_y = dropdown_y + dropdown_h + 8.0f;
  const float bottom_margin = 40.0f; // 下部のキャラクター名表示用の余白
  const float anim_area_height = (panel.y + panel.height) - anim_area_y - bottom_margin;
  Rectangle anim_area{panel.x + 20.0f, anim_area_y, panel.width - 40.0f,
                      std::max(0.0f, anim_area_height)};

  // アニメーションエリアもクリッピング内で描画
  BeginScissorMode(static_cast<int>(panel.x), static_cast<int>(panel.y),
                   static_cast<int>(panel.width), static_cast<int>(panel.height));
  
  const auto *selDef = GetSelectedDef();
  if (selDef) {
    // 選択されたユニットがいる場合
    std::string entity_id = selDef->id;
    auto *provider = const_cast<Game::Graphics::AsepriteJsonAtlasProvider*>(GetProvider(entity_id));
    
    // 可能な限りアニメーションを試みる
    if (provider) {
      std::string clipName = preview_.playingAttack ? "attack" : preview_.selectedAnimation;
      // クリップが存在しない場合はフォールバック
      if (!provider->HasClip(clipName)) {
        if (provider->HasClip("walk")) clipName = "walk";
        else if (provider->HasClip("idle")) clipName = "idle";
        else if (provider->HasClip("attack")) clipName = "attack";
        else if (provider->HasClip("death")) clipName = "death";
        else clipName = "";
      }
      if (!clipName.empty()) {
        DrawSpriteAnim(anim_area, *provider, clipName, preview_.animTimer, preview_.currentFrame);
      } else {
        // クリップが見つからない場合はアイコンを表示
        DrawIcon(anim_area, entity_id);
      }
    } else {
      // Provider取得失敗時はアイコンを表示
      DrawIcon(anim_area, entity_id);
    }
  } else {
    // 選択されたユニットがいない場合のプレースホルダー
    float t = fmodf(preview_.animTimer, 1.0f);
    float alpha = 0.6f + 0.4f * sinf(t * 6.28f);
    DrawRectangleRounded(anim_area, 0.1f, 6,
        Color{60, 100, 200, static_cast<unsigned char>(alpha * 255)});
    DrawRectangleLinesEx(anim_area, 2.0f, Color{120, 170, 240, 230});
    
    // プレースホルダーメッセージ
    const char *placeholderMsg = u8"キャラクターを選択してください";
    Vector2 ps = MeasureTextEx(font_, placeholderMsg, 18.0f, 2.0f);
    DrawTextEx(font_, placeholderMsg,
               {anim_area.x + (anim_area.width - ps.x) * 0.5f,
                anim_area.y + (anim_area.height - ps.y) * 0.5f},
               18.0f, 2.0f, Color{200, 200, 200, static_cast<unsigned char>(alpha * 255)});
  }
  EndScissorMode();
  
  // ドロップダウンを最後に描画して前面に表示（リストはパネルの外に出る可能性があるため、ScissorModeは使わない）
  DrawAnimationDropdown(dropdown_area);

  if (def) {
    std::string header = def->name.empty() ? def->id : def->name;
    Vector2 hs = MeasureTextEx(font_, header.c_str(), 20.0f, 2.0f);
    DrawTextEx(font_, header.c_str(),
               {panel.x + (panel.width - hs.x) * 0.5f,
                panel.y + panel.height - hs.y - 10.0f},
               20.0f, 2.0f, RAYWHITE);
  }
  
  // デバッグ情報表示
  if (preview_.showDebug && HasPreviewAnimation()) {
    const auto *selDef = GetSelectedDef();
    std::string entity_id = selDef ? selDef->id : "";
    auto *provider = const_cast<Game::Graphics::AsepriteJsonAtlasProvider*>(GetProvider(entity_id));
    if (provider) {
      std::string clipName = preview_.playingAttack ? "attack" : preview_.selectedAnimation;
      if (provider->HasClip(clipName)) {
        int frameCount = provider->GetFrameCount(clipName);
        float fps = provider->GetClipFps(clipName);
        if (fps <= 0.0f) fps = 12.0f;
        bool isLooping = provider->IsLooping(clipName);
        
        // デバッグ情報エリア（左上）
        const float debug_x = panel.x + 12.0f;
        const float debug_y = panel.y + title_height + 4.0f;
        const float debug_font_size = 14.0f;
        const float debug_line_height = 18.0f;
        float debug_current_y = debug_y;
        
        // 背景（半透明）
        Rectangle debug_bg{debug_x - 4.0f, debug_y - 4.0f, 200.0f, 120.0f};
        DrawRectangleRounded(debug_bg, 0.08f, 4, Color{0, 0, 0, 180});
        DrawRectangleLinesEx(debug_bg, 1.0f, Color{100, 150, 200, 200});
        
        // フレーム情報
        std::string frameInfo = "Frame: " + std::to_string(preview_.currentFrame + 1) + " / " + std::to_string(frameCount);
        DrawTextEx(font_, frameInfo.c_str(), {debug_x, debug_current_y}, debug_font_size, 1.0f, Color{200, 255, 200, 255});
        debug_current_y += debug_line_height;
        
        // FPS情報
        std::ostringstream fpsStream;
        fpsStream << std::fixed << std::setprecision(1) << fps;
        std::string fpsInfo = "FPS: " + fpsStream.str();
        DrawTextEx(font_, fpsInfo.c_str(), {debug_x, debug_current_y}, debug_font_size, 1.0f, Color{200, 200, 255, 255});
        debug_current_y += debug_line_height;
        
        // クリップ名
        std::string clipInfo = "Clip: " + clipName;
        DrawTextEx(font_, clipInfo.c_str(), {debug_x, debug_current_y}, debug_font_size, 1.0f, Color{255, 200, 200, 255});
        debug_current_y += debug_line_height;
        
        // ミラーリング状態
        std::string mirrorInfo = "Mirror: " + std::string(preview_.mirrorH ? "H" : "") + 
                                 (preview_.mirrorV ? "V" : "") + 
                                 (!preview_.mirrorH && !preview_.mirrorV ? "None" : "");
        DrawTextEx(font_, mirrorInfo.c_str(), {debug_x, debug_current_y}, debug_font_size, 1.0f, Color{255, 255, 200, 255});
        debug_current_y += debug_line_height;
        
        // ループ状態
        std::string loopInfo = "Loop: " + std::string(isLooping ? "Yes" : "No");
        DrawTextEx(font_, loopInfo.c_str(), {debug_x, debug_current_y}, debug_font_size, 1.0f, Color{200, 255, 255, 255});
        debug_current_y += debug_line_height;
        
        // 経過時間
        std::ostringstream timeStream;
        timeStream << std::fixed << std::setprecision(2) << preview_.animTimer;
        std::string timeInfo = "Time: " + timeStream.str() + "s";
        DrawTextEx(font_, timeInfo.c_str(), {debug_x, debug_current_y}, debug_font_size, 1.0f, Color{255, 255, 255, 255});
      }
    }
  }
}

void FormationScene::TryMockUpgrade() {
  if (enhancement_.level >= enhancement_.maxLevel) {
    enhancement_.message = "最大レベルに到達しています";
    return;
  }
  if (enhancement_.currency < enhancement_.upgradeCost) {
    enhancement_.message = "資源が不足しています";
    return;
  }

  enhancement_.currency -= enhancement_.upgradeCost;
  enhancement_.level += 1;
  enhancement_.upgradeCost += 80;
  enhancement_.message = "強化しました (仮動作)";
}

void FormationScene::Draw() {
  ClearBackground(Color{16, 20, 28, 255});

  LayoutInfo layout = BuildLayout();

  const float title_size = 40.0f;
  const char *title = u8"編成";
  Vector2 ts = MeasureTextEx(font_, title, title_size, 2.0f);
  DrawTextEx(font_, title,
             {(static_cast<float>(screen_width_) - ts.x) * 0.5f, TITLE_Y},
             title_size, 2.0f, RAYWHITE);

  Rectangle back_btn{24.0f, 20.0f, 180.0f, 44.0f};
  float slot_btn_y = layout.slotPanel.y + 8.0f;
  float slot_btn_h = 30.0f;
  float margin = 10.0f;
  float remove_w = 170.0f;
  float reset_w = 150.0f;
  float remove_x =
      layout.slotPanel.x + layout.slotPanel.width - margin - remove_w;
  float reset_x = remove_x - 8.0f - reset_w;
  Rectangle reset_btn{reset_x, slot_btn_y, reset_w, slot_btn_h};
  Rectangle remove_btn{remove_x, slot_btn_y, remove_w, slot_btn_h};

  DrawRectangleRounded(back_btn, 0.14f, 6, Color{60, 100, 150, 230});
  DrawRectangleLinesEx(back_btn, 2.0f, Color{180, 210, 255, 230});
  Vector2 bb = MeasureTextEx(font_, u8"確定して戻る", 22.0f, 2.0f);
  DrawTextEx(font_, u8"確定して戻る",
             {back_btn.x + (back_btn.width - bb.x) * 0.5f,
              back_btn.y + (back_btn.height - bb.y) * 0.5f},
             22.0f, 2.0f, RAYWHITE);

  DrawCandidates(layout.candidatePanel, layout.candidatesPerRow,
                 layout.candidateRows);
  DrawSlots(layout.slotPanel, layout.slotCols);
  // スロット上に重ねて描画
  DrawRectangleRounded(reset_btn, 0.14f, 6, Color{90, 70, 140, 230});
  DrawRectangleLinesEx(reset_btn, 2.0f, Color{180, 210, 255, 230});
  Vector2 rb = MeasureTextEx(font_, u8"編成リセット", 20.0f, 2.0f);
  DrawTextEx(font_, u8"編成リセット",
             {reset_btn.x + (reset_btn.width - rb.x) * 0.5f,
              reset_btn.y + (reset_btn.height - rb.y) * 0.5f},
             20.0f, 2.0f, RAYWHITE);

  DrawRectangleRounded(remove_btn, 0.14f, 6, Color{100, 80, 90, 230});
  DrawRectangleLinesEx(remove_btn, 2.0f, Color{180, 210, 255, 230});
  Vector2 rem = MeasureTextEx(font_, u8"選択ユニットを外す", 18.0f, 2.0f);
  DrawTextEx(font_, u8"選択ユニットを外す",
             {remove_btn.x + (remove_btn.width - rem.x) * 0.5f,
              remove_btn.y + (remove_btn.height - rem.y) * 0.5f},
             18.0f, 2.0f, RAYWHITE);
  DrawStatusPanel(layout.statusPanel, GetSelectedDef());
  DrawCharacterPreview(layout.previewPanel, GetSelectedDef());

  if (drag_.active && drag_.candidateIndex >= 0 &&
      drag_.candidateIndex < static_cast<int>(candidates_.size())) {
    Rectangle ghost{drag_.currentPos.x - ICON_SIZE * 0.5f,
                    drag_.currentPos.y - ICON_SIZE * 0.5f, ICON_SIZE,
                    ICON_SIZE};
    DrawIcon(ghost, candidates_[drag_.candidateIndex]);
    DrawRectangleLinesEx(ghost, 2.0f, Color{255, 255, 255, 180});
  }

  const char *helper =
      u8"[Enter] 決定してホームへ  [Space] 選択をセット  [Del] クリア  "
      u8"[←→] スロット選択  [↑↓] 候補選択  [右クリック] クリア  "
      u8"[プレビューをクリック] 攻撃表示 / 強化";
  Vector2 hs = MeasureTextEx(font_, helper, 18.0f, 2.0f);
  DrawTextEx(font_, helper,
             {(static_cast<float>(screen_width_) - hs.x) * 0.5f,
              static_cast<float>(screen_height_) - 60.0f},
             18.0f, 2.0f, LIGHTGRAY);
}

} // namespace Game::Scenes
