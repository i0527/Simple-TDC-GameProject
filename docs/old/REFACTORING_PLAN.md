# リファクタリング計画書

**作成日**: 2025-12-02  
**プロジェクト**: Simple-TDC-GameProject  
**目的**: コードベースの品質向上、統一、保守性の改善

---

## 目次

1. [概要](#1-概要)
2. [リファクタリング戦略](#2-リファクタリング戦略)
3. [優先順位付けマトリクス](#3-優先順位付けマトリクス)
4. [フェーズ別実装計画](#4-フェーズ別実装計画)
5. [移行ガイドライン](#5-移行ガイドライン)
6. [リスク評価と対策](#6-リスク評価と対策)
7. [成功指標](#7-成功指標)

---

## 1. 概要

### 1.1 現状の課題

[CODE_ANALYSIS.md](./CODE_ANALYSIS.md)の分析結果から、以下の主要課題が特定されました：

| 課題 | 影響度 | 緊急度 | 対応優先度 |
|-----|-------|-------|-----------|
| アーキテクチャの混在（3つ） | 高 | 高 | 🔴 最優先 |
| コンポーネント定義の重複 | 高 | 高 | 🔴 最優先 |
| Singletonパターンの過剰使用 | 中 | 中 | 🟡 重要 |
| Roguelike実装未完成 | 中 | 低 | 🟡 重要 |
| WebUIエディタ未着手 | 低 | 低 | 🟢 将来 |
| ドキュメント重複・散在 | 中 | 高 | 🟡 重要 |

### 1.2 リファクタリングの目標

**短期目標（1-2週間）:**
- ✅ コード重複の削減
- ✅ ドキュメントの統合整理
- ✅ ビルドターゲットの明確化

**中期目標（1-2ヶ月）:**
- ✅ アーキテクチャの統一（新アーキへ移行）
- ✅ Roguelike機能の実装完了（Phase 1-3）
- ✅ テストインフラの整備

**長期目標（3ヶ月以上）:**
- ✅ WebUIエディタの実装
- ✅ パフォーマンス最適化
- ✅ クロスプラットフォーム対応

---

## 2. リファクタリング戦略

### 2.1 基本方針

#### 1. 段階的移行（Strangler Fig Pattern）

旧コードを一気に削除せず、新アーキテクチャで置き換えながら段階的に移行：

```
Phase 1: 新旧共存
├─ 旧: SimpleTDCGame (維持・バグ修正のみ)
├─ 新: SimpleTDCGame_NewArch (積極開発)
└─ RL: NetHackGame (実装進行)

Phase 2: 新アーキ優先
├─ 旧: 非推奨マーク、ドキュメント移行案内
├─ 新: メイン開発ターゲット
└─ RL: Phase 1-3完成

Phase 3: 旧アーキ廃止
├─ 旧: 削除
├─ 新: SimpleTDCGame に名称変更
└─ RL: 統合完了
```

#### 2. ブランチ戦略

```
main (リリース可能な安定版)
  │
  ├─ develop (開発統合ブランチ)
  │   │
  │   ├─ feature/refactor-components (コンポーネント整理)
  │   ├─ feature/remove-singletons (Singleton削減)
  │   ├─ feature/roguelike-phase1 (Roguelike Phase 1)
  │   └─ feature/doc-consolidation (ドキュメント統合)
  │
  └─ hotfix/* (緊急修正)
```

#### 3. テスト駆動リファクタリング

```
1. 既存機能のテストを追加
   └─ 動作を保証

2. リファクタリング実施
   └─ テストが通ることを確認

3. 不要コードの削除
   └─ デッドコード除去
```

### 2.2 コード削減方針

#### 削減対象（段階的廃止）

| ファイル/クラス | 現在の状態 | 移行先 | 削除時期 |
|---------------|-----------|--------|---------|
| `Components.h` | 旧定義 | `ComponentsNew.h` | Phase 1 |
| `Game.h/cpp` | 旧ゲームクラス | `GameNew.h/cpp` | Phase 2 |
| `SceneManager` (Singleton) | 旧シーン管理 | 新アーキのシーン | Phase 2 |
| `ConfigManager` (Singleton) | 旧設定管理 | `GameContext` | Phase 1 |
| `Systems.h` (クラス版) | 旧システム | 静的関数版 | Phase 1 |

#### 保持対象（改善して継続使用）

| ファイル/クラス | 理由 | 改善内容 |
|---------------|------|---------|
| `UIManager` | UI統合が有用 | Singleton削減検討 |
| `ResourceManager` | リソース管理必須 | DI対応 |
| Core層全般 | 基盤として優秀 | ドキュメント改善 |

---

## 3. 優先順位付けマトリクス

### 3.1 評価軸

- **価値（Value）**: ユーザー/開発者への利益
- **複雑度（Complexity）**: 実装の難しさ
- **影響範囲（Impact）**: 変更の波及範囲

### 3.2 タスクマトリクス

```
    高価値
      ↑
      │  A: 今すぐやる       B: 計画的に実施
      │  ┌─────────────┬─────────────┐
      │  │ • コンポーネント │ • Roguelike  │
      │  │   統合          │   Phase 1-3 │
      │  │ • ドキュメント  │ • テスト整備  │
      │  │   統合          │             │
      │  ├─────────────┼─────────────┤
      │  │ • Singleton削減│ • WebUIエディタ│
      │  │ • アセット整備  │ • 最適化     │
      │  └─────────────┴─────────────┘
      │  C: 余裕があれば   D: やらない
      ↓
    低価値
      ←─── 低複雑度 ──────── 高複雑度 ───→
```

**A象限（最優先）:**
1. コンポーネント定義の統合
2. ドキュメントの統合整理
3. ビルド設定の明確化

**B象限（計画的実施）:**
4. Roguelike Phase 1-3実装
5. テストインフラ整備
6. Singleton削減

**C象限（余裕があれば）:**
7. アセットの拡充
8. CI/CD強化

**D象限（将来的に）:**
9. WebUIエディタ
10. パフォーマンス最適化

---

## 4. フェーズ別実装計画

### Phase 1: 基盤整理（1-2週間）

**目標:** 重複削減、ドキュメント統合、ビルド明確化

#### タスク1.1: コンポーネント統合

**目的:** `Components.h`の廃止、`ComponentsNew.h`への統一

**手順:**
```bash
1. ComponentsNew.h の確認
   - Core/Game/TD層が適切に統合されているか検証

2. 旧Components.h使用箇所の特定
   find . -name "*.cpp" -o -name "*.h" | xargs grep "Components.h"

3. 移行ガイドの作成
   - 各コンポーネントの新旧対応表
   - コード例

4. 段階的置き換え
   - ファイルごとに #include "Components.h" → "ComponentsNew.h"
   - コンパイル確認

5. Components.h に非推奨警告追加
   #pragma message("Components.h is deprecated. Use ComponentsNew.h")

6. 1週間後、Components.h削除
```

**期待効果:**
- ✅ コード重複削減
- ✅ メンテナンス性向上
- ✅ 混乱の解消

#### タスク1.2: ドキュメント統合

**目的:** docs/フォルダーの整理と統合

**実施内容:**
```
1. 新規作成
   ├─ CODE_ANALYSIS.md (本分析レポート)
   ├─ REFACTORING_PLAN.md (本ドキュメント)
   ├─ INTEGRATION_STATUS.md (3プロジェクト統合状況)
   └─ docs/README.md (ドキュメント索引)

2. 既存整理
   ├─ HANDOVER.md (新アーキ詳細)
   ├─ ROGUELIKE_SYSTEM_DESIGN.md (Roguelike設計)
   ├─ CHARACTER_SYSTEM_DESIGN.md (キャラクターシステム)
   ├─ MIGRATION_GUIDE.md (UIManager移行ガイド)
   ├─ FONT_SETUP.md (フォント設定)
   ├─ BUILD_WITH_NINJA.md (ビルド手順)
   └─ OPTIMIZATION_SUMMARY.md (最適化まとめ)

3. カテゴリ別再構成
   docs/
   ├─ README.md (索引)
   ├─ architecture/
   │   ├─ CODE_ANALYSIS.md
   │   ├─ REFACTORING_PLAN.md
   │   └─ INTEGRATION_STATUS.md
   ├─ design/
   │   ├─ ROGUELIKE_SYSTEM_DESIGN.md
   │   └─ CHARACTER_SYSTEM_DESIGN.md
   ├─ guides/
   │   ├─ MIGRATION_GUIDE.md
   │   ├─ FONT_SETUP.md
   │   └─ BUILD_WITH_NINJA.md
   └─ technical/
       ├─ HANDOVER.md
       └─ OPTIMIZATION_SUMMARY.md
```

**期待効果:**
- ✅ ドキュメント発見性向上
- ✅ 情報重複の削減
- ✅ 新規参加者のオンボーディング改善

#### タスク1.3: ビルドターゲット明確化

**目的:** 3つのターゲットの役割を明確化

**実施内容:**
```cmake
# CMakeLists.txt 冒頭にコメント追加

# ============================================================================
# ビルドターゲット
# ============================================================================
# 1. SimpleTDCGame (旧アーキテクチャ)
#    - 状態: 保守モード（バグ修正のみ）
#    - 用途: 後方互換性維持
#    - 推奨: 新規開発では使用しない
#
# 2. SimpleTDCGame_NewArch (新アーキテクチャ・推奨)
#    - 状態: アクティブ開発
#    - 用途: タワーディフェンスゲーム
#    - 推奨: メイン開発ターゲット
#
# 3. NetHackGame (Roguelikeゲーム)
#    - 状態: 開発中
#    - 用途: ローグライクゲーム
#    - 推奨: Roguelike機能開発
# ============================================================================
```

**README.md更新:**
```markdown
## ビルド

### 推奨ターゲット（新規開発）
```bash
cmake --build build --target SimpleTDCGame_NewArch
```

### その他のターゲット
- `SimpleTDCGame`: 旧アーキ（保守モード）
- `NetHackGame`: Roguelike（開発中）
```

**期待効果:**
- ✅ 開発者の混乱防止
- ✅ 新規貢献者のガイド
- ✅ ターゲット選択の明確化

---

### Phase 2: アーキテクチャ統一（1-2ヶ月）

**目標:** 新アーキテクチャへの完全移行、Roguelike実装

#### タスク2.1: Singleton削減

**対象:** ConfigManager, SceneManager

**戦略:**
```cpp
// Before: Singleton
class ConfigManager {
    static ConfigManager& GetInstance();
    // ...
};

// After: DIパターン
class ConfigManager {
public:
    ConfigManager(const std::string& configPath);
    // ...
};

// GameContextに登録
context.Register<ConfigManager>("assets/config.json");
```

**段階的移行:**
1. 新しいDI版クラスを作成（既存と並存）
2. 新アーキでDI版を使用
3. 旧アーキは既存Singleton維持（互換性）
4. 旧アーキ廃止時にSingleton削除

**期待効果:**
- ✅ テスタビリティ向上
- ✅ 依存関係の明示化
- ✅ マルチインスタンス対応

#### タスク2.2: シーン統合

**目的:** 旧シーンの機能を新アーキに移植

**現状のシーン:**
```
旧アーキ:
├─ TitleScene
├─ HomeScene
├─ TDGameScene
├─ TDTestGameScene
└─ NethackGameScene (使用中？)

新アーキ:
└─ (main_new.cpp内で直接実装)
```

**統合計画:**
```cpp
// 新アーキのシステム方式
namespace Scenes {
    // シーンは単なる初期化関数
    void InitializeTDScene(GameNew& game) {
        // エンティティ生成
        // システム登録
    }
    
    void InitializeRoguelikeScene(GameNew& game) {
        // Roguelike固有の初期化
    }
}

// GameNewでシーン切り替え
game.LoadScene("TD");
game.LoadScene("Roguelike");
```

**期待効果:**
- ✅ シーン切り替え機能の統一
- ✅ コード重複削減
- ✅ 新旧アーキの橋渡し

#### タスク2.3: Roguelike Phase 1-3実装

**Phase 1: 基本移動とターン制（1週間）**

実装内容:
```
□ GridPosition, TurnActor コンポーネント
□ MapData 固定マップ生成
□ TileRenderer 実装
□ TurnManager 実装
□ InputSystem (方向キー処理)
□ GridMovementSystem
```

検証:
```
✓ プレイヤーが方向キーで移動
✓ 壁で移動が止まる
✓ ターンが進行する
```

**Phase 2: ダンジョン生成と視界（1週間）**

実装内容:
```
□ DungeonGenerator (BSP分割)
□ FOVSystem (Shadowcasting)
□ 視界に応じた描画
□ 階段移動
```

検証:
```
✓ ランダムダンジョンが生成される
✓ 視界システムが動作する
✓ 階段で階層移動できる
```

**Phase 3: モンスターと戦闘（2週間）**

実装内容:
```
□ MonsterAI コンポーネント
□ CombatStats コンポーネント
□ AISystem (追跡・徘徊)
□ CombatSystem (ダメージ計算)
□ DeathSystem
□ MessageLog
```

検証:
```
✓ モンスターが出現する
✓ モンスターが追いかけてくる
✓ 戦闘ができる
✓ メッセージが表示される
```

**期待効果:**
- ✅ Roguelike基本機能の完成
- ✅ 設計ドキュメントと実装の一致
- ✅ 3プロジェクトの実質的統合

---

### Phase 3: 品質向上（2-3ヶ月）

**目標:** テスト整備、CI/CD強化、パフォーマンス改善

#### タスク3.1: テストインフラ整備

**導入技術:**
- Google Test (単体テスト)
- Google Mock (モック)
- CMake CTest統合

**テスト方針:**
```
1. コンポーネントテスト
   - POD型の検証（サイズ、アライメント）
   
2. システムテスト
   - 各システムの入出力検証
   - エッジケース処理
   
3. 統合テスト
   - ゲームシナリオのエンドツーエンドテスト
```

**CMake統合:**
```cmake
enable_testing()

add_executable(ComponentTests tests/ComponentTests.cpp)
target_link_libraries(ComponentTests gtest gtest_main)
add_test(NAME ComponentTests COMMAND ComponentTests)
```

**カバレッジ目標:**
- Phase 3.1: 30%
- Phase 3.2: 50%
- Phase 3.3: 70%

#### タスク3.2: CI/CD強化

**現状:** GitHub Actions（Windowsビルドのみ）

**改善計画:**
```yaml
# .github/workflows/ci.yml
name: CI/CD

on: [push, pull_request]

jobs:
  build-and-test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: cmake --build build
      - name: Test
        run: ctest --test-dir build
      - name: Code Coverage
        run: # カバレッジレポート生成
      - name: Upload Artifacts
        uses: actions/upload-artifact@v4
```

**追加機能:**
- ✅ 自動テスト実行
- ✅ コードカバレッジ測定
- ✅ 静的解析（Clang-Tidy）
- ✅ ビルドキャッシュ

#### タスク3.3: パフォーマンス最適化

**プロファイリング:**
```
1. Tracy統合
   - フレームタイムプロファイリング
   - メモリ使用量監視

2. ホットスポット特定
   - システム実行時間測定
   - ボトルネック分析
```

**最適化候補:**
```
□ 空間分割（Quadtree）
  - 衝突判定の高速化
  
□ オブジェクトプーリング
  - エンティティ生成の高速化
  
□ バッチレンダリング
  - 描画コールの削減
  
□ マルチスレッド対応
  - システムの並列実行
```

---

## 5. 移行ガイドライン

### 5.1 開発者向けガイド

#### 新規機能開発時

**推奨フロー:**
```
1. 新アーキテクチャ（SimpleTDCGame_NewArch）で開発
2. ComponentsNew.h を使用
3. 静的関数システムで実装
4. GameContext経由で依存取得
```

**非推奨:**
```
✗ 旧アーキテクチャでの新規開発
✗ Singleton の新規追加
✗ Components.h の使用
```

#### バグ修正時

**旧アーキテクチャ:**
```
- クリティカルバグのみ修正
- 新アーキに同じ修正を適用
- Issue に "legacy" ラベル追加
```

**新アーキテクチャ:**
```
- 通常通り修正
- テスト追加推奨
```

### 5.2 コードレビューガイドライン

**レビュー観点:**
```
□ 新アーキテクチャを使用しているか？
□ Singletonを避けているか？
□ ComponentsNew.h を使用しているか？
□ 適切にコメントされているか？
□ テストが追加されているか？（Phase 3以降）
```

**自動チェック（将来）:**
```cpp
// .clang-tidy
Checks: '-*,
  modernize-*,
  readability-*,
  performance-*'
```

---

## 6. リスク評価と対策

### 6.1 技術的リスク

| リスク | 影響度 | 確率 | 対策 |
|-------|-------|------|------|
| 旧アーキ依存コードの見落とし | 高 | 中 | 段階的移行、並存期間設定 |
| パフォーマンス劣化 | 中 | 低 | プロファイリング、ベンチマーク |
| テストの不足 | 高 | 高 | テストカバレッジ測定、必須化 |
| 破壊的変更による互換性喪失 | 低 | 低 | セマンティックバージョニング |

### 6.2 スケジュールリスク

| リスク | 影響度 | 確率 | 対策 |
|-------|-------|------|------|
| Phase 2が長引く | 中 | 中 | Phase分割、MVP優先 |
| Roguelike実装が難航 | 中 | 高 | Phase 1のみ必須、2-3は柔軟に |
| WebUI実装が遅延 | 低 | 高 | Phase 4以降に延期可 |

### 6.3 リソースリスク

| リスク | 影響度 | 確率 | 対策 |
|-------|-------|------|------|
| アセット不足 | 低 | 高 | フォールバック機能活用 |
| 開発者リソース不足 | 中 | 中 | 優先度調整、外部貢献募集 |

---

## 7. 成功指標

### 7.1 定量指標

| 指標 | 現状 | Phase 1 | Phase 2 | Phase 3 |
|-----|------|---------|---------|---------|
| コード行数 | 21,378 | -5% | -10% | -15% |
| 重複コード率 | 高 | 中 | 低 | 最小 |
| Singleton数 | 5 | 5 | 3 | 1 |
| テストカバレッジ | 0% | 10% | 30% | 50% |
| ビルド時間 | ベースライン | -5% | -10% | -15% |
| ドキュメント数 | 7 | 11 | 12 | 15 |

### 7.2 定性指標

**Phase 1 完了条件:**
- ✅ ドキュメントが整理され、README から辿れる
- ✅ Components.h が非推奨化されている
- ✅ ビルドターゲットの役割が明確

**Phase 2 完了条件:**
- ✅ 新アーキがメイン開発ターゲットとして機能
- ✅ Roguelike Phase 1 が動作する
- ✅ Singleton が3つ以下に削減

**Phase 3 完了条件:**
- ✅ テストカバレッジ 50% 以上
- ✅ CI/CD で自動テスト実行
- ✅ パフォーマンス改善が確認できる

### 7.3 ユーザー価値指標

| 指標 | 測定方法 | 目標 |
|-----|---------|------|
| ビルド成功率 | GitHub Actions | 95%以上 |
| 新規貢献者のオンボーディング時間 | アンケート | 50%削減 |
| バグ報告から修正までの時間 | Issue追跡 | 30%削減 |

---

## 付録A: チェックリスト

### Phase 1 チェックリスト

**コンポーネント統合:**
- [ ] ComponentsNew.h の内容確認
- [ ] Components.h 使用箇所の特定
- [ ] 移行ガイド作成
- [ ] 段階的置き換え実施
- [ ] Components.h に非推奨警告追加
- [ ] Components.h 削除

**ドキュメント統合:**
- [ ] CODE_ANALYSIS.md 作成
- [ ] REFACTORING_PLAN.md 作成
- [ ] INTEGRATION_STATUS.md 作成
- [ ] docs/README.md 作成（索引）
- [ ] カテゴリ別ディレクトリ作成
- [ ] 既存ドキュメント移動

**ビルド明確化:**
- [ ] CMakeLists.txt コメント追加
- [ ] README.md ビルドセクション更新
- [ ] 各ターゲットの役割明記

### Phase 2 チェックリスト

**Singleton削減:**
- [ ] ConfigManager DI版作成
- [ ] SceneManager 代替方式検討
- [ ] GameContext統合
- [ ] 移行テスト

**Roguelike Phase 1:**
- [ ] GridPosition, TurnActor 実装
- [ ] MapData 実装
- [ ] TileRenderer 実装
- [ ] TurnManager 実装
- [ ] InputSystem 実装
- [ ] GridMovementSystem 実装
- [ ] 動作確認テスト

**Roguelike Phase 2:**
- [ ] DungeonGenerator 実装
- [ ] FOVSystem 実装
- [ ] 視界描画実装
- [ ] 階段移動実装
- [ ] 動作確認テスト

**Roguelike Phase 3:**
- [ ] MonsterAI 実装
- [ ] CombatStats 実装
- [ ] AISystem 実装
- [ ] CombatSystem 実装
- [ ] DeathSystem 実装
- [ ] MessageLog 実装
- [ ] 動作確認テスト

### Phase 3 チェックリスト

**テスト整備:**
- [ ] Google Test 導入
- [ ] コンポーネントテスト作成
- [ ] システムテスト作成
- [ ] CTest 統合
- [ ] カバレッジ測定開始

**CI/CD強化:**
- [ ] 自動テスト実行
- [ ] カバレッジレポート
- [ ] 静的解析追加
- [ ] ビルドキャッシュ

**最適化:**
- [ ] Tracy 統合
- [ ] プロファイリング実施
- [ ] ホットスポット特定
- [ ] 最適化実装
- [ ] ベンチマーク比較

---

## 付録B: 参考資料

### 内部ドキュメント
- [CODE_ANALYSIS.md](./CODE_ANALYSIS.md) - コード分析レポート
- [INTEGRATION_STATUS.md](./INTEGRATION_STATUS.md) - 統合状況
- [HANDOVER.md](./HANDOVER.md) - 新アーキテクチャ詳細
- [ROGUELIKE_SYSTEM_DESIGN.md](./ROGUELIKE_SYSTEM_DESIGN.md) - Roguelike設計

### 外部リソース
- [Refactoring Guru](https://refactoring.guru/) - リファクタリングパターン
- [Strangler Fig Pattern](https://martinfowler.com/bliki/StranglerFigApplication.html) - 段階的移行
- [EnTT Documentation](https://github.com/skypjack/entt/wiki) - ECSベストプラクティス

---

**改訂履歴:**
- 2025-12-02: 初版作成
