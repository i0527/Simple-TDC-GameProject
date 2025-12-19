# ドキュメント一覧

Simple TDC Game Project の設計・実装ドキュメント集です。

---

## 📋 現行設計ドキュメント

### 主要設計

| ドキュメント | 説明 | 最終更新 |
|------------|------|----------|
| [CHARACTER_RENDERING_DESIGN.md](CHARACTER_RENDERING_DESIGN.md) | **キャラクター描画システム設計** - Provider パターン・段階移行対応 | 2025-12-18 |
| [MIGRATION_GUIDE_RENDERING.md](MIGRATION_GUIDE_RENDERING.md) | 描画システム移行ガイド - 段階実装手順 | 2025-12-18 |
| [PHASE1_IMPLEMENTATION_REPORT.md](PHASE1_IMPLEMENTATION_REPORT.md) | Phase 1 実装完了レポート - GridSheetProvider + 基本描画 | 2025-12-18 |
| [IMPLEMENTATION_ROADMAP.md](IMPLEMENTATION_ROADMAP.md) | 実装ロードマップ - フェーズ別タスク管理 | 2025-12-08 |
| [TECHNICAL_REQUIREMENTS.md](TECHNICAL_REQUIREMENTS.md) | 技術要件仕様書 | - |
| [USER_DATA_SAVE_AND_LOADING.md](USER_DATA_SAVE_AND_LOADING.md) | ユーザーデータ保存・ロード仕様 | - |
| [RECONSTRUCTION_REPORT.md](RECONSTRUCTION_REPORT.md) | プロジェクト再構築レポート | - |

### 設計詳細

| カテゴリ | 内容 |
|---------|------|
| **アーキテクチャ** | ECS（EnTT）ベース、3層分離（Shared/Game/Editor） |
| **描画** | 統一スプライトシートパイプライン（メイン256×256、サブ128×128以下） |
| **UI** | 1920×1080 FHD固定、日本語対応（NotoSansJP-Medium.ttf） |
| **ビルド** | CMake + FetchContent、Windows（MSVC）対応 |

---

## 📦 旧設計ドキュメント（参考）

旧設計の記録は [`old/`](old/) ディレクトリに保管されています。

| ドキュメント | 説明 |
|------------|------|
| [old/CHARACTER_SYSTEM_DESIGN.md](old/CHARACTER_SYSTEM_DESIGN.md) | 旧キャラクターシステム設計（セルアニメ/スプライト二重管理） |
| [old/DEVELOPER_MANUAL.md](old/DEVELOPER_MANUAL.md) | 開発者マニュアル |
| [old/WEBUI_TO_INGAME_EDITOR_MIGRATION.md](old/WEBUI_TO_INGAME_EDITOR_MIGRATION.md) | WebUIからインゲームエディタへの移行計画 |

その他の旧ドキュメントは [`old/`](old/) 内を参照してください。

---

## 🎯 クイックリファレンス

### 新規開発者向け

1. **プロジェクト概要**: [RECONSTRUCTION_REPORT.md](RECONSTRUCTION_REPORT.md)
2. **実装状況**: [IMPLEMENTATION_ROADMAP.md](IMPLEMENTATION_ROADMAP.md)
3. **技術仕様**: [TECHNICAL_REQUIREMENTS.md](TECHNICAL_REQUIREMENTS.md)

### キャラクター実装

1. **描画設計**: [CHARACTER_RENDERING_DESIGN.md](CHARACTER_RENDERING_DESIGN.md) (v3.0)
   - **新設計**: Provider抽象化・段階移行対応
   - **Phase 1**: GridSheetProvider（グリッド形式）
   - **Phase 2**: AsepriteJsonAtlasProvider（Packed形式）
   - **Phase 3**: TexturePackerAtlasProvider（統合アトラス）

2. **実装状況**: [PHASE1_IMPLEMENTATION_REPORT.md](PHASE1_IMPLEMENTATION_REPORT.md)
   - ✅ Phase 1 コード実装完了
   - 次: ビルド・テスト・統合

3. **移行ガイド**: [MIGRATION_GUIDE_RENDERING.md](MIGRATION_GUIDE_RENDERING.md)
   - 段階的なリファクタリング手順
   - 既存コード破壊を回避しながら段階実装

4. **アセット配置**:

   ```
   assets/
   ├── mainCharacters/{name}/{name}.png, clips.json
   └── subCharacters/{name}/{name}.png, clips.json
   ```

5. **JSON定義**:
   - EntityDef: `assets/definitions/entities_*.json`
   - GridSheet clips: JSON形式（start/count/loop/fps）
   - Aseprite: JSON Array形式（Phase 2以降）

### データ管理

- **セーブデータ**: [USER_DATA_SAVE_AND_LOADING.md](USER_DATA_SAVE_AND_LOADING.md)
- **スキーマ**: `assets/schemas/`

---

## 🔄 設計変更履歴

### 2025-12-18: Provider パターン導入 + 段階移行対応（v2.0 → v3.0）

**変更内容**:

- **アーキテクチャ**: Grid/Aseprite/TexturePacker の 2分岐 → Provider 抽象化で一本化
- **段階移行**: Grid → Packed → 統合アトラス への移行を容易に
- **コンポーネント**: Animation/Transform/Sprite を新設計に更新
- **システム**: RenderingSystem の 2分岐ロジック廃止、統一描画化

**実装成果物**:

- ✅ FrameRef, AnimClip, SpriteSet データ構造
- ✅ IFrameProvider インターフェース（5メソッド）
- ✅ GridSheetProvider Phase 1 実装（GridSheetProvider.h/.cpp）
- ✅ SpriteRenderer 統一描画（SpriteRenderer.h/.cpp）
- ✅ AnimationSystem 新実装（AnimationSystem.h/.cpp）
- ✅ NewRenderingSystem 統一描画（NewRenderingSystem.h/.cpp）
- ✅ NewCoreComponents 更新コンポーネント（NewCoreComponents.h）
- ✅ 移行ガイド（MIGRATION_GUIDE_RENDERING.md）
- ✅ Phase 1 実装レポート（PHASE1_IMPLEMENTATION_REPORT.md）

**影響範囲**:

- `shared/include/Data/Graphics/` - 新FrameRef/AnimClip/IFrameProvider
- `game/include/Game/Graphics/` - GridSheetProvider, SpriteRenderer
- `game/include/Game/Components/` - NewCoreComponents
- `game/include/Game/Systems/` - AnimationSystem, NewRenderingSystem
- `game/src/Game/Graphics/` - 実装ファイル
- `game/src/Game/Systems/` - 実装ファイル

**詳細**:

- [CHARACTER_RENDERING_DESIGN.md](CHARACTER_RENDERING_DESIGN.md) (v3.0)
- [MIGRATION_GUIDE_RENDERING.md](MIGRATION_GUIDE_RENDERING.md)
- [PHASE1_IMPLEMENTATION_REPORT.md](PHASE1_IMPLEMENTATION_REPORT.md)

**前の設計（v2.0 - アーカイブ）**: [old/CHARACTER_SYSTEM_DESIGN.md](old/CHARACTER_SYSTEM_DESIGN.md)

---

## 📝 ドキュメント作成ガイドライン

新しいドキュメントを追加する際は以下に従ってください:

1. **Markdown形式**: `.md` 拡張子
2. **日本語**: 全文日本語（コード例は英語変数名可）
3. **配置**:
   - 現行設計: `docs/` 直下
   - 旧設計・参考資料: `docs/old/`
4. **リンク**: 相対パスでクロスリファレンス
5. **更新日**: ドキュメント冒頭に記載

---

**管理**: GitHub Copilot + プロジェクトリード  
**最終更新**: 2025-12-18
