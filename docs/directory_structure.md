# Cat Tower Defense - ディレクトリ構造規則

**最終更新**: 2026-01-07  
**方針**: シンプル × 直感的 × 迅速開発

---

## 📁 基本方針

### 規則1: 同一ディレクトリ配置（案1採用）

ヘッダーファイル（`.h`, `.hpp`）とソースファイル（`.cpp`, `.c`）は**同じディレクトリ**に配置します。

```
game/
├── main/
│   ├── main.cpp
├── core/
│   ├── api/
│   │   ├── BaseSystemAPI.hpp          ← ヘッダーとソースを同じディレクトリ
│   │   └── BaseSystemAPI.cpp
│   ├── system/
│   │   ├── GameSystem.hpp
│   │   └── GameSystem.cpp
│   └── states/
│       ├── TitleScreen.hpp
│       └── TitleScreen.cpp
└── utils/
    └── Log.h                          ← ヘッダーのみ（実装ファイル不要）
```

---

## 📋 ディレクトリ構造の詳細

### 1. `game/main/` - エントリーポイント

```
main/
└── main.cpp          # プログラムエントリーポイント
```

**規則**:
- `main.cpp`は必ず存在する（エントリーポイント）
- `GameSystem`を使用してゲーム全体のライフサイクルを管理

**実装例**:

```cpp
// game/main/main.cpp
#include "core/system/GameSystem.hpp"
#include "utils/Log.h"

int main() {
    game::core::GameSystem system;
    int initResult = system.Initialize();
    if (initResult != 0) {
        return initResult;
    }
    int runResult = system.Run();
    system.Shutdown();
    return runResult;
}
```

---

### 2. `game/core/` - コア機能

**基本構造**:

```
core/
├── api/              # システムAPI層
├── config/           # 設定・型定義
├── ecs/              # ECS関連（モジュール定義）
├── init/             # 初期化関連
├── states/           # ステート/シーン
└── system/           # コアシステム
```

**規則**:
- 基本的な型定義やユーティリティ
- 他のモジュールから広く使用される機能
- サブディレクトリで機能別に分類

---

### 2.1. `game/core/api/` - システムAPI層

```
api/
├── BaseSystemAPI.hpp         # Raylib統合API
├── BaseSystemAPI.cpp
├── GameModuleAPI.hpp         # EnTTラッパー
└── GameModuleAPI.cpp
```

**責務**:
- **BaseSystemAPI**: Raylib統合API（リソース・描画・入力・オーディオ管理）
- **GameModuleAPI**: EnTTレジストリのラッパー（エンティティ・コンポーネント操作）

**規則**:
- ヘッダーとソースを分離
- 1クラス = 1ヘッダー + 1ソースファイル

---

### 2.2. `game/core/config/` - 設定・型定義

```
config/
├── GameConfig.hpp            # 解像度・FPS設定
├── GameState.hpp             # ゲームステートenum（Home, Game, OverlayState追加済み）
└── SharedContext.hpp         # 共有コンテキスト（GameSystem*ポインタ追加済み）
```

**責務**:
- ゲーム設定（解像度、FPSなど）
- ゲームステート定義
- モジュール間共有コンテキスト

**規則**:
- 設定ファイルはヘッダーのみ（実装ファイル不要）
- enum、struct、定数の定義

---

### 2.3. `game/core/ecs/` - ECS関連（モジュール定義）

```
ecs/
├── Components/               # ✅ ECSコンポーネント（POD）実装済み
│   ├── Position.hpp
│   ├── Health.hpp
│   ├── Stats.hpp
│   ├── Movement.hpp
│   ├── Combat.hpp
│   ├── Sprite.hpp
│   ├── Animation.hpp
│   └── CharacterId.hpp
├── defineComponents.hpp      # ✅ コンポーネント一元管理（実装済み）
├── defineModules.hpp         # モジュール定義（将来実装予定）
└── IModule.hpp              # モジュールインターフェース
```

**責務**:
- モジュールインターフェース定義
- コンポーネント定義の一元管理（✅ 実装済み）
- モジュール定義の一元管理（将来実装予定）

**規則**:
- インターフェースはヘッダーのみ（実装ファイル不要）
- コンポーネントはPOD（Plain Old Data）で実装
- すべてのコンポーネントは`defineComponents.hpp`で一元管理

**実装済みコンポーネント**:
- `Position`: 位置情報（x, y）
- `Health`: HP情報（current, max）
- `Stats`: ステータス情報
- `Movement`: 移動情報
- `Combat`: 戦闘情報
- `Sprite`: スプライト情報
- `Animation`: アニメーション情報
- `CharacterId`: キャラクターID

**将来実装予定**:

```
ecs/
└── modules/                 # ゲームモジュール実装
    ├── MovementModule.hpp
    ├── MovementModule.cpp
    ├── CombatModule.hpp
    ├── CombatModule.cpp
    └── ...
```

---

### 2.4. `game/core/init/` - 初期化関連

```
init/
├── ResourceInitializer.hpp   # リソース初期化
└── ResourceInitializer.cpp
```

**責務**:
- リソーススキャン・読み込み処理
- 進捗管理
- 初期化画面・エラー画面の描画

**規則**:
- ヘッダーとソースを分離
- 1クラス = 1ヘッダー + 1ソースファイル

---

### 2.5. `game/core/states/` - ステート/シーン

```
states/
├── IScene.hpp                # シーン基底インターフェース
├── TitleScreen.hpp/cpp      # ✅ タイトル画面（実装済み）
├── HomeScreen.hpp/cpp       # ✅ ホーム画面（実装済み）
└── overlays/                 # オーバーレイシーン
    ├── IOverlay.hpp          # オーバーレイ基底インターフェース
    ├── StageSelectOverlay.hpp/cpp  # ✅ 実装済み
    ├── FormationOverlay.hpp/cpp    # ✅ 実装済み
    ├── EnhancementOverlay.hpp/cpp   # ✅ 実装済み
    ├── CodexOverlay.hpp/cpp         # ✅ 実装済み
    ├── SettingsOverlay.hpp/cpp     # ✅ 実装済み
    ├── GachaOverlay.hpp/cpp        # ✅ 実装済み
    ├── LicenseOverlay.hpp/cpp       # ✅ 実装済み（スクロール機能付き）
    └── home/                 # ✅ ホーム画面専用コンポーネント（実装済み）
        ├── TabBarManager.hpp/cpp
        ├── ResourceHeader.hpp/cpp
        └── ContentContainer.hpp/cpp
```

**責務**:
- 各ゲームステート（シーン）の実装
- ステート固有の更新・描画処理
- 遷移リクエスト管理
- オーバーレイの実装（UI層固有の概念）

**規則**:
- ヘッダーとソースを分離
- 1ステート = 1ヘッダー + 1ソースファイル
- 命名: `[ステート名]Screen.h/cpp`（シーン）、`[名前]Overlay.h/cpp`（オーバーレイ）

**実装済みオーバーレイ**:
- ✅ `StageSelectOverlay` - ステージ選択（Tileコンポーネント使用）
- ✅ `FormationOverlay` - 編成（Listコンポーネント使用）
- ✅ `EnhancementOverlay` - 強化（Listコンポーネント使用）
- ✅ `CodexOverlay` - 図鑑（Listコンポーネント使用）
- ✅ `SettingsOverlay` - 設定（Panel/Buttonコンポーネント使用）
- ✅ `GachaOverlay` - ガチャ（Cardコンポーネント使用）
- ✅ `LicenseOverlay` - ライセンス情報（スクロール機能付き）

**将来実装予定**:

```
states/
├── IScene.hpp
├── TitleScreen.hpp/cpp      # ✅ 実装済み
├── HomeScreen.hpp/cpp       # ✅ 実装済み
├── GameScreen.hpp/cpp       # 将来実装予定
└── overlays/
    └── (既に実装済み)
```

---

### 2.6. `game/core/ui/` - UIコンポーネント層

```
ui/
├── IUIComponent.hpp          # UIコンポーネント基底インターフェース
└── components/
    ├── Card.hpp/cpp          # カード型コンポーネント
    ├── List.hpp/cpp          # リスト型コンポーネント
    ├── Tile.hpp/cpp          # タイル型コンポーネント
    ├── Panel.hpp/cpp         # パネルコンポーネント
    └── Button.hpp/cpp        # ボタンコンポーネント
```

**責務**:
- UI要素の統一的なコンポーネント化
- カード、リスト、タイルなどの再利用可能なUI要素
- レイアウト管理、イベント処理、子要素管理

**規則**:
- ヘッダーとソースを分離
- 1コンポーネント = 1ヘッダー + 1ソースファイル
- すべてのコンポーネントは`IUIComponent`を実装

**注意**: 現在の実装では、すべてのUIコンポーネントは直接ImGuiを呼び出して描画しています。  
将来の描画バックエンド抽象化では、`IUIRenderer`インターフェース経由で描画するように変更します。

---

### 2.7. `game/core/system/` - コアシステム

```
system/
├── GameSystem.hpp            # ゲームシステム統合クラス
├── GameSystem.cpp
├── ModuleSystem.hpp          # モジュール管理システム
├── ModuleSystem.cpp
├── OverlayManager.hpp        # オーバーレイ管理システム
└── OverlayManager.cpp
```

**責務**:
- **GameSystem**: アプリケーション全体の初期化・終了管理、メインループ管理、オーバーレイ統合
- **ModuleSystem**: モジュールの登録・管理、ライフサイクル管理
- **OverlayManager**: オーバーレイのスタック管理（LIFO）、更新・描画処理

**規則**:
- ヘッダーとソースを分離
- 1クラス = 1ヘッダー + 1ソースファイル

---

### 3. `game/utils/` - ユーティリティ

```
utils/
└── Log.h                    # ログユーティリティ
```

**規則**:
- ユーティリティ関数やマクロ定義
- ヘッダーのみでもOK（実装ファイル不要の場合）

---

## 🔧 ファイル命名規則

### ヘッダーファイル

- 拡張子: `.h` または `.hpp`
- 命名: `ClassName.h`（クラス名と一致）
- 例: `GameSystem.hpp`, `BaseSystemAPI.hpp`, `TitleScreen.hpp`

### ソースファイル

- 拡張子: `.cpp` または `.c`
- 命名: `ClassName.cpp`（クラス名と一致）
- 例: `GameSystem.cpp`, `BaseSystemAPI.cpp`, `TitleScreen.cpp`

### コンポーネント（将来実装予定）

- 拡張子: `.h` のみ（実装ファイル不要）
- 命名: `ComponentName.h` または `Components.h`（複数定義）
- 例: `Position.h`, `Health.h`, `Components.h`

---

## 📝 Includeパス規則

### 同じディレクトリ内のファイル

```cpp
// GameSystem.cpp
#include "GameSystem.hpp"  // 同じディレクトリのヘッダー
```

### 他のディレクトリのファイル

```cpp
// GameSystem.cpp
#include "GameSystem.hpp"                    // 同じディレクトリ
#include "../api/BaseSystemAPI.hpp"          // 親ディレクトリ経由
#include "../config/GameState.hpp"
#include "../ecs/IModule.hpp"
```

### 絶対パス（非推奨）

```cpp
// ❌ 非推奨
#include "game/core/system/GameSystem.hpp"
```

---

## ✅ チェックリスト

新しいファイルを追加する際の確認事項:

- [ ] ヘッダーとソースは同じディレクトリに配置したか？
- [ ] ファイル名はクラス名と一致しているか？
- [ ] コンポーネントはヘッダーのみか？（実装ファイル不要）
- [ ] includeパスは相対パスで統一しているか？
- [ ] ディレクトリ構造は機能ごとに分かれているか？
- [ ] `core/`のサブディレクトリに適切に配置されているか？

---

## 🎯 メリット

### ✅ シンプルさ

- 関連ファイルが近くに配置され、直感的
- ディレクトリ構造が浅く、理解しやすい
- `core/`のサブディレクトリで機能別に分類

### ✅ 迅速開発

- 新しいファイルを追加する場所が明確
- includeパスが短く、記述が簡単
- モジュール追加が容易（IModuleを実装するだけ）

### ✅ 保守性

- 機能ごとにディレクトリが分かれている
- ファイルの役割が明確
- API層とモジュールシステムで責務が分離

---

## 📊 ディレクトリ別の責務

| ディレクトリ | 責務 | ファイル例 |
|------------|------|-----------|
| `main/` | エントリーポイント | `main.cpp` |
| `core/api/` | システムAPI層 | `BaseSystemAPI.hpp/cpp`, `GameModuleAPI.hpp/cpp` |
| `core/config/` | 設定・型定義 | `GameConfig.hpp`, `GameState.hpp`, `SharedContext.hpp` |
| `core/ecs/` | ECS関連（モジュール定義） | `IModule.hpp`, `defineComponents.hpp`（将来実装予定） |
| `core/init/` | 初期化関連 | `ResourceInitializer.hpp/cpp` |
| `core/states/` | ステート/シーン | `TitleScreen.hpp/cpp` |
| `core/system/` | コアシステム | `GameSystem.hpp/cpp`, `ModuleSystem.hpp/cpp` |
| `utils/` | ユーティリティ関数 | `Log.h` |

---

## 🚫 禁止事項

### ❌ ヘッダーとソースを別ディレクトリに配置

```
❌ 悪い例
game/
├── include/
│   └── GameSystem.hpp
└── src/
    └── GameSystem.cpp
```

### ❌ ファイル名とクラス名が不一致

```
❌ 悪い例
GameSystem.hpp の中に class MyGameSystem { ... }
```

### ❌ コンポーネントに実装ファイルを作成

```
❌ 悪い例
ecs/components/Position.h
ecs/components/Position.cpp  ← 不要！（PODなので実装不要）
```

### ❌ 不適切なディレクトリ配置

```
❌ 悪い例
game/
├── GameSystem.hpp          ← core/system/に配置すべき
└── BaseSystemAPI.hpp       ← core/api/に配置すべき
```

---

## 📌 まとめ

1. **ヘッダーとソースは同じディレクトリ**に配置
2. **コンポーネントはヘッダーのみ**（POD、実装不要）
3. **システムはヘッダーとソースを分離**
4. **includeパスは相対パス**で統一
5. **ファイル名はクラス名と一致**
6. **`core/`のサブディレクトリで機能別に分類**

この規則に従うことで、シンプルで保守しやすいコードベースを維持できます。

---

**この規則は `.cursorrules` と `docs/simple_architecture.md` と整合しています。**
