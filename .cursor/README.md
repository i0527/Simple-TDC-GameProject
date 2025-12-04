# .cursor ディレクトリ

このディレクトリには、Cursor IDE でのマルチエージェント開発に最適化されたドキュメントが含まれています。

---

## 📚 ドキュメント一覧

### 1. CURSOR_DEVELOPMENT_GUIDE.md（メインガイド）

**用途**: Cursor IDE でのマルチエージェント開発のための包括的なガイド

**内容**:

- プロジェクト概要
- Cursor IDE 設定方法
- マルチエージェント開発戦略
- 設計改善の要点
- 推奨される設計パターン
- AIエージェント向けタスク分割
- コーディング規約と実装例
- トラブルシューティング

**対象**: 全開発者（初めての人は必読）

---

### 2. AI_AGENT_QUICK_REFERENCE.md（クイックリファレンス）

**用途**: AIエージェントが即座に参照できるクイックリファレンス

**内容**:

- 最重要ルール（5秒チェック）
- エージェント役割別チートシート
- 頻出コードスニペット
- ファイル配置ルール
- よくあるエラーと解決策
- タスク分割のヒント

**対象**: AI エージェント、経験者

---

### 3. ARCHITECTURE_DIAGRAMS.md（アーキテクチャ図解）

**用途**: プロジェクトの構造を視覚的に理解する

**内容**:

- システム全体図
- レイヤーアーキテクチャ
- ECS データフロー
- 依存性注入パターン
- ビルドターゲット関係
- マルチエージェント開発フロー
- シーン遷移図
- リファクタリング戦略

**対象**: 全開発者（アーキテクチャ理解）

---

### 4. UNIFIED_PLATFORM_TODO.md（統合プラットフォーム実装計画）⭐ NEW

**用途**: 統合ゲームプラットフォーム完成に向けた詳細タスク管理

**内容**:

- Phase 1-8の詳細タスクリスト
- マイルストーン管理（4段階）
- Cursor開発ワークフロー
- 優先順位付きタスク管理
- 各Phaseの完了条件
- Cursor Composer/Chat活用例

**対象**: 全開発者（実装タスク管理）

---

## 🚀 クイックスタート

### 初めての人

1. **CURSOR_DEVELOPMENT_GUIDE.md** を読む（30分）
2. **ARCHITECTURE_DIAGRAMS.md** でアーキテクチャを理解（10分）
3. **UNIFIED_PLATFORM_TODO.md** で現在のタスクを確認（5分）
4. **AI_AGENT_QUICK_REFERENCE.md** をブックマーク

### 経験者

1. **UNIFIED_PLATFORM_TODO.md** で作業するPhaseを選択（3分）
2. **AI_AGENT_QUICK_REFERENCE.md** で最新ルールを確認（5分）
3. 必要に応じて **CURSOR_DEVELOPMENT_GUIDE.md** を参照

---

## 📋 使い方

### Cursor Composer での利用

プロンプトに以下を含めることで、AIに適切なコンテキストを提供できます:

```
.cursor/CURSOR_DEVELOPMENT_GUIDE.md の設計パターンに従って実装してください。
```

### マルチエージェント開発

各エージェントの役割は **CURSOR_DEVELOPMENT_GUIDE.md** のセクション3を参照：

1. アーキテクト - 設計
2. コーダー - 実装
3. リファクター - 改善
4. ドキュメンター - 文書化
5. テスター - テスト

---

## 🔗 関連ドキュメント

### プロジェクトルート

- **.cursorrules** - Cursor プロジェクトルール（簡潔版）
- **.github/copilot-instructions.md** - コーディング規約

### docs/ ディレクトリ

- **docs/EXECUTIVE_SUMMARY.md** - プロジェクト概要
- **docs/CODE_ANALYSIS.md** - 詳細分析
- **docs/REFACTORING_PLAN.md** - 改善計画
- **docs/README.md** - ドキュメント索引

---

## 🎯 重要な制約（再掲）

```
✅ DO:
1. SimpleTDCGame_NewArch ターゲットを使用
2. ComponentsNew.h をインクルード
3. 依存性注入（DI）パターンを使用
4. JSON解析は try-catch で囲む
5. ECS コンポーネントは POD 型

❌ DON'T:
1. Singleton パターンを使用
2. Components.h をインクルード
3. SimpleTDCGame での新規開発
4. コンポーネントにロジックを実装
5. 生ポインタでメモリ管理
```

---

## 🔄 更新履歴

| 日付 | 変更内容 |
|------|---------|
| 2025-12-03 | 初版作成（3ドキュメント） |
| 2025-12-04 | UNIFIED_PLATFORM_TODO.md追加（統合プラットフォーム実装計画） |

---

## 📞 フィードバック

このドキュメントに関する質問や改善提案は GitHub Issues でお知らせください。

---

**Cursor IDE でのマルチエージェント開発を最大限に活用してください！**
