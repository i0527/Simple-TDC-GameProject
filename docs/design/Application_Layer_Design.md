# Phase 4: Application層詳細設計 - SceneManager & Scenes（統合最適版）

**プロジェクト**: SimpleTDCGame_NewArch  
**バージョン**: 1.0.0（Application層詳細設計版）  
**作成日**: 2025-12-08 / 08:04 JST  
**目的**: TD Layer 完成後、Application層（シーン管理・UI）の詳細仕様を確定

---

## 📑 目次

1. [Application層 全体概要](#application層-全体概要)
2. [SceneManager 設計（シーン遷移管理）](#scenemanager-設計シーン遷移管理)
3. [BaseScene（シーン基底クラス）](#basesceneシーン基底クラス)
4. [Scene詳細設計（5種類）](#scene詳細設計5種類)
5. [UI System（UIウィジェット・レイアウト）](#ui-systemuiウィジェットレイアウト)
6. [StateManagement（ゲーム状態管理）](#statemanagementゲーム状態管理)
7. [シーン遷移フロー](#シーン遷移フロー)
8. [入力・イベントハンドリング](#入力イベントハンドリング)

---

## Application層 全体概要

### レイヤーアーキテクチャ

```
Application Layer
┌─────────────────────────────────────────────────┐
│                                                  │
│  ┌─ SceneManager（シーン遷移管理） ────────────┐│
│  │  - Scene Stack 管理                         ││
│  │  - Scene Push/Pop                           ││
│  │  - トランジション処理                       ││
│  └──────────────────────────────────────────────┘│
│                                                  │
│  ┌─ Scenes（5種類） ────────────────────────────┐│
│  │  ✅ TitleScene（タイトル）                   ││
│  │  ✅ HomeScene（ホーム）                      ││
│  │  ✅ StageSelectionScene（ステージ選択）     ││
│  │  ✅ TDGameScene（TD ゲーム本体）            ││
│  │  ✅ ResultScene（リザルト）                 ││
│  └──────────────────────────────────────────────┘│
│                                                  │
│  ┌─ UI System（ウィジェット・レイアウト） ──────┐│
│  │  - Button, Text, Panel                      ││
│  │  - LayoutManager（自動配置）                ││
│  │  - EventSystem（UI イベント）               ││
│  └──────────────────────────────────────────────┘│
│                                                  │
│  ┌─ StateManagement（ゲーム状態） ──────────────┐│
│  │  - PlayerState（プレイヤー情報）            ││
│  │  - GameState（ゲーム進捗）                  ││
│  │  - SessionState（現セッション状態）         ││
│  └──────────────────────────────────────────────┘│
│                                                  │
└─────────────────────────────────────────────────┘
         ↓
   Raylib Rendering
         ↓
    Game Window
```

### 設計方針

```yaml
SceneManager特徴:
  ✅ Stack型シーン管理（Push/Pop）
  ✅ トランジション効果（フェード・スライド）
  ✅ シーン間データ受け渡し
  ✅ 非ブロッキング非同期ロード対応

Scene特徴:
  ✅ 統一インターフェース（IScene）
  ✅ ライフサイクル管理（Init/Update/Draw/Shutdown）
  ✅ 独立した状態管理
  ✅ UI System との統合

UI System特徴:
  ✅ Raylib 統合描画
  ✅ アンカー・オフセット式レイアウト
  ✅ イベントリスナーパターン
  ✅ ホットリロード対応
```

---

## SceneManager 設計（シーン遷移管理）

### SceneManager インターフェース

```cpp
// game/include/Game/Application/SceneManager.h
namespace Game::Application {

class IScene;  // 前方宣言

class SceneManager {
public:
  enum class TransitionType {
    IMMEDIATE,      // 即座に切り替え
    FADE_IN_OUT,    // フェードイン・アウト
    SLIDE_LEFT,     // 左スライド
    SLIDE_RIGHT,    // 右スライド
  };

private:
  // ===== シーンスタック =====
  std::vector<std::unique_ptr<IScene>> scene_stack_;
  std::unique_ptr<IScene> next_scene_;
  TransitionType current_transition_;
  
  // ===== トランジション状態 =====
  float transition_duration_ = 0.5f;
  float transition_elapsed_ = 0.0f;
  bool is_transitioning_ = false;
  
  // ===== 画面サイズ =====
  int screen_width_;
  int screen_height_;
  
  // ===== ゲーム状態 =====
  Shared::Core::GameContext& context_;

public:
  SceneManager(int screen_width, int screen_height,
              Shared::Core::GameContext& context);
  
  // ===== シーン遷移 =====
  void PushScene(std::unique_ptr<IScene> scene,
                TransitionType transition = TransitionType::FADE_IN_OUT);
  void PopScene(TransitionType transition = TransitionType::FADE_IN_OUT);
  void ReplaceScene(std::unique_ptr<IScene> scene,
                   TransitionType transition = TransitionType::FADE_IN_OUT);
  
  // ===== フレーム処理 =====
  void Update(float delta_time);
  void Draw();
  
  // ===== シーンアクセス =====
  IScene* GetCurrentScene() const;
  bool IsTransitioning() const { return is_transitioning_; }

private:
  void UpdateTransition(float delta_time);
  void DrawTransition();
  void OnTransitionComplete();
};

} // namespace Game::Application
```

### SceneManager 実装の流れ

```cpp
// game/src/Game/Application/SceneManager.cpp
namespace Game::Application {

void SceneManager::PushScene(std::unique_ptr<IScene> scene,
                            TransitionType transition) {
  next_scene_ = std::move(scene);
  current_transition_ = transition;
  is_transitioning_ = true;
  transition_elapsed_ = 0.0f;
}

void SceneManager::Update(float delta_time) {
  if (is_transitioning_) {
    UpdateTransition(delta_time);
  } else {
    if (scene_stack_.empty()) return;
    scene_stack_.back()->Update(delta_time);
  }
}

void SceneManager::Draw() {
  if (scene_stack_.empty()) return;
  
  scene_stack_.back()->Draw();
  
  if (is_transitioning_) {
    DrawTransition();
  }
}

void SceneManager::UpdateTransition(float delta_time) {
  transition_elapsed_ += delta_time;
  
  if (transition_elapsed_ >= transition_duration_) {
    OnTransitionComplete();
    return;
  }
  
  float progress = transition_elapsed_ / transition_duration_;
  
  // トランジション種別ごとの処理
  switch (current_transition_) {
    case TransitionType::FADE_IN_OUT:
      // 古いシーンをフェードアウト
      // 新しいシーンをフェードイン
      break;
    case TransitionType::SLIDE_LEFT:
      // スライド左アニメーション
      break;
    default:
      break;
  }
}

void SceneManager::OnTransitionComplete() {
  scene_stack_.push_back(std::move(next_scene_));
  scene_stack_.back()->OnEnter();
  is_transitioning_ = false;
  next_scene_ = nullptr;
}

} // namespace Game::Application
```

---

## BaseScene（シーン基底クラス）

### IScene インターフェース

```cpp
// game/include/Game/Application/Scenes/IScene.h
namespace Game::Application {

class IScene {
public:
  virtual ~IScene() = default;
  
  // ===== ライフサイクル =====
  virtual void OnEnter() = 0;      // シーン開始時
  virtual void OnExit() = 0;       // シーン終了時
  
  // ===== フレーム処理 =====
  virtual void Update(float delta_time) = 0;
  virtual void Draw() = 0;
  
  // ===== アクセッサ =====
  virtual std::string GetSceneId() const = 0;
};

} // namespace Game::Application
```

### BaseScene 基底実装

```cpp
// game/include/Game/Application/Scenes/BaseScene.h
namespace Game::Application {

class BaseScene : public IScene {
protected:
  std::string scene_id_;
  Shared::Core::GameContext& context_;
  
  // ===== UI System =====
  std::unique_ptr<UIManager> ui_manager_;
  
  // ===== 状態 =====
  bool is_active_ = false;
  float scene_elapsed_time_ = 0.0f;

public:
  BaseScene(const std::string& scene_id,
           Shared::Core::GameContext& context);
  
  virtual ~BaseScene() = default;
  
  // ===== ライフサイクル実装 =====
  void OnEnter() override;
  void OnExit() override;
  
  // ===== アクセッサ =====
  std::string GetSceneId() const override { return scene_id_; }
  bool IsActive() const { return is_active_; }
  float GetElapsedTime() const { return scene_elapsed_time_; }
  UIManager* GetUIManager() { return ui_manager_.get(); }

protected:
  // ===== サブクラス用フック =====
  virtual void OnEnterImpl() {}
  virtual void OnExitImpl() {}
  virtual void UpdateImpl(float delta_time) {}
  virtual void DrawImpl() {}
};

} // namespace Game::Application
```

---

## Scene詳細設計（5種類）

### 1. TitleScene（タイトル画面）

```cpp
// game/include/Game/Application/Scenes/TitleScene.h
namespace Game::Application {

class TitleScene : public BaseScene {
private:
  enum TitleState {
    IDLE,
    LOADING_SAVEFILE,
    SHOWING_MENU,
  };
  
  TitleState state_ = IDLE;
  
  // ===== UI エレメント =====
  UIButton* btn_continue_;
  UIButton* btn_new_game_;
  UIButton* btn_settings_;
  UIButton* btn_exit_;
  
  UIPanel* save_list_panel_;
  std::vector<UIButton*> save_slot_buttons_;
  
  // ===== ゲーム状態 =====
  struct SaveFile {
    int slot_id;
    std::string player_name;
    int level;
    std::string timestamp;
  };
  std::vector<SaveFile> available_saves_;

public:
  TitleScene(Shared::Core::GameContext& context);
  
  void Update(float delta_time) override;
  void Draw() override;

protected:
  void OnEnterImpl() override;
  void OnExitImpl() override;
  void UpdateImpl(float delta_time) override;
  void DrawImpl() override;

private:
  void InitializeUI();
  void LoadSaveFiles();
  void OnContinueClicked();
  void OnNewGameClicked();
  void OnSettingsClicked();
  void OnExitClicked();
  void OnSaveSlotSelected(int slot_id);
};

} // namespace Game::Application
```

**画面構成**:
```
┌─────────────────────────────────────────┐
│                                          │
│        SimpleTDCGame - Title             │
│                                          │
│     ┌──────────────────────────────┐    │
│     │  ▶ 続きから                  │    │
│     ├──────────────────────────────┤    │
│     │  ▶ 新規ゲーム                │    │
│     ├──────────────────────────────┤    │
│     │  ▶ 設定                      │    │
│     ├──────────────────────────────┤    │
│     │  ▶ 終了                      │    │
│     └──────────────────────────────┘    │
│                                          │
└─────────────────────────────────────────┘
```

---

### 2. HomeScene（ホーム画面）

```cpp
// game/include/Game/Application/Scenes/HomeScene.h
namespace Game::Application {

class HomeScene : public BaseScene {
private:
  enum HomeMenuState {
    MENU_CLOSED,
    MENU_OPEN,
    SUBMENU_DECK,
    SUBMENU_STRENGTHEN,
    SUBMENU_ALBUM,
  };
  
  HomeMenuState menu_state_ = MENU_OPEN;
  
  // ===== UI エレメント =====
  // ヘッダー
  UIText* txt_player_name_;
  UIText* txt_level_;
  UIText* txt_gold_;
  UIText* txt_gems_;
  
  // メインメニュー
  UIButton* btn_stage_select_;
  UIButton* btn_deck_edit_;
  UIButton* btn_strengthen_;
  UIButton* btn_album_;
  UIButton* btn_gacha_;
  UIButton* btn_settings_;
  UIButton* btn_exit_;
  
  // サブメニュー（デッキ編集）
  UIPanel* deck_panel_;
  std::vector<UIButton*> deck_slot_buttons_;
  
  // ===== ゲーム状態 =====
  PlayerState player_state_;  // 後述
  int selected_deck_slot_ = 0;

public:
  HomeScene(Shared::Core::GameContext& context);
  
  void Update(float delta_time) override;
  void Draw() override;

protected:
  void OnEnterImpl() override;
  void OnExitImpl() override;
  void UpdateImpl(float delta_time) override;
  void DrawImpl() override;

private:
  void InitializeUI();
  void UpdateUIFromPlayerState();
  void OnStageSelectClicked();
  void OnDeckEditClicked();
  void OnStrengthenClicked();
  void OnAlbumClicked();
  void OnGachaClicked();
  void OnSettingsClicked();
  void OnExitClicked();
};

} // namespace Game::Application
```

**画面構成**:
```
┌──────────────────────────────────────┐
│ [Player] [Level:5] [Gold:10000] [💎500]│
├──────────────────────────────────────┤
│                                       │
│  ┌─────────────────────────────────┐ │
│  │ ▶ ステージ選択                  │ │
│  ├─────────────────────────────────┤ │
│  │ ▶ 編成                          │ │
│  ├─────────────────────────────────┤ │
│  │ ▶ キャラ強化                    │ │
│  ├─────────────────────────────────┤ │
│  │ ▶ 図鑑                          │ │
│  ├─────────────────────────────────┤ │
│  │ ▶ ガチャ                        │ │
│  ├─────────────────────────────────┤ │
│  │ ▶ 設定                          │ │
│  └─────────────────────────────────┘ │
│                                       │
└──────────────────────────────────────┘
```

---

### 3. StageSelectionScene（ステージ選択画面）

```cpp
// game/include/Game/Application/Scenes/StageSelectionScene.h
namespace Game::Application {

class StageSelectionScene : public BaseScene {
private:
  enum SelectionState {
    LISTING_STAGES,
    SHOWING_DETAILS,
    CONFIRMING_DECK,
    LOADING_GAME,
  };
  
  SelectionState state_ = LISTING_STAGES;
  
  // ===== UI エレメント =====
  UIScrollPanel* stage_list_;
  std::vector<UIButton*> stage_buttons_;
  
  UIPanel* detail_panel_;
  UIText* txt_stage_name_;
  UIText* txt_difficulty_;
  UIText* txt_description_;
  UIText* txt_enemy_info_;
  UIButton* btn_detail_start_;
  UIButton* btn_detail_back_;
  
  UIPanel* deck_confirm_panel_;
  UIText* txt_deck_main_[3];
  UIText* txt_deck_sub_[5];
  UIButton* btn_change_deck_;
  UIButton* btn_confirm_start_;
  UIButton* btn_confirm_back_;
  
  // ===== ゲーム状態 =====
  std::vector<Shared::Data::StageDef> available_stages_;
  int selected_stage_index_ = -1;
  std::string selected_deck_id_;

public:
  StageSelectionScene(Shared::Core::GameContext& context);
  
  void Update(float delta_time) override;
  void Draw() override;

protected:
  void OnEnterImpl() override;
  void OnExitImpl() override;
  void UpdateImpl(float delta_time) override;
  void DrawImpl() override;

private:
  void LoadStageList();
  void ShowStageDetails(int index);
  void ShowDeckConfirmation();
  void OnStageSelected(int index);
  void OnStartGameClicked();
};

} // namespace Game::Application
```

**画面構成**:
```
ステージリスト → ステージ詳細 → デッキ確認 → TDGame
```

---

### 4. TDGameScene（TD ゲーム本体）

```cpp
// game/include/Game/Application/Scenes/TDGameScene.h
namespace Game::Application {

class TDGameScene : public BaseScene {
private:
  // ===== ゲームエンジン =====
  std::unique_ptr<Game::World::GameEngine> game_engine_;
  std::unique_ptr<Game::Managers::CharacterManager> character_mgr_;
  std::unique_ptr<Game::Managers::StageManager> stage_mgr_;
  std::unique_ptr<Game::Managers::EnemyManager> enemy_mgr_;
  
  // ===== ゲーム状態 =====
  enum GamePlayState {
    LOADING,
    READY,
    PLAYING,
    PAUSED,
    VICTORY,
    DEFEAT,
  };
  
  GamePlayState gameplay_state_ = LOADING;
  std::string current_stage_id_;
  
  // ===== UI エレメント（HUD） =====
  UIText* txt_player_hp_;
  UIText* txt_cost_;
  UIProgressBar* bar_cost_;
  UIText* txt_wave_info_;
  UIText* txt_time_;
  
  UIButton* skill_buttons_[2];     // スキルボタン
  UIText* txt_skill_cooldown_[2];
  
  std::vector<UIButton*> unit_buttons_;  // 出撃ボタン
  
  // ポーズメニュー
  UIPanel* pause_menu_;
  UIButton* btn_resume_;
  UIButton* btn_settings_;
  UIButton* btn_retry_;
  UIButton* btn_home_;
  
  // ===== 描画 =====
  float camera_x_ = 0.0f;
  float game_field_height_ = 400.0f;
  float game_field_y_ = 80.0f;

public:
  TDGameScene(Shared::Core::GameContext& context,
             const std::string& stage_id);
  
  void Update(float delta_time) override;
  void Draw() override;

protected:
  void OnEnterImpl() override;
  void OnExitImpl() override;
  void UpdateImpl(float delta_time) override;
  void DrawImpl() override;

private:
  // ===== ゲーム処理 =====
  void InitializeGameEngine();
  void LoadStageData();
  void UpdateGameplayState();
  void RenderGameField();
  void RenderHUD();
  void RenderPauseMenu();
  
  // ===== イベントハンドラ =====
  void OnUnitButtonClicked(int unit_index);
  void OnSkillButtonClicked(int skill_index);
  void OnPausePressed();
  void OnResumePressed();
  void OnRetryPressed();
  void OnHomePressed();
  void OnGameWon();
  void OnGameLost();
};

} // namespace Game::Application
```

**画面構成**:
```
┌─────────────────────────────────────┐
│ HP: ███░░░ CP: ███░░░ Wave 2/3 2:45 │
├─────────────────────────────────────┤
│                                      │
│        [ゲームフィールド]            │  ← GameEngine による描画
│        (敵・ユニット表示)            │
│                                      │
├─────────────────────────────────────┤
│ [⚔️攻撃UP] [🛡️防御] [猫] [犬] [鳥]  │
│   冷却中:5s  冷却中:0s    300  250 200│
└─────────────────────────────────────┘
```

---

### 5. ResultScene（リザルト画面）

```cpp
// game/include/Game/Application/Scenes/ResultScene.h
namespace Game::Application {

class ResultScene : public BaseScene {
private:
  enum ResultState {
    SHOWING_RESULTS,
    WAITING_INPUT,
    TRANSITIONING,
  };
  
  ResultState state_ = SHOWING_RESULTS;
  
  // ===== ゲーム結果 =====
  struct GameResult {
    bool is_victory;
    int elapsed_time;  // 秒
    int gold_earned;
    int exp_earned;
    std::vector<std::string> items_earned;
    float star_rating;  // 1.0～3.0
  };
  
  GameResult game_result_;
  
  // ===== UI エレメント =====
  UIText* txt_result_title_;  // "VICTORY!" / "DEFEAT!"
  UIText* txt_time_;
  UIText* txt_rewards_;
  UIText* txt_gold_;
  UIText* txt_exp_;
  UIText* txt_items_;
  UIPanel* star_rating_panel_;
  
  UIButton* btn_retry_;
  UIButton* btn_next_;       // Victoryの場合のみ
  UIButton* btn_select_;
  UIButton* btn_home_;
  
  // ===== アニメーション =====
  float show_elapsed_time_ = 0.0f;
  float show_duration_ = 2.0f;  // リザルト表示時間

public:
  ResultScene(Shared::Core::GameContext& context,
             const GameResult& result);
  
  void Update(float delta_time) override;
  void Draw() override;

protected:
  void OnEnterImpl() override;
  void OnExitImpl() override;
  void UpdateImpl(float delta_time) override;
  void DrawImpl() override;

private:
  void InitializeUI();
  void UpdateResultDisplay();
  void DrawStarRating();
  void OnRetryClicked();
  void OnNextClicked();
  void OnSelectClicked();
  void OnHomeClicked();
};

} // namespace Game::Application
```

**画面構成**:
```
┌─────────────────────────────────────┐
│                                      │
│          ★ VICTORY! ★               │
│                                      │
│      クリア時間: 45秒                │
│      獲得ゴールド: 500              │
│      獲得経験値: 250                │
│      アイテム: [素材×3]             │
│                                      │
│      評価: ★★★                     │
│                                      │
│  [リトライ] [次へ] [選択] [ホーム]   │
│                                      │
└─────────────────────────────────────┘
```

---

## UI System（UIウィジェット・レイアウト）

### UIManager

```cpp
// game/include/Game/Application/UI/UIManager.h
namespace Game::Application {

class UIManager {
private:
  // ===== ウィジェット管理 =====
  std::vector<std::shared_ptr<IUIWidget>> widgets_;
  std::unordered_map<std::string, std::shared_ptr<IUIWidget>> widget_map_;
  
  // ===== イベントシステム =====
  EventSystem<UIEventData> ui_event_system_;
  
  // ===== フォーカス管理 =====
  std::shared_ptr<IUIWidget> focused_widget_;
  
  // ===== レイアウト =====
  LayoutManager layout_mgr_;

public:
  UIManager();
  
  // ===== ウィジェット操作 =====
  template<typename T>
  T* CreateWidget(const std::string& widget_id, 
                 const UIRect& rect) {
    auto widget = std::make_shared<T>(widget_id, rect);
    widgets_.push_back(widget);
    widget_map_[widget_id] = widget;
    return static_cast<T*>(widget.get());
  }
  
  std::shared_ptr<IUIWidget> GetWidget(const std::string& widget_id);
  void RemoveWidget(const std::string& widget_id);
  void ClearWidgets();
  
  // ===== イベント =====
  void Subscribe(const std::string& event_type,
                std::function<void(const UIEventData&)> callback);
  void Emit(const std::string& event_type, const UIEventData& data);
  
  // ===== フレーム処理 =====
  void Update(float delta_time, const InputState& input);
  void Draw();
  
  // ===== フォーカス管理 =====
  void SetFocus(const std::string& widget_id);
  void ClearFocus();

private:
  void HandleInput(const InputState& input);
  void UpdateWidgets(float delta_time);
  void DrawWidgets();
};

} // namespace Game::Application
```

### UIWidget 基底クラス

```cpp
// game/include/Game/Application/UI/IUIWidget.h
namespace Game::Application {

struct UIRect {
  float x, y, width, height;
};

struct UIEventData {
  std::string event_type;
  std::string source_id;
  std::any data;
};

class IUIWidget {
protected:
  std::string widget_id_;
  UIRect rect_;
  bool is_visible_ = true;
  bool is_enabled_ = true;
  Color background_color_ = {200, 200, 200, 255};
  Color border_color_ = {100, 100, 100, 255};
  bool has_border_ = false;

public:
  virtual ~IUIWidget() = default;
  
  virtual void Update(float delta_time, const InputState& input) = 0;
  virtual void Draw() = 0;
  
  // ===== プロパティ =====
  virtual std::string GetId() const { return widget_id_; }
  virtual UIRect GetRect() const { return rect_; }
  virtual void SetRect(const UIRect& rect) { rect_ = rect; }
  
  virtual void SetVisible(bool visible) { is_visible_ = visible; }
  virtual bool IsVisible() const { return is_visible_; }
  
  virtual void SetEnabled(bool enabled) { is_enabled_ = enabled; }
  virtual bool IsEnabled() const { return is_enabled_; }
  
  // ===== ヒット判定 =====
  virtual bool IsPointInside(const glm::vec2& point) const {
    return point.x >= rect_.x && point.x <= rect_.x + rect_.width &&
           point.y >= rect_.y && point.y <= rect_.y + rect_.height;
  }
};

} // namespace Game::Application
```

### UIWidget 実装例

#### UIButton

```cpp
// game/include/Game/Application/UI/UIButton.h
namespace Game::Application {

class UIButton : public IUIWidget {
private:
  std::string label_;
  std::function<void()> on_click_;
  
  enum ButtonState {
    NORMAL,
    HOVERED,
    PRESSED,
    DISABLED,
  };
  
  ButtonState button_state_ = NORMAL;
  float press_duration_ = 0.0f;

public:
  UIButton(const std::string& id, const UIRect& rect);
  
  void SetLabel(const std::string& label) { label_ = label; }
  void SetOnClick(std::function<void()> callback) { on_click_ = callback; }
  
  void Update(float delta_time, const InputState& input) override;
  void Draw() override;

private:
  void UpdateButtonState(const InputState& input);
  Color GetButtonColor() const;
};

} // namespace Game::Application
```

#### UIText

```cpp
// game/include/Game/Application/UI/UIText.h
namespace Game::Application {

class UIText : public IUIWidget {
private:
  std::string text_;
  int font_size_ = 20;
  Color text_color_ = {0, 0, 0, 255};
  
  enum TextAlignment {
    LEFT,
    CENTER,
    RIGHT,
  };
  
  TextAlignment alignment_ = LEFT;

public:
  UIText(const std::string& id, const UIRect& rect);
  
  void SetText(const std::string& text) { text_ = text; }
  void SetFontSize(int size) { font_size_ = size; }
  void SetTextColor(const Color& color) { text_color_ = color; }
  void SetAlignment(TextAlignment align) { alignment_ = align; }
  
  void Update(float delta_time, const InputState& input) override;
  void Draw() override;
};

} // namespace Game::Application
```

#### UIPanel

```cpp
// game/include/Game/Application/UI/UIPanel.h
namespace Game::Application {

class UIPanel : public IUIWidget {
private:
  std::vector<std::shared_ptr<IUIWidget>> children_;

public:
  UIPanel(const std::string& id, const UIRect& rect);
  
  void AddChild(std::shared_ptr<IUIWidget> child);
  void RemoveChild(const std::string& child_id);
  
  void Update(float delta_time, const InputState& input) override;
  void Draw() override;
};

} // namespace Game::Application
```

#### UIProgressBar

```cpp
// game/include/Game/Application/UI/UIProgressBar.h
namespace Game::Application {

class UIProgressBar : public IUIWidget {
private:
  float current_value_ = 0.5f;
  float max_value_ = 1.0f;
  Color fill_color_ = {50, 200, 50, 255};

public:
  UIProgressBar(const std::string& id, const UIRect& rect);
  
  void SetValue(float value) { 
    current_value_ = glm::clamp(value, 0.0f, max_value_);
  }
  void SetMaxValue(float max_val) { max_value_ = max_val; }
  float GetValue() const { return current_value_; }
  
  void Update(float delta_time, const InputState& input) override;
  void Draw() override;
};

} // namespace Game::Application
```

### LayoutManager（自動レイアウト）

```cpp
// game/include/Game/Application/UI/LayoutManager.h
namespace Game::Application {

class LayoutManager {
public:
  enum class LayoutType {
    VERTICAL,
    HORIZONTAL,
    GRID,
    ANCHOR_BASED,
  };
  
  struct LayoutParams {
    LayoutType type;
    float spacing = 10.0f;
    bool auto_resize = false;
  };

public:
  // ===== レイアウト計算 =====
  static void ApplyVerticalLayout(
    const std::vector<std::shared_ptr<IUIWidget>>& widgets,
    const UIRect& container,
    float spacing = 10.0f);
  
  static void ApplyHorizontalLayout(
    const std::vector<std::shared_ptr<IUIWidget>>& widgets,
    const UIRect& container,
    float spacing = 10.0f);
  
  static void ApplyGridLayout(
    const std::vector<std::shared_ptr<IUIWidget>>& widgets,
    const UIRect& container,
    int columns,
    float spacing = 10.0f);
  
  // ===== アンカーベースレイアウト =====
  enum class Anchor {
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    MIDDLE_LEFT,
    CENTER,
    MIDDLE_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT,
  };
  
  static UIRect CalculateAnchorRect(
    const UIRect& parent,
    Anchor anchor,
    float width, float height,
    const glm::vec2& offset = {0, 0});
};

} // namespace Game::Application
```

---

## StateManagement（ゲーム状態管理）

### PlayerState（プレイヤー情報）

```cpp
// game/include/Game/Application/State/PlayerState.h
namespace Game::Application {

struct PlayerState {
  // ===== プレイヤー基本情報 =====
  std::string player_id;
  std::string player_name;
  int level = 1;
  int exp = 0;
  int next_level_exp = 100;
  
  // ===== 通貨 =====
  int gold = 0;
  int gems = 0;
  
  // ===== キャラクター情報 =====
  struct OwnedCharacter {
    std::string character_id;
    int current_level;
    int skill_level;
    int evolution_stage;
  };
  std::vector<OwnedCharacter> owned_characters;
  
  // ===== デッキ情報 =====
  struct Deck {
    std::string deck_id;
    std::string deck_name;
    std::vector<std::string> main_units;    // 3個
    std::vector<std::string> ability_skills; // 2個
    std::vector<std::string> sub_units;      // 5個
  };
  std::vector<Deck> decks;
  int current_deck_index = 0;
  
  // ===== ステージ進捗 =====
  int current_stage = 1;
  std::vector<bool> completed_stages;
  std::unordered_map<std::string, int> stage_best_rating;
  
  // ===== 設定 =====
  struct Settings {
    float master_volume = 0.8f;
    float bgm_volume = 0.7f;
    float se_volume = 0.9f;
    std::string language = "ja";
  };
  Settings settings;
};

} // namespace Game::Application
```

### GameState（ゲーム進捗状態）

```cpp
// game/include/Game/Application/State/GameState.h
namespace Game::Application {

struct GameState {
  // ===== 現在のセッション =====
  std::string current_stage_id;
  int current_wave = 0;
  float stage_elapsed_time = 0.0f;
  
  // ===== ゲーム進行状態 =====
  enum SessionState {
    IN_HOME,
    IN_STAGE_SELECTION,
    IN_GAME,
    IN_RESULT,
    IN_SETTINGS,
  };
  SessionState session_state = IN_HOME;
  
  // ===== 前回のゲーム結果 =====
  struct LastGameResult {
    bool is_victory;
    int elapsed_time;
    int gold_earned;
    int exp_earned;
  };
  LastGameResult last_result;
  
  // ===== 現在の編成 =====
  std::string current_deck_id;
};

} // namespace Game::Application
```

### GlobalStateManager（グローバル状態管理）

```cpp
// game/include/Game/Application/State/GlobalStateManager.h
namespace Game::Application {

class GlobalStateManager {
private:
  // シングルトン
  static GlobalStateManager* instance_;
  
  // ===== 状態 =====
  PlayerState player_state_;
  GameState game_state_;

public:
  static GlobalStateManager& GetInstance() {
    if (!instance_) {
      instance_ = new GlobalStateManager();
    }
    return *instance_;
  }
  
  // ===== プレイヤー状態 =====
  PlayerState& GetPlayerState() { return player_state_; }
  const PlayerState& GetPlayerState() const { return player_state_; }
  
  void SavePlayerState(const std::string& filepath);
  void LoadPlayerState(const std::string& filepath);
  
  // ===== ゲーム状態 =====
  GameState& GetGameState() { return game_state_; }
  const GameState& GetGameState() const { return game_state_; }
  
  // ===== ユーティリティ =====
  void AddGold(int amount);
  void AddExp(int amount);
  void AddCharacter(const std::string& character_id);
  void LevelUpCharacter(const std::string& character_id);
};

} // namespace Game::Application
```

---

## シーン遷移フロー

### 全体フロー図

```
TitleScene
  ├─ 続きから → LoadGame → HomeScene
  ├─ 新規ゲーム → NewGame → HomeScene
  └─ 終了 → Exit

HomeScene
  ├─ ステージ選択 → StageSelectionScene
  ├─ 編成 → (サブメニュー)
  ├─ 設定 → SettingsScene
  └─ 終了 → TitleScene

StageSelectionScene
  ├─ ステージ選択 → 詳細表示 → デッキ確認 → TDGameScene
  └─ キャンセル → HomeScene

TDGameScene
  ├─ Victory → ResultScene(Victory)
  ├─ Defeat → ResultScene(Defeat)
  ├─ Pause
  │  ├─ 再開 → TDGameScene
  │  ├─ リトライ → TDGameScene(Reset)
  │  └─ 戻る → HomeScene
  └─ 敵全滅 → ResultScene

ResultScene
  ├─ Victory時:
  │  ├─ 次へ → StageSelectionScene(Next)
  │  ├─ 選択 → StageSelectionScene
  │  └─ ホーム → HomeScene
  └─ Defeat時:
     ├─ リトライ → TDGameScene(Reset)
     ├─ 選択 → StageSelectionScene
     └─ ホーム → HomeScene
```

---

## 入力・イベントハンドリング

### InputState

```cpp
// game/include/Game/Application/Input/InputState.h
namespace Game::Application {

struct InputState {
  // ===== キー入力 =====
  bool key_pressed[256] = {false};
  bool key_released[256] = {false};
  
  // ===== マウス入力 =====
  glm::vec2 mouse_position = {0.0f, 0.0f};
  bool mouse_left_pressed = false;
  bool mouse_left_released = false;
  bool mouse_right_pressed = false;
  bool mouse_right_released = false;
  
  // ===== スクロール =====
  float scroll_delta = 0.0f;
  
  // ===== ゲームパッド（将来対応） =====
  // ...
};

} // namespace Game::Application
```

### InputManager

```cpp
// game/include/Game/Application/Input/InputManager.h
namespace Game::Application {

class InputManager {
private:
  InputState current_input_;
  InputState previous_input_;

public:
  void Update();  // 毎フレーム呼び出し
  const InputState& GetCurrentInput() const { return current_input_; }
  const InputState& GetPreviousInput() const { return previous_input_; }
  
  // ===== 便利メソッド =====
  bool IsKeyPressed(int key) const;
  bool IsKeyReleased(int key) const;
  bool IsMouseLeftPressed() const;
  bool IsMouseLeftReleased() const;
  glm::vec2 GetMousePosition() const;
};

} // namespace Game::Application
```

### イベントシステム

```cpp
// game/include/Game/Application/Events/EventSystem.h
namespace Game::Application {

template<typename EventType>
class EventSystem {
private:
  std::unordered_map<std::string,
    std::vector<std::function<void(const EventType&)>>> subscribers_;

public:
  void Subscribe(const std::string& event_type,
                std::function<void(const EventType&)> callback) {
    subscribers_[event_type].push_back(callback);
  }
  
  void Emit(const std::string& event_type, const EventType& event) {
    if (subscribers_.count(event_type)) {
      for (auto& callback : subscribers_[event_type]) {
        callback(event);
      }
    }
  }
};

} // namespace Game::Application
```

---

## 実装優先度

### Phase 4.1: Core Infrastructure（4日）

```
Day 1:
  ✅ SceneManager 実装
  ✅ BaseScene + IScene インターフェース
  ✅ Transition エフェクト

Day 2:
  ✅ UIManager 実装
  ✅ UIButton, UIText, UIPanel
  ✅ UIProgressBar

Day 3:
  ✅ LayoutManager 実装
  ✅ Event/Input System
  ✅ InputManager 統合

Day 4:
  ✅ StateManagement 実装
  ✅ PlayerState, GameState
  ✅ GlobalStateManager
```

### Phase 4.2: Scene実装（1週間）

```
Day 1-2:
  ✅ TitleScene
  ✅ HomeScene（基本機能）
  ✅ UI統合テスト

Day 3-4:
  ✅ StageSelectionScene
  ✅ ResultScene
  ✅ ステージリスト機能

Day 5-6:
  ✅ TDGameScene（HUD統合）
  ✅ GameEngine との連携
  ✅ 状態遷移テスト

Day 7:
  ✅ 全Scene統合テスト
  ✅ トランジション テスト
  ✅ パフォーマンス計測
```

---

## チェックリスト

```
SceneManager:
  ☐ Push/Pop/Replace 実装
  ☐ トランジション効果実装
  ☐ Scene Stack 管理
  ☐ HotReload対応

BaseScene & Scenes:
  ☐ 5種類のScene実装
  ☐ ライフサイクル（OnEnter/OnExit）
  ☐ Update/Draw メソッド
  ☐ UI統合

UI System:
  ☐ UIManager 実装
  ☐ 5種類のUIWidget
  ☐ LayoutManager（自動配置）
  ☐ イベントシステム

StateManagement:
  ☐ PlayerState 定義
  ☐ GameState 定義
  ☐ GlobalStateManager（シングルトン）
  ☐ Save/Load 機能

Input/Event:
  ☐ InputManager 実装
  ☐ InputState 構造体
  ☐ EventSystem テンプレート
  ☐ Scene への入力伝播
```

---

## 次のドキュメント

- [ ] **Raylib統合設計** (Graphics + Input Implementation)
- [ ] **実装スケジュール・タスク分解** (実装フェーズの詳細)
- [ ] **テスト戦略** (Unit/Integration/E2E)

---

## サマリー

Application 層（SceneManager & Scenes）の設計が完成しました：

```
✅ Stack型SceneManager でシーン遷移を一元管理
✅ 5種類のScene（Title/Home/Selection/TDGame/Result）
✅ 統一された UIManager と 複数ウィジェット
✅ PlayerState/GameState による状態管理
✅ EventSystem による疎結合なイベントハンドリング
✅ InputManager による入力統一

🎉 Core Layer + Game Layer + TD Layer + Application Layer
   で完全な 4層アーキテクチャが完成しました！

次は Raylib 統合と実装フェーズへ！
```

