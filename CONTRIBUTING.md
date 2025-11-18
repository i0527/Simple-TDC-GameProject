# 貢献ガイドライン

Simple-TDC-GameProjectへの貢献に興味を持っていただきありがとうございます！このドキュメントでは、プロジェクトに貢献する際のガイドラインを説明します。

## コミット規約

### コミットメッセージの形式
コミットメッセージは以下の形式に従ってください：

```
<type>: <subject>

<body>
```

### Type（種類）
- `feat`: 新機能の追加
- `fix`: バグ修正
- `refactor`: リファクタリング（機能変更なし）
- `docs`: ドキュメントの更新
- `style`: コードフォーマット、セミコロン追加など（機能に影響しない変更）
- `test`: テストの追加・修正
- `chore`: ビルドプロセスや補助ツールの変更

### 例
```
feat: プレイヤーの移動システムを追加

- 矢印キーでプレイヤーを移動できるようにした
- 画面端でのラップ処理を実装
```

```
fix: JSON設定ファイルの読み込みエラーを修正

config.jsonが存在しない場合にクラッシュする問題を修正
デフォルト値で続行するように変更
```

### コミットの粒度
- **1コミット = 1機能単位**を心がける
- 大きな変更は複数のコミットに分割する
- 各コミットは独立してビルド可能な状態にする

## ブランチ戦略

### ブランチの種類

#### メインブランチ
- `main`: プロダクションリリース用の安定版ブランチ
- `develop`: 次のリリースに向けた開発用ブランチ

#### サポートブランチ

##### Feature branches（機能ブランチ）
- 命名規則: `feature/<機能名>`
- 分岐元: `develop`
- マージ先: `develop`
- 例: `feature/enemy-ai`, `feature/collision-system`

新機能の開発に使用します。

```bash
# 機能ブランチの作成
git checkout develop
git checkout -b feature/new-feature

# 開発後、developにマージ
git checkout develop
git merge --no-ff feature/new-feature
git branch -d feature/new-feature
```

##### Bugfix branches（バグ修正ブランチ）
- 命名規則: `bugfix/<バグ名>`
- 分岐元: `develop`
- マージ先: `develop`
- 例: `bugfix/player-movement`, `bugfix/json-parsing`

開発中のバグ修正に使用します。

##### Hotfix branches（緊急修正ブランチ）
- 命名規則: `hotfix/<バージョン>`
- 分岐元: `main`
- マージ先: `main` および `develop`
- 例: `hotfix/0.1.1`

本番環境の緊急バグ修正に使用します。

```bash
# ホットフィックスブランチの作成
git checkout main
git checkout -b hotfix/0.1.1

# 修正後、mainとdevelopの両方にマージ
git checkout main
git merge --no-ff hotfix/0.1.1
git tag -a v0.1.1

git checkout develop
git merge --no-ff hotfix/0.1.1
git branch -d hotfix/0.1.1
```

## プルリクエスト（PR）の作成

### PRを作成する前に
1. 最新の`develop`ブランチから分岐していることを確認
2. ローカルでビルドが成功することを確認
3. コーディング規約に従っていることを確認

### PRの説明
- 変更内容を明確に記述
- 関連するIssueがあればリンク
- スクリーンショットやGIFを添付（UIに変更がある場合）

### レビュープロセス
1. PRを作成すると、自動的にCI/CDが実行されます
2. ビルドが成功することを確認してください
3. レビュアーからのフィードバックに対応してください
4. 承認後、マージされます

## コーディング規約

詳細は[.github/copilot-instructions.md](.github/copilot-instructions.md)を参照してください。

### 主要なルール
- C++17の機能を活用する
- EnTTコンポーネントはシンプルなデータ構造として定義
- JSON解析は必ずtry-catchで囲む
- スマートポインタを使用してメモリ管理を適切に行う
- コードの可読性を重視する

## Issue報告

バグ報告や機能要望は、GitHubのIssueテンプレートを使用して報告してください。

### バグ報告
- 再現手順を明確に記述
- 期待される動作と実際の動作を記載
- 環境情報（OS、ビルド構成など）を含める

### 機能要望
- 提案する機能の概要を記述
- 解決したい課題を明確にする
- 可能であれば実装案を提示

## 質問・サポート

質問がある場合は、GitHubのIssueまたはDiscussionsを使用してください。

## ライセンス

このプロジェクトに貢献することで、あなたの貢献がプロジェクトのライセンス（MIT）の下で公開されることに同意したものとみなします。
