# 開発者モード設計書

## 概要

WebUIを廃止し、ゲーム内に統合された開発者モードを実装する。開発者モードでは、デバッグUI、エディタ、ホットリロード機能を提供し、レベルデザイナーが効率的にゲームを開発できる環境を構築する。

## 設計方針

### 1. WebUI廃止の理由

- **プロセス分離の問題**: HTTPサーバーとゲーム本体が別プロセスで動作することによる同期の複雑さ
- **開発効率**: ゲーム内で直接編集できることで、即座にフィードバックを得られる
- **配布の簡素化**: 外部サーバーが不要になり、単一実行ファイルで完結
- **UI一貫性**: Asepriteライクなウィンドウ配置により、柔軟なワークスペースを実現

### 2. 開発者モードの目標

- ゲーム画面を侵食しない独立したUI配置システム
- TD専用の直感的なエンティティ動作定義
- リアルタイムプレビューとホットリロード
- レベルデザイナーに優しいワークフロー

## アーキテクチャ設計

### レンダリング階層構造

```
┌─────────────────────────────────────────────────────┐
│         メインウィンドウ (可変サイズ)                    │
│                                                     │
│  ┌────────────────────────────────────────────┐   │
│  │  ゲーム本体 (FHD固定 RenderTexture)         │   │
│  │  1920x1080                                │   │
│  │                                           │   │
│  │  [ゲームシーン描画領域]                     │   │
│  │                                           │   │
│  └────────────────────────────────────────────┘   │
│                                                     │
│  ┌──────────────┐  ┌──────────────┐              │
│  │ デバッグUI   │  │ エンティティ  │              │
│  │ ウィンドウ   │  │ インスペクタ  │              │
│  │              │  │              │              │
│  └──────────────┘  └──────────────┘              │
│                                                     │
│  ┌─────────────────────────────────────────┐      │
│  │ エディタパネル (ドッキング可能)           │      │
│  │  - ステージエディタ                      │      │
│  │  - エンティティ定義エディタ              │      │
│  │  - イベントエディタ                      │      │
│  └─────────────────────────────────────────┘      │
└─────────────────────────────────────────────────────┘
```

### モード切り替え

```cpp
enum class GameMode {
    Play,           // 通常プレイモード
    Developer       // 開発者モード
};
```

**開発者モード起動**:
- F1キーで開発者モードのON/OFF
- 開発者モードON時に各種エディタウィンドウが利用可能
- ゲーム本体は独立したRenderTextureで描画され、エディタUIに干渉されない

## UI システム設計

### 1. ウィンドウ管理システム

Asepriteライクな自由配置可能なウィンドウシステムを実装する。

```cpp
namespace Game::DevMode {

/**
 * @brief ウィンドウ配置情報
 */
struct WindowLayout {
    std::string windowId;
    ImVec2 position;
    ImVec2 size;
    bool isVisible;
    bool isDocked;
    std::string dockSpaceId;
};

/**
 * @brief ワークスペース設定
 */
struct Workspace {
    std::string name;
    std::vector<WindowLayout> windows;
    
    void Save(const std::string& filepath);
    void Load(const std::string& filepath);
};

/**
 * @brief 開発者モードUI管理
 */
class DevModeUI {
public:
    void Initialize();
    void Update(float deltaTime);
    void Render();
    
    // ワークスペース管理
    void SaveWorkspace(const std::string& name);
    void LoadWorkspace(const std::string& name);
    void ResetToDefaultWorkspace();
    
    // ウィンドウ表示制御
    void ShowWindow(const std::string& windowId);
    void HideWindow(const std::string& windowId);
    void ToggleWindow(const std::string& windowId);
    
private:
    std::map<std::string, std::unique_ptr<EditorWindow>> windows_;
    Workspace currentWorkspace_;
    bool isDeveloperMode_ = false;
};

} // namespace Game::DevMode
```

### 2. エディタウィンドウの種類

| ウィンドウ名 | 説明 | 主要機能 |
|------------|------|---------|
| **Game View** | ゲーム本体の表示 | FHD固定のゲーム画面表示 |
| **Entity Editor** | エンティティ動作定義 | スプライト、AI、スキル、効果音設定 |
| **Stage Editor** | ステージ制作 | Wave構成、配置、背景設定 |
| **Event Editor** | イベント制作 | トリガー、アクション、シーケンス |
| **UI Editor** | UI定義 | レイアウト、ボタン、テキスト配置 |
| **Inspector** | 選択オブジェクト詳細 | プロパティ編集、コンポーネント表示 |
| **Hierarchy** | エンティティ階層 | シーン内エンティティ一覧 |
| **Console** | ログ・デバッグ情報 | エラー、警告、情報ログ表示 |
| **Asset Browser** | アセット管理 | スプライト、サウンド、JSONファイル |

## ゲーム本体とエディタの分離

### RenderTexture活用

ゲーム本体は独立したRenderTextureに描画し、エディタUIと完全に分離する。

```cpp
class DeveloperModeRenderer {
public:
    void Initialize(int windowWidth, int windowHeight);
    void BeginFrame();
    void EndFrame();
    
    // ゲーム本体描画
    void RenderGameView();
    
    // エディタUI描画
    void RenderEditorUI();
    
private:
    Core::GameRenderer gameRenderer_;  // FHD固定レンダラー
    ImGuiID dockspaceId_;
    
    void SetupDockSpace();
    void RenderGameViewWindow();
};
```

**描画フロー**:

```cpp
void DeveloperModeRenderer::BeginFrame() {
    BeginDrawing();
    ClearBackground(DARKGRAY);
}

void DeveloperModeRenderer::RenderGameView() {
    // ゲーム本体をRenderTextureに描画
    gameRenderer_.BeginRender();
    // ... ゲームシーン描画 ...
    gameRenderer_.EndRender();
}

void DeveloperModeRenderer::RenderEditorUI() {
    // ImGuiフレーム開始
    rlImGuiBegin();
    
    // DockSpaceセットアップ
    SetupDockSpace();
    
    // Game View ウィンドウ
    RenderGameViewWindow();
    
    // その他のエディタウィンドウ
    entityEditor_.Render();
    stageEditor_.Render();
    // ...
    
    rlImGuiEnd();
}

void DeveloperModeRenderer::EndFrame() {
    EndDrawing();
}
```

## ホットリロードシステム

### 設計方針

1. **ファイル監視**: JSON定義ファイルの変更を監視
2. **自動リロード**: 変更検知時に該当データを再読み込み
3. **即時反映**: ゲーム実行中にリアルタイムで反映
4. **エラー処理**: 不正なJSONの場合は前回の状態を維持

### 実装

```cpp
namespace Core {

/**
 * @brief ホットリロードシステム
 */
class HotReloadSystem {
public:
    void Initialize(const std::string& watchPath);
    void Update();  // 毎フレーム呼び出し
    
    // コールバック登録
    using ReloadCallback = std::function<void(const std::string& filepath)>;
    void RegisterCallback(const std::string& pattern, ReloadCallback callback);
    
private:
    std::string watchPath_;
    std::map<std::string, std::filesystem::file_time_type> fileTimes_;
    std::map<std::string, std::vector<ReloadCallback>> callbacks_;
    
    void CheckFileChanges();
    void NotifyCallbacks(const std::string& filepath);
};

} // namespace Core
```

**使用例**:

```cpp
// 初期化時
hotReload_.RegisterCallback("*.char.json", [this](const std::string& path) {
    std::cout << "Reloading character: " << path << std::endl;
    definitionLoader_.ReloadCharacter(path);
});

hotReload_.RegisterCallback("*.stage.json", [this](const std::string& path) {
    std::cout << "Reloading stage: " << path << std::endl;
    definitionLoader_.ReloadStage(path);
});

// メインループ内
hotReload_.Update();
```

## TD専用エンティティ動作システム

### にゃんこ大戦争風のシンプルな動作定義

TD専用のエンティティ動作システムでは、以下の要素を定義可能にする:

1. **スプライトの状態**: idle, walk, attack, death など
2. **エフェクトとサウンドのタイミング**: アニメーション同期
3. **移動・待機・攻撃条件**: 距離、HP、クールダウン
4. **AI行動パターン**: シンプルなステートマシン

### エンティティ動作定義 (JSON)

```json
{
  "entityId": "basic_soldier",
  "name": "基本兵士",
  "description": "近接攻撃を行う基本ユニット",
  
  "sprites": {
    "idle": {
      "texturePath": "assets/sprites/soldier_idle.png",
      "frameCount": 4,
      "frameDuration": 0.2,
      "loop": true
    },
    "walk": {
      "texturePath": "assets/sprites/soldier_walk.png",
      "frameCount": 6,
      "frameDuration": 0.15,
      "loop": true
    },
    "attack": {
      "texturePath": "assets/sprites/soldier_attack.png",
      "frameCount": 8,
      "frameDuration": 0.1,
      "loop": false,
      "events": [
        {
          "frame": 4,
          "type": "damage",
          "value": 10
        },
        {
          "frame": 4,
          "type": "sound",
          "soundId": "sword_swing"
        },
        {
          "frame": 5,
          "type": "effect",
          "effectId": "slash_effect",
          "offsetX": 30,
          "offsetY": 0
        }
      ]
    },
    "death": {
      "texturePath": "assets/sprites/soldier_death.png",
      "frameCount": 6,
      "frameDuration": 0.15,
      "loop": false
    }
  },
  
  "stats": {
    "maxHp": 100,
    "speed": 50,
    "attackRange": 80,
    "attackCooldown": 2.0,
    "attackDamage": 10
  },
  
  "behavior": {
    "type": "melee_fighter",
    "states": {
      "idle": {
        "sprite": "idle",
        "transitions": [
          {
            "condition": "enemy_in_range",
            "targetState": "attack",
            "range": 80
          },
          {
            "condition": "always",
            "targetState": "walk"
          }
        ]
      },
      "walk": {
        "sprite": "walk",
        "moveSpeed": 50,
        "transitions": [
          {
            "condition": "enemy_in_range",
            "targetState": "attack",
            "range": 80
          }
        ]
      },
      "attack": {
        "sprite": "attack",
        "transitions": [
          {
            "condition": "animation_finished",
            "targetState": "idle"
          }
        ]
      },
      "death": {
        "sprite": "death",
        "transitions": []
      }
    },
    "initialState": "idle"
  },
  
  "sounds": {
    "spawn": "soldier_spawn",
    "attack": "sword_swing",
    "hit": "soldier_hit",
    "death": "soldier_death"
  }
}
```

### 条件システム

エンティティの状態遷移に使用する条件:

| 条件タイプ | パラメータ | 説明 |
|-----------|----------|------|
| `enemy_in_range` | `range` | 指定範囲内に敵がいる |
| `no_enemy_in_range` | `range` | 指定範囲内に敵がいない |
| `hp_below` | `percentage` | HPが指定%以下 |
| `cooldown_ready` | `cooldownId` | クールダウンが完了 |
| `animation_finished` | - | アニメーションが完了 |
| `distance_to_base` | `distance`, `compare` | 拠点までの距離比較 |
| `random` | `probability` | ランダム発火 (0.0-1.0) |
| `always` | - | 常に真 |

### アニメーションイベント

アニメーションの特定フレームで発火するイベント:

| イベントタイプ | パラメータ | 説明 |
|-------------|----------|------|
| `damage` | `value` | ダメージを与える |
| `sound` | `soundId` | サウンド再生 |
| `effect` | `effectId`, `offsetX`, `offsetY` | エフェクト生成 |
| `spawn` | `entityId`, `offsetX`, `offsetY` | エンティティ生成 |
| `heal` | `value` | HP回復 |
| `state_change` | `targetState` | 状態遷移 |

## エディタ機能詳細

### 1. エンティティエディタ

**機能**:
- エンティティ定義の作成・編集・削除
- スプライトアニメーションプレビュー
- ステータス調整
- 行動AI設定（ビジュアルステートマシン）
- アニメーションイベントのタイムライン編集

**UI構成**:
```
┌─────────────────────────────────────────┐
│ Entity Editor                           │
├─────────────────────────────────────────┤
│ ┌───────────┐  ┌──────────────────────┐│
│ │Entity List│  │ Properties           ││
│ │           │  │                      ││
│ │ Soldier   │  │ ID: basic_soldier    ││
│ │ Archer ←  │  │ Name: 基本兵士        ││
│ │ Tank      │  │                      ││
│ │           │  │ [Stats]              ││
│ │           │  │  HP: [100] ▔         ││
│ │           │  │  Speed: [50] ▔       ││
│ │           │  │  Damage: [10] ▔      ││
│ │           │  │                      ││
│ │           │  │ [Sprites]            ││
│ │           │  │  ○ idle   ▶ Preview  ││
│ │           │  │  ○ walk   ▶ Preview  ││
│ │           │  │  ○ attack ▶ Preview  ││
│ └───────────┘  └──────────────────────┘│
│                                         │
│ ┌─────────────────────────────────────┐│
│ │ Animation Timeline                  ││
│ │ [Frame 0] [Frame 1] ... [Frame 7]   ││
│ │    ↑              ↑                 ││
│ │  Sound         Effect               ││
│ └─────────────────────────────────────┘│
└─────────────────────────────────────────┘
```

### 2. ステージエディタ

**機能**:
- Wave構成の定義
- エンティティ配置
- 背景・環境設定
- タイミング調整
- プレビュー再生

**Wave定義例**:
```json
{
  "stageId": "stage_001",
  "name": "初級ステージ",
  "waves": [
    {
      "waveNumber": 1,
      "spawns": [
        {
          "entityId": "basic_soldier",
          "count": 5,
          "interval": 2.0,
          "delay": 0.0,
          "lane": 1
        }
      ]
    },
    {
      "waveNumber": 2,
      "spawns": [
        {
          "entityId": "basic_soldier",
          "count": 3,
          "interval": 1.5,
          "delay": 0.0,
          "lane": 0
        },
        {
          "entityId": "archer",
          "count": 2,
          "interval": 3.0,
          "delay": 5.0,
          "lane": 2
        }
      ]
    }
  ]
}
```

### 3. イベントエディタ

**機能**:
- ゲームイベントの定義
- トリガー条件設定
- アクションシーケンス
- カットシーン制作

**イベント定義例**:
```json
{
  "eventId": "boss_intro",
  "triggers": [
    {
      "type": "wave_start",
      "waveNumber": 5
    }
  ],
  "actions": [
    {
      "type": "pause_game",
      "duration": 0.5
    },
    {
      "type": "show_message",
      "message": "ボスが出現！",
      "duration": 2.0
    },
    {
      "type": "play_sound",
      "soundId": "boss_roar"
    },
    {
      "type": "resume_game"
    }
  ]
}
```

### 4. UIエディタ

**機能**:
- UI要素の配置
- レイアウト調整
- インタラクション設定
- ローカライゼーション対応

## プレビュー機能

### リアルタイムプレビュー

エディタで編集中のデータをゲーム本体にリアルタイム反映:

1. **エンティティプレビュー**:
   - 単体エンティティをゲーム画面にスポーン
   - アニメーション再生
   - ステータス変更の即時反映

2. **ステージプレビュー**:
   - Wave進行のシミュレーション
   - 早送り/一時停止/巻き戻し
   - デバッグ情報表示

3. **UIプレビュー**:
   - 実際の画面サイズでのプレビュー
   - インタラクションテスト

## 実装計画

### Phase 1: 基盤構築
- [ ] 開発者モード切り替え機能
- [ ] DockSpaceベースのUI管理システム
- [ ] ゲームビューとエディタUIの分離
- [ ] 基本的なウィンドウレイアウト

### Phase 2: ホットリロード
- [ ] ファイル監視システム
- [ ] 自動リロード機能
- [ ] エラーハンドリング

### Phase 3: エンティティエディタ
- [ ] エンティティ定義のCRUD
- [ ] スプライトプレビュー
- [ ] アニメーションタイムライン
- [ ] ステートマシンビジュアライザー

### Phase 4: ステージエディタ
- [ ] Wave定義エディタ
- [ ] プレビュー機能
- [ ] タイムライン編集

### Phase 5: イベント・UIエディタ
- [ ] イベントシステム
- [ ] UIレイアウトエディタ

### Phase 6: 最適化・仕上げ
- [ ] パフォーマンス最適化
- [ ] ユーザビリティ改善
- [ ] ドキュメント整備

## まとめ

この設計により、WebUIを完全に廃止し、ゲーム内で完結する開発環境を構築する。Asepriteライクな柔軟なウィンドウ配置、TD専用の直感的なエンティティ定義、ホットリロードによる即時フィードバックにより、レベルデザイナーの生産性を大幅に向上させる。
