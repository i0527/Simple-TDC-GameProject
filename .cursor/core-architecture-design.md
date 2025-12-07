# コアアーキテクチャ設計

> **重要**: このドキュメントは新アーキテクチャ（`include/new/`, `src/new/`）のコア層設計を定義します。

### 関連ドキュメント

- アニメーション/状態管理の詳細: `motion-animation-integration-guide.md`
- スキーマ詳細（states/effects など）: `json-schema-design.md`
- 内部エディタ仕様: `internal-editor-design.md`
- UIシステム: `ui-system-design.md`

### 推奨読書順

1. `new-arch-migration-plan.md`（全体方針）
2. `core-architecture-design.md`（コア層）
3. `json-schema-design.md`（データスキーマ）
4. `motion-animation-integration-guide.md`（アニメ/状態）
5. 以降、必要に応じて各専門設計（TD/戦闘/UI/エディタ等）

## 目的

- DI（依存性注入）パターンによる疎結合設計
- ECS（Entity Component System）の適切な統合
- データ駆動設計の実現
- 仮想FHDレンダリングとスケーリング

---

## 1. GameContext（DIコンテナ）

### 1.1 設計方針

`GameContext`は新アーキテクチャの中心的なDIコンテナとして機能します。すべてのシステムが依存するリソースを集約管理します。

### 1.2 クラス設計

```cpp
namespace New::Core {

class GameContext {
public:
    // コンストラクタ・デストラクタ
    GameContext();
    ~GameContext();
    
    // コピー禁止、ムーブ許可
    GameContext(const GameContext&) = delete;
    GameContext& operator=(const GameContext&) = delete;
    GameContext(GameContext&&) = default;
    GameContext& operator=(GameContext&&) = default;
    
    // ECSレジストリ取得
    entt::registry& GetRegistry() { return registry_; }
    const entt::registry& GetRegistry() const { return registry_; }
    
    // リソースマネージャー取得
    IResourceManager& GetResourceManager() { return *resourceManager_; }
    const IResourceManager& GetResourceManager() const { return *resourceManager_; }
    
    // 入力マネージャー取得
    IInputManager& GetInputManager() { return *inputManager_; }
    const IInputManager& GetInputManager() const { return *inputManager_; }
    
    // データレジストリ取得
    Data::DefinitionRegistry& GetDefinitionRegistry() { return *definitionRegistry_; }
    const Data::DefinitionRegistry& GetDefinitionRegistry() const { return *definitionRegistry_; }

    // アニメーションレジストリ取得（パーツアニメーション）
    Animation::AnimationRegistryCache& GetAnimationRegistryCache() { return *animationRegistryCache_; }
    const Animation::AnimationRegistryCache& GetAnimationRegistryCache() const { return *animationRegistryCache_; }
    
    // ゲームレンダラー取得
    GameRenderer& GetRenderer() { return *renderer_; }
    const GameRenderer& GetRenderer() const { return *renderer_; }
    
    // システムランナー取得
    SystemRunner& GetSystemRunner() { return *systemRunner_; }
    const SystemRunner& GetSystemRunner() const { return *systemRunner_; }
    
    // 初期化
    bool Initialize();
    void Shutdown();
    
    // 状態確認
    bool IsInitialized() const { return initialized_; }

private:
    bool initialized_ = false;
    
    // ECSレジストリ（所有）
    entt::registry registry_;
    
    // リソース管理（所有）
    std::unique_ptr<IResourceManager> resourceManager_;
    std::unique_ptr<IInputManager> inputManager_;
    
    // データ管理（所有）
    std::unique_ptr<Data::DefinitionRegistry> definitionRegistry_;

    // アニメーション管理（所有: assets/motions/* と states.json をキャッシュ）
    std::unique_ptr<Animation::AnimationRegistryCache> animationRegistryCache_;
    
    // レンダリング（所有）
    std::unique_ptr<GameRenderer> renderer_;
    
    // システム実行（所有）
    std::unique_ptr<SystemRunner> systemRunner_;
};

} // namespace New::Core
```

### 1.3 所有関係と責務

- `GameContext` は `entt::registry` を所有する唯一の場所。ライフタイムは GameContext に従う。
- `World` は `registry` への参照を保持する薄いラッパーで、名前付きエンティティ管理・ビュー/グループ作成・イベント仲介を担う。所有権は持たない。
- `Initialize()` で `World` を生成し、`GetWorld()` で提供する（`unique_ptr<World>` を保持）。
- システムは原則 `World` 経由で ECS を扱い、直接 `registry` に触れるのは特殊ケース（デバッグ/検証）に限定する。

### 1.4 利用指針

- エンティティ生成/破棄・名前解決・ビュー取得は `World` API を使用する。
- 既存コードで `GetRegistry()` を直接使用している箇所は、可能な限り `GetWorld()` に置き換える。
- `World` の追加責務（例: イベントディスパッチ）は実装と同期して拡張する。

### 1.3 インターフェース定義

```cpp
namespace New::Core {

// リソースマネージャーインターフェース
class IResourceManager {
public:
    virtual ~IResourceManager() = default;
    
    // テクスチャ管理
    virtual Texture2D GetTexture(const std::string& path) = 0;
    virtual void UnloadTexture(const std::string& path) = 0;
    
    // フォント管理
    virtual Font GetFont(const std::string& path, int size) = 0;
    virtual void UnloadFont(const std::string& path, int size) = 0;
    
    // サウンド管理
    virtual Sound GetSound(const std::string& path) = 0;
    virtual void UnloadSound(const std::string& path) = 0;
    
    // クリーンアップ
    virtual void ClearAll() = 0;
};

// 入力マネージャーインターフェース
class IInputManager {
public:
    virtual ~IInputManager() = default;
    
    // キー入力
    virtual bool IsKeyPressed(int key) = 0;
    virtual bool IsKeyDown(int key) = 0;
    virtual bool IsKeyReleased(int key) = 0;
    
    // マウス入力（仮想座標変換済み）
    virtual Vector2 GetMousePosition() = 0;
    virtual bool IsMouseButtonPressed(int button) = 0;
    
    // 入力更新
    virtual void Update() = 0;
};

} // namespace New::Core
```

---

## 2. World（ECS世界）

### 2.1 設計方針

`World`はECSの世界を表し、エンティティの作成・削除・クエリを提供します。`GameContext`のレジストリをラップして、より高レベルなAPIを提供します。

### 2.2 クラス設計

```cpp
namespace New::Core {

class World {
public:
    explicit World(entt::registry& registry);
    ~World() = default;
    
    // エンティティ作成
    entt::entity CreateEntity();
    entt::entity CreateEntity(const std::string& name);
    
    // エンティティ削除
    void DestroyEntity(entt::entity entity);
    
    // エンティティ名管理
    void SetEntityName(entt::entity entity, const std::string& name);
    std::string GetEntityName(entt::entity entity) const;
    entt::entity FindEntityByName(const std::string& name) const;
    
    // コンポーネント操作
    template<typename Component>
    Component& AddComponent(entt::entity entity, Component&& component);
    
    template<typename Component>
    Component& GetComponent(entt::entity entity);
    
    template<typename Component>
    bool HasComponent(entt::entity entity) const;
    
    template<typename Component>
    void RemoveComponent(entt::entity entity);
    
    // ビュー取得
    template<typename... Components>
    auto View();
    
    template<typename... Components>
    auto View() const;
    
    // グループ取得（注意: ネスト不可）
    template<typename... Get, typename... Exclude>
    auto Group(entt::get_t<Get...> get, entt::exclude_t<Exclude...> exclude = {});
    
    // レジストリ直接アクセス（必要時のみ）
    entt::registry& GetRegistry() { return registry_; }
    const entt::registry& GetRegistry() const { return registry_; }

private:
    entt::registry& registry_;
    
    // エンティティ名マッピング
    std::unordered_map<std::string, entt::entity> nameToEntity_;
    std::unordered_map<entt::entity, std::string> entityToName_;
};

} // namespace New::Core
```

### 2.3 使用例

```cpp
// Worldの使用例
auto& world = gameContext.GetWorld();
auto entity = world.CreateEntity("Player");

// コンポーネント追加
world.AddComponent<Transform>(entity, Transform{100.0f, 200.0f});
world.AddComponent<Sprite>(entity, Sprite{"player.png"});

// ビューでイテレート
auto view = world.View<Transform, Sprite>();
for (auto entity : view) {
    auto& transform = view.get<Transform>(entity);
    auto& sprite = view.get<Sprite>(entity);
    // 処理...
}
```

---

## 3. GameRenderer（仮想FHDレンダリング）

### 3.1 設計方針

`GameRenderer`は仮想解像度（1920x1080）でゲームを描画し、実際のウィンドウサイズにスケーリングします。内部エディタでは`RenderTexture`として使用されます。

### 3.2 クラス設計

```cpp
namespace New::Core {

class GameRenderer {
public:
    // コンストラクタ・デストラクタ
    GameRenderer();
    ~GameRenderer();
    
    // 初期化・終了
    bool Initialize(int virtualWidth = 1920, int virtualHeight = 1080);
    void Shutdown();
    
    // レンダリング開始・終了
    void BeginRender();
    void EndRender();
    
    // 仮想座標変換
    Vector2 ScreenToVirtual(Vector2 screenPos) const;
    Vector2 VirtualToScreen(Vector2 virtualPos) const;
    
    // スケール取得
    float GetScaleX() const { return scaleX_; }
    float GetScaleY() const { return scaleY_; }
    
    // 仮想解像度取得
    int GetVirtualWidth() const { return virtualWidth_; }
    int GetVirtualHeight() const { return virtualHeight_; }
    
    // 実ウィンドウ解像度取得
    int GetScreenWidth() const;
    int GetScreenHeight() const;
    
    // RenderTexture取得（内部エディタ用）
    RenderTexture2D& GetRenderTarget() { return renderTarget_; }
    const RenderTexture2D& GetRenderTarget() const { return renderTarget_; }
    
    // スケーリング描画（実ウィンドウに描画）
    void RenderScaled();
    
    // クリア
    void Clear(Color color);

private:
    bool initialized_ = false;
    
    // 仮想解像度
    int virtualWidth_ = 1920;
    int virtualHeight_ = 1080;
    
    // スケール
    float scaleX_ = 1.0f;
    float scaleY_ = 1.0f;
    
    // レンダーターゲット（仮想解像度）
    RenderTexture2D renderTarget_ = {0};
    
    // スケール計算
    void CalculateScale();
};

} // namespace New::Core
```

### 3.3 実装の詳細

```cpp
// GameRenderer.cpp の実装例
void GameRenderer::BeginRender() {
    BeginTextureMode(renderTarget_);
    ClearBackground(BLACK);
}

void GameRenderer::EndRender() {
    EndTextureMode();
}

Vector2 GameRenderer::ScreenToVirtual(Vector2 screenPos) const {
    return {
        screenPos.x / scaleX_,
        screenPos.y / scaleY_
    };
}

Vector2 GameRenderer::VirtualToScreen(Vector2 virtualPos) const {
    return {
        virtualPos.x * scaleX_,
        virtualPos.y * scaleY_
    };
}

void GameRenderer::RenderScaled() {
    // 実ウィンドウにスケーリング描画
    Rectangle sourceRect = {0, 0, (float)virtualWidth_, -(float)virtualHeight_};
    Rectangle destRect = {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()};
    DrawTexturePro(renderTarget_.texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
}
```

---

## 4. SystemRunner（システム実行管理）

### 4.1 設計方針

`SystemRunner`はすべてのシステムを登録・管理し、適切な順序で実行します。システム間の依存関係を考慮した実行順序を保証します。

### 4.2 クラス設計

```cpp
namespace New::Core {

// システム基底インターフェース
class ISystem {
public:
    virtual ~ISystem() = default;
    
    // 初期化・終了
    virtual bool Initialize(GameContext& context) { return true; }
    virtual void Shutdown() {}
    
    // 更新（毎フレーム）
    virtual void Update(GameContext& context, float deltaTime) = 0;
    
    // 描画（毎フレーム）
    virtual void Render(GameContext& context) {}
    
    // 実行優先度（小さいほど先に実行）
    virtual int GetUpdatePriority() const { return 100; }
    virtual int GetRenderPriority() const { return 100; }
    
    // 有効/無効
    virtual void SetEnabled(bool enabled) { enabled_ = enabled; }
    virtual bool IsEnabled() const { return enabled_; }

protected:
    bool enabled_ = true;
};

// システムランナー
class SystemRunner {
public:
    SystemRunner();
    ~SystemRunner();
    
    // システム登録
    void RegisterSystem(std::unique_ptr<ISystem> system);
    
    // 初期化
    bool Initialize(GameContext& context);
    void Shutdown();
    
    // 実行
    void Update(GameContext& context, float deltaTime);
    void Render(GameContext& context);
    
    // システム取得
    template<typename T>
    T* GetSystem();
    
    // 有効/無効
    void SetSystemEnabled(const std::string& name, bool enabled);

private:
    std::vector<std::unique_ptr<ISystem>> systems_;
    std::unordered_map<std::string, ISystem*> systemMap_;
    
    // 優先度でソート
    void SortSystems();
};

} // namespace New::Core
```

### 4.3 システム実装例

```cpp
// システム実装例
namespace New::Game::Systems {

class RenderSystem : public Core::ISystem {
public:
    void Update(Core::GameContext& context, float deltaTime) override {
        // 更新処理（必要時のみ）
    }
    
    void Render(Core::GameContext& context) override {
        auto& world = context.GetWorld();
        auto& renderer = context.GetRenderer();
        
        renderer.BeginRender();
        
        // スプライト描画
        auto view = world.View<Transform, Sprite>();
        for (auto entity : view) {
            auto& transform = view.get<Transform>(entity);
            auto& sprite = view.get<Sprite>(entity);
            
            DrawTexture(sprite.texture, (int)transform.x, (int)transform.y, WHITE);
        }
        
        renderer.EndRender();
        renderer.RenderScaled(); // 実ウィンドウに描画
    }
    
    int GetRenderPriority() const override { return 50; }
};

} // namespace New::Game::Systems
```

---

## シーン/状態管理の方針（軽量）

- 役割とAPI: `IScene` は `Enter/Exit/Update/Render/OnHotReload` を持ち、`SceneManager` が push/pop/replace と `pendingTransition` を適用する。Enter/Exit でリソース境界を確保し、UIセット/解除・BGM/SE 切替・入力バインド再設定を統一する。
- シーン粒度: タイトル / ホーム / ステージ選択 / 戦闘 / リザルト / 設定に限定。戻り先は SceneManager がスタックで保持し、遷移トリガ（UI/ゲーム内イベント/エディタ操作）を列挙して受け付ける。
- GameState 分離: 戦闘内の `GameState` は `Loading/Ready/Playing/Paused/Victory/Defeat` を `GameStateSystem` で管理し、シーン遷移と分離。`pendingTransition` は原則 Update 後に適用し、例外でエラー/緊急停止のみ Update 前に即時適用（優先度: Critical > System > User）。
- 連動ポイント: 状態エントリで UI レイヤ切替・入力マップ切替・サウンドキュー再生・システム停止/再開（例: Paused で AI/Spawn 停止、Render/UITick は継続）。
- シーンの境界: シーン責務は「リソース・UI・BGM・入力マップの切替＋SystemRunner再構成」に限定し、ゲーム進行ロジックは状態マシンに閉じ込める。戦闘シーン内の遷移は `GameStateSystem` のみが扱い、シーンは終端通知を受けて次シーン要求を出すだけとする。
- SystemRunner 再構成: 戦闘シーン Enter 時のみ再登録を許可し、その他シーンはシステム固定（UI差し替えのみ）。`TDGameMode(LineTD/MapTD)` 切替は戦闘シーンを Exit→Enter させて反映し、ホットリロードと競合しないようにする。
- 遷移フロー: ホーム → ステージ選択 → ロード(Loading) → Ready → Playing → Victory/Defeat → リザルト → 戻る/リトライ。各遷移で `StageLoader` + `MapValidator`/`ReferenceValidator` を呼び、ステージ定義・編成・設定を受け渡す。
- HotReload/検証: 手順を統一（1)ファイル監視→2)ローカル検証→3)ゲーム側再検証→4)適用→5)OnHotReload呼び出し、失敗時は4–5をスキップして旧キャッシュ維持）。`OnHotReload` は現在シーンに通知し、安全側へフェイル（Paused 等）させる。
- エディタ常駐: F1〜F4 エディタは常駐し、現在シーン/状態を参照してプレビュー対象と入力バインドを再設定する。シーン切替時にプレビュー更新とホットリロード対象の再選択を行う。
- デフォルト動作の割り切り: BGM/SE はデフォルト短フェード切替（ジングル優先時はフェードアウト）。リソースプリロードは必要最小限のみ。
- SceneArgs/戻り先: 戻り先はすべて明示指定（スタック不使用）。`SceneArgs` に `returnTo` を必須で持たせ、設定・図鑑も呼び出し元を明示する。
- 入力優先度: エディタ > UI > ゲーム入力で固定（前提）。デバッグ用グローバルショートカットのみ例外。ポーズ中は戦闘入力を無効化し UI のみ許可。
- UIレイアウト: シーンごとにロードするレイアウトを一覧管理し、戦闘HUDのみ常駐（解放しない別レイヤ）。他シーンUIは切替時に差し替え。切替は原則即時、フェードは任意の後付け。
- BGM/サウンド: シーン単位で BGM ID を選択し、既定は即時切替。短フェードはオプション。戦闘終端（Victory/Defeat）はジングル再生＋BGMフェードアウトを既定動作とする。
- リソースライフサイクル: 戦闘 Enter でステージ定義・敵/タワーアセット・BGM/SE・戦闘UIをロードし、Exit で解放。ホーム/ステージ選択は共通キャッシュを再利用し、定義とUIは保持、戦闘専用リソースのみ都度解放。
- エラー/フォールバック: ステージロード/スキーマ検証失敗時はステージ選択へ戻し、エラー表示。BGM欠落はデフォルトトラック、テクスチャ欠落はプレースホルダーを使用。
- Dev/Editor連携: シーン切替時に `sceneId`, `gameState`, `stageId`, `tdMode` をエディタへ通知。戦闘中のホットリロードはステージ定義のみ許可、編成は次戦闘から適用。
- Telemetry/ログ: 遷移ログ（from→to, trigger, result/理由）と GameState 終端理由を SceneManager / GameStateSystem で記録。HotReload の適用/失敗ログは共通ロガーへ出力。
- パフォーマンス/制約: 戦闘→リザルト遷移の許容時間は短時間（キャッシュ前提、再ロードは避ける）。SystemRunner 再構成は 1 フレ未満を目標（必要なら事前プリロードで隠す）。

### 遷移ログ/デバッグUI出力（最小セット）

- 記録項目: from, to, trigger, result, reason, duration、GameState 終端理由（Victory/Defeat/Abort）。
- 出力先: 既定はデバッグUI（ログパネル）に表示。ファイル保存は後回しとし、必要に応じテレメトリ送信を追加。

---

## 5. データ層設計

### 5.1 DefinitionRegistry（定義レジストリ）

```cpp
namespace New::Data {

class DefinitionRegistry {
public:
    // エンティティ定義
    void RegisterEntity(const std::string& id, const EntityDef& def);
    const EntityDef* GetEntity(const std::string& id) const;
    
    // 波定義
    void RegisterWave(const std::string& id, const WaveDef& def);
    const WaveDef* GetWave(const std::string& id) const;
    
    // 能力定義
    void RegisterAbility(const std::string& id, const AbilityDef& def);
    const AbilityDef* GetAbility(const std::string& id) const;
    
    // UIレイアウト定義
    void RegisterUILayout(const std::string& id, const UILayoutDef& def);
    const UILayoutDef* GetUILayout(const std::string& id) const;
    
    // クリア
    void Clear();

private:
    std::unordered_map<std::string, EntityDef> entities_;
    std::unordered_map<std::string, WaveDef> waves_;
    std::unordered_map<std::string, AbilityDef> abilities_;
    std::unordered_map<std::string, UILayoutDef> uiLayouts_;
};

} // namespace New::Data
```

### 5.2 DataLoader（データローダー）

```cpp
namespace New::Data::Loaders {

class DataLoaderBase {
public:
    virtual ~DataLoaderBase() = default;
    
    // JSONファイル読み込み
    virtual bool LoadFromFile(const std::string& path) = 0;
    
    // JSON文字列読み込み
    virtual bool LoadFromString(const std::string& json) = 0;
    
    // エラー取得
    std::string GetLastError() const { return lastError_; }

protected:
    std::string lastError_;
    
    // JSON解析（エラーハンドリング付き）
    bool ParseJSON(const std::string& json, nlohmann::json& out);
};

class EntityLoader : public DataLoaderBase {
public:
    bool LoadFromFile(const std::string& path) override;
    bool LoadFromString(const std::string& json) override;
    
    // 定義取得
    std::vector<EntityDef> GetDefinitions() const { return definitions_; }
    
    // レジストリに登録
    void RegisterTo(DefinitionRegistry& registry);

private:
    std::vector<EntityDef> definitions_;
    
    // JSONから定義に変換
    bool ParseEntityDef(const nlohmann::json& json, EntityDef& def);
};

} // namespace New::Data::Loaders
```

### 5.3 SchemaValidator（JSONスキーマバリデータ）

```cpp
namespace New::Data::Validators {

class SchemaValidator {
public:
    // スキーマ定義読み込み
    bool LoadSchema(const std::string& schemaPath);
    
    // JSON検証
    bool Validate(const nlohmann::json& json, const std::string& schemaName);
    
    // エラー取得
    std::vector<std::string> GetErrors() const { return errors_; }

private:
    std::unordered_map<std::string, nlohmann::json> schemas_;
    std::vector<std::string> errors_;
    
    // スキーマ検証ロジック
    bool ValidateObject(const nlohmann::json& json, const nlohmann::json& schema, const std::string& path);
};

} // namespace New::Data::Validators
```

- 責務分離メモ: バリデータは構造・型・必須キーの検証に限定し、UI/TD/アニメのビヘイビア解決は各Registry（例: `UIBehaviorRegistry`）側で行う。UIの場合は **スキーマ検証 → ネイティブ変換（マッピング） → バインディング/イベント通知** の順でローダが処理する。

### 5.4 AnimationRegistryCache（パーツアニメーション＋状態テーブル）

- 役割: `assets/motions/<character>.json` と `assets/characters/<character>/states.json` を読み込み、キャラクターIDをキーにキャッシュする。
- 構成:
  - `AnimationRegistry`（クリップ群＋テクスチャ）  
  - `StateDefinition`（状態テーブル、トリガー、イベント、割込み可否、onComplete）
- API例:

```cpp
class AnimationRegistryCache {
public:
    bool LoadCharacter(const std::string& characterId,
                       const std::string& motionPath,
                       const std::string& statePath);
    const AnimationRegistry* GetRegistry(const std::string& characterId) const;
    const StateDefinition* GetStateDef(const std::string& characterId) const;
    void ReloadAll(); // HotReloadSystem から呼ばれる
};
```

- DI: `GameContext` が所有し、`StateSystem` / `AnimationPlaybackSystem` / Editor から参照。
- HotReload: `HotReloadSystem` が motion/state のパスを監視し、変更時に `ReloadAll()` を呼ぶ。
- キャッシュ戦略:
  - `registries_`/`stateDefs_` を characterId で保持。共有パーツテクスチャはパスをキーにプールし、複数キャラで再利用。
  - リロード検証: `ValidateStatesDef` で `clips` の存在をチェックし、矛盾があれば前回キャッシュにロールバック。

### 5.5 HotReloadSystem（監視と復旧）

- 役割: ファイル監視、差分検出、指定フックでリロードを実行。
- API:

```cpp
class HotReloadSystem {
public:
    bool Initialize(const std::string& assetsPath);
    void RegisterHook(const std::string& pattern,
                      std::function<bool(const std::string&)> onFileChanged);
    void Update(float dt);
    struct ReloadResult {
        bool success;
        std::string errorMessage;
        std::string filePath;
        bool recoverable; // キャッシュに戻せるか
    };
    std::vector<ReloadResult> GetRecentErrors() const;
};
```

- リロード失敗時は `recoverable=true` の場合キャッシュを維持し、DevMode Validationパネルへエラーを通知。

### 5.6 監視対象パス（推奨）

- `assets/new/definitions/entities/*.json`
- `assets/new/definitions/waves/*.json`
- `assets/new/definitions/abilities/*.json`
- `assets/new/definitions/ui/*.json`（UIレイアウト定義を監視する場合）
- `assets/new/definitions/stages/*.json`（ステージ定義を監視する場合）
- `assets/motions/*.json`
- `assets/characters/*/states.json`
- `assets/effects/*.json`

**実装メモ**: UIローダ/バリデータが未実装の場合は、先に実装してから監視対象に含める。

**HotReload / Validation フロー（共通方針）**  

- 監視（`HotReloadSystem` へパス登録）  
- 検証（`SchemaValidator`）  
- ローダ（ネイティブ構造体へ変換してキャッシュ更新）  
- 適用 / ロールバック（失敗時は前回キャッシュ保持、部分的に検証成功したデータのみ反映も可）  
- DevMode通知（エラー・差分をエディタに表示）

**テスト方針（自動回帰）**  

- スキーマ＋ローダの回帰テストを追加し、「JSON → ネイティブ構造体」変換までを自動検証する。UI/TD/アニメ/エフェクトの代表ケースとバリデーション失敗ケースを含める。

**DevMode 運用範囲**  

- ライブ編集対象: UI レイアウト、モーション、ウェーブ（TD）の JSON。検証失敗は警告表示、適用は検証成功部分のみ。致命エラー時はロールバック維持。

**データバージョニング方針**  

- `schemaVersion` は設けず最新版前提で運用する（バージョン分岐はコード側で不要）。破壊的変更時はスキーマとローダを同時更新し、HotReload テストで検証する。

## ログ/テレメトリ/計測ポリシー

- ログレベル: TRACE/DEBUG/INFO/WARN/ERROR。DevModeはDEBUG以上、リリースはINFO以上を出力。  
- 出力先: コンソール（DevMode）、ローリングファイル（例: `logs/game_YYYYMMDD.log`, 最大10MB×5）。フォーマット `[HH:MM:SS.mmm][LEVEL][System] message`。  
- FPS/フレーム予算: `FrameStats` で Update/Render の所要時間を計測し、目標 60FPS（Update≒12ms/Render≒8ms）。Devパネルとログにスナップショットを表示。  
- テレメトリ: イベントキューに `boss_spawned` / `wave_started` / `wave_finished` / `player_hit` / `player_down` / `victory` / `defeat` / `timescale_changed` を送信し、DevModeオーバーレイとログに出す（リリースはサンプリング可）。

## 設定の即時反映ポリシー（簡易）

- 即時反映: 音量/UIスケール/字幕強度/色覚・コントラスト/フォントサイズ。  
- 再初期化または次起動: 解像度/フルスクリーン/VSync/FPS上限（可能ならレンダラー再初期化で即時反映、不可なら次起動まで保留）。  
- 保存: 変更時に即時保存し、終了時にも冗長保存。

## HotReload 適用順（簡潔）

- 監視（`HotReloadSystem`）→ スキーマ検証 → ネイティブ変換 → 適用 / 部分適用（成功分のみ） → ロールバック（致命/検証失敗時は前回キャッシュ保持） → 通知（DevModeトースト＋ログ）。

## テスト戦略（簡易スモーク）

- スキーマ/ローダ回帰: JSON→構造体変換（entities/ui_layout/stage/party presets/settings）。未知キー無視・欠落補完と致命エラー扱いを確認。  
- 編成/ステージスモーク: HotReloadでプリセット(main3+sub5+abilities2)とステージ定義が適用されるか、部分失敗時にキャッシュ維持されるかを確認。  
- 設定フォールバック: 解像度/フルスクリーン/VSync/FPS上限の再初期化失敗時に元設定へ戻し警告するかを確認。  
- Devツール簡易チェック: HotReloadの partial/error 通知、トースト重複抑制、timescale変更イベントのログ出力を確認。

## ビルドフラグ/起動オプション方針（セキュリティ/デバッグ）

- ビルドフラグ例: `DEV_BUILD`, `DEBUG_LOG`, `CHEAT_CMDS`。  
- リリースビルドではデバッグUI/チートコマンド/詳細ログを無効化し、`CHEAT_CMDS` をオフに固定。  
- 起動オプションでのデバッグ有効化は Dev/Debug ビルドのみに限定する。

## HotReload 履歴とエラー表示

- 履歴保持: 直近 50 件または 10 分をメモリ保持。  
- 表示フォーマット: `[HH:MM:SS] <path> <status> <summary>`（status = ok/reloaded/warn/error/partial）。  
- エラー時: 検証エラーを最上位に表示し、部分適用可なら "partial" を付与。DevModeパネルとログ両方に同一フォーマットで記録。

---

## 6. 初期化フロー

### 6.1 初期化順序

```cpp
// main_new.cpp の初期化例
int main() {
    // 1. Raylib初期化
    InitWindow(1920, 1080, "Simple TDC Game - New Architecture");
    InitAudioDevice();
    SetTargetFPS(60);
    
    // 2. GameContext作成
    New::Core::GameContext context;
    if (!context.Initialize()) {
        std::cerr << "Failed to initialize GameContext" << std::endl;
        return -1;
    }
    
    // 3. データ読み込み
    New::Data::Loaders::EntityLoader entityLoader;
    if (!entityLoader.LoadFromFile("assets/new/definitions/entities/player.json")) {
        std::cerr << "Failed to load entities" << std::endl;
    }
    entityLoader.RegisterTo(context.GetDefinitionRegistry());
    
    // 4. システム登録
    auto& systemRunner = context.GetSystemRunner();
    systemRunner.RegisterSystem(std::make_unique<New::Game::Systems::InputSystem>());
    systemRunner.RegisterSystem(std::make_unique<New::Game::Systems::MovementSystem>());
    systemRunner.RegisterSystem(std::make_unique<New::Game::Systems::RenderSystem>());
    
    if (!systemRunner.Initialize(context)) {
        std::cerr << "Failed to initialize systems" << std::endl;
        return -1;
    }
    
    // 5. メインループ
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // 更新
        systemRunner.Update(context, deltaTime);
        
        // 描画
        BeginDrawing();
        {
            ClearBackground(BLACK);
            systemRunner.Render(context);
        }
        EndDrawing();
    }
    
    // 6. クリーンアップ
    systemRunner.Shutdown();
    context.Shutdown();
    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}
```

---

## 7. 実装チェックリスト

### 7.1 GameContext実装

- [ ] `GameContext`クラス実装
- [ ] `IResourceManager`インターフェース定義
- [ ] `ResourceManager`実装
- [ ] `IInputManager`インターフェース定義
- [ ] `InputManager`実装（仮想座標変換対応）

### 7.2 World実装

- [ ] `World`クラス実装
- [ ] エンティティ名管理機能
- [ ] コンポーネント操作メソッド
- [ ] ビュー・グループ取得メソッド

### 7.3 GameRenderer実装

- [ ] `GameRenderer`クラス実装
- [ ] 仮想解像度レンダリング
- [ ] 座標変換機能
- [ ] スケーリング描画

### 7.4 SystemRunner実装

- [ ] `ISystem`インターフェース定義
- [ ] `SystemRunner`クラス実装
- [ ] 優先度ベースの実行順序
- [ ] システム登録・取得機能

### 7.5 データ層実装

- [ ] `DefinitionRegistry`実装
- [ ] `DataLoaderBase`実装
- [ ] `EntityLoader`実装
- [ ] `SchemaValidator`実装

---

## 8. パフォーマンス要件（目安）

- フレームレート: 目標60FPS、下限50FPS（デバッグ時）。
- フレーム予算: Update ≈12ms / Render ≈8ms / Overhead ≈3ms。
- メモリ: Textureキャッシュ <100MB、同時エンティティ ~1000体、AnimationRegistry per キャラ 2–5MB、総目安 <500MB。
- DrawCalls: 目標 <200/frame。パーツバッチャで zOrder ソート後、テクスチャ単位グルーピング。
- レンダリング注意: Raylib RenderTexture はY反転、`sourceRect.height` を負にして補正。
- ECS注意: EnTTのグループは入れ子不可。複合クエリは `view` を使用。

---

## 9. 実装ロードマップ（推奨順）

1) Core基盤: `GameContext` / `World` / `GameRenderer` / `SystemRunner`
2) Data層: `DefinitionRegistry` + ローダ群（Entity/Wave/Ability/UI + `SchemaValidator`）
3) Animation: `AnimationRegistry` + `AnimationRegistryCache`（motions/states ローダ込み）
4) 基本システム: Input / Movement / Render（仮想FHD）
5) 状態管理: `StateSystem` + `AnimationPlaybackSystem` + `AnimationEventSystem`
6) Dev/Editor: `DevModeManager` + 内部エディタ（特に MotionEditor）

---

## 10. 参考資料

- [設計原則と制約](design-principles-and-constraints.md)
- [ディレクトリ構成とビルドターゲット](new-arch-dirs-and-targets.md)
- [ライブラリ注意点ガイド](libs_guide.md)

---

**作成日**: 2025-12-06  
**バージョン**: 1.0  
**対象**: 新アーキテクチャ開発者向け
