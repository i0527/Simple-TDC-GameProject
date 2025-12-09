# WebUI廃止とゲーム内エディタ統合ガイド

## 概要

このドキュメントでは、既存のWebUI（HTTPサーバー）を廃止し、ImGuiベースのゲーム内エディタに移行する方法について説明します。

## 移行の背景

### WebUIの問題点

1. **プロセス分離**: HTTPサーバーとゲームが別プロセスで動作し、同期が複雑
2. **起動の煩雑さ**: サーバーとゲームを別々に起動する必要がある
3. **配布の複雑さ**: 実行ファイルと別にWebサーバーが必要
4. **開発効率**: ブラウザとゲームウィンドウを切り替える必要がある

### ゲーム内エディタの利点

1. **統合環境**: すべてが1つのアプリケーション内で完結
2. **即時反映**: ゲーム実行中にリアルタイムで編集結果を確認
3. **ワークフロー改善**: Asepriteライクなウィンドウ配置で効率的な作業
4. **配布簡素化**: 単一の実行ファイルで完結

## 移行計画

### Phase 1: 開発者モード基盤構築

#### 1.1 開発者モードの追加

既存の `EditorManager` を拡張し、完全な開発者モードとして再構築:

**ファイル構成**:
```
include/Game/DevMode/
├── DevModeManager.h          # 開発者モード管理
├── WindowManager.h           # ウィンドウ配置管理
├── Workspace.h               # ワークスペース設定
└── Editors/
    ├── EntityEditor.h        # エンティティエディタ
    ├── StageEditor.h         # ステージエディタ
    ├── EventEditor.h         # イベントエディタ
    └── UIEditor.h            # UIエディタ

src/Game/DevMode/
├── DevModeManager.cpp
├── WindowManager.cpp
├── Workspace.cpp
└── Editors/
    ├── EntityEditor.cpp
    ├── StageEditor.cpp
    ├── EventEditor.cpp
    └── UIEditor.cpp
```

#### 1.2 RenderTextureベースのゲームビュー

ゲーム本体を独立したRenderTextureに描画し、エディタUIと分離:

```cpp
class DevModeRenderer {
public:
    void Initialize(int windowWidth, int windowHeight) {
        gameRenderer_.Initialize(windowWidth, windowHeight);
        rlImGuiSetup(true);  // ImGui初期化
    }
    
    void BeginFrame() {
        BeginDrawing();
        ClearBackground(DARKGRAY);
    }
    
    void RenderGameView() {
        // ゲーム本体をRenderTextureに描画
        gameRenderer_.BeginRender();
        // ... ゲームシーン描画 ...
        gameRenderer_.EndRender();
    }
    
    void RenderDevModeUI() {
        rlImGuiBegin();
        
        // DockSpace作成
        ImGuiID dockspace_id = ImGui::DockSpaceOverViewport();
        
        // Game Viewウィンドウ
        if (ImGui::Begin("Game View")) {
            // RenderTextureをImGuiウィンドウ内に表示
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();
            rlImGuiImageRenderTexture(&gameRenderer_.GetRenderTarget());
        }
        ImGui::End();
        
        // エディタウィンドウ
        entityEditor_.Render();
        stageEditor_.Render();
        // ...
        
        rlImGuiEnd();
    }
    
    void EndFrame() {
        EndDrawing();
    }
    
private:
    Core::GameRenderer gameRenderer_;
    Game::DevMode::EntityEditor entityEditor_;
    Game::DevMode::StageEditor stageEditor_;
    // ...
};
```

### Phase 2: HTTPServer機能の移行

#### 2.1 機能マッピング

| HTTPServer機能 | 移行先 | 備考 |
|---------------|--------|------|
| GET /characters | EntityEditor | エンティティ一覧表示 |
| POST /characters | EntityEditor | エンティティ作成 |
| PUT /characters/:id | EntityEditor | エンティティ編集 |
| DELETE /characters/:id | EntityEditor | エンティティ削除 |
| GET /stages | StageEditor | ステージ一覧表示 |
| POST /stages | StageEditor | ステージ作成 |
| PUT /stages/:id | StageEditor | ステージ編集 |
| GET /ui | UIEditor | UIレイアウト一覧 |
| POST /ui | UIEditor | UIレイアウト作成 |
| WebSocket通知 | ホットリロード | ファイル監視システム |

#### 2.2 データアクセスレイヤーの共通化

HTTPServerとエディタで共通のデータアクセス層を使用:

```cpp
namespace Data {

/**
 * @brief エンティティデータ管理
 */
class EntityDataManager {
public:
    EntityDataManager(DefinitionRegistry* registry, DefinitionLoader* loader)
        : registry_(registry), loader_(loader) {}
    
    // CRUD操作
    std::vector<CharacterDef> GetAllCharacters();
    std::optional<CharacterDef> GetCharacter(const std::string& id);
    bool CreateCharacter(const CharacterDef& def);
    bool UpdateCharacter(const std::string& id, const CharacterDef& def);
    bool DeleteCharacter(const std::string& id);
    
    // ファイル操作
    bool SaveToFile(const std::string& id);
    bool LoadFromFile(const std::string& filepath);
    
private:
    DefinitionRegistry* registry_;
    DefinitionLoader* loader_;
};

} // namespace Data
```

このクラスをHTTPServerとエディタの両方で使用することで、コードの重複を避ける。

### Phase 3: ホットリロードの実装

#### 3.1 ファイル監視システム

```cpp
namespace Core {

class HotReloadSystem {
public:
    void Initialize(const std::string& watchPath) {
        watchPath_ = watchPath;
        UpdateFileTimestamps();
    }
    
    void Update() {
        CheckFileChanges();
    }
    
    void RegisterCallback(
        const std::string& pattern,
        std::function<void(const std::string&)> callback
    ) {
        callbacks_[pattern].push_back(callback);
    }
    
private:
    std::string watchPath_;
    std::map<std::string, std::filesystem::file_time_type> fileTimes_;
    std::map<std::string, std::vector<std::function<void(const std::string&)>>> callbacks_;
    
    void UpdateFileTimestamps() {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(watchPath_)) {
            if (entry.is_regular_file()) {
                fileTimes_[entry.path().string()] = entry.last_write_time();
            }
        }
    }
    
    void CheckFileChanges() {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(watchPath_)) {
            if (!entry.is_regular_file()) continue;
            
            auto path = entry.path().string();
            auto newTime = entry.last_write_time();
            
            auto it = fileTimes_.find(path);
            if (it == fileTimes_.end()) {
                // 新規ファイル
                fileTimes_[path] = newTime;
                NotifyCallbacks(path);
            } else if (it->second != newTime) {
                // ファイル変更
                it->second = newTime;
                NotifyCallbacks(path);
            }
        }
    }
    
    void NotifyCallbacks(const std::string& filepath) {
        for (const auto& [pattern, callbacks] : callbacks_) {
            if (MatchPattern(filepath, pattern)) {
                for (const auto& callback : callbacks) {
                    callback(filepath);
                }
            }
        }
    }
    
    bool MatchPattern(const std::string& filepath, const std::string& pattern) {
        // 簡易パターンマッチング（*.char.json など）
        if (pattern.starts_with("*")) {
            return filepath.ends_with(pattern.substr(1));
        }
        return filepath.find(pattern) != std::string::npos;
    }
};

} // namespace Core
```

#### 3.2 エディタ統合

```cpp
// ゲーム初期化時
hotReload_.Initialize("assets/definitions");

hotReload_.RegisterCallback("*.char.json", [this](const std::string& path) {
    std::cout << "Reloading character: " << path << std::endl;
    
    try {
        loader_->ReloadCharacter(path);
        
        // エディタUIに通知
        if (devMode_.IsActive()) {
            devMode_.GetEntityEditor().RefreshList();
        }
        
        // ゲーム内のエンティティを更新
        UpdateLiveEntities(path);
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to reload: " << e.what() << std::endl;
    }
});

// メインループ
while (!WindowShouldClose()) {
    hotReload_.Update();
    // ...
}
```

### Phase 4: HTTPServerの段階的廃止

#### 4.1 開発モードフラグ

既存のHTTPServerを残しつつ、開発モードを選択可能にする:

```cpp
enum class DevelopmentEnvironment {
    InGameEditor,    // ゲーム内エディタ（推奨）
    WebUI,           // 旧WebUI（非推奨、互換性のため残す）
    Both             // 両方（デバッグ用）
};

class GameConfig {
public:
    DevelopmentEnvironment devEnv = DevelopmentEnvironment::InGameEditor;
    bool enableHotReload = true;
    // ...
};
```

#### 4.2 設定ファイル

`assets/config.json` で開発環境を選択:

```json
{
  "development": {
    "environment": "in_game_editor",
    "enableHotReload": true,
    "httpServer": {
      "enabled": false,
      "port": 8080
    }
  }
}
```

#### 4.3 条件付きコンパイル

最終的には、HTTPServer関連のコードを条件付きコンパイルで除外:

```cpp
#ifdef ENABLE_WEB_UI
    if (config.devEnv == DevelopmentEnvironment::WebUI) {
        httpServer_.Initialize(8080, "assets/definitions", &registry_, &loader_);
        httpServer_.Start();
    }
#endif
```

### Phase 5: 完全移行

#### 5.1 HTTPServer削除

すべてのHTTPServer関連コードを削除:

```bash
# 削除対象
include/Core/HTTPServer.h
src/Core/HTTPServer.cpp

# CMakeLists.txtから削除
# - cpp-httplib の FetchContent
# - HTTPServer.cpp のビルド設定
```

#### 5.2 ドキュメント更新

- README.mdからWebUI関連の説明を削除
- 開発者モードの使用方法を追記
- ビルド手順の簡素化

## エディタ機能実装ガイド

### EntityEditor実装例

```cpp
namespace Game::DevMode {

class EntityEditor {
public:
    void Initialize(Data::EntityDataManager* dataManager) {
        dataManager_ = dataManager;
        RefreshList();
    }
    
    void Render() {
        if (!ImGui::Begin("Entity Editor", &visible_)) {
            ImGui::End();
            return;
        }
        
        // 左: エンティティリスト
        ImGui::BeginChild("EntityList", ImVec2(250, 0), true);
        RenderEntityList();
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        // 右: エンティティ詳細
        ImGui::BeginChild("EntityDetails", ImVec2(0, 0), true);
        if (selectedEntityId_.has_value()) {
            RenderEntityDetails();
        } else {
            ImGui::Text("エンティティを選択してください");
        }
        ImGui::EndChild();
        
        ImGui::End();
    }
    
    void RefreshList() {
        entities_ = dataManager_->GetAllCharacters();
    }
    
private:
    Data::EntityDataManager* dataManager_;
    std::vector<CharacterDef> entities_;
    std::optional<std::string> selectedEntityId_;
    bool visible_ = true;
    
    void RenderEntityList() {
        if (ImGui::Button("新規作成", ImVec2(-1, 0))) {
            CreateNewEntity();
        }
        
        ImGui::Separator();
        
        for (const auto& entity : entities_) {
            bool isSelected = selectedEntityId_ == entity.id;
            if (ImGui::Selectable(entity.name.c_str(), isSelected)) {
                selectedEntityId_ = entity.id;
            }
        }
    }
    
    void RenderEntityDetails() {
        auto entity = dataManager_->GetCharacter(*selectedEntityId_);
        if (!entity) return;
        
        // 基本情報
        ImGui::Text("ID: %s", entity->id.c_str());
        
        char nameBuf[256];
        strncpy(nameBuf, entity->name.c_str(), sizeof(nameBuf));
        if (ImGui::InputText("名前", nameBuf, sizeof(nameBuf))) {
            entity->name = nameBuf;
        }
        
        // ステータス
        ImGui::SeparatorText("ステータス");
        ImGui::DragFloat("最大HP", &entity->maxHp, 1.0f, 1.0f, 10000.0f);
        ImGui::DragFloat("速度", &entity->speed, 1.0f, 0.0f, 1000.0f);
        ImGui::DragFloat("攻撃範囲", &entity->attackRange, 1.0f, 0.0f, 1000.0f);
        
        // 保存ボタン
        if (ImGui::Button("保存", ImVec2(-1, 0))) {
            dataManager_->UpdateCharacter(*selectedEntityId_, *entity);
            dataManager_->SaveToFile(*selectedEntityId_);
        }
    }
    
    void CreateNewEntity() {
        // 新規エンティティ作成ダイアログ
        ImGui::OpenPopup("新規エンティティ");
        
        if (ImGui::BeginPopupModal("新規エンティティ")) {
            static char idBuf[256] = "";
            static char nameBuf[256] = "";
            
            ImGui::InputText("ID", idBuf, sizeof(idBuf));
            ImGui::InputText("名前", nameBuf, sizeof(nameBuf));
            
            if (ImGui::Button("作成")) {
                CharacterDef newEntity;
                newEntity.id = idBuf;
                newEntity.name = nameBuf;
                // デフォルト値設定
                newEntity.maxHp = 100;
                newEntity.speed = 50;
                
                dataManager_->CreateCharacter(newEntity);
                RefreshList();
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::SameLine();
            if (ImGui::Button("キャンセル")) {
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
    }
};

} // namespace Game::DevMode
```

## ワークスペース機能

### ワークスペース保存・読み込み

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
    
    void Save(const std::string& filepath) {
        nlohmann::json j;
        j["name"] = name;
        
        for (const auto& window : windows) {
            nlohmann::json windowJson;
            windowJson["id"] = window.windowId;
            windowJson["position"] = {window.position.x, window.position.y};
            windowJson["size"] = {window.size.x, window.size.y};
            windowJson["visible"] = window.isVisible;
            j["windows"].push_back(windowJson);
        }
        
        std::ofstream file(filepath);
        file << j.dump(2);
    }
    
    void Load(const std::string& filepath) {
        std::ifstream file(filepath);
        nlohmann::json j;
        file >> j;
        
        name = j["name"];
        windows.clear();
        
        for (const auto& windowJson : j["windows"]) {
            WindowLayout layout;
            layout.windowId = windowJson["id"];
            layout.position = ImVec2(
                windowJson["position"][0],
                windowJson["position"][1]
            );
            layout.size = ImVec2(
                windowJson["size"][0],
                windowJson["size"][1]
            );
            layout.isVisible = windowJson["visible"];
            windows.push_back(layout);
        }
    }
};

} // namespace Game::DevMode
```

## 移行チェックリスト

### 開発段階

- [ ] 開発者モード基盤実装
  - [ ] DevModeManager作成
  - [ ] RenderTextureベースのゲームビュー
  - [ ] DockSpaceセットアップ
  - [ ] ワークスペース管理

- [ ] エディタウィンドウ実装
  - [ ] EntityEditor
  - [ ] StageEditor
  - [ ] EventEditor
  - [ ] UIEditor
  - [ ] Inspector
  - [ ] Console
  - [ ] Hierarchy

- [ ] ホットリロード実装
  - [ ] ファイル監視システム
  - [ ] コールバック登録機構
  - [ ] エラーハンドリング

- [ ] データアクセス層統合
  - [ ] EntityDataManager
  - [ ] StageDataManager
  - [ ] UIDataManager

### テスト段階

- [ ] 機能テスト
  - [ ] エンティティCRUD操作
  - [ ] ステージ編集
  - [ ] ホットリロード動作確認
  - [ ] プレビュー機能

- [ ] パフォーマンステスト
  - [ ] ファイル監視のオーバーヘッド
  - [ ] ImGuiレンダリング負荷

### 移行完了

- [ ] HTTPServer廃止
  - [ ] コード削除
  - [ ] CMakeLists.txt更新
  - [ ] 依存関係削除

- [ ] ドキュメント更新
  - [ ] README.md
  - [ ] DEVELOPER_MANUAL.md
  - [ ] チュートリアル追加

- [ ] 配布準備
  - [ ] 実行ファイル単体での動作確認
  - [ ] アセットファイル構成確認

## まとめ

WebUIからゲーム内エディタへの移行により、開発環境がシンプルになり、レベルデザイナーの作業効率が向上します。段階的な移行により、既存機能を維持しながら新システムに移行できます。
