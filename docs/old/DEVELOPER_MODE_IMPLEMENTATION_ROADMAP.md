# 開発者モード実装ロードマップ

## 概要

WebUIを廃止し、ゲーム内開発者モードを実装するための詳細なロードマップ。各フェーズの実装内容、依存関係、検証項目を定義する。

## 全体スケジュール

```
Phase 1: 基盤構築 (2週間)
  ├─ Week 1: DevModeManager, RenderTexture分離
  └─ Week 2: DockSpace, ウィンドウ管理

Phase 2: ホットリロード (1週間)
  └─ Week 3: ファイル監視、自動リロード

Phase 3: エンティティエディタ (2週間)
  ├─ Week 4: 基本CRUD、リスト表示
  └─ Week 5: アニメーションエディタ、ステートマシン

Phase 4: ステージエディタ (1週間)
  └─ Week 6: Wave編集、プレビュー

Phase 5: その他エディタ (1週間)
  └─ Week 7: イベント、UI、統合テスト

Phase 6: 最適化・ドキュメント (1週間)
  └─ Week 8: パフォーマンス、ドキュメント、WebUI削除
```

## Phase 1: 基盤構築 (2週間)

### Week 1: コア機能実装

#### 1.1 DevModeManager作成

**ファイル**: `include/Game/DevMode/DevModeManager.h`, `src/Game/DevMode/DevModeManager.cpp`

```cpp
namespace Game::DevMode {

class DevModeManager {
public:
    void Initialize();
    void Update(float deltaTime);
    void Render();
    
    void ToggleDevMode();
    bool IsActive() const { return isActive_; }
    
private:
    bool isActive_ = false;
    // ...
};

} // namespace Game::DevMode
```

**タスク**:
- [ ] DevModeManagerクラス作成
- [ ] F1キーでON/OFF切り替え
- [ ] 状態管理 (isActive_)
- [ ] 初期化・更新・描画フロー

**検証項目**:
- [ ] F1キーで開発者モードが起動する
- [ ] 再度F1キーでモードが終了する
- [ ] ゲームループが正常に動作する

#### 1.2 RenderTexture分離

**ファイル**: `include/Game/DevMode/DevModeRenderer.h`, `src/Game/DevMode/DevModeRenderer.cpp`

```cpp
namespace Game::DevMode {

class DevModeRenderer {
public:
    void Initialize(int windowWidth, int windowHeight);
    void BeginFrame();
    void RenderGameView();
    void RenderDevModeUI();
    void EndFrame();
    
private:
    Core::GameRenderer gameRenderer_;
};

} // namespace Game::DevMode
```

**タスク**:
- [ ] DevModeRendererクラス作成
- [ ] GameRendererをラップ
- [ ] ゲーム本体をRenderTextureに描画
- [ ] ImGuiフレーム管理

**検証項目**:
- [ ] ゲーム画面がRenderTextureに正しく描画される
- [ ] ImGuiが正常に初期化される
- [ ] FHD解像度が維持される

### Week 2: ウィンドウシステム

#### 2.1 DockSpace実装

**ファイル**: `src/Game/DevMode/DevModeRenderer.cpp`

```cpp
void DevModeRenderer::RenderDevModeUI() {
    rlImGuiBegin();
    
    // DockSpace作成
    ImGuiID dockspace_id = ImGui::DockSpaceOverViewport();
    
    // メニューバー
    RenderMenuBar();
    
    // ゲームビュー
    RenderGameViewWindow();
    
    rlImGuiEnd();
}
```

**タスク**:
- [ ] DockSpaceセットアップ
- [ ] メニューバー作成
- [ ] ゲームビューウィンドウ
- [ ] ウィンドウドッキング有効化

**検証項目**:
- [ ] ウィンドウをドラッグしてドッキングできる
- [ ] メニューバーからウィンドウを開ける
- [ ] レイアウトが保存・復元される

#### 2.2 ワークスペース機能

**ファイル**: `include/Game/DevMode/Workspace.h`, `src/Game/DevMode/Workspace.cpp`

```cpp
namespace Game::DevMode {

struct WindowLayout {
    std::string windowId;
    ImVec2 position;
    ImVec2 size;
    bool isVisible;
};

struct Workspace {
    std::string name;
    std::vector<WindowLayout> windows;
    
    void Save(const std::string& filepath);
    void Load(const std::string& filepath);
};

} // namespace Game::DevMode
```

**タスク**:
- [ ] Workspace構造体定義
- [ ] JSON保存・読み込み
- [ ] デフォルトワークスペース作成
- [ ] メニューから保存・読み込み

**検証項目**:
- [ ] ワークスペースがJSONに保存される
- [ ] 保存したワークスペースを読み込める
- [ ] デフォルトレイアウトが適用される

## Phase 2: ホットリロード (1週間)

### Week 3: ファイル監視システム

#### 3.1 HotReloadSystem実装

**ファイル**: `include/Core/HotReloadSystem.h`, `src/Core/HotReloadSystem.cpp`

```cpp
namespace Core {

class HotReloadSystem {
public:
    void Initialize(const std::string& watchPath);
    void Update();
    
    using ReloadCallback = std::function<void(const std::string&)>;
    void RegisterCallback(const std::string& pattern, ReloadCallback callback);
    
private:
    std::map<std::string, std::filesystem::file_time_type> fileTimes_;
    std::map<std::string, std::vector<ReloadCallback>> callbacks_;
    
    void CheckFileChanges();
    void NotifyCallbacks(const std::string& filepath);
};

} // namespace Core
```

**タスク**:
- [ ] HotReloadSystemクラス作成
- [ ] ファイルタイムスタンプ監視
- [ ] コールバック登録機構
- [ ] パターンマッチング

**検証項目**:
- [ ] JSONファイル変更を検知する
- [ ] 適切なコールバックが呼ばれる
- [ ] エラー時に前の状態を維持する

#### 3.2 DefinitionLoader連携

**タスク**:
- [ ] ReloadCharacter() メソッド追加
- [ ] ReloadStage() メソッド追加
- [ ] エラーハンドリング強化
- [ ] コンソールへのログ出力

**検証項目**:
- [ ] キャラクター定義の変更が即座に反映される
- [ ] ステージ定義の変更が反映される
- [ ] 不正なJSONでもクラッシュしない

#### 3.3 メインループ統合

**ファイル**: `src/main_new.cpp` (または対象のエントリポイント)

```cpp
// 初期化
hotReload_.Initialize("assets/definitions");

hotReload_.RegisterCallback("*.char.json", [this](const std::string& path) {
    loader_->ReloadCharacter(path);
    devMode_.GetEntityEditor().RefreshList();
});

// メインループ
while (!WindowShouldClose()) {
    hotReload_.Update();
    // ...
}
```

**タスク**:
- [ ] メインループでUpdate()呼び出し
- [ ] 各種定義ファイルのコールバック登録
- [ ] エディタUIへの変更通知

**検証項目**:
- [ ] ゲーム実行中にファイルを編集できる
- [ ] 変更が自動的にゲームに反映される
- [ ] UIが自動的に更新される

## Phase 3: エンティティエディタ (2週間)

### Week 4: 基本機能

#### 4.1 EntityEditor基本UI

**ファイル**: `include/Game/DevMode/Editors/EntityEditor.h`, `src/Game/DevMode/Editors/EntityEditor.cpp`

```cpp
namespace Game::DevMode {

class EntityEditor {
public:
    void Initialize(Data::DefinitionRegistry* registry);
    void Render();
    void RefreshList();
    
private:
    std::vector<Data::CharacterDef> entities_;
    std::optional<std::string> selectedEntityId_;
    
    void RenderEntityList();
    void RenderEntityDetails();
    void RenderStatsEditor();
};

} // namespace Game::DevMode
```

**タスク**:
- [ ] 左パネル: エンティティリスト
- [ ] 右パネル: 詳細表示
- [ ] 基本情報編集 (ID, Name, Description)
- [ ] ステータス編集 (HP, Speed, Damage)

**検証項目**:
- [ ] エンティティリストが表示される
- [ ] 選択したエンティティの詳細が表示される
- [ ] ステータスを編集できる

#### 4.2 CRUD操作

**タスク**:
- [ ] 新規作成ダイアログ
- [ ] 保存機能 (JSONファイル書き込み)
- [ ] 削除機能 (確認ダイアログ付き)
- [ ] 複製機能

**検証項目**:
- [ ] 新しいエンティティを作成できる
- [ ] 編集結果がJSONに保存される
- [ ] エンティティを削除できる
- [ ] 既存エンティティを複製できる

### Week 5: アニメーション・AI

#### 5.1 アニメーションエディタ

**タスク**:
- [ ] スプライトリスト表示
- [ ] アニメーション設定 (frameCount, frameDuration)
- [ ] テクスチャパス選択 (ファイルブラウザ)
- [ ] プレビュー機能

**検証項目**:
- [ ] スプライトアニメーションを追加・編集できる
- [ ] アニメーションがプレビューできる
- [ ] テクスチャファイルを選択できる

#### 5.2 アニメーションタイムライン

**タスク**:
- [ ] タイムライン描画
- [ ] フレームイベント追加
- [ ] イベントタイプ選択 (Damage, Sound, Effect)
- [ ] イベントパラメータ編集

**検証項目**:
- [ ] タイムライン上にフレームが表示される
- [ ] クリックでイベントを追加できる
- [ ] イベントのパラメータを編集できる

#### 5.3 ステートマシンエディタ

**タスク**:
- [ ] ノードグラフ描画 (ImNodes使用を検討)
- [ ] 状態ノード作成
- [ ] 遷移エッジ作成
- [ ] 条件設定UI

**検証項目**:
- [ ] ビジュアルにステートを配置できる
- [ ] 状態間の遷移を作成できる
- [ ] 遷移条件を設定できる

**注**: ImNodesライブラリを使用すると実装が容易。または、シンプルなリスト形式でも可。

## Phase 4: ステージエディタ (1週間)

### Week 6: Wave編集

#### 6.1 StageEditor基本UI

**ファイル**: `include/Game/DevMode/Editors/StageEditor.h`, `src/Game/DevMode/Editors/StageEditor.cpp`

```cpp
namespace Game::DevMode {

class StageEditor {
public:
    void Initialize(Data::DefinitionRegistry* registry);
    void Render();
    
private:
    std::vector<Data::StageDef> stages_;
    std::optional<std::string> selectedStageId_;
    
    void RenderStageList();
    void RenderWaveEditor();
    void RenderPreview();
};

} // namespace Game::DevMode
```

**タスク**:
- [ ] ステージリスト表示
- [ ] Wave追加・削除
- [ ] エンティティスポーン設定
- [ ] タイミング調整 (count, interval, delay)

**検証項目**:
- [ ] ステージリストが表示される
- [ ] Waveを追加・削除できる
- [ ] スポーン設定を編集できる

#### 6.2 プレビュー機能

**タスク**:
- [ ] プレビューボタン
- [ ] Wave進行シミュレーション
- [ ] 再生/一時停止/停止コントロール
- [ ] 早送り機能

**検証項目**:
- [ ] プレビューでWaveが再生される
- [ ] 一時停止・再開が機能する
- [ ] 早送りで素早く確認できる

## Phase 5: その他エディタ (1週間)

### Week 7: 統合

#### 7.1 EventEditor

**タスク**:
- [ ] イベントリスト表示
- [ ] トリガー設定
- [ ] アクションシーケンス編集
- [ ] 保存・読み込み

**検証項目**:
- [ ] イベントを作成・編集できる
- [ ] トリガー条件を設定できる

#### 7.2 UIEditor

**タスク**:
- [ ] UIレイアウト一覧
- [ ] 要素配置編集
- [ ] プレビュー機能

**検証項目**:
- [ ] UIレイアウトを編集できる
- [ ] プレビューで確認できる

#### 7.3 Console・Inspector・Hierarchy

**タスク**:
- [ ] Consoleウィンドウ (既存EditorManagerから移植)
- [ ] Inspectorウィンドウ
- [ ] Hierarchyウィンドウ

**検証項目**:
- [ ] ログが正しく表示される
- [ ] エンティティを選択できる
- [ ] プロパティが表示される

## Phase 6: 最適化・仕上げ (1週間)

### Week 8: 完成

#### 8.1 パフォーマンス最適化

**タスク**:
- [ ] ファイル監視の頻度最適化
- [ ] ImGuiレンダリング最適化
- [ ] 不要な再描画を削減

**検証項目**:
- [ ] FPSが60以上維持される
- [ ] メモリリークがない
- [ ] CPU使用率が適切

#### 8.2 WebUI削除

**タスク**:
- [ ] HTTPServer.h/cpp削除
- [ ] CMakeLists.txtから削除
- [ ] cpp-httplib依存関係削除
- [ ] 関連ドキュメント更新

**検証項目**:
- [ ] ビルドが成功する
- [ ] HTTPServer関連コードが完全に削除されている

#### 8.3 ドキュメント整備

**タスク**:
- [ ] README.md更新
- [ ] 開発者モード使用ガイド作成
- [ ] スクリーンショット追加
- [ ] チュートリアル動画作成 (オプション)

**検証項目**:
- [ ] 初見ユーザーが使い方を理解できる
- [ ] すべての機能が文書化されている

## 依存関係管理

### 外部ライブラリ

既存:
- ImGui (rlImGui経由)
- Raylib
- EnTT
- nlohmann/json

追加検討:
- **ImNodes** (ステートマシンエディタ用) - オプション
  ```cmake
  FetchContent_Declare(
    imnodes
    GIT_REPOSITORY https://github.com/Nelarius/imnodes.git
    GIT_TAG v0.5
  )
  ```

### ビルド設定

**CMakeLists.txt更新箇所**:

```cmake
# 開発者モードソースファイル追加
set(DEVMODE_SOURCES
    src/Game/DevMode/DevModeManager.cpp
    src/Game/DevMode/DevModeRenderer.cpp
    src/Game/DevMode/Workspace.cpp
    src/Game/DevMode/Editors/EntityEditor.cpp
    src/Game/DevMode/Editors/StageEditor.cpp
    src/Game/DevMode/Editors/EventEditor.cpp
    src/Game/DevMode/Editors/UIEditor.cpp
)

# ホットリロードシステム
set(HOTRELOAD_SOURCES
    src/Core/HotReloadSystem.cpp
)

# SimpleTDCGame_NewArchに追加
target_sources(SimpleTDCGame_NewArch PRIVATE
    ${DEVMODE_SOURCES}
    ${HOTRELOAD_SOURCES}
)
```

## テスト計画

### 単体テスト

各コンポーネントの単体テスト:

- [ ] HotReloadSystem
  - ファイル変更検知
  - コールバック呼び出し
  
- [ ] Workspace
  - 保存・読み込み
  - JSON形式

### 統合テスト

エンドツーエンドのテスト:

- [ ] エンティティ作成 → 保存 → リロード → ゲーム反映
- [ ] ステージ編集 → プレビュー → 実際のゲーム
- [ ] ワークスペース保存 → 再起動 → 復元

### ユーザビリティテスト

レベルデザイナーによる実使用テスト:

- [ ] 直感的に操作できるか
- [ ] エラーメッセージが分かりやすいか
- [ ] ワークフローが効率的か

## リスク管理

| リスク | 影響 | 対策 |
|-------|------|------|
| ImGuiパフォーマンス低下 | 中 | 描画最適化、必要なウィンドウのみ更新 |
| ファイル監視の負荷 | 低 | 監視頻度を調整、変更検知の効率化 |
| ステートマシンエディタの複雑さ | 高 | まずはリスト形式で実装、後でビジュアル化 |
| WebUI削除による機能損失 | 中 | 事前に機能マッピング、段階的移行 |

## 成功基準

### 必須要件

- [ ] F1キーで開発者モードを起動できる
- [ ] エンティティをGUIで作成・編集できる
- [ ] ステージをGUIで作成・編集できる
- [ ] ホットリロードが機能する
- [ ] JSONファイルが正しく保存される
- [ ] WebUIが完全に削除されている

### 望ましい要件

- [ ] ビジュアルステートマシンエディタ
- [ ] アニメーションタイムライン
- [ ] リアルタイムプレビュー
- [ ] ワークスペース管理
- [ ] 詳細なエラーメッセージ

## まとめ

このロードマップに従うことで、8週間で開発者モードを完成させ、WebUIを完全に廃止できる。各フェーズは独立しており、段階的に機能を追加していける。最優先は基盤構築とホットリロードであり、その後にエディタ機能を追加していく。
