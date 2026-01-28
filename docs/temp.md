# Cat Tower Defense - 開発ドキュメント

このドキュメントは、プロジェクトのアーキテクチャ、設計、実装についての詳細な説明をまとめたものです。

---

## 目次

1. [.cursorで設定されているルール](#1-cursorで設定されているルール)
2. [プロジェクトのアーキテクチャ](#2-プロジェクトのアーキテクチャ)
3. [API層について](#3-api層について)
4. [ゲームシステムの設計と実装](#4-ゲームシステムの設計と実装)
5. [データ志向設計の設計と実装](#5-データ志向設計の設計と実装)
6. [デバッグAPI機能の統合](#6-デバッグapi機能の統合)
7. [処理効率化・UI改善](#7-処理効率化ui改善)

---

## 1. .cursorで設定されているルール

### 1.1. メインの開発ルール (`.cursorrules`)

プロジェクトルートの`.cursorrules`に主要な開発ルールが記載されています。

#### プロジェクト情報
- 言語: C++17
- アーキテクチャ: ECS (EnTT)
- ランタイム: Raylib 5.x + ImGui
- ビルド: CMake + Visual Studio 2022
- 解像度: 1920x1080固定
- フォント: `assets/fonts/NotoSansJP-Medium.ttf`

#### 命名規約
- クラス/構造体: PascalCase (`GameManager`)
- 関数/メソッド: PascalCase (`UpdatePosition`)
- 変数: camelCase、プライベートは末尾にアンダースコア (`playerSpeed`, `registry_`)
- 定数: UPPER_CASE (`MAX_ENTITIES`)
- 名前空間: PascalCase (`Components`, `Systems`)

#### コーディング規約
- ヘッダーは`#pragma once`を使用
- ECS設計: コンポーネントはPOD（データのみ）、ロジックはシステムに集約
- JSONパースは必ずtry-catchで囲む
- ディレクトリ構造: ヘッダーとソースは同じディレクトリに配置

#### 絶対禁止パターン
- グローバル変数（代わりにDI/ServiceContainer使用）
- new/delete（代わりにスマートポインタ）
- 複雑なポリモーフィズム（代わりにECSのComponent + Tag）
- 循環参照（代わりにweak_ptrまたは参照）
- Raylibリソースの多重削除（代わりにshared_ptr + カスタムデリータ）

#### ログパターン
- `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`, `LOG_DEBUG`マクロを使用
- spdlogのfmtスタイルでフォーマット

#### ビルド方法
- 必ず`tools/`ディレクトリのスクリプトを使用
- PowerShell: `.\tools\build.ps1`
- コマンドプロンプト: `tools\build_debug.bat`

### 1.2. GitHub Copilot指示 (`.github/copilot-instructions.md`)

GitHub Copilot向けの補足規約が記載されています。

- ECSコンポーネントはPOD型
- JSON解析はtry-catch必須
- コミット規約（feat:, fix:, refactor:など）
- エラーハンドリングとメモリ管理（スマートポインタ推奨）

### 1.3. ユーザールール
- Always respond in Japanese（常に日本語で応答）

---

## 2. プロジェクトのアーキテクチャ

### 2.1. 全体像

**Cat Tower Defense**は、**ECS（EnTT）**を中心とした**C++17**ゲームです。  
Raylib + ImGuiで描画・UIを行い、**シーン＋オーバーレイ**で画面遷移を管理する構成になっています。

```
main.cpp
    └── GameSystem（エントリ・メインループ）
            ├── API層（BaseSystemAPI, ECSystemAPI, UI, Audio, 戦闘関連...）
            ├── SharedContext（全モジュール共有のコンテキスト）
            ├── シーン（Init → Title → Home / Game / Editor）
            └── OverlayManager（オーバーレイのスタック管理）
```

### 2.2. エントリとメインループ

**`game/main/main.cpp`**

```cpp
GameSystem system;
system.Initialize();  // 各種API・シーン・オーバーレイ初期化
system.Run();         // メインループ（Update → Render）
system.Shutdown();
```

### 2.3. GameSystemの役割

**`game/core/system/GameSystem`**

- **責務**
  - アプリケーション全体の初期化・終了管理
  - メインループとFPS制御
  - **各種APIの所有・初期化**（BaseSystemAPI, ECSystemAPI, Input, UI, Audio, 戦闘系など）
  - **SharedContext**の構築と共有
  - **シーン遷移**（Init → Title → Home / Game / Editor）
  - **OverlayManager**の統合（オーバーレイからの遷移・終了リクエストの処理）

### 2.4. API層

**`game/core/api/`**

- **BaseSystemAPI**: Raylib統合API（リソース・描画・オーディオ管理）
- **ECSystemAPI**: EnTTレジストリのラッパー（エンティティ・コンポーネント操作）
- **InputSystemAPI**: 入力統合API
- **UISystemAPI**: UI共通API
- **AudioControlAPI**: オーディオ制御API
- **SceneOverlayControlAPI**: シーン/オーバーレイ制御API
- **BattleProgressAPI / BattleSetupAPI / SetupAPI / GameplayDataAPI / DebugUIAPI**: 戦闘・セットアップ・ゲームプレイデータ・デバッグUIなど

### 2.5. SharedContext

**`game/core/config/SharedContext.hpp`**

- **役割**: モジュール間で共有する「現在のコンテキスト」
- **主な内容**
  - 各種APIへのポインタ（Base, Input, EC, Audio, UI, SceneOverlay, Battle系, Debugなど）
  - 現在ステージID、編成データ、戦闘セットアップデータ
  - **GameState**（Initializing / Title / Home / Game / Editor）
  - デルタタイム、一時停止、終了リクエスト等

### 2.6. シーン（Scenes）

**`game/core/states/`**

- **IScene**: 各シーンが実装するインターフェース
- **実装例**: InitScene, TitleScreen, HomeScreen, GameScene, EditorScene

### 2.7. オーバーレイ（Overlays）

**`game/core/states/overlays/`**

- **IOverlay**: 各オーバーレイが実装するインターフェース
- **実装例**: StageSelectOverlay, FormationOverlay, EnhancementOverlay, CodexOverlay, SettingsOverlay, GachaOverlay, LicenseOverlay

---

## 3. API層について

### 3.1. 設計思想

- **責務の分離**: 低レベル処理をAPI層に集約し、ゲームロジックから分離
- **依存性注入**: APIは`SharedContext`経由で共有され、各モジュールが参照
- **統一インターフェース**: 各APIは明確な責務を持ち、一貫した設計
- **テスト容易性**: 抽象化により、モック化やテストが容易

### 3.2. 主要API一覧

#### BaseSystemAPI（基盤API）
- Raylib統合の基盤
- ウィンドウ・リソース・描画・オーディオ・タイミング・衝突判定を統合
- 内部解像度: 1920x1080固定
- RenderTexture: 内部解像度で描画し、ウィンドウにスケール

#### RenderSystemAPI（描画API）
- 描画処理の統一インターフェース
- 解像度スケーリング、テキスト描画、プリミティブ描画、テクスチャ描画

#### ResourceSystemAPI（リソースAPI）
- テクスチャ・サウンド・フォント・JSONの読み込みと管理
- リソーススキャン、進捗管理、テクスチャ管理、フォント管理

#### ECSystemAPI（ECS API）
- EnTTの`entt::registry`をラップ
- エンティティ操作、コンポーネント操作、ビュー作成、コンテキスト変数、生成ヘルパー

#### InputSystemAPI（入力API）
- キーボード・マウス入力の統一インターフェース
- キーボード、マウス、便利メソッド、座標補助、クリック消費

#### UISystemAPI（UI API）
- UI共通機能を統括
- UIイベント、描画ヘルパー、テーマ、アセット参照、UIエフェクト

#### AudioControlAPI（オーディオ制御API）
- BGM・SEの再生・音量・フェードを統合管理
- BGM、SE、音量制御

#### SceneOverlayControlAPI（シーン/オーバーレイ制御API）
- シーンとオーバーレイのライフサイクル・遷移を統合管理
- シーン管理、更新・描画、オーバーレイ管理、遷移管理

#### GameplayDataAPI（ゲームプレイデータAPI）
- キャラクター・アイテム・ステージ・プレイヤーデータの読み込み・検索・更新を統合
- キャラクター、アイテム/パッシブ、タワーアタッチメント、ステージ、プレイヤーデータ、編成、セーブ

#### SetupAPI（セットアップAPI）
- 非戦闘ロード・Wave/ステージロード・ECS生成を統合
- Wave/ステージロード、ECS生成

#### BattleSetupAPI（戦闘セットアップAPI）
- 戦闘開始前のセットアップデータ構築を統合
- セットアップ構築

#### BattleProgressAPI（戦闘進行API）
- 戦闘進行（Wave・スポーン・経済・勝敗・戦闘ロジック）を統合管理
- 初期化、更新、状態取得、状態操作、戦闘統計、攻撃ログ

#### DebugUIAPI（デバッグUI API）
- シーン共通のデバッグ/チートUIを提供
- 表示切替、パネル登録、共通パネル

### 3.3. API層の依存関係

```
GameSystem
    ├── BaseSystemAPI（所有）
    │   ├── RenderSystemAPI（所有）
    │   ├── ResourceSystemAPI（所有）
    │   ├── AudioSystemAPI（所有）
    │   ├── WindowSystemAPI（所有）
    │   ├── TimingSystemAPI（所有）
    │   └── CollisionSystemAPI（所有）
    ├── ECSystemAPI（所有）
    ├── InputSystemAPI（所有）
    ├── UISystemAPI（所有）
    ├── AudioControlAPI（所有）
    ├── SceneOverlayControlAPI（所有）
    │   └── OverlayManager（所有）
    ├── SetupAPI（所有）
    ├── BattleSetupAPI（所有）
    ├── BattleProgressAPI（所有）
    ├── GameplayDataAPI（所有）
    └── DebugUIAPI（所有）

SharedContext（全APIへのポインタを保持）
    └── 各シーン・オーバーレイ・モジュールが参照
```

### 3.4. API層の使用パターン

#### パターン1: SharedContext経由でアクセス
```cpp
// シーン・オーバーレイ内
void MyScene::Update(float deltaTime) {
    auto& systemAPI = *sharedContext_->systemAPI;
    auto& inputAPI = *sharedContext_->inputAPI;
    auto& ecsAPI = *sharedContext_->ecsAPI;
    
    // API使用
    if (inputAPI->IsLeftClickPressed()) {
        // 処理
    }
}
```

#### パターン2: 直接APIを所有
```cpp
// GameSystem内
class GameSystem {
    std::unique_ptr<BaseSystemAPI> systemAPI_;
    std::unique_ptr<ECSystemAPI> ecsAPI_;
    // ...
};
```

#### パターン3: サブAPIへのアクセス
```cpp
// BaseSystemAPI経由でサブAPIにアクセス
systemAPI->Render().BeginRender();
systemAPI->Resource().GetTexturePtr("icon");
systemAPI->Audio().PlaySound("se_click");
```

---

## 4. ゲームシステムの設計と実装

### 4.1. 全体アーキテクチャ

**GameSystem**は、アプリケーション全体のライフサイクルを管理する統合クラスです。

```
main.cpp
    └── GameSystem
            ├── API層（所有・管理）
            ├── SharedContext（共有コンテキスト）
            ├── シーン（所有・管理）
            └── メインループ（Update → Render）
```

### 4.2. GameSystemの責務

#### 主要責務
1. アプリケーション全体の初期化・終了管理
2. メインループとFPS制御
3. 各種APIの所有・初期化
4. SharedContextの構築と共有
5. シーン遷移管理
6. OverlayManager統合（オーバーレイからの遷移・終了リクエスト処理）

#### 所有するリソース
- API層（BaseSystemAPI, AudioControlAPI, ECSystemAPI, InputSystemAPI, UISystemAPI, DebugUIAPI, SceneOverlayControlAPI, SetupAPI, BattleProgressAPI, BattleSetupAPI, GameplayDataAPI）
- シーン（InitScene, TitleScreen, HomeScreen, GameScene, EditorScene）
- 共有コンテキスト（SharedContext）
- ゲームステート（GameState）

### 4.3. 初期化プロセス（Initialize）

#### 初期化順序
1. 基盤システム（Raylib、ウィンドウ、リソース）
2. オーディオ
3. 入力
4. UI
5. ゲームプレイデータ（JSON読み込み）
6. SharedContext構築
7. デバッグUI
8. セットアップAPI
9. 戦闘セットアップAPI
10. シーン/オーバーレイ制御API
11. 戦闘進行API
12. シーン作成・登録
13. 初期化シーンの開始

### 4.4. メインループ（Run）

#### ループ構造
1. デルタタイム取得
2. 入力更新
3. オーディオ更新
4. シーン/オーバーレイ更新
5. 遷移・終了リクエスト処理
6. 描画フェーズ
7. UIカーソル描画
8. ImGui描画（EndFrame内）

### 4.5. シーン遷移管理

#### 遷移処理（transitionTo）
- 同じ状態への遷移を防止（リトライ時は再初期化）
- 現在のシーンのクリーンアップ
- 新しいシーンの初期化
- 状態更新

#### ゲームステート
- Initializing: リソース初期化中
- Title: タイトル画面
- Home: ホーム画面
- Game: ゲーム画面（バトル）
- Editor: エディタ画面

### 4.6. 終了処理（Shutdown）

#### シャットダウン順序
1. シーン終了
2. 戦闘進行API終了
3. デバッグUI終了
4. 戦闘セットアップAPI終了
5. セットアップAPI終了
6. ゲームプレイデータ終了
7. シーン/オーバーレイ制御API終了
8. UI API終了
9. 入力API終了
10. オーディオAPI終了
11. 基盤システム終了（ログシステムも終了）

### 4.7. 設計パターン

#### 使用されているパターン
1. **ファサードパターン**: `GameSystem`が複雑なサブシステムを単一インターフェースで統合
2. **依存性注入（DI）**: `SharedContext`経由でAPIを共有
3. **ステートパターン**: `GameState`によるシーン管理
4. **オブザーバーパターン**: シーン/オーバーレイからの遷移リクエスト
5. **RAII**: `unique_ptr`による自動リソース管理

---

## 5. データ志向設計の設計と実装

### 5.1. データ志向設計の基本原則

#### 設計思想
- **データとロジックの分離**: コンポーネントはデータのみ（POD型）、ロジックはシステムに集約
- **一括処理**: 同じコンポーネントを持つエンティティをまとめて処理
- **キャッシュ効率**: 連続メモリアクセスでキャッシュミスを削減
- **データ駆動**: データ構造を中心に設計

### 5.2. コンポーネント設計（POD型）

すべてのコンポーネントはPOD（Plain Old Data）型として実装されています。

#### コンポーネント一覧
- `Position`: 位置情報（x, y）
- `Health`: HP情報（current, max）
- `Stats`: ステータス（attack, defense）
- `Movement`: 移動情報（speed, velocity）
- `Combat`: 戦闘情報（attack_type, attack_size, attack_span, last_attack_time）
- `Sprite`: スプライト情報（sheet_path, frame_width, frame_height）
- `Animation`: アニメーション情報（frame_count, current_frame, frame_timer）
- `CharacterId`: キャラクターID（id）
- `Team`: 陣営情報（faction）
- `Equipment`: 装備データ
- `PassiveSkills`: パッシブスキルデータ

#### コンポーネント設計の原則
1. データのみ: ロジックは持たない（最小限のヘルパーメソッドのみ）
2. POD型: 標準レイアウト、トリビアル型
3. 小さく保つ: 必要最小限のデータのみ
4. 組み合わせ可能: 複数のコンポーネントを組み合わせて使用

### 5.3. システム実装（一括処理）

#### ビューを使った一括処理
EnTTの`view`を使って、同じコンポーネントを持つエンティティを一括処理します。

```cpp
// BattleRenderer.cpp - アニメーション更新（一括処理）
void BattleRenderer::UpdateAnimations(ECSystemAPI* ecsAPI, float deltaTime) {
    // Animationコンポーネントを持つ全エンティティを取得
    auto view = ecsAPI->View<ecs::components::Animation>();
    
    // 一括処理（連続メモリアクセス、キャッシュ効率良）
    for (auto e : view) {
        auto& anim = view.get<ecs::components::Animation>(e);
        // 処理
    }
}
```

### 5.4. メモリレイアウトとキャッシュ効率

#### EnTTのメモリ管理
EnTTは各コンポーネントタイプごとに連続メモリでストレージを管理します。

```
従来のオブジェクト指向（メモリ分散）:
[Entity1: Position, Health, Sprite]
[Entity2: Position, Health, Sprite]
[Entity3: Position, Health, Sprite]
→ メモリが分散、キャッシュミス多発

データ志向設計（連続メモリ）:
Positionストレージ: [Pos1][Pos2][Pos3]...  ← 連続メモリ
Healthストレージ:   [HP1][HP2][HP3]...    ← 連続メモリ
Spriteストレージ:   [Spr1][Spr2][Spr3]... ← 連続メモリ
→ 連続メモリアクセス、キャッシュ効率良
```

### 5.5. データアクセスパターン

#### ビューの種類
- 基本ビュー（指定コンポーネントを持つエンティティ）
- 除外ビュー（指定コンポーネントを持たないエンティティ）
- エンティティのみイテレーション
- コンポーネントを直接取得

### 5.6. パフォーマンス最適化

#### メモリ効率
- ログはデバッグモードのみ
- フレームレート安全な処理（delta_timeで正規化）

#### キャッシュ効率
- 連続メモリアクセス（キャッシュ効率良）
- ビューの再利用

---

## 6. デバッグAPI機能の統合

### 6.1. 統合の目的

**DebugUIAPI**は、シーン共通のデバッグ/チートUIを提供するAPIです。開発・テスト時にゲーム状態を確認・操作できます。

### 6.2. 統合の実装

#### GameSystemへの統合

**初期化（Initialize）**
```cpp
bool GameSystem::InitializeDebugUI() {
    debugUIAPI_ = std::make_unique<DebugUIAPI>();
    if (!debugUIAPI_->Initialize(&sharedContext_)) {
        LOG_ERROR("Failed to initialize DebugUIAPI!");
        return false;
    }
    sharedContext_.debugUIAPI = debugUIAPI_.get();
    return true;
}
```

**終了処理（Shutdown）**
```cpp
void GameSystem::ShutdownDebugUI() {
    if (debugUIAPI_) {
        debugUIAPI_->Shutdown();
        debugUIAPI_.reset();
    }
    sharedContext_.debugUIAPI = nullptr;
}
```

#### SceneOverlayControlAPIへの統合

**更新処理（Update）**
```cpp
SceneOverlayUpdateResult SceneOverlayControlAPI::Update(GameState state, float deltaTime) {
    // DebugUIAPIの表示切替を更新（F1キーでトグル）
    if (sharedContext_->debugUIAPI) {
        sharedContext_->debugUIAPI->UpdateToggle();
    }
    // ...
}
```

**描画処理（RenderImGui）**
```cpp
void SceneOverlayControlAPI::RenderImGui(GameState state) {
    scene->RenderImGui();
    overlayManager_->RenderImGui(*sharedContext_);
    // DebugUIAPIの描画（最前面に表示）
    if (sharedContext_ && sharedContext_->debugUIAPI) {
        sharedContext_->debugUIAPI->Render();
    }
}
```

#### 入力システムとの統合

**デバッグトグルキー（F1）**
```cpp
bool InputSystemAPI::IsDebugTogglePressed() const {
    return IsKeyPressed(KEY_F1);
}
```

### 6.3. 提供機能

#### 共通パネル（RenderCommonPanel）

**通貨編集（Currency）**
- 現在値表示: gold, gems, tickets, maxTickets
- 編集: 入力フィールドで値を変更
- 適用: `PlayerDataManager`に反映
- 保存: セーブファイルに保存
- リロード: セーブファイルから再読み込み

**マネージャー/コンテキスト情報（Managers / SharedContext）**
- API状態: systemAPI, ecsAPI, sceneOverlayAPI, gameplayDataAPI の有無
- データ数: characters, stages の数
- ゲーム状態: currentStageId, deltaTime, isPaused, requestShutdown

**BaseSystemAPI情報**
- 初期化状態: IsInitialized, IsResourcesInitialized, IsImGuiInitialized
- パフォーマンス: FPS
- 解像度: 画面解像度、内部解像度
- オーディオ: master/SE/BGM ボリューム、現在のBGM名

**テクスチャキャッシュ（Texture Cache）**
- 一覧表示: 全テクスチャのキー、ID、サイズ、メモリ使用量
- フィルタ: キー文字列で検索
- テーブル表示: スクロール可能なテーブル

**ロック状態編集（Lock State Editing）**
- キャラクター: 全キャラクターのロック/アンロック状態を編集
- ステージ: 全ステージのロック/クリア状態を編集
- 検索: 名前・IDで検索
- 保存: ロック状態をセーブファイルに保存

#### カスタムパネル登録機能

**パネル登録**
```cpp
int panelId = debugUIAPI->RegisterPanel("MyPanel", [](SharedContext& ctx) {
    // カスタム描画処理
});
```

---

## 7. 処理効率化・UI改善

### 7.1. テクスチャキャッシュシステム

#### テクスチャキャッシュの実装

**BaseSystemAPI**でテクスチャを`shared_ptr`で管理し、重複読み込みを防止。

```cpp
std::unordered_map<std::string, std::shared_ptr<Texture2D>> textures_;
```

**利点**:
- 重複読み込み防止
- メモリ効率（`shared_ptr`で共有）
- 高速アクセス（`unordered_map`でO(1)）

### 7.2. 読みやすいテキスト色の自動計算

#### 輝度キャッシュシステム

背景テクスチャの輝度を計算し、読みやすいテキスト色（黒/白）を自動選択。

```cpp
std::unordered_map<std::string, float> textureLuminanceCache_;      // 輝度キャッシュ
std::unordered_map<std::string, Color> textureTextColorCache_;       // テキスト色キャッシュ

Color RenderSystemAPI::GetReadableTextColor(const std::string& textureKey,
                                            float luminanceThreshold = 0.6f) {
    // 1. テキスト色キャッシュを確認
    // 2. 輝度キャッシュを確認
    // 3. 輝度を計算（初回のみ）
    // 4. 輝度に基づいてテキスト色を決定
}
```

**利点**:
- 初回のみ輝度計算、以降はキャッシュ利用
- 背景に応じた可読性の向上
- UIの自動調整

### 7.3. UIコンポーネントの最適化

#### リストコンポーネントのページネーション

**Listコンポーネント**でページネーションを実装し、大量アイテムでも描画負荷を抑制。

```cpp
int itemsPerPage_;        // 1ページあたりのアイテム数
float scrollOffset_;      // スクロールオフセット
```

**利点**:
- 描画アイテム数の削減
- スクロール性能の向上
- メモリ使用量の抑制

#### キャラクターリストの遅延読み込み

**FormationOverlay**と**CharacterEnhancementOverlay**で、初回のみキャラクターリストを読み込み。

```cpp
// キャラクター一覧の更新（初回のみ読み込み）
if (m_characterList.available_characters.empty() && ctx.gameplayDataAPI) {
    FilterAvailableCharacters(ctx);
}
```

**利点**:
- 初期化時間の短縮
- メモリ使用量の削減
- 必要な時のみデータ読み込み

### 7.4. テキスト折り返しのキャッシュ

#### CodexOverlayでの実装

テキスト折り返し結果をキャッシュし、再計算を回避。

```cpp
std::string infoCachedKey_;        // キャッシュキー
float infoCachedMaxWidth_;         // キャッシュ時の最大幅
std::vector<std::string> infoWrappedLines_;  // 折り返し結果
```

**利点**:
- 折り返し計算の削減
- 描画性能の向上
- UTF-8対応の折り返し

### 7.5. UIエフェクトの最適化

#### エフェクト関数のインライン化

**UIEffects**の関数をインライン化し、呼び出しオーバーヘッドを削減。

```cpp
namespace UIEffects {
    inline void DrawGradientPanel(BaseSystemAPI* api, ...) { ... }
    inline void DrawCard3D(BaseSystemAPI* api, ...) { ... }
    inline void DrawGlowingBorder(BaseSystemAPI* api, ...) { ... }
    inline void DrawModernButton(BaseSystemAPI* api, ...) { ... }
    inline float CalculatePulseAlpha(float time, ...) { ... }
    inline void DrawParticles(BaseSystemAPI* api, ...) { ... }
}
```

**利点**:
- 関数呼び出しオーバーヘッドの削減
- コンパイラ最適化の促進
- コードの再利用性

### 7.6. APIポインタのキャッシュ

#### GachaOverlayでの実装

**SharedContext**から取得したAPIポインタをキャッシュ。

```cpp
GameplayDataAPI* cachedGameplayDataAPI_ = nullptr;

void GachaOverlay::Update(SharedContext& ctx, float deltaTime) {
    // 初回のみAPIポインタをキャッシュ
    if (!cachedGameplayDataAPI_ && ctx.gameplayDataAPI) {
        cachedGameplayDataAPI_ = ctx.gameplayDataAPI;
    }
}
```

**利点**:
- ポインタ参照の削減
- コードの簡潔化
- パフォーマンスの向上

### 7.7. 描画最適化

#### 表示状態チェック

非表示コンポーネントの描画をスキップ。

```cpp
void List::Render() {
    if (!visible_) {
        return;  // 非表示時は描画しない
    }
    // 描画処理
}
```

#### 子要素の条件付き更新

```cpp
void List::Update(float deltaTime) {
    // 子要素の更新（表示されているもののみ）
    for (auto& child : children_) {
        if (child && child->IsVisible()) {
            child->Update(deltaTime);
        }
    }
}
```

### 7.8. UI改善

#### 統一されたUIエフェクト

- グラデーションパネル: `DrawGradientPanel()`
- 3Dカード効果: `DrawCard3D()`（影、ハイライト、ボーダー）
- グロー効果: `DrawGlowingBorder()`（パルスアニメーション対応）
- モダンボタン: `DrawModernButton()`（ホバー、無効状態対応）
- パーティクル: `DrawParticles()`（背景装飾）

#### UIコンポーネントの統一インターフェース

**IUIComponent**で統一インターフェースを提供。

```cpp
class IUIComponent {
    virtual bool Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void Shutdown() = 0;
    
    virtual void SetVisible(bool visible) = 0;
    virtual bool IsVisible() const = 0;
    virtual void SetEnabled(bool enabled) = 0;
    virtual bool IsEnabled() const = 0;
    
    virtual UIEventResult HandleEvent(const UIEvent& ev) = 0;
    // ...
};
```

**利点**:
- コンポーネントの統一的な扱い
- 拡張性の向上
- 保守性の向上

---

## まとめ

このプロジェクトは、以下の設計原則に基づいて実装されています：

1. **ECSアーキテクチャ**: データ志向設計による高性能なゲームロジック
2. **API層分離**: 低レベル処理の抽象化と依存性注入
3. **シーン+オーバーレイ**: 柔軟な画面遷移管理
4. **SharedContext**: モジュール間の情報共有
5. **キャッシュ戦略**: テクスチャ、輝度、テキスト色などのキャッシュ
6. **UI最適化**: ページネーション、遅延読み込み、表示状態チェック
7. **デバッグ機能**: 開発・テスト効率を向上させるデバッグUI

これらの設計により、パフォーマンス、保守性、拡張性を向上させています。
