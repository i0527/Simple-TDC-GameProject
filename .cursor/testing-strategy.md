# テスト戦略（新アーキテクチャ）
**作成日**: 2025-12-07  
**対象**: `SimpleTDCGame_NewArch`（include/, src/ の新配置）

---

## 目的
- Core/Data/Systems/Editor 各層の品質担保
- JSON駆動の安全性とホットリロードの安定性を確保
- CI/CD 上での自動検証を標準化

## テスト階層
1. ユニットテスト（Core/Data）
   - 対象: `GameContext`, `World`, `SystemRunner`, `SchemaValidator`
   - ツール: GoogleTest または Catch2
   - 目標: カバレッジ 80% 以上、例外安全性の検証（JSONパースは必ず try-catch で検証）

2. コンポーネント/システムテスト（ECS）
   - 対象: ECS コンポーネント追加/除去、ビュー/グループの動作、SystemRunner の更新順序
   - 重点: POD 準拠、エンティティ生成/破棄のリークなし、名前付きエンティティ参照の整合性

3. データ駆動テスト（JSON I/O）
   - 対象: `assets/definitions` の JSON ロード、`SchemaValidator` によるバリデーション、ホットリロード
   - 追加チェック: スキーマバージョン (`schemaVersion`) の検証、未知フィールドの警告、欠落フィールドのデフォルト補完

4. 統合テスト（描画/更新ループ）
   - 対象: `main_new` の初期化→メインループ→終了まで
   - 観点: 仮想FHDレンダリングの解像度スケール、60FPS ターゲット、システム更新での例外未発生

5. E2E/シナリオテスト（将来）
   - 対象: LineTD 1ステージのプレイ可能フロー（Phase 3 想定）
   - 観点: 波生成、タワー配置、UI操作

## パフォーマンス計測
- 計測ポイント: Update < 12ms, Render < 8ms, JSON初回ロード < 100ms, ホットリロード < 50ms
- 推奨ツール: Tracy Profiler, Visual Studio Profiler
- 定常監視: フレーム時間ヒストグラムをログ/オーバーレイで出力

## CI/CD ガイド
- GitHub Actions で以下を実行:
  - `cmake --preset ninja-release`（依存キャッシュ活用）
  - `cmake --build --preset ninja-release --target SimpleTDCGame_NewArch`
  - ユニット/コンポーネントテスト実行（ctest）
- 成果物: `build/bin/Release/SimpleTDCGame_NewArch.exe` とテストレポート

## テストデータとスキーマ
- 配置: `assets/definitions/`（既存）に対し、スキーマ実体は `assets/schemas/` へ追加予定
- 最低限のスキーマ:
  - `entity.schema.json`
  - `wave.schema.json`
  - `ability.schema.json`
  - `ui_layout.schema.json`
- バージョン管理: `schemaVersion` を必須フィールドとして付与

## 進め方（MVP志向）
1. Phase 1: Core/Data のユニットテストと SchemaValidator の基本ケース
2. Phase 2: JSON ロード + ホットリロードの統合テスト
3. Phase 3: LineTD 最小ステージのシナリオテスト
4. 継続: パフォーマンス計測と CI の自動化

---

## 最小テストケース例（追加）

### ユニット
- GameContext初期化: リソース/入力/DefinitionRegistry が非 null、World が生成されること。
- SchemaValidator:  
  - happy path: 正常JSONで `Validate` が成功しエラーなし。  
  - sad path: 必須欠落・型違いで失敗し、エラー内容を返す。

### コンポーネント/システム
- World API: Create/Destroy/Name解決/HasComponent が期待通り動くこと。

### 統合（ライト）
- LineTD WaveScheduler: サンプルwave JSONをロードし、時間経過でSpawnRequestが期待数生成されること（タイムステップで検証）。

### CI 実行例
- `ctest --output-on-failure` （ユニット＋ライト統合を対象）

---

## 参考
- `ARCHITECTURE_EVALUATION_AND_IMPROVEMENTS.md` §3.3 テスト戦略の不足
- `QUICK_FIXES_GUIDE.md` 緊急対応後のフォロー項目

