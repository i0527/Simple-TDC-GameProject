# Simple TDC Game - WebUI Editor

統合WebUIエディター - エンティティ、ステージ、UIエディターを統合したWebベースのエディターです。

## 機能

- **エンティティエディター**: キャラクター定義の作成・編集
- **ステージエディター**: ステージとWaveの編集
- **UIエディター**: UIレイアウトの編集

## セットアップ

```bash
cd tools/webui-editor
npm install
```

## 開発サーバーの起動

```bash
npm run dev
```

開発サーバーは `http://localhost:3000` で起動します。

## ビルド

```bash
npm run build
```

## 使用方法

1. C++ゲームサーバーを起動（HTTPサーバーを有効化）
2. ブラウザで `http://localhost:3000` にアクセス
3. サイドバーからエディターを選択
4. データを編集して保存

## 技術スタック

- React 18
- TypeScript
- Vite
- Tailwind CSS
- React Flow (将来のノードエディター用)
- Zustand (状態管理)
- Axios (API通信)

## プロジェクト構造

```
tools/webui-editor/
├── src/
│   ├── api/          # APIクライアント
│   ├── components/   # Reactコンポーネント
│   │   ├── Layout/   # レイアウトコンポーネント
│   │   └── Editors/  # エディターコンポーネント
│   ├── hooks/        # カスタムフック
│   ├── types/        # TypeScript型定義
│   └── main.tsx      # エントリーポイント
├── package.json
└── vite.config.ts
```

