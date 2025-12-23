# 旧システム削除計画

**作成日**: 2025-12-08  
**目的**: 戦闘・編成システムにおける旧システムの安全な削除

---

## 📊 調査結果

### 現在のシステム構成

#### ✅ 新システム（使用中・保持）

1. **ECS Systems** (`game/include/Game/Systems/`)
   - `AttackSystem` - 攻撃処理 ✅ 使用中
   - `MovementSystem` - 移動処理 ✅ 使用中
   - `SkillSystem` - スキル処理 ✅ 使用中
   - `AnimationSystem` - アニメーション処理 ✅ 使用中
   - `RenderingSystem` - 描画処理（旧） ✅ 使用中
   - `NewRenderingSystem` - 描画処理（新） ✅ 使用中

2. **Managers** (`game/include/Game/Managers/`)
   - `FormationManager` - 編成管理 ✅ 使用中
   - `EntityManager` - エンティティ管理 ⚠️ 要確認
   - `SkillManager` - スキル管理 ⚠️ 要確認
   - `StageManager` - ステージ管理 ⚠️ 要確認

#### ❌ 旧システム（削除対象）

1. **ドキュメントに記載されているが実装されていないシステム**
   - `CombatSystem` - 存在しない
   - `CombatDetectionSystem` - 存在しない
   - `LaneMovementSystem` - 存在しない
   - `HealthSystem` - 存在しない
   - `DeathSystem` - 存在しない
   - `ProjectileSystem` - 存在しない
   - `KnockbackSystem` - 存在しない
   - `AbilitySystem` - 存在しない
   - `BaseHealthSystem` - 存在しない
   - `ResourceSystem` - 存在しない

2. **未使用の可能性があるマネージャー**
   - `EntityManager` - `DefinitionRegistry`のラッパーのみ
   - `SkillManager` - `DefinitionRegistry`のラッパーのみ
   - `StageManager` - `DefinitionRegistry`のラッパーのみ

---

## 🎯 削除計画

### Phase 1: 未使用マネージャーの確認と削除

#### 1.1 EntityManager, SkillManager, StageManager の使用箇所確認

**確認項目**:
- `entity_manager_->` の使用箇所
- `skill_manager_->` の使用箇所
- `stage_manager_->` の使用箇所

**判定基準**:
- 直接使用されていない → 削除候補
- `DefinitionRegistry`のラッパーのみ → 削除候補

#### 1.2 削除手順

1. **使用箇所の確認**
   ```bash
   grep -r "entity_manager_->" game/
   grep -r "skill_manager_->" game/
   grep -r "stage_manager_->" game/
   ```

2. **削除対象の特定**
   - 使用されていないマネージャーを特定
   - `DefinitionRegistry`を直接使用するように変更

3. **削除実行**
   - `GameApp.h/cpp` からマネージャーの削除
   - マネージャーファイルの削除
   - CMakeLists.txt からの削除

---

### Phase 2: ドキュメントの整理

#### 2.1 古いドキュメントの整理

**対象ドキュメント**:
- `docs/old/HANDOVER.md` - 旧システムの記載
- `docs/old/INTEGRATION_STATUS.md` - 旧システムの記載
- `docs/old/design/td-systems-design.md` - 旧システムの記載

**対応**:
- 旧システムの記載を削除または「アーカイブ済み」と明記
- 現在のシステム構成を反映

---

## ⚠️ 注意事項

### 削除前に確認すべき項目

1. **ビルドテスト**
   - 削除前にビルドが成功することを確認
   - 削除後にビルドが成功することを確認

2. **動作確認**
   - ゲームが正常に動作することを確認
   - 編成システムが正常に動作することを確認
   - 戦闘システムが正常に動作することを確認

3. **依存関係の確認**
   - 他のシステムが削除対象に依存していないことを確認
   - エディタが削除対象に依存していないことを確認

---

## 📝 削除チェックリスト

### EntityManager, SkillManager, StageManager

- [ ] `entity_manager_->` の使用箇所を確認
- [ ] `skill_manager_->` の使用箇所を確認
- [ ] `stage_manager_->` の使用箇所を確認
- [ ] 使用されていないことを確認
- [ ] `GameApp.h` から削除
- [ ] `GameApp.cpp` から削除
- [ ] マネージャーファイルを削除
- [ ] `CMakeLists.txt` から削除
- [ ] ビルドテスト
- [ ] 動作確認

---

## 🔄 削除後の状態

### 保持されるシステム

- `AttackSystem` - 攻撃処理
- `MovementSystem` - 移動処理
- `SkillSystem` - スキル処理
- `AnimationSystem` - アニメーション処理
- `RenderingSystem` - 描画処理（旧）
- `NewRenderingSystem` - 描画処理（新）
- `FormationManager` - 編成管理

### 削除されるシステム

- `EntityManager` - `DefinitionRegistry`で代替
- `SkillManager` - `DefinitionRegistry`で代替
- `StageManager` - `DefinitionRegistry`で代替

---

## 📚 参考資料

- [プロジェクト概要](docs/EXECUTIVE_SUMMARY.md)
- [コード分析](docs/CODE_ANALYSIS.md)
- [リファクタリング計画](docs/REFACTORING_PLAN.md)

