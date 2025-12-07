# 新アーキテクチャ設計文書 - マスター索引

**最終更新**: 2025-12-07  
**バージョン**: 1.1

---

## はじめに

このディレクトリ (`.cursor/`) には、新アーキテクチャ（`include/`, `src/`）の設計文書が含まれています。

**新アーキテクチャの目標**:
- ImGui内部エディタを中核としたデータ駆動設計
- 依存性注入（DI）による疎結合アーキテクチャ
- JSON駆動による3基幹システム（直線TD・マップTD・UI）の拡張性
- 旧コードとの並行開発による段階的移行

---

## 📚 推奨読書順

### 🔰 初めての方（アーキテクチャ全体を理解したい）

1. **[新アーキテクチャ移行プラン](new-arch-migration-plan.md)** (45行)
   - 全体方針と実施ステップの概要
   
2. **[コアアーキテクチャ設計](core-architecture-design.md)** (843行)
   - GameContext, SystemRunner, GameRenderer等のコア層設計
   
3. **[設計原則と制約](design-principles-and-constraints.md)** (488行)
   - グローバル変数禁止、DIパターン、ECS原則等
   
4. **[ディレクトリ構成とビルドターゲット](new-arch-dirs-and-targets.md)** (474行)
   - ファイル配置、CMakeターゲット、移行戦略

### 🎮 ゲームシステム実装者

1. **[JSONスキーマ設計](json-schema-design.md)** (1396行)
   - 全JSONスキーマ定義（entities, waves, abilities, ui等）
   
2. **[TDシステム設計](td-systems-design.md)** (787行)
   - 直線型TD・マップ型TDの詳細設計
   
3. **[UIシステム設計](ui-system-design.md)** (774行)
   - UIレイアウト、バインディング、アニメーション
   
4. **[ゲーム状態とステージ設計](game-state-and-stage-design.md)** (200行)
   - ステージ遷移、セーブ/ロード

### 🛠️ エディタ開発者

1. **[内部エディタ設計](internal-editor-design.md)** (363行)
   - F1〜F4ショートカット、3ペイン構成、ワークスペース
   
2. **[モーション/アニメーション統合ガイド](motion-animation-integration-guide.md)** (270行)
   - パーツアニメーション、状態管理、エディタ統合
   
3. **[モーションエディタ要件](motion-editor-requirements.md)** (55行)
   - モーションエディタ（F4）の詳細要件

### 🔧 サブシステム実装者

1. **[戦闘・衝突システム](combat-collision-design.md)** (143行)
2. **[カメラ・ビューポート](camera-viewport-design.md)** (73行)
3. **[オーディオシステム](audio-system-design.md)** (87行)
4. **[パーティクルエフェクト](particle-effect-system-design.md)** (119行)
5. **[イベントシステム](event-system-design.md)** (36行)

### 📖 ライブラリ参照

1. **[ライブラリガイド](libs_guide.md)** (502行) 📌 **推奨**
   - または [ゲーム開発ライブラリガイド](gamedev_libs_guide.md) (693行)
   - 注: これらは統合予定。現在は`libs_guide.md`を使用してください。
   
2. **[Raylibリソース管理](raylib_resource_mgmt.md)** (991行)
   - Raylibの罠と対処法、RAII実装

3. **[ライブラリ概要](libs-overview.md)** (15行)
   - 簡易リファレンス

---

## 📂 文書一覧（カテゴリ別）

### コア設計

| 文書名 | 行数 | 説明 |
|--------|------|------|
| [core-architecture-design.md](core-architecture-design.md) | 843 | GameContext, World, GameRenderer, SystemRunner |
| [design-principles-and-constraints.md](design-principles-and-constraints.md) | 488 | 設計原則（詳細版） |
| [design-principles.md](design-principles.md) | 462 | 設計原則（概要版）※統合予定 |
| [new-arch-dirs-and-targets.md](new-arch-dirs-and-targets.md) | 474 | ディレクトリ構成、CMake設定 |
| [new-arch-migration-plan.md](new-arch-migration-plan.md) | 45 | 移行プラン概要 |

### データ層

| 文書名 | 行数 | 説明 |
|--------|------|------|
| [json-schema-design.md](json-schema-design.md) | 1396 | 全JSONスキーマ定義 |
| `assets/schemas/*.schema.json` | - | 実体スキーマ（entity/wave/ability/ui_layout） |

### 品質保証

| 文書名 | 行数 | 説明 |
|--------|------|------|
| [testing-strategy.md](testing-strategy.md) | 74 | テスト階層、CI/CD、スキーマ検証方針 |

### 非機能

| 文書名 | 行数 | 説明 |
|--------|------|------|
| [security-guidelines.md](security-guidelines.md) | 32 | I/O・JSON解析・リソース管理の基本チェック |
| [accessibility.md](accessibility.md) | 33 | 色覚/操作/テキスト/アニメーションの基本方針 |

### ゲームシステム

| 文書名 | 行数 | 説明 |
|--------|------|------|
| [td-systems-design.md](td-systems-design.md) | 787 | TD全体設計 |
| [linea-td-detailed-design.md](linea-td-detailed-design.md) | 150 | 直線型TD詳細 ※td-systems-design.mdに統合予定 |
| [ui-system-design.md](ui-system-design.md) | 774 | UIレイアウト・バインディング |
| [game-state-and-stage-design.md](game-state-and-stage-design.md) | 200 | ステージ遷移・セーブロード |

### サブシステム

| 文書名 | 行数 | 説明 |
|--------|------|------|
| [motion-animation-integration-guide.md](motion-animation-integration-guide.md) | 270 | アニメーション・状態管理 |
| [motion-editor-requirements.md](motion-editor-requirements.md) | 55 | モーションエディタ要件 ※統合予定 |
| [combat-collision-design.md](combat-collision-design.md) | 143 | 戦闘・衝突システム |
| [camera-viewport-design.md](camera-viewport-design.md) | 73 | カメラ・ビューポート |
| [audio-system-design.md](audio-system-design.md) | 87 | オーディオシステム |
| [particle-effect-system-design.md](particle-effect-system-design.md) | 119 | パーティクルエフェクト |
| [event-system-design.md](event-system-design.md) | 36 | イベントシステム |

### エディタ

| 文書名 | 行数 | 説明 |
|--------|------|------|
| [internal-editor-design.md](internal-editor-design.md) | 363 | 内部エディタ全体設計 |

### ライブラリ

| 文書名 | 行数 | 説明 |
|--------|------|------|
| [libs_guide.md](libs_guide.md) | 502 | ライブラリ注意点ガイド |
| [gamedev_libs_guide.md](gamedev_libs_guide.md) | 693 | ゲーム開発ライブラリガイド ※統合予定 |
| [raylib_resource_mgmt.md](raylib_resource_mgmt.md) | 991 | Raylibリソース管理詳細 |
| [libs-overview.md](libs-overview.md) | 15 | 簡易リファレンス |

---

## 🔍 トピック別索引

### 依存性注入（DI）
- [core-architecture-design.md](core-architecture-design.md) § 1. GameContext
- [design-principles-and-constraints.md](design-principles-and-constraints.md) § 8. 依存性注入パターン

### JSON駆動設計
- [json-schema-design.md](json-schema-design.md)
- [design-principles-and-constraints.md](design-principles-and-constraints.md) § 5. データ駆動設計

### ECS（Entity Component System）
- [core-architecture-design.md](core-architecture-design.md) § 2. World
- [design-principles-and-constraints.md](design-principles-and-constraints.md) § 6. ECS設計原則

### 仮想FHDレンダリング
- [core-architecture-design.md](core-architecture-design.md) § 3. GameRenderer
- [design-principles-and-constraints.md](design-principles-and-constraints.md) § 7. 仮想FHDレンダリング

### ホットリロード
- [core-architecture-design.md](core-architecture-design.md) § 5.5 HotReloadSystem
- [json-schema-design.md](json-schema-design.md) - 各スキーマにリロード対応

### 内部エディタ
- [internal-editor-design.md](internal-editor-design.md)
- [motion-editor-requirements.md](motion-editor-requirements.md)

### タワーディフェンス
- [td-systems-design.md](td-systems-design.md)
- [linea-td-detailed-design.md](linea-td-detailed-design.md)

### アニメーション
- [motion-animation-integration-guide.md](motion-animation-integration-guide.md)
- [core-architecture-design.md](core-architecture-design.md) § 5.4 AnimationRegistryCache

---

## ⚠️ 既知の問題と改善計画

詳細は [ARCHITECTURE_EVALUATION_AND_IMPROVEMENTS.md](ARCHITECTURE_EVALUATION_AND_IMPROVEMENTS.md) を参照

### 緊急対応が必要
- `World`クラスの設計vs実装の不一致
- 重複文書の統合（design-principles*, libs*, td-systems*）

### 近日中に対応予定
- 実装フェーズの明確化（MVP定義）
- テスト戦略文書の作成
- JSONスキーマファイル（.schema.json）の実体作成

---

## 📝 ドキュメント管理ルール

### 新規文書追加時
1. このINDEX.mdを更新
2. 関連文書へのクロスリファレンスを追加
3. 推奨読書順を見直し

### 文書更新時
- 文書冒頭の「最終更新」日付を更新
- 大きな変更は変更履歴セクションに記録

### 文書統合時
- 統合前の文書は削除せず、統合先へのリンクに置き換え
- 3ヶ月後に旧文書を削除

---

## 🔗 関連リソース

- **実装コード**: `include/new/`, `src/new/`
- **アセット**: `assets/new/definitions/`
- **ビルド設定**: プロジェクトルートの `CMakeLists.txt`
- **既存設計文書**: `.cursor/` 配下（旧アーキテクチャ）

---

## 📧 質問・フィードバック

設計に関する質問や提案がある場合は、以下を参照：
- GitHub Issue: 設計に関するディスカッション
- PR: 文書の改善提案

---

**次のステップ**: 
- 初めての方 → [新アーキテクチャ移行プラン](new-arch-migration-plan.md)を読む
- 実装者 → [コアアーキテクチャ設計](core-architecture-design.md)を読む
- 評価レポート → [ARCHITECTURE_EVALUATION_AND_IMPROVEMENTS.md](ARCHITECTURE_EVALUATION_AND_IMPROVEMENTS.md)を読む
