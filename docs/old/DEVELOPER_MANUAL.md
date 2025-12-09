# 開発者向けマニュアル

## 目次

1. [クイックスタート](#クイックスタート)
2. [プロジェクト構造](#プロジェクト構造)
3. [開発環境セットアップ](#開発環境セットアップ)
4. [C++ 開発ガイド](#c-開発ガイド)
5. [WebUI 開発ガイド](#webui-開発ガイド)
6. [データ定義システム](#データ定義システム)
7. [API リファレンス](#api-リファレンス)
8. [トラブルシューティング](#トラブルシューティング)

---

## クイックスタート

### 初回セットアップ（5分）

```bash
# 1. リポジトリのクローン
git clone https://github.com/i0527/Simple-TDC-GameProject.git
cd Simple-TDC-GameProject

# 2. Ninja ビルドシステムのセットアップ
./scripts/bootstrap-ninja.ps1

# 3. C++ プロジェクトのビルド
cmake --preset ninja
cmake --build --preset ninja-release --target SimpleTDCGame_NewArch

# 4. ゲームの実行
./build/ninja/bin/SimpleTDCGame_NewArch.exe
```

### WebUI エディターのセットアップ（3分）

```bash
# 1. WebUI プロジェクトに移動
cd tools/webui-editor

# 2. 依存関係をインストール
npm install

# 3. 開発サーバーを起動
npm run dev
# → http://localhost:3000 でアクセス可能
```

---

## プロジェクト構造

### ディレクトリレイアウト

```
Simple-TDC-GameProject/
├── include/                    # C++ ヘッダーファイル
│   ├── Application/           # ゲーム応用層
│   ├── Core/                  # コア機能
│   ├── Domain/                # TD/Roguelike ドメイン層
│   ├── Data/                  # データ定義
│   ├── Scenes/                # シーン管理
│   └── Roguelike/             # Roguelike固有
│
├── src/                       # C++ ソースファイル
│   ├── Application/
│   ├── Core/
│   ├── Domain/
│   ├── Data/
│   ├── Scenes/
│   ├── main_unified.cpp       # メインエントリーポイント
│   └── ...
│
├── tools/webui-editor/        # WebUI エディター（React + TypeScript）
│   ├── src/
│   │   ├── components/        # React コンポーネント
│   │   ├── types/             # TypeScript 型定義
│   │   ├── api/               # API クライアント
│   │   ├── hooks/             # React カスタムフック
│   │   └── App.tsx            # メインアプリケーション
│   ├── package.json
│   ├── vite.config.ts
│   └── tsconfig.json
│
├── assets/                    # ゲームアセット
│   ├── definitions/           # JSON ゲーム定義ファイル
│   │   ├── characters/        # キャラクター定義
│   │   ├── stages/            # ステージ定義
│   │   ├── ui/                # UI レイアウト定義
│   │   ├── skills/            # スキル定義
│   │   ├── effects/           # エフェクト定義
│   │   └── sounds/            # サウンド定義
│   ├── schemas/               # JSON Schema検証
│   └── fonts/                 # フォントファイル
│
├── docs/                      # プロジェクトドキュメント
│   ├── README.md              # ドキュメント索引
│   ├── DEVELOPER_MANUAL.md    # このファイル
│   ├── QUICK_REFERENCE.md     # クイックリファレンス
│   ├── ARCHITECTURE.md        # アーキテクチャ詳細
│   ├── API_REFERENCE.md       # REST API リファレンス
│   ├── DATA_DEFINITION.md     # データ定義ガイド
│   ├── WEBUI_GUIDE.md         # WebUI 開発ガイド
│   └── TROUBLESHOOTING.md     # トラブルシューティング
│
├── scripts/                   # ビルドスクリプト
│   ├── bootstrap-ninja.ps1
│   ├── build-with-ninja.ps1
│   └── build.ps1
│
├── CMakeLists.txt             # CMake ビルド定義
├── CMakePresets.json          # CMake プリセット
└── README.md                  # プロジェクト README

```

### ビルドターゲット

```cpp
// メインターゲット（推奨）
SimpleTDCGame_NewArch    # 統一ゲーム（新アーキテクチャ）

// 開発用ターゲット（参考用）
SimpleTDCGame           # 旧アーキテクチャ
NetHackGame             # Roguelike ゲーム
```

---

## 開発環境セットアップ

### 必須要件

- **C++ コンパイラ**: Visual Studio 2022 以上 (Windows)
- **CMake**: 3.20 以上
- **Ninja**: ビルドシステム
- **Node.js**: 18.0 以上 (WebUI 開発用)
- **Git**: バージョン管理

### インストール手順

#### 1. C++ ビルド環境（Windows）

```powershell
# Visual Studio 2022 のインストール
# → C++ デスクトップ開発ツール を選択

# CMake インストール
choco install cmake ninja

# または手動インストール
# https://cmake.org/download/
# https://ninja-build.org/
```

#### 2. Node.js と npm

```bash
# Node.js インストール（18.0 以上）
# https://nodejs.org/

# 確認
node --version
npm --version
```

#### 3. ローカルセットアップ

```bash
# リポジトリクローン
git clone https://github.com/i0527/Simple-TDC-GameProject.git
cd Simple-TDC-GameProject

# ビルドディレクトリ作成
mkdir build

# Ninja ビルドシステムセットアップ
./scripts/bootstrap-ninja.ps1
```

---

## C++ 開発ガイド

### 新規システムの追加手順

#### 1. ヘッダーファイル作成

```cpp
// include/Core/Systems/MyNewSystem.h
#pragma once

#include <entt/entt.hpp>
#include "Core/ISystem.h"

namespace Core {

class MyNewSystem : public ISystem {
private:
    // 依存サービス
    GameContext& context_;
    
public:
    MyNewSystem(GameContext& context);
    
    void Initialize() override;
    void PreUpdate(float deltaTime) override;
    void Update(float deltaTime) override;
    void PostUpdate(float deltaTime) override;
    void Shutdown() override;
};

}  // namespace Core
```

#### 2. ソースファイル実装

```cpp
// src/Core/Systems/MyNewSystem.cpp
#include "Core/Systems/MyNewSystem.h"

namespace Core {

MyNewSystem::MyNewSystem(GameContext& context)
    : context_(context) {}

void MyNewSystem::Initialize() {
    // 初期化処理
}

void MyNewSystem::Update(float deltaTime) {
    auto& registry = context_.GetRegistry();
    
    // システムロジック
    auto view = registry.view<ComponentA, ComponentB>();
    for (auto entity : view) {
        auto& compA = view.get<ComponentA>(entity);
        auto& compB = view.get<ComponentB>(entity);
        // 処理
    }
}

void MyNewSystem::Shutdown() {
    // クリーンアップ
}

}  // namespace Core
```

#### 3. ゲームに登録

```cpp
// src/Application/UnifiedGame.cpp
void UnifiedGame::RegisterCoreSystems() {
    auto& context = *context_;
    
    // 新しいシステムを登録
    context.Emplace<Core::MyNewSystem>();
    
    // SystemRunner に登録
    systemRunner_.AddSystem<Core::MyNewSystem>(
        SystemRunner::Phase::Update
    );
}
```

### コンポーネント定義のベストプラクティス

```cpp
// ✅ 正しい: POD型、ロジックなし
struct Transform {
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;
};

// ❌ 間違い: ロジックが含まれている
struct Transform {
    float x, y;
    void Move(float dx, float dy) {  // NG!
        x += dx;
        y += dy;
    }
};
```

### イベント発行とリッスン

```cpp
// イベント定義
namespace TD {
    struct EnemyDefeated {
        entt::entity enemy;
        int gold_reward;
    };
}

// イベント発行
class CombatSystem {
    void OnEnemyDeath(entt::entity entity) {
        auto& registry = context_.GetRegistry();
        registry.emplace<TD::EnemyDefeated>(
            registry.create(),
            entity,
            50
        );
    }
};

// イベントリッスン
class RewardSystem {
    void Update(float deltaTime) {
        auto& registry = context_.GetRegistry();
        auto view = registry.view<TD::EnemyDefeated>();
        for (auto entity : view) {
            auto& event = view.get<TD::EnemyDefeated>(entity);
            GiveReward(event.gold_reward);
            registry.destroy(entity);
        }
    }
};
```

---

## WebUI 開発ガイド

### React コンポーネント作成

```typescript
// src/components/Editors/MyEditor.tsx
import { useState } from 'react'
import { MyDef } from '../../types/my'

interface MyEditorProps {
  item: MyDef
  onSave: (item: MyDef) => void
  onCancel: () => void
}

export default function MyEditor({
  item,
  onSave,
  onCancel,
}: MyEditorProps) {
  const [edited, setEdited] = useState<MyDef>(item)

  return (
    <div className="h-full overflow-auto p-8">
      <div className="mx-auto max-w-4xl">
        <h2 className="text-2xl font-bold mb-6">
          編集: {edited.name}
        </h2>
        
        {/* フォーム */}
        <input
          value={edited.name}
          onChange={(e) =>
            setEdited({ ...edited, name: e.target.value })
          }
        />
        
        {/* ボタン */}
        <button onClick={() => onSave(edited)}>
          保存
        </button>
      </div>
    </div>
  )
}
```

### API クライアント利用

```typescript
// src/components/MyComponent.tsx
import { useApiClient } from '../hooks/useApiClient'

export default function MyComponent() {
  const { apiClient } = useApiClient()
  const [items, setItems] = useState([])

  useEffect(() => {
    const loadItems = async () => {
      try {
        const data = await apiClient.getCharacters?.()
        setItems(data)
      } catch (error) {
        console.error('Failed to load items:', error)
      }
    }

    loadItems()
  }, [apiClient])

  return (
    <div>
      {items.map(item => (
        <div key={item.id}>{item.name}</div>
      ))}
    </div>
  )
}
```

### ビルドと実行

```bash
# 開発サーバー起動
npm run dev

# プロダクション向けビルド
npm run build

# ビルド成果物のプレビュー
npm run preview
```

---

## データ定義システム

### JSON ファイル形式

#### キャラクター定義

```json
// assets/definitions/characters/hero.character.json
{
  "id": "hero",
  "name": "勇者",
  "description": "高いステータスを持つ主人公",
  "gameMode": "TD",
  "visual": {
    "sprite": {
      "atlasPath": "sprites/hero.png",
      "jsonPath": "sprites/hero.json"
    },
    "frameWidth": 64,
    "frameHeight": 64,
    "scale": 1.5,
    "defaultAnimation": "idle"
  },
  "stats": {
    "hp": 150.0,
    "attack": 25.0,
    "defense": 15.0,
    "moveSpeed": 150.0
  },
  "combat": {
    "attackType": "single",
    "attackRange": 50.0,
    "attackCooldown": 1.0
  },
  "td": {
    "cost": 500.0,
    "rechargeTime": 10.0
  }
}
```

#### スキル定義

```json
// assets/definitions/skills/fireball.skill.json
{
  "id": "fireball",
  "name": "ファイアボール",
  "cooldown": 5.0,
  "activateOnAttack": true,
  "targetType": "area",
  "maxTargets": 5,
  "effects": [
    {
      "type": "Damage",
      "value": 50,
      "isPercentage": false
    },
    {
      "type": "StatusApply",
      "statusEffectId": "burn"
    }
  ]
}
```

#### エフェクト定義

```json
// assets/definitions/effects/explosion.effect.json
{
  "id": "explosion",
  "name": "爆発エフェクト",
  "emitters": [
    {
      "id": "particles",
      "shape": "circle",
      "radius": 50,
      "emissionMode": "burst",
      "lifetimeMin": 1.0,
      "lifetimeMax": 2.0,
      "blendMode": "additive"
    }
  ],
  "duration": 2.0,
  "startSoundId": "explosion_sound"
}
```

#### サウンド定義

```json
// assets/definitions/sounds/explosion.sound.json
{
  "id": "explosion",
  "name": "爆発音",
  "type": "sfx",
  "priority": "high",
  "volume": 0.9,
  "maxInstances": 3,
  "is3D": true,
  "minDistance": 10.0,
  "maxDistance": 500.0
}
```

### ファイル検証

```bash
# JSON Schema により自動検証
assets/schemas/character.schema.json
assets/schemas/skill.schema.json
assets/schemas/effect.schema.json
assets/schemas/sound.schema.json
```

---

## API リファレンス

### REST API エンドポイント

#### キャラクター API

```
GET    /api/characters           # 一覧取得
POST   /api/characters           # 新規作成
PUT    /api/characters/{id}      # 更新
DELETE /api/characters/{id}      # 削除
```

#### スキル API（後续実装予定）

```
GET    /api/skills
POST   /api/skills
PUT    /api/skills/{id}
DELETE /api/skills/{id}
```

#### ゲーム状態 API

```
GET    /api/game/state           # ゲーム全体状態
GET    /api/game/td/state        # TD モード状態
GET    /api/game/roguelike/state # Roguelike モード状態
```

#### SSE（Server-Sent Events）

```
GET    /events                   # リアルタイム更新ストリーム
```

### API 例

```bash
# キャラクター取得
curl http://localhost:8080/api/characters

# キャラクター作成
curl -X POST http://localhost:8080/api/characters \
  -H "Content-Type: application/json" \
  -d @character.json

# ゲーム状態取得
curl http://localhost:8080/api/game/state

# リアルタイム更新の監視
curl -N http://localhost:8080/events
```

---

## トラブルシューティング

### C++ ビルドエラー

#### エラー: `Components.h` が見つからない

**原因**: 非推奨ファイルをインクルートしている

**解決策**:
```cpp
// ❌ 非推奨
#include "Components.h"

// ✅ 使用
#include "Data/Definitions/CharacterDef.h"
```

#### エラー: コンパイラが新C++17機能を認識しない

**解決策**:
```bash
# CMake キャッシュクリア
rm -r build
cmake --preset ninja
```

### WebUI エラー

#### エラー: `Cannot find module`

**解決策**:
```bash
# node_modules の再インストール
rm -rf node_modules package-lock.json
npm install
```

#### エラー: ポート 3000 が既に使用中

**解決策**:
```bash
# 別のポートで起動
npm run dev -- --port 3001
```

### ゲーム実行エラー

#### エラー: `assets/definitions` が見つからない

**原因**: ゲーム実行パスが正しくない

**解決策**:
```bash
# ビルド出力ディレクトリから実行
cd build/ninja/bin
./SimpleTDCGame_NewArch.exe
```

#### エラー: フォントが表示されない

**解決策**:
```bash
# フォントセットアップを実行
docs/FONT_SETUP.md を参照
```

---

## よくある質問（FAQ）

### Q: 新しいキャラクターを追加するには？

**A**: `assets/definitions/characters/` に JSON ファイルを作成するか、WebUI エディターから作成します。

```bash
# または手動作成
cp assets/definitions/characters/cupslime.character.json \
   assets/definitions/characters/my_character.character.json
```

### Q: スキルの新しい効果タイプを追加したい

**A**: 以下の手順に従います：

1. `include/Core/Definitions.h` の `SkillEffectType` enum に追加
2. WebUI `src/types/skill.ts` に型定義を追加
3. SkillEditor.tsx に UI を追加
4. C++ ロジックでハンドルを実装

### Q: WebUI と C++ ゲームを同時実行できるか？

**A**: はい、異なるプロセスで実行可能です：

```bash
# ターミナル 1: ゲーム実行
./build/ninja/bin/SimpleTDCGame_NewArch.exe

# ターミナル 2: WebUI 起動
cd tools/webui-editor
npm run dev
```

### Q: ホットリロード機能の使い方

**A**: ゲーム実行中に `assets/definitions/` のファイルを編集すると自動的に再読み込みされます。

```bash
# ゲーム起動後、ファイルを編集
# → ゲーム内に自動反映
```

---

## リソースリンク

- [プロジェクト README](../README.md)
- [クイックリファレンス](QUICK_REFERENCE.md)
- [アーキテクチャ詳細](ARCHITECTURE.md)
- [API リファレンス](API_REFERENCE.md)
- [データ定義ガイド](DATA_DEFINITION.md)
- [WebUI 開発ガイド](WEBUI_GUIDE.md)
- [Cursor Rules](.cursor/CURSOR_DEVELOPMENT_GUIDE.md)

---

**最終更新**: 2025-12-04  
**対応バージョン**: Phase 6 完了時点

