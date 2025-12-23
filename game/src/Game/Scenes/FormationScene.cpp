#include "Game/Scenes/FormationScene.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unordered_map>

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
  if (!def || def->display.atlas_texture.empty()) {
    return nullptr;
  }

  std::string pngPath = def->display.atlas_texture;
  if (!std::filesystem::exists(pngPath)) {
    return nullptr;
  }

  Texture2D tex = LoadTexture(pngPath.c_str());
  if (tex.id == 0) {
    return nullptr;
  }

  // sprite_actions から最初のJSONをロード
  nlohmann::json atlasJson;
  bool loaded = false;
  for (const auto &[action, jsonPath] : def->display.sprite_actions) {
    std::filesystem::path fullJsonPath = std::filesystem::path(pngPath).parent_path() / jsonPath;
    if (std::filesystem::exists(fullJsonPath)) {
      std::ifstream file(fullJsonPath.string());
      if (file) {
        file >> atlasJson;
        loaded = true;
        break;
      }
    }
  }
  if (!loaded) {
    return nullptr;
  }

  auto provider = std::make_unique<Game::Graphics::AsepriteJsonAtlasProvider>(tex, atlasJson);

  auto [inserted, _] = provider_cache_.emplace(entity_id, std::move(provider));
  return inserted->second.get();
}

void FormationScene::DrawIcon(const Rectangle &rect,
                              const std::string &entity_id) const {
  const auto *def = definitions_.GetEntity(entity_id);

  auto resolveIconPath = [](const Shared::Data::EntityDef* d) -> std::string {
    if (!d) return {};
    namespace fs = std::filesystem;

    auto exists_path = [](const fs::path& p) { return !p.empty() && fs::exists(p); };

    if (exists_path(d->display.icon)) {
      return fs::path(d->display.icon).lexically_normal().generic_string();
    }

    fs::path hint = d->display.icon.empty() ? fs::path(d->display.atlas_texture) : fs::path(d->display.icon);
    if (!hint.empty()) {
      fs::path folder = hint.parent_path().filename();
      std::string tier = d->type.empty() ? "main" : d->type; // main / sub
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
      DrawTexturePro(it->second, 
                     {0.0f, 0.0f, (float)it->second.width, (float)it->second.height}, 
                     rect, 
                     {0.0f, 0.0f}, 
                     0.0f, 
                     WHITE);
      return;
    }
  }
  
  // Fallback: draw sprite
  auto *provider = const_cast<Game::Graphics::AsepriteJsonAtlasProvider*>(GetProvider(entity_id));
  if (provider) {
    sprite_renderer_.DrawSprite(
        *provider, "idle", 0.0f,
        Vector2{rect.x + rect.width / 2.0f, rect.y + rect.height}, // 足元基準
        Vector2{rect.width / 64.0f, rect.height / 64.0f}, // スケール (64x64想定)
        0.0f, WHITE);
  } else {
    DrawRectangleRec(rect, Color{60, 100, 200, 255});
    DrawRectangleLinesEx(rect, 2.0f, Color{120, 170, 240, 255});
  }
}

void FormationScene::DrawSpriteAnim(const Rectangle &area,
                                    Game::Graphics::AsepriteJsonAtlasProvider &provider,
                                    const std::string &clipName, float elapsed,
                                    int current_frame) const {
  sprite_renderer_.DrawSprite(
      provider, clipName, current_frame,
      Vector2{area.x + area.width / 2.0f, area.y + area.height}, // 足元基準
      Vector2{area.width / 64.0f, area.height / 64.0f}, // スケール (64x64想定)
      0.0f, WHITE);
}

void FormationScene::UpdatePreview(float delta_time) {
  if (!HasPreviewAnimation()) {
    preview_.animTimer = 0.0f;
    preview_.currentFrame = 0;
    preview_.playingAttack = false;
    return;
  }

  const auto *def = GetSelectedDef();
  std::string entity_id = def ? def->id : "";
  auto *provider = const_cast<Game::Graphics::AsepriteJsonAtlasProvider*>(GetProvider(entity_id));
  if (!provider) {
    preview_.animTimer = 0.0f;
    preview_.currentFrame = 0;
    return;
  }

  const std::string &clipName = preview_.playingAttack ? "attack" : "walk";
  if (!provider->HasClip(clipName)) {
    return;
  }

  float fps = provider->GetClipFps(clipName);
  float frameDuration = 1.0f / fps;
  int frameCount = provider->GetFrameCount(clipName);

  preview_.animTimer += delta_time;
  while (preview_.animTimer >= frameDuration) {
    preview_.animTimer -= frameDuration;
    preview_.currentFrame = (preview_.currentFrame + 1) % frameCount;
  }

  if (preview_.playingAttack) {
    // revert to walk after short burst
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

  const auto *def = GetSelectedDef();
  preview_.animTimer = 0.0f;
  preview_.currentFrame = 0;
  preview_.playingAttack = false;
}

FormationScene::LayoutInfo FormationScene::BuildLayout() const {
  LayoutInfo layout{};

  const int cand_count = static_cast<int>(candidates_.size());
  const int slot_count = static_cast<int>(slots_.size());
  const float cand_panel_width =
      static_cast<float>(screen_width_) - PANEL_MARGIN * 2.0f;
  int per_row = cand_count; // 全件を横に並べる
  int rows = 1;
  int visible_rows = 1; // 1行表示

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

  float preview_h = PREVIEW_PANEL_BASE_H;
  int slot_cols = 5; // 固定で 2 x 5
  int slot_rows = slot_count > 0 ? (slot_count + slot_cols - 1) / slot_cols : 1;
  slot_rows = std::min(slot_rows, 2);
  float slot_panel_h =
      SLOT_HEADER_H + static_cast<float>(slot_rows) * SLOT_H +
      static_cast<float>(std::max(0, slot_rows - 1)) * SLOT_GAP + 8.0f;

  float preview_and_slots_h = preview_h + 12.0f + slot_panel_h;
  if (preview_and_slots_h > available_h) {
    float diff = preview_and_slots_h - available_h;
    preview_h = std::max(PREVIEW_PANEL_MIN_H, preview_h - diff);
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
  bool right_click = IsMouseButtonPressed(MOUSE_RIGHT_BUTTON);

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

  // Preview click triggers attack animation (only if animation exists)
  if (click && CheckCollisionPointRec(mouse, layout.previewPanel) &&
      HasPreviewAnimation()) {
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
  draw_stat("RANGE", fmt2(def->stats.range));
  draw_stat("MOVE", fmt2(def->stats.move_speed));
  draw_stat("KNOCKBACK", fmt2(def->stats.knockback));
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

  Rectangle anim_area{panel.x + 20.0f, panel.y + 44.0f, panel.width - 40.0f,
                      panel.height - 64.0f};

  if (HasPreviewAnimation()) {
    const auto *selDef = GetSelectedDef();
    std::string entity_id = selDef ? selDef->id : "";
    auto *provider = const_cast<Game::Graphics::AsepriteJsonAtlasProvider*>(GetProvider(entity_id));
    if (provider) {
      const std::string &clipName = preview_.playingAttack ? "attack" : "walk";
      DrawSpriteAnim(anim_area, *provider, clipName, preview_.animTimer, preview_.currentFrame);
    }
  } else {
    // Placeholder
    float t = fmodf(preview_.animTimer, 1.0f);
    float alpha = 0.6f + 0.4f * sinf(t * 6.28f);
    DrawRectangleRounded(anim_area, 0.1f, 6,
        Color{60, 100, 200, static_cast<unsigned char>(alpha * 255)});
    DrawRectangleLinesEx(anim_area, 2.0f, Color{120, 170, 240, 230});
  }

  if (def) {
    std::string header = def->name.empty() ? def->id : def->name;
    Vector2 hs = MeasureTextEx(font_, header.c_str(), 20.0f, 2.0f);
    DrawTextEx(font_, header.c_str(),
               {panel.x + (panel.width - hs.x) * 0.5f,
                panel.y + panel.height - hs.y - 10.0f},
               20.0f, 2.0f, RAYWHITE);
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
