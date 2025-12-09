# クイックリファレンス

よく使うコマンド、設定、リソースの素早い参照。

## 🚀 ビルド・実行

### C++ ゲーム

```bash
# 初回セットアップ
cmake --preset ninja
cmake --build --preset ninja-release --target SimpleTDCGame_NewArch

# ビルド（デバッグ）
cmake --build --preset ninja-debug

# ビルド（リリース）
cmake --build --preset ninja-release

# クリーンビルド
rm -r build
cmake --preset ninja
cmake --build --preset ninja-release

# 実行
./build/ninja/bin/SimpleTDCGame_NewArch.exe
```

### WebUI エディター

```bash
cd tools/webui-editor

# 自動セットアップと起動（推奨）
.\setup-and-run.ps1

# 手動インストール
npm install

# 開発サーバー
npm run dev                    # http://localhost:3000

# ビルド
npm run build

# プレビュー
npm run preview

# リント
npm run lint
```

> **トラブルシューティング**: WebUIが起動しない場合は [WEBUI_SETUP_GUIDE.md](WEBUI_SETUP_GUIDE.md) を参照

---

## 📁 ファイルレイアウト

```
Simple-TDC-GameProject/
├── include/              # C++ ヘッダー
├── src/                  # C++ ソース
├── tools/webui-editor/   # WebUI
├── assets/definitions/   # JSON 定義
│   ├── characters/
│   ├── stages/
│   ├── ui/
│   ├── skills/
│   ├── effects/
│   └── sounds/
├── docs/                 # ドキュメント
└── scripts/              # ビルドスクリプト
```

---

## 🏗️ プロジェクト構造

### ビルドターゲット

```bash
# メイン（推奨）
SimpleTDCGame_NewArch    # 統一ゲーム

# 参考
SimpleTDCGame            # 旧アーキテクチャ
NetHackGame              # Roguelike
```

### ディレクトリ分類

| ディレクトリ | 用途 |
|-------------|------|
| `include/Application/` | ゲーム応用層 |
| `include/Core/` | コア機能 |
| `include/Domain/` | TD/Roguelike ドメイン |
| `include/Data/` | データ定義 |
| `src/` | 実装ファイル |
| `tools/webui-editor/` | WebUI エディター |
| `assets/definitions/` | JSON ゲーム定義 |

---

## 📝 C++ コマンド集

### 新規システムの追加

```cpp
// 1. ヘッダー作成
// include/Core/Systems/MySystem.h

// 2. ソース実装
// src/Core/Systems/MySystem.cpp

// 3. GameContext に登録（src/Application/UnifiedGame.cpp）
context.Emplace<Core::MySystem>();
systemRunner_.AddSystem<Core::MySystem>(
    SystemRunner::Phase::Update
);
```

### イベント発行・リッスン

```cpp
// イベント定義
namespace MyApp {
    struct MyEvent {
        int value;
    };
}

// イベント発行
auto event_entity = registry.create();
registry.emplace<MyApp::MyEvent>(event_entity, 42);

// イベント受信
auto view = registry.view<MyApp::MyEvent>();
for (auto entity : view) {
    auto& event = view.get<MyApp::MyEvent>(entity);
    // 処理
    registry.destroy(entity);  // イベント削除
}
```

### コンポーネント定義（正しい例）

```cpp
// ✅ 正しい: POD 型、ロジックなし
struct Transform {
    float x = 0.0f;
    float y = 0.0f;
};

// ❌ 間違い: ロジック含む
struct Transform {
    float x, y;
    void Move(float dx, float dy) {}  // NG!
};
```

---

## 💻 TypeScript/WebUI コマンド集

### コンポーネント作成テンプレート

```typescript
// src/components/Editors/MyEditor.tsx
import { useState } from 'react'
import { MyDef } from '../../types/my'

interface MyEditorProps {
  item: MyDef
  onSave: (item: MyDef) => void
}

export default function MyEditor({ item, onSave }: MyEditorProps) {
  const [edited, setEdited] = useState<MyDef>(item)

  return (
    <div className="p-8">
      <h2 className="text-2xl font-bold mb-6">{edited.name}</h2>
      <button onClick={() => onSave(edited)}>保存</button>
    </div>
  )
}
```

### API クライアント利用

```typescript
import { useApiClient } from '../hooks/useApiClient'

export default function MyComponent() {
  const { apiClient } = useApiClient()

  const handleFetch = async () => {
    const data = await apiClient.getCharacters?.()
    console.log(data)
  }

  return <button onClick={handleFetch}>取得</button>
}
```

---

## 🔌 REST API エンドポイント

### キャラクター

```bash
# 一覧取得
curl http://localhost:8080/api/characters

# 単体取得
curl http://localhost:8080/api/characters/{id}

# 新規作成
curl -X POST http://localhost:8080/api/characters \
  -H "Content-Type: application/json" \
  -d '{"id":"hero","name":"勇者"}'

# 更新
curl -X PUT http://localhost:8080/api/characters/{id} \
  -d @character.json

# 削除
curl -X DELETE http://localhost:8080/api/characters/{id}
```

### ゲーム状態

```bash
# ゲーム全体状態
curl http://localhost:8080/api/game/state

# TD モード状態
curl http://localhost:8080/api/game/td/state

# Roguelike モード状態
curl http://localhost:8080/api/game/roguelike/state
```

### リアルタイム更新

```bash
# SSE ストリーム監視
curl -N http://localhost:8080/events
```

---

## 📦 JSON 定義テンプレート

### キャラクター

```json
{
  "id": "hero",
  "name": "勇者",
  "gameMode": "TD",
  "stats": {
    "hp": 150.0,
    "attack": 25.0,
    "defense": 15.0,
    "moveSpeed": 150.0
  },
  "combat": {
    "attackType": "single",
    "attackRange": 50.0
  }
}
```

### スキル

```json
{
  "id": "fireball",
  "name": "ファイアボール",
  "cooldown": 5.0,
  "targetType": "area",
  "effects": [
    {
      "type": "Damage",
      "value": 50,
      "isPercentage": false
    }
  ]
}
```

### エフェクト

```json
{
  "id": "explosion",
  "name": "爆発",
  "emitters": [
    {
      "shape": "circle",
      "emissionRate": 50
    }
  ]
}
```

### サウンド

```json
{
  "id": "explosion",
  "name": "爆発音",
  "type": "sfx",
  "volume": 0.9,
  "maxInstances": 3
}
```

---

## 🐛 よくあるエラー・解決策

| エラー | 原因 | 解決策 |
|--------|------|--------|
| `Components.h not found` | 非推奨ヘッダー使用 | `#include "Data/Definitions/..."`に変更 |
| `port 3000 already in use` | WebUI ポート重複 | `npm run dev -- --port 3001` |
| `Cannot find module` | npm 依存関係 | `npm install` 再実行 |
| `Build ninja not found` | ビルド環境未セット | `cmake --preset ninja` 実行 |
| `Assets not found` | 実行パス誤り | `build/ninja/bin/` から実行 |

---

## 🎯 チェックリスト

### 新機能開発前

- [ ] ドキュメント確認（DEVELOPER_MANUAL.md）
- [ ] ブランチ作成・切り替え
- [ ] 既存テスト確認
- [ ] ビルド動作確認

### コミット前

- [ ] コンパイル確認
- [ ] 基本的なテスト実施
- [ ] コード品質確認
- [ ] コミットメッセージ作成

### PR 作成時

- [ ] ドキュメント更新
- [ ] テストコード追加
- [ ] PR ディスクリプション作成
- [ ] 関連 Issue リンク

---

## 📚 ドキュメント位置

| ドキュメント | 説明 |
|-------------|------|
| `docs/DEVELOPER_MANUAL.md` | 詳細開発ガイド |
| `docs/CHARACTER_SYSTEM_DESIGN.md` | ゲームシステム設計 |
| `docs/ARCHITECTURE.md` | 技術アーキテクチャ *(近日作成)* |
| `docs/API_REFERENCE.md` | API 完全仕様 *(近日作成)* |

---

## 🔗 便利なリンク

- [GitHub Repository](https://github.com/i0527/Simple-TDC-GameProject)
- [Issues](https://github.com/i0527/Simple-TDC-GameProject/issues)
- [Pull Requests](https://github.com/i0527/Simple-TDC-GameProject/pulls)
- [Releases](https://github.com/i0527/Simple-TDC-GameProject/releases)

---

**最終更新**: 2025-12-04  
**対応バージョン**: Phase 6 完了
