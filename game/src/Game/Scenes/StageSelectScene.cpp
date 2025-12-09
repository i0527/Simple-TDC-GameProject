#include "Game/Scenes/StageSelectScene.h"

#include <algorithm>

namespace {
constexpr float ITEM_HEIGHT = 60.0f;
constexpr float ITEM_GAP = 10.0f;
constexpr float LIST_MARGIN_X = 80.0f;
constexpr float LIST_MARGIN_Y = 120.0f;
} // namespace

namespace Game::Scenes {

StageSelectScene::StageSelectScene(
    Shared::Data::DefinitionRegistry &definitions, const Font &font)
    : definitions_(definitions), font_(font) {
  BuildStageList();
}

void StageSelectScene::BuildStageList() {
  stages_.clear();
  auto all = definitions_.GetAllStages();
  stages_.reserve(all.size());
  for (auto s : all) {
    StageEntry e;
    e.id = s->id;
    e.domain = s->domain;
    e.subdomain = s->subdomain;
    e.name = s->name.empty() ? s->id : s->name;
    stages_.push_back(std::move(e));
  }
  std::sort(stages_.begin(), stages_.end(),
            [](const StageEntry &a, const StageEntry &b) {
              if (a.domain == b.domain) {
                return a.subdomain < b.subdomain;
              }
              return a.domain < b.domain;
            });
  if (stages_.empty()) {
    selected_index_ = -1;
  } else {
    selected_index_ = (selected_index_ >= 0 &&
                       selected_index_ < static_cast<int>(stages_.size()))
                          ? selected_index_
                          : 0;
  }
}

void StageSelectScene::HandleInput() {
  // 戻る操作（Esc / マウスサイド / ゲームパッドB）
  bool back_key = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE);
  bool back_mouse = IsMouseButtonPressed(MOUSE_BUTTON_SIDE);
  bool back_pad =
      IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT) || // B
      IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);      // Start/Back

  if (back_key || back_mouse || back_pad) {
    quit_requested_ = true;
    start_requested_ = false;
    selected_stage_id_.clear();
    return;
  }
  if (stages_.empty())
    return;

  int count = static_cast<int>(stages_.size());
  bool has_selection = (selected_index_ >= 0 && selected_index_ < count);
  if (IsKeyPressed(KEY_DOWN)) {
    selected_index_ = (selected_index_ + 1 + count) % count;
  }
  if (IsKeyPressed(KEY_UP)) {
    selected_index_ = (selected_index_ - 1 + count) % count;
  }
  if (has_selection && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
    selected_stage_id_ = stages_[selected_index_].id;
    start_requested_ = true;
    return;
  }

  // Mouse selection
  Vector2 mouse = GetMousePosition();
  bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  float y = LIST_MARGIN_Y;
  for (int i = 0; i < count; ++i) {
    Rectangle rect{LIST_MARGIN_X, y, GetScreenWidth() - LIST_MARGIN_X * 2.0f,
                   ITEM_HEIGHT};
    if (CheckCollisionPointRec(mouse, rect)) {
      selected_index_ = i;
      if (click && has_selection) {
        selected_stage_id_ = stages_[i].id;
        start_requested_ = true;
        return;
      }
    }
    y += ITEM_HEIGHT + ITEM_GAP;
  }
}

void StageSelectScene::Update(float) { HandleInput(); }

void StageSelectScene::Draw() {
  ClearBackground(Color{18, 24, 32, 255});

  const float title_size = 40.0f;
  const char *title = "Stage Select";
  Vector2 ts = MeasureTextEx(font_, title, title_size, 2.0f);
  DrawTextEx(font_, title,
             {(GetScreenWidth() - ts.x) * 0.5f, LIST_MARGIN_Y - 80.0f},
             title_size, 2.0f, RAYWHITE);

  if (stages_.empty()) {
    const char *msg = "No stages found";
    Vector2 ms = MeasureTextEx(font_, msg, 24.0f, 2.0f);
    DrawTextEx(font_, msg, {(GetScreenWidth() - ms.x) * 0.5f, LIST_MARGIN_Y},
               24.0f, 2.0f, GRAY);
    return;
  }

  float y = LIST_MARGIN_Y;
  for (int i = 0; i < static_cast<int>(stages_.size()); ++i) {
    const auto &s = stages_[i];
    bool selected = (i == selected_index_);
    Rectangle rect{LIST_MARGIN_X, y, GetScreenWidth() - LIST_MARGIN_X * 2.0f,
                   ITEM_HEIGHT};
    Color fill = selected ? Color{60, 100, 160, 255} : Color{40, 60, 90, 200};
    DrawRectangleRounded(rect, 0.12f, 6, fill);
    DrawRectangleLinesEx(rect, 2.0f, Color{160, 200, 255, 255});

    std::string line =
        s.domain + " - " + std::to_string(s.subdomain) + " : " + s.name;
    Vector2 ls = MeasureTextEx(font_, line.c_str(), 22.0f, 2.0f);
    DrawTextEx(font_, line.c_str(),
               {rect.x + 12.0f, rect.y + (rect.height - ls.y) * 0.5f}, 22.0f,
               2.0f, RAYWHITE);
    y += ITEM_HEIGHT + ITEM_GAP;
  }

  const char *helper =
      "[Enter/Space] Start   [Up/Down/Click] Select   [Esc] Home";
  Vector2 hs = MeasureTextEx(font_, helper, 18.0f, 2.0f);
  DrawTextEx(font_, helper,
             {(GetScreenWidth() - hs.x) * 0.5f,
              static_cast<float>(GetScreenHeight()) - 60.0f},
             18.0f, 2.0f, LIGHTGRAY);

  // Back button (click対応)
  const float btn_w = 180.0f;
  const float btn_h = 50.0f;
  Rectangle back_btn{(GetScreenWidth() - btn_w) * 0.5f,
                     static_cast<float>(GetScreenHeight()) - 110.0f, btn_w,
                     btn_h};
  Vector2 mouse = GetMousePosition();
  bool hover = CheckCollisionPointRec(mouse, back_btn);
  Color base = hover ? Color{70, 110, 170, 230} : Color{50, 80, 130, 210};
  DrawRectangleRounded(back_btn, 0.14f, 6, base);
  DrawRectangleLinesEx(back_btn, 2.0f, Color{180, 210, 255, 230});
  const char *back_txt = "Back to Home (Esc)";
  Vector2 bt = MeasureTextEx(font_, back_txt, 20.0f, 2.0f);
  DrawTextEx(font_, back_txt,
             {back_btn.x + (back_btn.width - bt.x) * 0.5f,
              back_btn.y + (back_btn.height - bt.y) * 0.5f},
             20.0f, 2.0f, RAYWHITE);
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hover) {
    quit_requested_ = true;
    start_requested_ = false;
    selected_stage_id_.clear();
  }
}

std::string StageSelectScene::ConsumeSelectedStage() {
  std::string out = selected_stage_id_;
  selected_stage_id_.clear();
  start_requested_ = false;
  return out;
}

bool StageSelectScene::ConsumeQuitRequest() {
  bool out = quit_requested_;
  if (quit_requested_) {
    quit_requested_ = false;
    start_requested_ = false;
    selected_stage_id_.clear();
  }
  return out;
}

} // namespace Game::Scenes
