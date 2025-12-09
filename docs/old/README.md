# Simple-TDC-GameProject ドキュメント

> **スタートガイド**: [開発者向けマニュアル](DEVELOPER_MANUAL.md) を最初に読んでください
> **アーカイブ**: 旧資料は `docs/archive/` 配下（例: `archive/docs/CODE_ANALYSIS.md`）

## 📚 ドキュメント構成

### 🚀 初心者向け（まずこれを読む）

- **[開発者向けマニュアル](DEVELOPER_MANUAL.md)** - 環境セットアップからシステム追加まで、すべての開発者向けの包括的ガイド
- **[クイックリファレンス](QUICK_REFERENCE.md)** - よく使うコマンドやリソースの素早い参照
- **[ドキュメント索引ガイド](README_REORGANIZED.md)** - ドキュメント全体の分類・ナビゲーション

### 🏗️ アーキテクチャ・設計

- [EXECUTIVE_SUMMARY.md](EXECUTIVE_SUMMARY.md) - プロジェクト全体概要
- **[DEVELOPER_MODE_DESIGN.md](DEVELOPER_MODE_DESIGN.md)** - 開発者モード設計書
- **[TD_ENTITY_BEHAVIOR_SYSTEM.md](TD_ENTITY_BEHAVIOR_SYSTEM.md)** - TD専用エンティティ動作システム設計
- [archive/docs/CHARACTER_SYSTEM_DESIGN.md](archive/docs/CHARACTER_SYSTEM_DESIGN.md) - キャラクター・ステータスシステム設計 (アーカイブ)
- [archive/docs/ROGUELIKE_SYSTEM_DESIGN.md](archive/docs/ROGUELIKE_SYSTEM_DESIGN.md) - Roguelike システム設計 (アーカイブ)
- [archive/docs/INTEGRATION_STATUS.md](archive/docs/INTEGRATION_STATUS.md) - 統合状況・進捗 (アーカイブ)

### 📖 技術ガイド

- [BUILD_WITH_NINJA.md](BUILD_WITH_NINJA.md) - Ninja ビルドシステム
- [FONT_SETUP.md](FONT_SETUP.md) - フォント設定
- [MIGRATION_GUIDE.md](MIGRATION_GUIDE.md) - 旧アーキテクチャからの移行
- **[WEBUI_TO_INGAME_EDITOR_MIGRATION.md](WEBUI_TO_INGAME_EDITOR_MIGRATION.md)** - WebUI廃止とゲーム内エディタ移行ガイド
- **[DEVELOPER_MODE_QUICK_REFERENCE.md](DEVELOPER_MODE_QUICK_REFERENCE.md)** - 開発者モード クイックリファレンス
- **[DEVELOPER_MODE_IMPLEMENTATION_ROADMAP.md](DEVELOPER_MODE_IMPLEMENTATION_ROADMAP.md)** - 開発者モード実装ロードマップ

### 🔧 保守・改善

- [archive/docs/CODE_ANALYSIS.md](archive/docs/CODE_ANALYSIS.md) - コード品質分析 (アーカイブ)
- [archive/docs/REFACTORING_PLAN.md](archive/docs/REFACTORING_PLAN.md) - リファクタリング計画 (アーカイブ)
- [archive/docs/OPTIMIZATION_SUMMARY.md](archive/docs/OPTIMIZATION_SUMMARY.md) - パフォーマンス最適化 (アーカイブ)

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
2. archive/docs/CHARACTER_SYSTEM_DESIGN.md
3. archive/docs/CODE_ANALYSIS.md
```

### ImGui エディター開発者

```
1. 開発者向けマニュアル
2. archive/docs/CHARACTER_SYSTEM_DESIGN.md
3. archive/docs/CODE_ANALYSIS.md
```

### ゲームデザイナー

```
1. archive/docs/CHARACTER_SYSTEM_DESIGN.md
2. archive/docs/ROGUELIKE_SYSTEM_DESIGN.md
3. DATA_DEFINITION.md (後续作成)
```

---

## 📊 ドキュメント覚書

| ドキュメント | ステータス | 最終更新 |
|-------------|----------|---------|
| DEVELOPER_MANUAL.md | ✅ 新規作成 | 2025-12-04 |
| README_REORGANIZED.md | ✅ 新規作成 | 2025-12-04 |
| DEVELOPER_MODE_DESIGN.md | ✅ 新規作成 | 2025-12-04 |
| TD_ENTITY_BEHAVIOR_SYSTEM.md | ✅ 新規作成 | 2025-12-04 |
| WEBUI_TO_INGAME_EDITOR_MIGRATION.md | ✅ 新規作成 | 2025-12-04 |
| DEVELOPER_MODE_QUICK_REFERENCE.md | ✅ 新規作成 | 2025-12-04 |
| archive/docs/CHARACTER_SYSTEM_DESIGN.md | ✅ アーカイブ | - |
| archive/docs/ROGUELIKE_SYSTEM_DESIGN.md | ✅ アーカイブ | - |
| archive/docs/CODE_ANALYSIS.md | ✅ アーカイブ | - |
| EXECUTIVE_SUMMARY.md | ✅ 既存 | - |
| BUILD_WITH_NINJA.md | ✅ 既存 | - |
| FONT_SETUP.md | ✅ 既存 | - |
| MIGRATION_GUIDE.md | ✅ 既存 | - |
| archive/docs/INTEGRATION_STATUS.md | ✅ アーカイブ | - |
| archive/docs/REFACTORING_PLAN.md | ✅ アーカイブ | - |
| archive/docs/OPTIMIZATION_SUMMARY.md | ✅ アーカイブ | - |
| HANDOVER.md | ✅ 既存 | - |
| ARCHITECTURE.md | ⏳ 計画中 | - |
| IMGUI_EDITOR_GUIDE.md | ⏳ 計画中 | - |
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

**archive/docs/CHARACTER_SYSTEM_DESIGN.md**

- キャラクター定義（JSON スキーマ）
- ステータスシステム
- アニメーション管理
- スキル・能力システム
- ゲーム設計概要

**archive/docs/ROGUELIKE_SYSTEM_DESIGN.md**

- ダンジョン生成
- 戦闘・AI システム
- アイテムシステム
- ターンベースロジック

### 技術・アーキテクチャ

**archive/docs/CODE_ANALYSIS.md**

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
