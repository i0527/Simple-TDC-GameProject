# 実装ロードマップ

**プロジェクト**: Simple TDC Game - New Architecture  
**バージョン**: 0.2.0  
**最終更新**: 2025-12-08

---

## 📊 現在の状態

### ✅ Phase 1: 基盤構築（完了）

**達成状況**: 90% 完了

#### 完了項目

- [x] アーキテクチャ設計（完全分離型）
- [x] ディレクトリ構造構築（Shared/Game/Editor）
- [x] Shared Layer 実装
  - [x] GameContext（コンテキスト管理）
  - [x] EventSystem（イベントシステム）
  - [x] FileWatcher（ファイル監視）
  - [x] DefinitionRegistry（定義レジストリ）
  - [x] Loaders（Entity/Skill/Stage/Wave/Ability）
  - [x] Validators（データ検証）
- [x] Game Layer 基盤実装
  - [x] CoreComponents（ECSコンポーネント定義）
  - [x] EntityManager（エンティティ管理）
  - [x] SkillManager（スキル管理）
  - [x] StageManager（ステージ管理）
  - [x] GameApp（アプリケーション基盤）
- [x] Editor Layer 基盤実装
  - [x] EditorApp（エディタアプリケーション基盤）
- [x] CMake ビルドシステム
  - [x] FetchContent による依存関係自動取得
  - [x] マルチターゲットビルド
- [x] Game Layer ビルド成功・動作確認

#### 未完了項目

- [ ] Editor Layer ビルド修正（ImGui/rlImGui互換性問題）
- [ ] FHD解像度対応（1920x1080固定）
- [ ] 日本語フォント統合（NotoSansJP-Medium.ttf）
- [ ] 全UI日本語化

---

## 🎯 Phase 1.5: 基盤完成（次の優先タスク）

**目標**: Phase 1の残タスク完了 + UI基盤整備

**期間**: 1-2日  
**優先度**: 🔴 最高

### タスクリスト

#### 1. Editor Layer ビルド修正 🔴

**優先度**: 最高  
**担当ファイル**: `editor/CMakeLists.txt`, `editor/src/Editor/Application/EditorApp.cpp`

- [x] ImGui バージョンを v1.89.9 に固定（v1.90.x互換性問題回避）
- [x] rlImGui ライブラリの手動ビルド設定
- [x] ビルドテスト・動作確認
- [x] エディタウィンドウの起動確認

**実装詳細**:

```cmake
# editor/CMakeLists.txt
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9  # 重要: v1.90.x は rlImGui と互換性問題
)
```

**検証方法**:

```powershell
cmake --build build --config Debug -j 8
.\build\editor\Debug\SimpleTDCEditor.exe
```

---

#### 2. 日本語フォント統合 🟡

**優先度**: 高  
**担当ファイル**: `shared/include/Core/FontManager.h/cpp`, `game/src/Game/Application/GameApp.cpp`, `editor/src/Editor/Application/EditorApp.cpp`

##### 2.1. FontManager クラス作成

- [x] `shared/include/Core/FontManager.h` 作成
- [x] `shared/src/Core/FontManager.cpp` 実装
- [x] NotoSansJP-Medium.ttf ロード機能
- [x] 対応文字範囲設定（ひらがな、カタカナ、ASCII、JIS第一・第二水準）

**実装詳細**:

```cpp
// shared/include/Core/FontManager.h
namespace Shared::Core {
    class FontManager {
    public:
        FontManager();
        ~FontManager();
        
        // Raylib用フォントロード
        Font LoadJapaneseFont(const char* filePath, int fontSize);
        
        // ImGui用フォントロード
        ImFont* LoadImGuiJapaneseFont(const char* filePath, float fontSize);
        
    private:
        std::vector<int> GetJapaneseCodepoints();
    };
}
```

##### 2.2. Game Layer フォント統合

- [x] GameApp に FontManager 統合
- [x] デフォルトフォントとして NotoSansJP 設定
- [x] テキスト描画テスト（日本語表示確認）

**実装箇所**:

```cpp
// game/src/Game/Application/GameApp.cpp
void GameApp::Initialize() {
    InitWindow(1920, 1080, "Simple TDC Game");
    
    fontManager_ = std::make_unique<FontManager>();
    defaultFont_ = fontManager_->LoadJapaneseFont(
        "assets/fonts/NotoSansJP-Medium.ttf", 32
    );
}
```

##### 2.3. Editor Layer フォント統合

- [x] EditorApp に FontManager 統合
- [x] ImGui デフォルトフォントとして NotoSansJP 設定
- [x] メニュー日本語表示テスト（初期メニュー表示で確認）

**実装箇所**:

```cpp
// editor/src/Editor/Application/EditorApp.cpp
void EditorApp::Initialize() {
    InitWindow(1920, 1080, "Simple TDC Editor");
    rlImGuiSetup(true);
    
    fontManager_ = std::make_unique<FontManager>();
    ImGuiIO& io = ImGui::GetIO();
    io.FontDefault = fontManager_->LoadImGuiJapaneseFont(
        "assets/fonts/NotoSansJP-Medium.ttf", 18.0f
    );
}
```

---

#### 3. FHD解像度対応 🟢

**優先度**: 中  
**担当ファイル**: `game/src/Game/Application/GameApp.cpp`, `editor/src/Editor/Application/EditorApp.cpp`

- [x] ウィンドウサイズを 1920x1080 に固定
- [x] リサイズ時のスケーリング実装（アスペクト比維持）
- [x] UI要素の配置調整（初期画面のみ／詳細UIは後続で調整）

**実装詳細**:

```cpp
// game/src/Game/Application/GameApp.cpp
constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;

void GameApp::Initialize() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simple TDC Game");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
}

void GameApp::HandleResize() {
    int currentWidth = GetScreenWidth();
    int currentHeight = GetScreenHeight();
    
    // アスペクト比を維持したスケーリング
    float scaleX = (float)currentWidth / SCREEN_WIDTH;
    float scaleY = (float)currentHeight / SCREEN_HEIGHT;
    float scale = std::min(scaleX, scaleY);
    
    // レターボックス/ピラーボックス対応
    // ...
}
```

---

#### 4. UI日本語化 🟢

**優先度**: 中  
**担当ファイル**: すべてのUI表示箇所

##### 4.1. Game Layer UI

- [x] タイトル画面（初期タイトル文言を日本語化／詳細UIは後続）
- [x] ゲーム画面（「体力」「コスト」「Wave」プレースホルダー表示）
- [ ] リザルト画面（「クリア」「失敗」「リトライ」）

##### 4.2. Editor Layer UI

- [x] メニューバー（初期メニューを日本語化済み）
- [ ] エンティティエディタ（「名前」「体力」「攻撃力」）
- [ ] スキルエディタ（「効果」「範囲」「クールダウン」）
- [ ] ステージエディタ（「ステージ名」「Wave設定」）

---

## 🚀 Phase 2: ゲームロジック実装

**目標**: 実際にプレイ可能なTDゲーム実装

**期間**: 1-2週間  
**優先度**: 🟡 高

### タスクリスト

#### 1. ECS Systems 実装 🔴

**優先度**: 最高

##### 1.1. Movement System

**担当ファイル**: `game/include/Game/Systems/MovementSystem.h/cpp`

- [x] 移動コンポーネント（Velocity, Direction）定義（Velocityのみ使用中、Directionは後続）
- [x] 位置更新ロジック
- [ ] 衝突判定との連携
- [ ] スピードバフ/デバフ対応

**実装詳細**:

```cpp
// game/include/Game/Systems/MovementSystem.h
namespace Game::Systems {
    class MovementSystem {
    public:
        void Update(entt::registry& registry, float deltaTime);
        
    private:
        void UpdatePositions(entt::registry& registry, float deltaTime);
        void HandleBoundaries(entt::registry& registry);
    };
}
```

##### 1.2. Attack System

**担当ファイル**: `game/include/Game/Systems/AttackSystem.h/cpp`

- [x] 攻撃範囲判定（最短距離ターゲット）
- [x] ターゲット選択ロジック（最短距離優先、同陣営除外）
- [x] ダメージ計算（HP<=0でDead付与）
- [ ] 攻撃アニメーション連携

**実装詳細**:

```cpp
// game/include/Game/Systems/AttackSystem.h
namespace Game::Systems {
    class AttackSystem {
    public:
        void Update(entt::registry& registry, float deltaTime);
        
    private:
        entt::entity FindTarget(entt::registry& registry, 
                                const Position& attackerPos,
                                float range,
                                TargetType targetType);
        void ApplyDamage(entt::registry& registry, 
                        entt::entity attacker,
                        entt::entity target);
    };
}
```

##### 1.3. Skill System

**担当ファイル**: `game/include/Game/Systems/SkillSystem.h/cpp`

- [x] スキル発動条件チェック（プレースホルダ、クールダウン完了時に発動）
- [x] クールダウン管理（単一タイマー、暫定デフォルト5秒）
- [ ] スキル効果適用（バフ/デバフ/範囲攻撃）
- [ ] エフェクト生成連携

##### 1.4. Rendering System

**担当ファイル**: `game/include/Game/Systems/RenderingSystem.h/cpp`

> **📋 設計参照**: [`docs/CHARACTER_RENDERING_DESIGN.md`](CHARACTER_RENDERING_DESIGN.md) - 統一描画パイプライン

- [x] スプライト描画（プレースホルダー矩形＋HPバー）
- [ ] アニメーション管理（アトラス統一形式）
  - [ ] メインキャラ（256×256）対応
  - [ ] サブキャラ（128×128以下）対応
  - [ ] 統一描画パイプライン実装
- [ ] エフェクト描画
- [x] UI描画（プレースホルダーHUDテキスト）

**実装方針**:

```cpp
// 旧形式（グリッド）分岐を削除し、アトラスベース統一描画に移行
// - Animation::useAtlas を常に true
// - グリッド描画コードの削除
// - DrawAtlasFrame() に一本化
```

---

#### 2. Scene Manager 実装 🟡

**優先度**: 高  
**担当ファイル**: `game/include/Game/Scenes/SceneManager.h/cpp`

##### 2.1. SceneManager 基盤

- [x] SceneManager クラス作成（即時切替のみ）
- [x] シーン切り替え機能（Push/Pop/Replace）
- [x] シーンスタック管理
- [ ] トランジション効果

**実装詳細**:

```cpp
// game/include/Game/Scenes/SceneManager.h
namespace Game::Scenes {
    class SceneManager {
    public:
        void PushScene(std::unique_ptr<Scene> scene);
        void PopScene();
        void ChangeScene(std::unique_ptr<Scene> scene);
        
        void Update(float deltaTime);
        void Render();
        
    private:
        std::vector<std::unique_ptr<Scene>> sceneStack_;
        bool isTransitioning_;
    };
}
```

##### 2.2. Scene 実装

**担当ファイル**: `game/include/Game/Scenes/*.h/cpp`

- [x] **TitleScene**: タイトル画面（タイトル表示＋開始プロンプト点滅の最小版）
  - 「新規ゲーム」「続きから」「設定」ボタン: 未
  - BGM再生: 未
  
- [ ] **HomeScene**: ホーム画面
  - デッキ編成
  - ステージ選択への遷移
  - ショップ（将来実装）
  
- [ ] **StageSelectionScene**: ステージ選択
  - ステージ一覧表示
  - ステージ情報表示（難易度、報酬）
  - TDGameScene への遷移
  
- [x] **TDGameScene**: TDゲーム本体の土台（描画のみ）
  - エンティティスポーン: 未
  - Wave管理: 未
  - ゲームオーバー判定: 未
  - クリア判定: 未
  - レーン: 単一レーン（Y下寄せ、味方右→左/敵左→右の進撃方向に調整）
  - 挙動: 前方に敵がいると足を止めて攻撃（にゃんこ風ストッパー）
  
- [ ] **ResultScene**: リザルト画面
  - クリア/失敗表示
  - 獲得報酬表示
  - リトライ/ホームへ戻るボタン

---

#### 3. TD ゲームコア実装 🔴

**優先度**: 最高

##### 3.1. デッキシステム

**担当ファイル**: `game/include/Game/Core/DeckManager.h/cpp`

- [ ] デッキクラス（10スロット）
- [ ] エンティティ配置コスト管理
- [ ] クールダウン管理（同じエンティティの連続配置制限）
- [ ] デッキ編成UI

**実装詳細**:

```cpp
// game/include/Game/Core/DeckManager.h
namespace Game::Core {
    class DeckManager {
    public:
        static constexpr int MAX_DECK_SIZE = 10;
        
        bool AddToDeck(const std::string& entityId);
        bool RemoveFromDeck(int slotIndex);
        bool CanDeploy(int slotIndex) const;
        void Deploy(int slotIndex, const Vector2& position);
        
        void Update(float deltaTime);  // クールダウン更新
        
    private:
        std::array<std::string, MAX_DECK_SIZE> deck_;
        std::array<float, MAX_DECK_SIZE> cooldowns_;
        int currentCost_;
        int maxCost_;
    };
}
```

##### 3.2. Wave スポーンシステム

**担当ファイル**: `game/include/Game/Core/WaveManager.h/cpp`

- [ ] Wave 定義の読み込み
- [ ] スポーンタイミング管理
- [ ] 敵エンティティ生成
- [ ] Wave クリア判定

**実装詳細**:

```cpp
// game/include/Game/Core/WaveManager.h
namespace Game::Core {
    class WaveManager {
    public:
        void LoadWave(const WaveDef& waveDef);
        void StartWave();
        void Update(float deltaTime);
        
        bool IsWaveComplete() const;
        int GetCurrentWaveIndex() const;
        
    private:
        struct SpawnEvent {
            std::string entityId;
            float spawnTime;
            Vector2 spawnPosition;
        };
        
        std::vector<SpawnEvent> spawnQueue_;
        float elapsedTime_;
        int currentWaveIndex_;
    };
}
```

##### 3.3. 城HP管理

**担当ファイル**: `game/include/Game/Core/CastleManager.h/cpp`

- [ ] 城エンティティ管理
- [ ] ダメージ処理
- [ ] ゲームオーバー判定
- [ ] 城HPバー表示

---

#### 4. セーブ/ロード機能 🟢

**優先度**: 中  
**担当ファイル**: `game/include/Game/Core/SaveData.h/cpp`

- [ ] SaveData クラス作成
- [ ] プレイヤーデータ（所持エンティティ、リソース）
- [ ] ステージ進行状況
- [ ] JSON シリアライズ/デシリアライズ
- [ ] セーブファイル管理（複数スロット）

**実装詳細**:

```cpp
// game/include/Game/Core/SaveData.h
namespace Game::Core {
    struct SaveData {
        int playerId;
        std::vector<std::string> ownedEntities;
        std::map<std::string, int> resources;
        std::vector<std::string> clearedStages;
        
        nlohmann::json Serialize() const;
        static SaveData Deserialize(const nlohmann::json& json);
        
        void SaveToFile(const std::string& filePath) const;
        static SaveData LoadFromFile(const std::string& filePath);
    };
}
```

---

## 🖥️ Phase 3: エディタ機能実装

**目標**: JSONデータを視覚的に編集できるエディタ完成

**期間**: 1週間  
**優先度**: 🟢 中

### タスクリスト

#### 1. Entity Editor Window 🔴

**優先度**: 最高  
**担当ファイル**: `editor/include/Editor/Windows/EntityEditorWindow.h/cpp`

##### 1.1. UI実装

- [ ] エンティティ一覧ウィンドウ
- [ ] エンティティ詳細編集ウィンドウ
- [ ] 新規作成ダイアログ
- [ ] 削除確認ダイアログ

##### 1.2. 編集機能

- [ ] 基本パラメータ編集（名前、HP、攻撃力、防御力）
- [ ] スプライト選択
- [ ] スキルアサイン
- [ ] プレビュー表示

**実装詳細**:

```cpp
// editor/include/Editor/Windows/EntityEditorWindow.h
namespace Editor::Windows {
    class EntityEditorWindow {
    public:
        void Render();
        
    private:
        void RenderEntityList();
        void RenderEntityDetails();
        void RenderCreateDialog();
        
        std::vector<EntityDef> entities_;
        int selectedEntityIndex_;
        bool showCreateDialog_;
    };
}
```

---

#### 2. Skill Editor Window 🟡

**優先度**: 高  
**担当ファイル**: `editor/include/Editor/Windows/SkillEditorWindow.h/cpp`

- [ ] スキル一覧表示
- [ ] スキル効果設定（ダメージ、バフ、デバフ）
- [ ] 範囲・クールダウン設定
- [ ] エフェクト選択

---

#### 3. Stage/Wave Editor Window 🟡

**優先度**: 高  
**担当ファイル**: `editor/include/Editor/Windows/StageEditorWindow.h/cpp`, `WaveEditorWindow.h/cpp`

##### 3.1. Stage Editor

- [ ] ステージ一覧表示
- [ ] ステージ情報編集（名前、難易度、報酬）
- [ ] Wave 割り当て

##### 3.2. Wave Editor

- [ ] Wave 一覧表示
- [ ] スポーン設定（エンティティ、タイミング、位置）
- [ ] タイムライン表示
- [ ] プレビュー機能

---

#### 4. エディタ共通機能 🟢

**優先度**: 中

##### 4.1. ファイル操作

- [ ] JSON保存機能
- [ ] JSON読み込み機能
- [ ] 自動バックアップ
- [ ] 変更検知

##### 4.2. ホットリロード

- [ ] FileWatcher 連携
- [ ] ゲーム実行中のリアルタイム反映
- [ ] 変更通知UI

---

## 🎨 Phase 4: アセット統合・ビジュアル実装

**目標**: グラフィック・サウンド統合、UI洗練

**期間**: 1-2週間  
**優先度**: 🔵 低

> **📋 設計参照**: キャラクター描画については [`docs/CHARACTER_RENDERING_DESIGN.md`](CHARACTER_RENDERING_DESIGN.md) を参照

### タスクリスト

#### 1. グラフィック統合

##### 1.1 スプライトシート作成（新設計準拠）

**メインキャラクター（256×256）**:

- [ ] プレイヤーユニット（戦士、魔法使い、弓使い等）
- [ ] ボスキャラクター
- [ ] 重要敵キャラクター
- [ ] Aseprite エクスポート（JSON Array + Tags）

**サブキャラクター（128×128以下）**:

- [ ] 雑魚敵（スライム、ゴブリン等）
- [ ] サポートキャラクター
- [ ] エフェクトオブジェクト
- [ ] Aseprite エクスポート（JSON Array + Tags）

##### 1.2 アニメーション定義

- [ ] アトラス形式統一（Aseprite JSON）
- [ ] アニメーションタグ設定（idle, walk, attack, death）
- [ ] フレームレート調整（メイン: 8-12fps, サブ: 4-6fps）
- [ ] ループ設定

##### 1.3 エフェクトスプライト

- [ ] 攻撃エフェクト
- [ ] ヒットエフェクト
- [ ] スキルエフェクト
- [ ] UI エフェクト

##### 1.4 UI要素デザイン

- [ ] ボタン（日本語対応）
- [ ] パネル
- [ ] アイコン
- [ ] フォント統合（NotoSansJP）

#### 2. サウンド統合

- [ ] BGM統合
- [ ] SE統合
- [ ] サウンドマネージャー実装
- [ ] ボリューム設定

#### 3. UI/UX 改善

- [ ] メニューデザイン洗練
- [ ] トランジション効果
- [ ] パーティクルエフェクト
- [ ] チュートリアル実装

---

## 📊 進捗管理

### マイルストーン

| Phase | 開始予定 | 完了予定 | 状態 |
|-------|---------|---------|------|
| Phase 1: 基盤構築 | 2025-12-01 | 2025-12-08 | ✅ 90% |
| Phase 1.5: 基盤完成 | 2025-12-08 | 2025-12-09 | ✅ 完了 |
| Phase 2: ゲームロジック | 2025-12-10 | 2025-12-20 | ⏳ 未着手 |
| Phase 3: エディタ機能 | 2025-12-21 | 2025-12-28 | ⏳ 未着手 |
| Phase 4: アセット統合 | 2025-12-29 | 2026-01-10 | ⏳ 未着手 |

### 優先度凡例

- 🔴 最高: 即座に着手すべき
- 🟡 高: 次の優先タスク
- 🟢 中: 順次実装
- 🔵 低: 余裕があれば実装

---

## 📝 次のアクション

### 今日やること（2025-12-08）

1. ✅ 実装ロードマップ作成（このドキュメント）
2. ✅ Editor Layer ビルド修正
   - ImGui v1.89.9 への変更
   - rlImGui 手動ビルド設定
3. ✅ FontManager クラス設計・実装開始（実装・統合まで完了）
4. 🆕 リソース読み込み方針の明文化（エラー時の扱いとフォールバック）

### 今週やること（2025-12-08 〜 2025-12-14）

1. ✅ Phase 1.5 完全完了
   - Editor Layer ビルド成功
   - 日本語フォント統合
   - FHD解像度対応
   - UI日本語化（初期文言／メニューまで）
2. 🔄 Phase 2 着手（進行中）
   - Movement System: 位置更新ロジックまで実装／境界クランプ済み
   - Attack System: 射程内最短ターゲット検索＋ダメージ適用・クールダウンまで実装
   - Rendering System: プレースホルダー矩形＋HPバー／HUDプレースホルダー
   - SceneManager/TitleScene: 即時切替のみ・タイトル表示＋点滅プロンプト
   - TDGameScene: 描画＋簡易Waveカウンタ・初期スポーンのみ（Wave/Deck連携は未）

---

## ✅ 簡易テストチェックリスト（今回追加分）

- タイトル: メニュー（新規/続き/設定）をキーボード・マウスで選択できること、BGM表示とミュート切替が動くこと（title.ogg 無でも落ちない/ミュート表示）。
- 遷移: FADE・SLIDE_LEFT/RIGHT の両方で画面遷移が行われ、Replace/Push/Pop で演出が掛かること。
- Wave進行: 複数Waveを連続で走らせ、敵全滅で次Waveへ、プレイヤー全滅で敗北オーバーレイが出ること。
- Deck/コスト: 定義から取得したコスト・クールダウンでスポーン可否が決まり、不足時に出ないこと。リトライ時に状態がリセットされること。
- リソース: sprite_sheet 欠損時に矩形フォールバック描画が続行されること、title.ogg 欠損時に警告ログ＋ミュート表示で落ちないこと。
- エフェクト: 攻撃時に簡易ヒット枠が表示され、アニメーション（歩行/攻撃/死亡）が進むこと。

---

## 🔗 関連ドキュメント

- [プロジェクト再構成完了レポート](RECONSTRUCTION_REPORT.md)
- [技術要件仕様書](TECHNICAL_REQUIREMENTS.md)
- [アーキテクチャ設計 - 完全分離型](design/Architecture_Complete.md)
- [TD Layer ECS設計](design/TD_Layer_ECS_Design.md)
- [ゲームデザイン仕様書](design/TD_GameDesign.md)

---

**最終更新**: 2025-12-08  
**次回更新予定**: Phase 1.5 完了時

---

## ⚙️ リソース読み込みとエラーハンドリング方針（共通）

- Core/Shared: ファイル存在チェック＋例外/エラー返却（必ずログ）
- Game/Editor: ログを出し、可能ならプレースホルダーにフォールバック。致命的なら明示終了
- JSON: 規約どおり try-catch を徹底

### リソース別フォールバック

- フォント: エラー出力後、ゲーム/エディタを停止・終了（フォントなしでは続行しない）
- テクスチャ: プレースホルダー矩形で代理描画
- サウンド: デバッグコンソールに通知してスキップ（無音で続行）
- 上記以外: 大きなエラー対応は後追いで追加
