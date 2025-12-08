# Phase 4: HTTPサーバー統合とREST API実装

## 概要

Phase 4のHTTPサーバー統合とREST API基盤の実装を完了しました。これにより、WebUIエディター（Phase 5）の基盤が整いました。

**ブランチ**: `feature/phase4-http-server-integration`  
**コミット数**: 2コミット

## 実装内容

### 1. HTTPサーバー基盤（`Core::HTTPServer`）

- **cpp-httplib**を使用した軽量HTTPサーバー実装
- REST APIエンドポイントの提供
- Server-Sent Events (SSE)によるリアルタイム通信
- ファイル監視によるホットリロード機能

### 2. REST API実装

#### キャラクター定義API

- `GET /api/characters` - 一覧取得
- `GET /api/characters/:id` - 詳細取得
- `POST /api/characters` - 新規作成
- `PUT /api/characters/:id` - 更新
- `DELETE /api/characters/:id` - 削除

#### ステージ定義API

- `GET /api/stages` - 一覧取得
- `GET /api/stages/:id` - 詳細取得
- `POST /api/stages` - 新規作成
- `PUT /api/stages/:id` - 更新
- `DELETE /api/stages/:id` - 削除

#### UIレイアウト定義API

- `GET /api/ui` - 一覧取得
- `GET /api/ui/:id` - 詳細取得
- `POST /api/ui` - 新規作成
- `PUT /api/ui/:id` - 更新（リアルタイム編集対応）
- `DELETE /api/ui/:id` - 削除

### 3. 追加機能

#### バッチ操作API

- `POST /api/batch/ui` - UIレイアウトの一括作成
- `POST /api/batch/characters` - キャラクターの一括作成
- `DELETE /api/batch/ui` - UIレイアウトの一括削除
- `DELETE /api/batch/characters` - キャラクターの一括削除

#### 検索・フィルタリングAPI

- `GET /api/search/ui` - UIレイアウト検索
- `GET /api/search/characters` - キャラクター検索（レアリティ、ゲームモードでフィルタ）
- `GET /api/search/stages` - ステージ検索

#### エクスポート/インポートAPI

- `GET /api/export/ui` - UIレイアウト全件エクスポート
- `GET /api/export/characters` - キャラクター全件エクスポート
- `GET /api/export/stages` - ステージ全件エクスポート
- `POST /api/import/ui` - UIレイアウト一括インポート
- `POST /api/import/characters` - キャラクター一括インポート

#### 統計情報API

- `GET /api/stats` - 定義統計とパフォーマンス統計

#### 設定API

- `GET /api/config` - サーバー設定取得
- `PUT /api/config` - サーバー設定更新（実行時変更可能）

### 4. セキュリティ機能

- **レート制限**: クライアントIPごとのリクエスト数制限（デフォルト: 100リクエスト/分）
- **ボディサイズ制限**: リクエストボディサイズ制限（デフォルト: 10MB）
- **タイムアウト設定**: リクエストタイムアウト（デフォルト: 30秒）
- **開発モード**: ローカル開発時のセキュリティ制限緩和

### 5. 開発支援機能

- **開発モード**: セキュリティ制限の自動緩和、詳細ログ出力
- **エラーレスポンス詳細化**: 開発モード時は例外型、スタックトレースなどの詳細情報を提供
- **パフォーマンスモニタリング**: リクエスト統計、レスポンス時間、スループットの追跡
- **リクエストID追跡**: すべてのリクエストに一意IDを付与し、デバッグを容易に

### 6. データシリアライゼーション

- `Data::UISerializer` - UIレイアウト定義のJSONシリアライズ/デシリアライズ
- `Data::CharacterSerializer` - キャラクター定義のJSONシリアライズ/デシリアライズ
- `Data::StageSerializer` - ステージ定義のJSONシリアライズ/デシリアライズ

### 7. ホットリロード機能

- ファイル変更の自動検出
- SSE経由でのクライアント通知
- UIエディターでのリアルタイム更新対応

## 技術的な詳細

### 依存関係

- **cpp-httplib** v0.14.3（FetchContentで自動取得）

### アーキテクチャ

- 依存性注入パターン（`Core::GameContext`経由）
- Singleton禁止（プロジェクトルールに準拠）
- POD型コンポーネント（ECS原則に準拠）

### パフォーマンス

- レスポンスキャッシュ機能（GETリクエスト）
- 非同期ファイル監視
- スレッドセーフな実装

## 使用方法

### HTTPサーバーの有効化

```cpp
Application::UnifiedGame game;
game.Initialize("assets/definitions", true, 8080);  // HTTPサーバー有効化
game.Run();
```

### APIアクセス

- API情報: `http://localhost:8080/api`
- APIドキュメント: `http://localhost:8080/api/docs`
- ヘルスチェック: `http://localhost:8080/health`
- SSE接続: `http://localhost:8080/events`

### 開発モード

開発モードは自動的に有効化されます（`UnifiedGame`でHTTPサーバー初期化時に設定）。

開発モードでは：

- レート制限が無効化
- ボディサイズ制限が100MBに緩和
- タイムアウトが5分に延長
- DEBUGレベルのログが有効化

## テスト

- [ ] 各APIエンドポイントの動作確認
- [ ] ホットリロード機能の動作確認
- [ ] SSE接続の動作確認
- [ ] エラーハンドリングの確認
- [ ] パフォーマンス統計の確認

## 次のステップ（Phase 5）

- Phase 5B: エンティティエディター（React + TypeScript）
- Phase 5C: ステージエディター（React Flow）
- Phase 5D: UIエディター（WYSIWYG編集）

## 破壊的変更

なし（新機能追加のみ）

## 関連Issue

Phase 4: HTTPサーバー統合とREST API実装
