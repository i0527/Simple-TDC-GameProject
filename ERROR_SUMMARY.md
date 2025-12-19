# 要約エラーログ

## 🔴 根本原因: コンポーネント定義の二重化

**問題**: `CoreComponents.h` と `NewCoreComponents.h` で同一コンポーネントが重複定義
- `Transform`, `Stats`, `Team`, `Velocity`, `EntityDefId`, `AttackCooldown`, `SkillHolder`, `AbilityHolder`, `SkillCooldown`, `Sprite`, `Animation`
- 両方のヘッダーがインクルードされると C2011 (struct 再定義) が発生

**影響範囲**:
- `RenderingSystem.h`, `SkillSystem.h`, `MovementSystem.h`, `AttackSystem.h` → `CoreComponents.h` 使用
- `TDGameScene.h`, `NewRenderingSystem.h`, `AnimationSystem.h` → `NewCoreComponents.h` 使用
- 混在により型不一致・未定義エラーが連鎖

---

## 1. コンポーネント定義の二重化 (C2011)

**場所**: 
- `game/include/Game/Components/CoreComponents.h` (旧)
- `game/include/Game/Components/NewCoreComponents.h` (新)

**エラー**: 11個のコンポーネントが両方で定義され、再定義エラー

**解決策**: 
- `CoreComponents.h` を削除または非推奨化
- 全ファイルを `NewCoreComponents.h` に統一

---

## 2. GameContext / SimulationContext 型解決エラー

**場所**: 
- `shared/include/Core/GameContext.h` (59-60, 74)
- `shared/src/Core/GameContext.cpp` (50, 118)
- `shared/include/Shared/Simulation/SimulationContext.h` (69-70, 79)

**エラー**:
- C2143/C4430/C2334: `SimulationContext` 型が未解決
- C2065: `simulation_` 未定義
- C2662: `std::unique_ptr::reset` の this 変換失敗
- C2039/C2061: `Shared::Core::GameContext` が存在しない前提で参照

**原因**: 循環参照または前方宣言不足

---

## 3. IFrameProvider / Graphics 型エラー

**場所**:
- `game/src/Game/Systems/AnimationSystem.cpp` (18, 21, 24)
- `game/src/Game/Graphics/AsepriteJsonAtlasProvider.cpp` (148)

**エラー**:
- C2027: `Shared::Data::Graphics::IFrameProvider` が未定義型
- C2440: `const Texture2D*` → `Texture2D*` の const 修飾子削除

**原因**: 前方宣言のみで実体が未インクルード

---

## 4. TDGameScene / FormationScene 未定義型エラー

**場所**:
- `game/include/Game/Scenes/TDGameScene.h`
- `game/src/Game/Scenes/TDGameScene.cpp`
- `game/src/Game/Application/GameApp.cpp` (53, 312, 346, 359)

**エラー**:
- `Team`, `Stats`, `Velocity` が未定義 struct
- C2039: `Shared::Core::GameContext::GetSimulation` が存在しない
- C2672: `std::make_unique` の引数不一致 (8引数)
- C2084: `SpawnEntity` の二重定義

**原因**: `NewCoreComponents.h` のインクルード不足または型解決失敗

---

## 5. entt 連鎖エラー (C2139)

**場所**: `build/_deps/entt-src/src/entt/entity/component.hpp`

**エラー**: 
- `Game::Components::Transform` などが未定義クラスとして扱われる
- `__is_constructible`, `__is_assignable`, `__is_empty` に未定義型を渡す

**原因**: 
- `CoreComponents.h` と `NewCoreComponents.h` の混在
- インクルード順序により完全な定義が entt に到達していない

---

## 🎯 優先修正順序

1. **最優先**: `CoreComponents.h` の使用を全廃し、`NewCoreComponents.h` に統一
2. **次**: `GameContext` / `SimulationContext` の循環参照解決
3. **次**: `IFrameProvider` の前方宣言を実体インクルードに変更
4. **最後**: 型解決後の残存エラー修正

---

## 📋 修正チェックリスト

- [x] `CoreComponents.h` をインクルードしている全ファイルを `NewCoreComponents.h` に変更
- [ ] `CoreComponents.h` を削除または非推奨マーク（後で実施）
- [x] `GameContext.h` と `SimulationContext.h` の循環参照を解決
- [x] `IFrameProvider` の実体ヘッダーをインクルード
- [x] `GameContext.h` の `SimulationContext` を完全修飾名に変更
- [x] `FrameRef::texture` を `const Texture2D*` に変更
- [x] `TDGameScene.cpp` の `Transform` 初期化を修正（7パラメータ対応）

## ✅ 修正完了

主要なエラーを修正しました:
1. **コンポーネント定義の統一**: `NewCoreComponents.h` に後方互換性を追加し、全システムを統一
2. **循環参照の解決**: `SimulationContext.h` と `CharacterFactory.h` のインクルードを前方宣言に変更
3. **型解決の修正**: `GameContext.h` で `SimulationContext` を完全修飾名に変更
4. **const 修飾子の修正**: `FrameRef::texture` を `const Texture2D*` に変更
5. **Transform 初期化の修正**: 新しい7パラメータ形式に対応

