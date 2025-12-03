# ドキュメント索引

このディレクトリには、Simple-TDC-GameProjectの設計・実装・運用に関するドキュメントが格納されています。

---

## ⚡ クイックスタート

**新規参加者はこちらから:**
1. 📊 **[EXECUTIVE_SUMMARY.md](./EXECUTIVE_SUMMARY.md)** - プロジェクト概要と重要な推奨事項（5分で読める）
2. 📖 [CODE_ANALYSIS.md](./CODE_ANALYSIS.md) - 詳細な分析レポート
3. 📋 [INTEGRATION_STATUS.md](./INTEGRATION_STATUS.md) - 3プロジェクトの統合状況

**Cursor IDE 利用者はこちらも:**
- 🎯 **[../.cursor/CURSOR_DEVELOPMENT_GUIDE.md](../.cursor/CURSOR_DEVELOPMENT_GUIDE.md)** - Cursor IDE マルチエージェント開発ガイド（必読）
- ⚡ [../.cursor/AI_AGENT_QUICK_REFERENCE.md](../.cursor/AI_AGENT_QUICK_REFERENCE.md) - AIエージェント向けクイックリファレンス

---

## 📚 ドキュメント構成

### 📊 エグゼクティブサマリー

**最初に読むべきドキュメント**

| ドキュメント | 内容 | 対象読者 |
|-------------|------|---------|
| **[EXECUTIVE_SUMMARY.md](./EXECUTIVE_SUMMARY.md)** | プロジェクト分析の要約<br>・主要な発見と課題<br>・重要な推奨事項<br>・実装ロードマップ<br>・成功のための鍵 | **全員** |

### 🏗️ アーキテクチャ・分析

プロジェクト全体の構造、分析、改善計画に関するドキュメント

| ドキュメント | 内容 | 対象読者 |
|-------------|------|---------|
| **[CODE_ANALYSIS.md](./CODE_ANALYSIS.md)** | プロジェクト全体のコード分析レポート<br>・アーキテクチャパターン分析<br>・コンポーネント設計評価<br>・重複コードの特定<br>・品質評価と推奨事項 | 全員 |
| **[REFACTORING_PLAN.md](./REFACTORING_PLAN.md)** | リファクタリング計画書<br>・優先順位付けマトリクス<br>・フェーズ別実装計画<br>・移行ガイドライン<br>・成功指標 | 開発者、メンテナー |
| **[INTEGRATION_STATUS.md](./INTEGRATION_STATUS.md)** | 3プロジェクトの統合状況<br>・各プロジェクト詳細<br>・共通基盤（Core層）分析<br>・統合戦略とロードマップ | 全員 |
| **[HANDOVER.md](./HANDOVER.md)** | 新アーキテクチャハンドオーバー文書<br>・実装状況（Phase 0-3完了）<br>・コンポーネント一覧<br>・システムシグネチャ<br>・次のステップ（Phase 4-7） | 開発者 |

### 🎨 設計仕様

ゲームシステムの設計仕様書

| ドキュメント | 内容 | 対象読者 |
|-------------|------|---------|
| **[ROGUELIKE_SYSTEM_DESIGN.md](./ROGUELIKE_SYSTEM_DESIGN.md)** | NetHack風ローグライクシステム設計書<br>・タイルレンダリング設計<br>・コンポーネント設計<br>・ターンシステム<br>・ダンジョン生成<br>・Phase別実装ロードマップ | Roguelike開発者 |
| **[CHARACTER_SYSTEM_DESIGN.md](./CHARACTER_SYSTEM_DESIGN.md)** | キャラクター・ステージ定義システム設計<br>・現状分析と課題<br>・新アーキテクチャ提案<br>・WebUIエディター設計<br>・JSON定義フォーマット | TD開発者、エディタ開発者 |

### 📖 ガイド・手順書

開発・運用のための実践的なガイド

| ドキュメント | 内容 | 対象読者 |
|-------------|------|---------|
| **[BUILD_WITH_NINJA.md](./BUILD_WITH_NINJA.md)** | Ninjaビルドシステム使用ガイド<br>・インストール手順<br>・使用方法<br>・パフォーマンス比較 | 開発者 |
| **[FONT_SETUP.md](./FONT_SETUP.md)** | フォント設定ガイド<br>・日本語フォント設定<br>・raygui/ImGui統合<br>・トラブルシューティング | UI開発者 |
| **[MIGRATION_GUIDE.md](./MIGRATION_GUIDE.md)** | UIManager v2.0移行ガイド<br>・API変更点<br>・移行手順<br>・具体的な移行例<br>・よくある質問 | 既存コード移行者 |

### 📊 技術情報

技術的な詳細やまとめ

| ドキュメント | 内容 | 対象読者 |
|-------------|------|---------|
| **[OPTIMIZATION_SUMMARY.md](./OPTIMIZATION_SUMMARY.md)** | UIManager最適化まとめ<br>・フォント管理の効率化<br>・API削減<br>・パフォーマンス改善結果 | 技術者 |

### 🤖 Cursor IDE / AI開発支援

Cursor IDE でのマルチエージェント開発向けドキュメント

| ドキュメント | 内容 | 対象読者 |
|-------------|------|---------|
| **[../.cursor/CURSOR_DEVELOPMENT_GUIDE.md](../.cursor/CURSOR_DEVELOPMENT_GUIDE.md)** | Cursor IDE 開発ガイド<br>・プロジェクト概要<br>・マルチエージェント開発戦略<br>・設計改善の要点<br>・推奨設計パターン<br>・AIエージェント向けタスク分割<br>・コーディング規約と実装例 | **Cursor IDE 利用者（必読）** |
| **[../.cursor/AI_AGENT_QUICK_REFERENCE.md](../.cursor/AI_AGENT_QUICK_REFERENCE.md)** | AIエージェント向けクイックリファレンス<br>・最重要ルール<br>・エージェント役割別チートシート<br>・頻出コードスニペット<br>・よくあるエラーと解決策 | AIエージェント、経験者 |
| **[../.cursor/ARCHITECTURE_DIAGRAMS.md](../.cursor/ARCHITECTURE_DIAGRAMS.md)** | アーキテクチャ図解集<br>・システム全体図<br>・レイヤーアーキテクチャ<br>・ECSデータフロー<br>・マルチエージェント開発フロー | 全員（アーキテクチャ理解） |
| **[../.cursorrules](../.cursorrules)** | Cursor プロジェクトルール<br>・必須ルール<br>・コーディング規約<br>・重要な制約 | Cursor IDE 利用者 |

---

## 🗺️ ドキュメントマップ

目的別にドキュメントを探す

### 新規参加者向け

**最初に読むべきドキュメント:**
1. [CODE_ANALYSIS.md](./CODE_ANALYSIS.md) - プロジェクト全体を理解
2. [INTEGRATION_STATUS.md](./INTEGRATION_STATUS.md) - 3つのプロジェクトの関係を理解
3. [BUILD_WITH_NINJA.md](./BUILD_WITH_NINJA.md) - ビルド方法を学ぶ

**次に読むべきドキュメント（開発対象による）:**
- タワーディフェンス開発: [HANDOVER.md](./HANDOVER.md) → [CHARACTER_SYSTEM_DESIGN.md](./CHARACTER_SYSTEM_DESIGN.md)
- Roguelike開発: [ROGUELIKE_SYSTEM_DESIGN.md](./ROGUELIKE_SYSTEM_DESIGN.md)

### 既存開発者向け

**リファクタリング・改善:**
- [REFACTORING_PLAN.md](./REFACTORING_PLAN.md) - 改善計画
- [MIGRATION_GUIDE.md](./MIGRATION_GUIDE.md) - UIManager移行

**新機能開発:**
- [HANDOVER.md](./HANDOVER.md) - 新アーキテクチャ詳細
- [ROGUELIKE_SYSTEM_DESIGN.md](./ROGUELIKE_SYSTEM_DESIGN.md) - Roguelike設計

### メンテナー・レビュワー向け

**コードレビュー時:**
- [CODE_ANALYSIS.md](./CODE_ANALYSIS.md) - 品質基準
- [REFACTORING_PLAN.md](./REFACTORING_PLAN.md) - レビューガイドライン

**プロジェクト管理:**
- [INTEGRATION_STATUS.md](./INTEGRATION_STATUS.md) - 統合状況
- [REFACTORING_PLAN.md](./REFACTORING_PLAN.md) - ロードマップ

---

## 📋 クイックリファレンス

### プロジェクト構成

```
Simple-TDC-GameProject/
├── SimpleTDCGame (旧アーキテクチャ)
│   └── 状態: 保守モード
├── SimpleTDCGame_NewArch (新アーキテクチャ・推奨)
│   └── 状態: アクティブ開発
└── NetHackGame (Roguelike)
    └── 状態: 開発中（Phase 1-3実装予定）
```

詳細: [INTEGRATION_STATUS.md](./INTEGRATION_STATUS.md)

### アーキテクチャレイヤー

```
Application Layer (Game, GameNew, RoguelikeGame)
    ↓
Game/TD/Roguelike Layer (ドメイン固有)
    ↓
Core Layer (共通基盤)
    ↓
External Libraries (EnTT, Raylib, nlohmann/json)
```

詳細: [CODE_ANALYSIS.md](./CODE_ANALYSIS.md) > 3. アーキテクチャ分析

### 推奨開発ターゲット

| 目的 | ターゲット | ドキュメント |
|-----|-----------|------------|
| 新規TD開発 | SimpleTDCGame_NewArch | [HANDOVER.md](./HANDOVER.md) |
| Roguelike開発 | NetHackGame | [ROGUELIKE_SYSTEM_DESIGN.md](./ROGUELIKE_SYSTEM_DESIGN.md) |
| 旧コード保守 | SimpleTDCGame | なし（非推奨） |

---

## 🎯 よくある質問

### Q1: どのビルドターゲットを使えばいい？

**A:** 新規開発では `SimpleTDCGame_NewArch` を使用してください。

```bash
cmake --build build --target SimpleTDCGame_NewArch
```

詳細: [INTEGRATION_STATUS.md](./INTEGRATION_STATUS.md) > 2.2 プロジェクト2

### Q2: Components.h と ComponentsNew.h どちらを使う？

**A:** `ComponentsNew.h` を使用してください。`Components.h` は非推奨です。

```cpp
// 推奨
#include "ComponentsNew.h"

// 非推奨（旧コードのみ）
#include "Components.h"
```

詳細: [REFACTORING_PLAN.md](./REFACTORING_PLAN.md) > 4.1 タスク1.1

### Q3: Singleton を使うべき？

**A:** いいえ。新アーキテクチャでは依存性注入（DI）を使用します。

```cpp
// 非推奨
auto& config = ConfigManager::GetInstance();

// 推奨
class MySystem {
    ConfigManager& config_;
public:
    MySystem(ConfigManager& config) : config_(config) {}
};
```

詳細: [CODE_ANALYSIS.md](./CODE_ANALYSIS.md) > 6.2 アーキテクチャ上の問題

### Q4: Roguelike の実装状況は？

**A:** 設計は完了していますが、実装は未着手です。Phase 1から実装予定です。

詳細: [ROGUELIKE_SYSTEM_DESIGN.md](./ROGUELIKE_SYSTEM_DESIGN.md) > 9. 実装ロードマップ

### Q5: WebUIエディタはいつ実装される？

**A:** Phase 3以降（3ヶ月以降）の予定です。現在は設計段階です。

詳細: [CHARACTER_SYSTEM_DESIGN.md](./CHARACTER_SYSTEM_DESIGN.md) > 5. WebUI エディタ設計

---

## 🔄 ドキュメント更新履歴

| 日付 | ドキュメント | 変更内容 |
|------|------------|---------|
| 2025-12-02 | CODE_ANALYSIS.md | 新規作成（全体分析） |
| 2025-12-02 | REFACTORING_PLAN.md | 新規作成（改善計画） |
| 2025-12-02 | INTEGRATION_STATUS.md | 新規作成（統合状況） |
| 2025-12-02 | README.md (本ファイル) | 新規作成（索引） |
| 過去 | HANDOVER.md | 新アーキテクチャ詳細 |
| 過去 | ROGUELIKE_SYSTEM_DESIGN.md | Roguelike設計 |
| 過去 | CHARACTER_SYSTEM_DESIGN.md | キャラクターシステム設計 |
| 過去 | その他 | 各種ガイド・技術文書 |

---

## 📝 ドキュメント作成・更新ガイドライン

### ドキュメント追加時

1. **適切なカテゴリを選択**
   - アーキテクチャ: 全体構造、分析
   - 設計仕様: システム設計
   - ガイド: 手順書
   - 技術情報: まとめ、詳細

2. **本README.mdを更新**
   - 該当テーブルに追加
   - 必要に応じてクイックリファレンスも更新

3. **関連ドキュメントからリンク**
   - 双方向リンクを推奨

### ドキュメント更新時

1. **更新履歴を記録**
   - 各ドキュメント末尾または本READMEに記載

2. **影響範囲を確認**
   - 他ドキュメントとの整合性チェック

3. **レビューを受ける**
   - 重要な変更はPRでレビュー

---

## 🔗 外部リソース

### 公式ドキュメント

- [EnTT Wiki](https://github.com/skypjack/entt/wiki) - ECSライブラリ
- [Raylib Cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html) - レンダリング
- [nlohmann/json Documentation](https://json.nlohmann.me/) - JSON
- [Dear ImGui Documentation](https://github.com/ocornut/imgui) - GUI

### 参考資料

- [Refactoring Guru](https://refactoring.guru/) - リファクタリングパターン
- [Game Programming Patterns](https://gameprogrammingpatterns.com/) - ゲームデザインパターン
- [RogueBasin](http://www.roguebasin.com/) - ローグライク開発

---

## 📧 お問い合わせ

ドキュメントに関する質問・提案は以下へ：

- **Issue**: GitHub Issues で質問
- **PR**: ドキュメント改善のプルリクエスト歓迎

---

**最終更新**: 2025-12-02  
**メンテナー**: プロジェクトコントリビューター
