#include "Game/Scenes/FormationScene.h"

#include <algorithm>

namespace {
constexpr float TITLE_Y = 90.0f;
constexpr float PANEL_MARGIN = 60.0f;
constexpr float LIST_ITEM_H = 44.0f;
constexpr float LIST_ITEM_GAP = 8.0f;
constexpr float SLOT_W = 210.0f;
constexpr float SLOT_H = 84.0f;
constexpr float SLOT_GAP = 14.0f;
constexpr int SLOT_COLS = 4;
} // namespace

namespace Game::Scenes {

FormationScene::FormationScene(Font font, int screen_width, int screen_height,
                               Shared::Data::DefinitionRegistry &definitions,
                               Game::Managers::FormationManager &formation_manager)
    : font_(font), screen_width_(screen_width), screen_height_(screen_height),
      definitions_(definitions), formation_manager_(formation_manager) {
  RefreshData();
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

  if (candidates_.empty()) {
    selected_candidate_ = -1;
  } else {
    selected_candidate_ =
        std::clamp(selected_candidate_, 0, static_cast<int>(candidates_.size() - 1));
  }

  if (slots_.empty()) {
    selected_slot_ = -1;
  } else {
    selected_slot_ =
        std::clamp(selected_slot_, 0, static_cast<int>(slots_.size() - 1));
  }
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
  if (IsKeyPressed(KEY_ESCAPE)) {
    return_home_requested_ = true;
    return;
  }

  if (IsKeyPressed(KEY_ENTER)) {
    apply_requested_ = true;
    return;
  }

  const int cand_count = static_cast<int>(candidates_.size());
  const int slot_count = static_cast<int>(slots_.size());

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
    }
    if (IsKeyPressed(KEY_LEFT)) {
      selected_slot_ = (selected_slot_ - 1 + slot_count) % slot_count;
    }
  }

  if (IsKeyPressed(KEY_SPACE)) {
    AssignSelectedToSlot(selected_slot_);
  }

  if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
    formation_manager_.ClearSlot(selected_slot_);
    RefreshData();
  }

  // Mouse interactions
  Vector2 mouse = GetMousePosition();
  bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  bool right_click = IsMouseButtonPressed(MOUSE_RIGHT_BUTTON);

  // Candidate list rect
  Rectangle cand_panel{PANEL_MARGIN, TITLE_Y + 60.0f,
                       static_cast<float>(screen_width_) * 0.5f - PANEL_MARGIN * 1.5f,
                       static_cast<float>(screen_height_) - (TITLE_Y + 120.0f)};
  float y = cand_panel.y + 16.0f;
  for (int i = 0; i < cand_count; ++i) {
    Rectangle r{cand_panel.x + 14.0f, y, cand_panel.width - 28.0f, LIST_ITEM_H};
    if (CheckCollisionPointRec(mouse, r)) {
      if (click) {
        selected_candidate_ = i;
      }
    }
    y += LIST_ITEM_H + LIST_ITEM_GAP;
  }

  // Slots rect
  float slot_panel_w = static_cast<float>(SLOT_COLS) * SLOT_W +
                       static_cast<float>(SLOT_COLS - 1) * SLOT_GAP;
  Rectangle slot_panel{
      static_cast<float>(screen_width_) - PANEL_MARGIN - slot_panel_w,
      TITLE_Y + 60.0f, slot_panel_w,
      static_cast<float>(screen_height_) - (TITLE_Y + 120.0f)};

  float start_x = slot_panel.x;
  float start_y = slot_panel.y + 24.0f;
  const int cols = SLOT_COLS;
  for (int i = 0; i < slot_count; ++i) {
    int row = i / cols;
    int col = i % cols;
    Rectangle r{start_x + col * (SLOT_W + SLOT_GAP),
                start_y + row * (SLOT_H + SLOT_GAP), SLOT_W, SLOT_H};
    if (CheckCollisionPointRec(mouse, r)) {
      if (click) {
        selected_slot_ = i;
        AssignSelectedToSlot(i);
      } else if (right_click) {
        selected_slot_ = i;
        formation_manager_.ClearSlot(i);
        RefreshData();
      }
    }
  }
}

void FormationScene::Update(float) { HandleInput(); }

void FormationScene::DrawCandidates(const Rectangle &panel) const {
  DrawRectangleRounded(panel, 0.12f, 6, Color{38, 52, 72, 230});
  DrawRectangleLinesEx(panel, 2.0f, Color{140, 190, 240, 230});

  const float title_size = 28.0f;
  const char *title = u8"候補ユニット";
  Vector2 ts = MeasureTextEx(font_, title, title_size, 2.0f);
  DrawTextEx(font_, title,
             {panel.x + 16.0f, panel.y + 8.0f}, title_size, 2.0f, RAYWHITE);

  if (candidates_.empty()) {
    const char *empty = u8"候補がありません";
    Vector2 es = MeasureTextEx(font_, empty, 22.0f, 2.0f);
    DrawTextEx(font_, empty,
               {panel.x + (panel.width - es.x) * 0.5f,
                panel.y + panel.height * 0.5f - es.y * 0.5f},
               22.0f, 2.0f, GRAY);
    return;
  }

  float y = panel.y + 44.0f;
  for (int i = 0; i < static_cast<int>(candidates_.size()); ++i) {
    bool selected = (i == selected_candidate_);
    Rectangle row{panel.x + 12.0f, y, panel.width - 24.0f, LIST_ITEM_H};
    Color bg = selected ? Color{70, 110, 170, 240} : Color{48, 68, 96, 220};
    DrawRectangleRounded(row, 0.14f, 4, bg);
    DrawRectangleLinesEx(row, 2.0f, Color{160, 200, 255, 230});

    const auto *def = definitions_.GetEntity(candidates_[i]);
    std::string label = candidates_[i];
    if (def && !def->name.empty()) {
      label += " - " + def->name;
    }
    Vector2 ls = MeasureTextEx(font_, label.c_str(), 20.0f, 2.0f);
    DrawTextEx(font_, label.c_str(),
               {row.x + 12.0f, row.y + (row.height - ls.y) * 0.5f}, 20.0f, 2.0f,
               RAYWHITE);
    y += LIST_ITEM_H + LIST_ITEM_GAP;
  }
}

void FormationScene::DrawSlots(const Rectangle &panel) const {
  DrawRectangleRounded(panel, 0.12f, 6, Color{38, 52, 72, 230});
  DrawRectangleLinesEx(panel, 2.0f, Color{140, 190, 240, 230});

  const float title_size = 28.0f;
  const char *title = u8"編成スロット";
  Vector2 ts = MeasureTextEx(font_, title, title_size, 2.0f);
  DrawTextEx(font_, title,
             {panel.x + 16.0f, panel.y + 8.0f}, title_size, 2.0f, RAYWHITE);

  if (slots_.empty()) {
    const char *empty = u8"スロットがありません";
    Vector2 es = MeasureTextEx(font_, empty, 22.0f, 2.0f);
    DrawTextEx(font_, empty,
               {panel.x + (panel.width - es.x) * 0.5f,
                panel.y + panel.height * 0.5f - es.y * 0.5f},
               22.0f, 2.0f, GRAY);
    return;
  }

  const int cols = SLOT_COLS;
  float start_x = panel.x + 8.0f;
  float start_y = panel.y + 44.0f;

  for (int i = 0; i < static_cast<int>(slots_.size()); ++i) {
    int row = i / cols;
    int col = i % cols;
    Rectangle cell{start_x + col * (SLOT_W + SLOT_GAP),
                   start_y + row * (SLOT_H + SLOT_GAP), SLOT_W, SLOT_H};
    bool selected = (i == selected_slot_);

    Color bg = selected ? Color{70, 120, 180, 240} : Color{48, 68, 96, 220};
    DrawRectangleRounded(cell, 0.14f, 4, bg);
    DrawRectangleLinesEx(cell, 2.0f, Color{160, 200, 255, 230});

    std::string header = "Slot " + std::to_string(i + 1);
    Vector2 hs = MeasureTextEx(font_, header.c_str(), 18.0f, 2.0f);
    DrawTextEx(font_, header.c_str(), {cell.x + 10.0f, cell.y + 6.0f}, 18.0f,
               2.0f, LIGHTGRAY);

    const std::string &id = slots_[i];
    std::string label = id.empty() ? u8"未設定" : id;
    Color text_color = id.empty() ? GRAY : RAYWHITE;
    const auto *def = definitions_.GetEntity(id);
    if (def && !def->name.empty()) {
      label += " - " + def->name;
    }

    Vector2 ls = MeasureTextEx(font_, label.c_str(), 20.0f, 2.0f);
    DrawTextEx(font_, label.c_str(),
               {cell.x + 10.0f, cell.y + (cell.height - ls.y) * 0.5f}, 20.0f,
               2.0f, text_color);
  }
}

void FormationScene::Draw() {
  ClearBackground(Color{16, 20, 28, 255});

  const float title_size = 40.0f;
  const char *title = u8"編成";
  Vector2 ts = MeasureTextEx(font_, title, title_size, 2.0f);
  DrawTextEx(font_, title,
             {(static_cast<float>(screen_width_) - ts.x) * 0.5f, TITLE_Y},
             title_size, 2.0f, RAYWHITE);

  Rectangle cand_panel{
      PANEL_MARGIN, TITLE_Y + 60.0f,
      static_cast<float>(screen_width_) * 0.5f - PANEL_MARGIN * 1.5f,
      static_cast<float>(screen_height_) - (TITLE_Y + 120.0f)};

  float slot_panel_w = static_cast<float>(SLOT_COLS) * SLOT_W +
                       static_cast<float>(SLOT_COLS - 1) * SLOT_GAP;
  Rectangle slot_panel{
      static_cast<float>(screen_width_) - PANEL_MARGIN - slot_panel_w,
      TITLE_Y + 60.0f, slot_panel_w,
      static_cast<float>(screen_height_) - (TITLE_Y + 120.0f)};

  DrawCandidates(cand_panel);
  DrawSlots(slot_panel);

  const char *helper =
      u8"[Enter] 決定してホームへ  [Space] 選択をセット  [Del] クリア  "
      u8"[←→] スロット選択  [↑↓] 候補選択  [右クリック] クリア";
  Vector2 hs = MeasureTextEx(font_, helper, 18.0f, 2.0f);
  DrawTextEx(font_, helper,
             {(static_cast<float>(screen_width_) - hs.x) * 0.5f,
              static_cast<float>(screen_height_) - 60.0f},
             18.0f, 2.0f, LIGHTGRAY);
}

} // namespace Game::Scenes
