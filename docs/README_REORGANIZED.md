# ドキュメント索引・整理ガイド

このディレクトリには、Simple-TDC-GameProject 全体のドキュメントが集約されています。

## 📚 ドキュメント分類

### 🚀 スタートガイド

| ドキュメント | 目的 | 対象者 | 読む時間 |
|-------------|------|--------|---------|
| [DEVELOPER_MANUAL.md](DEVELOPER_MANUAL.md) | **初めにこれを読む** - 開発環境セットアップからシステム追加まで | 全開発者 | 30分 |
| [QUICK_REFERENCE.md](QUICK_REFERENCE.md) | よく使うコマンド・リソースのクイックリファレンス | 全開発者 | 5分 |
| [BUILD_WITH_NINJA.md](BUILD_WITH_NINJA.md) | Ninja ビルドシステムの詳細設定 | C++ 開発者 | 10分 |

### 🏗️ アーキテクチャ・設計

| ドキュメント | 目的 | 対象者 |
|-------------|------|--------|
| [EXECUTIVE_SUMMARY.md](EXECUTIVE_SUMMARY.md) | プロジェクト全体の概要・進捗状況 | マネージャー・新規開発者 |
| [ARCHITECTURE.md](ARCHITECTURE.md) | システムアーキテクチャの詳細 *(後续作成予定)* | アーキテクト・C++ 開発者 |
| [CHARACTER_SYSTEM_DESIGN.md](CHARACTER_SYSTEM_DESIGN.md) | キャラクター・ステータスシステム設計 | ゲーム設計・C++ 開発者 |
| [ROGUELIKE_SYSTEM_DESIGN.md](ROGUELIKE_SYSTEM_DESIGN.md) | Roguelike システムの詳細設計 | Roguelike 開発者 |
| [INTEGRATION_STATUS.md](INTEGRATION_STATUS.md) | 統合状況・依存関係の可視化 | アーキテクト |

### 📖 ガイド・マニュアル

| ドキュメント | 目的 | 対象者 |
|-------------|------|--------|
| [API_REFERENCE.md](API_REFERENCE.md) | REST API エンドポイント仕様 *(後续作成予定)* | Backend・WebUI 開発者 |
| [DATA_DEFINITION.md](DATA_DEFINITION.md) | JSON データ定義フォーマットガイド *(後续作成予定)* | ゲームデザイナー・全開発者 |
| [WEBUI_GUIDE.md](WEBUI_GUIDE.md) | WebUI エディター開発ガイド *(後续作成予定)* | フロントエンド開発者 |

### 🔧 保守・最適化

| ドキュメント | 目的 | 対象者 |
|-------------|------|--------|
| [CODE_ANALYSIS.md](CODE_ANALYSIS.md) | コード品質分析・改善案 | リファクタリング担当者 |
| [REFACTORING_PLAN.md](REFACTORING_PLAN.md) | リファクタリング計画 | リファクタリング担当者 |
| [OPTIMIZATION_SUMMARY.md](OPTIMIZATION_SUMMARY.md) | パフォーマンス最適化サマリー | 最適化担当者 |
| [MIGRATION_GUIDE.md](MIGRATION_GUIDE.md) | 旧アーキテクチャからの移行ガイド | 保守・リファクタリング担当者 |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | よくあるエラーと解決策 *(後续作成予定)* | 全開発者 |

### 📋 その他

| ドキュメント | 目的 |
|-------------|------|
| [FONT_SETUP.md](FONT_SETUP.md) | フォント設定・セットアップ |
| [HANDOVER.md](HANDOVER.md) | 開発フェーズの引き継ぎドキュメント |

---

## 🎯 シナリオ別・推奨読書順

### 新規開発者の場合

```
1. DEVELOPER_MANUAL.md        (環境セットアップ)
   ↓
2. QUICK_REFERENCE.md          (よく使うコマンド確認)
   ↓
3. CHARACTER_SYSTEM_DESIGN.md  (システム理解)
   ↓
4. CODE_ANALYSIS.md            (コード品質・ベストプラクティス確認)
```

### C++ システム開発者の場合

```
1. DEVELOPER_MANUAL.md         (基本セットアップ)
   ↓
2. EXECUTIVE_SUMMARY.md        (全体像把握)
   ↓
3. CHARACTER_SYSTEM_DESIGN.md  (ゲームシステム理解)
   ↓
4. ARCHITECTURE.md             (技術アーキテクチャ)
   ↓
5. CODE_ANALYSIS.md            (品質基準確認)
```

### WebUI フロントエンド開発者の場合

```
1. DEVELOPER_MANUAL.md         (WebUI セットアップ)
   ↓
2. WEBUI_GUIDE.md              (開発ガイド)
   ↓
3. API_REFERENCE.md            (API 仕様)
   ↓
4. DATA_DEFINITION.md          (データ形式理解)
```

### ゲームデザイナー・プランナーの場合

```
1. DEVELOPER_MANUAL.md         (概要確認)
   ↓
2. CHARACTER_SYSTEM_DESIGN.md  (キャラクター・ステータス)
   ↓
3. ROGUELIKE_SYSTEM_DESIGN.md  (Roguelike ゲーム設計)
   ↓
4. DATA_DEFINITION.md          (JSON 定義形式)
```

---

## 📝 ドキュメント作成・更新ガイド

### ドキュメント追加時のチェックリスト

- [ ] 適切なカテゴリに配置
- [ ] このファイル（README.md）に追記
- [ ] Table of Contents に記載
- [ ] 関連ドキュメントへリンク追加
- [ ] 最終更新日時を記載

### ドキュメント品質基準

```
✅ 必須要素:
- 明確なタイトルと目的
- 目次または構成
- 具体例・サンプルコード
- リンク・参照情報

✅ 推奨要素:
- 図表・ダイアグラム
- よくある質問（FAQ）
- トラブルシューティング
- 最終更新日時
```

---

## 🔗 ドキュメント間の関係図

```
DEVELOPER_MANUAL.md (全開発者向けスタート)
    ↓
    ├─→ QUICK_REFERENCE.md (クイックリファレンス)
    ├─→ C++ 開発
    │   ├─→ CHARACTER_SYSTEM_DESIGN.md
    │   ├─→ ARCHITECTURE.md
    │   └─→ CODE_ANALYSIS.md
    ├─→ WebUI 開発
    │   ├─→ WEBUI_GUIDE.md
    │   └─→ API_REFERENCE.md
    └─→ ゲーム設計
        ├─→ DATA_DEFINITION.md
        ├─→ CHARACTER_SYSTEM_DESIGN.md
        └─→ ROGUELIKE_SYSTEM_DESIGN.md
```

---

## 🚧 今後作成予定のドキュメント

| 優先度 | ドキュメント | 内容 |
|--------|-------------|------|
| 高 | ARCHITECTURE.md | 技術アーキテクチャ詳細 |
| 高 | API_REFERENCE.md | REST API 完全リファレンス |
| 高 | TROUBLESHOOTING.md | エラー解決ガイド |
| 中 | WEBUI_GUIDE.md | WebUI 開発完全ガイド |
| 中 | DATA_DEFINITION.md | JSON データ形式完全ガイド |
| 低 | PERFORMANCE_TUNING.md | パフォーマンスチューニングガイド |
| 低 | DEPLOYMENT.md | デプロイメント手順 |

---

## 💡 使用ガイド

### ドキュメント検索

```bash
# キーワード検索
grep -r "keyword" docs/

# ファイル一覧
ls -la docs/
```

### オンライン表示

GitHub では Markdown をネイティブ表示し、リンクナビゲーションが可能です。

### ローカルプレビュー

```bash
# VS Code Markdown プレビュー
# → Ctrl+K → V

# または Markdown プレビューアプリを使用
```

---

## 📊 ドキュメント統計

```
総ドキュメント数:     12+ ファイル
作成済み:             8 ファイル
作成予定:             7 ファイル
總ページ数:           ~500+ ページ相当
主要ドメイン:         3 (C++, WebUI, ゲーム設計)
```

---

**最終更新**: 2025-12-04  
**メンテナンス担当**: Development Team

