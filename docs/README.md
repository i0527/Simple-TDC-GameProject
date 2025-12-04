# Simple-TDC-GameProject ドキュメント

> **スタートガイド**: [開発者向けマニュアル](DEVELOPER_MANUAL.md) を最初に読んでください

## 📚 ドキュメント構成

### 🚀 初心者向け（まずこれを読む）

- **[開発者向けマニュアル](DEVELOPER_MANUAL.md)** - 環境セットアップからシステム追加まで、すべての開発者向けの包括的ガイド
- **[クイックリファレンス](QUICK_REFERENCE.md)** - よく使うコマンドやリソースの素早い参照
- **[ドキュメント索引ガイド](README_REORGANIZED.md)** - ドキュメント全体の分類・ナビゲーション

### 🏗️ アーキテクチャ・設計

- [EXECUTIVE_SUMMARY.md](EXECUTIVE_SUMMARY.md) - プロジェクト全体概要
- [CHARACTER_SYSTEM_DESIGN.md](CHARACTER_SYSTEM_DESIGN.md) - キャラクター・ステータスシステム設計
- [ROGUELIKE_SYSTEM_DESIGN.md](ROGUELIKE_SYSTEM_DESIGN.md) - Roguelike システム設計
- [INTEGRATION_STATUS.md](INTEGRATION_STATUS.md) - 統合状況・進捗

### 📖 技術ガイド

- [BUILD_WITH_NINJA.md](BUILD_WITH_NINJA.md) - Ninja ビルドシステム
- [FONT_SETUP.md](FONT_SETUP.md) - フォント設定
- [MIGRATION_GUIDE.md](MIGRATION_GUIDE.md) - 旧アーキテクチャからの移行

### 🔧 保守・改善

- [CODE_ANALYSIS.md](CODE_ANALYSIS.md) - コード品質分析
- [REFACTORING_PLAN.md](REFACTORING_PLAN.md) - リファクタリング計画
- [OPTIMIZATION_SUMMARY.md](OPTIMIZATION_SUMMARY.md) - パフォーマンス最適化

### 🤝 開発チーム向け

- [HANDOVER.md](HANDOVER.md) - フェーズ引き継ぎドキュメント

---

## 🎯 役割別・推奨ドキュメント

### 新規開発者
```
1. 開発者向けマニュアル (DEVELOPER_MANUAL.md)
2. クイックリファレンス (QUICK_REFERENCE.md)
3. ドキュメント索引ガイド (README_REORGANIZED.md)
```

### C++ 開発者
```
1. 開発者向けマニュアル
2. CHARACTER_SYSTEM_DESIGN.md
3. CODE_ANALYSIS.md
```

### WebUI フロントエンド開発者
```
1. 開発者向けマニュアル
2. WEBUI_GUIDE.md (後续作成)
3. API_REFERENCE.md (後续作成)
```

### ゲームデザイナー
```
1. CHARACTER_SYSTEM_DESIGN.md
2. ROGUELIKE_SYSTEM_DESIGN.md
3. DATA_DEFINITION.md (後续作成)
```

---

## 📊 ドキュメント覚書

| ドキュメント | ステータス | 最終更新 |
|-------------|----------|---------|
| DEVELOPER_MANUAL.md | ✅ 新規作成 | 2025-12-04 |
| README_REORGANIZED.md | ✅ 新規作成 | 2025-12-04 |
| CHARACTER_SYSTEM_DESIGN.md | ✅ 既存 | - |
| ROGUELIKE_SYSTEM_DESIGN.md | ✅ 既存 | - |
| CODE_ANALYSIS.md | ✅ 既存 | - |
| EXECUTIVE_SUMMARY.md | ✅ 既存 | - |
| BUILD_WITH_NINJA.md | ✅ 既存 | - |
| FONT_SETUP.md | ✅ 既存 | - |
| MIGRATION_GUIDE.md | ✅ 既存 | - |
| INTEGRATION_STATUS.md | ✅ 既存 | - |
| REFACTORING_PLAN.md | ✅ 既存 | - |
| OPTIMIZATION_SUMMARY.md | ✅ 既存 | - |
| HANDOVER.md | ✅ 既存 | - |
| ARCHITECTURE.md | ⏳ 計画中 | - |
| API_REFERENCE.md | ⏳ 計画中 | - |
| WEBUI_GUIDE.md | ⏳ 計画中 | - |
| DATA_DEFINITION.md | ⏳ 計画中 | - |
| TROUBLESHOOTING.md | ⏳ 計画中 | - |

---

## 🔍 ドキュメント内容速見表

### 必読ドキュメント（全開発者向け）

**開発者向けマニュアル (DEVELOPER_MANUAL.md)**
- ✅ クイックスタート（5分）
- ✅ プロジェクト構造
- ✅ 環境セットアップ
- ✅ C++ 開発ガイド
- ✅ WebUI 開発ガイド
- ✅ データ定義システム
- ✅ API リファレンス
- ✅ トラブルシューティング

### ゲームシステム設計

**CHARACTER_SYSTEM_DESIGN.md**
- キャラクター定義（JSON スキーマ）
- ステータスシステム
- アニメーション管理
- スキル・能力システム
- ゲーム設計概要

**ROGUELIKE_SYSTEM_DESIGN.md**
- ダンジョン生成
- 戦闘・AI システム
- アイテムシステム
- ターンベースロジック

### 技術・アーキテクチャ

**CODE_ANALYSIS.md**
- コード品質評価
- 改善案
- ベストプラクティス

**EXECUTIVE_SUMMARY.md**
- プロジェクト概要
- 進捗状況
- 優先度付きアクション

---

## 💡 効果的なドキュメント使い方

### ✅ する

- 開発前に関連ドキュメントを確認
- ドキュメント内のコード例をコピー＆修正
- わからないことはまずドキュメントを検索
- 新機能追加時にドキュメントも更新

### ❌ しない

- ドキュメントを過度に信頼して未検証のコマンドを実行
- 古いドキュメント情報に頼る（最終更新日を確認）
- ドキュメント更新なしに重大な変更を加える

---

## 🚧 ドキュメント拡張予定

近日作成予定：

1. **ARCHITECTURE.md** - システムアーキテクチャ詳細
2. **API_REFERENCE.md** - REST API 完全仕様
3. **TROUBLESHOOTING.md** - よくあるエラーと解決策
4. **WEBUI_GUIDE.md** - WebUI 開発完全ガイド
5. **DATA_DEFINITION.md** - JSON データ定義完全ガイド
6. **PERFORMANCE_TUNING.md** - パフォーマンス最適化ガイド
7. **DEPLOYMENT.md** - デプロイメント手順

---

## 📞 ドキュメント・フィードバック

ドキュメントに関するフィードバック・改善提案：

- 不明な点や誤り → GitHub Issues
- ドキュメント改善提案 → Pull Request
- 新しいドキュメント必要 → Discussion

---

**最終更新**: 2025-12-04  
**バージョン**: Phase 6 完了時点
