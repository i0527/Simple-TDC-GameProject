# ドキュメント一覧

Simple TDC Game Project の設計・実装ドキュメント集です。

---

## 📋 現行設計ドキュメント

### 主要設計

| ドキュメント | 説明 | 最終更新 |
|------------|------|----------|
| [CHARACTER_RENDERING_DESIGN.md](CHARACTER_RENDERING_DESIGN.md) | **キャラクター描画システム設計** - スプライトシート統一仕様 | 2025-12-18 |
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

1. **描画設計**: [CHARACTER_RENDERING_DESIGN.md](CHARACTER_RENDERING_DESIGN.md)
   - メインキャラ: 256×256 スプライトシート（Aseprite形式）
   - サブキャラ: 128×128以下 スプライトシート
   - 統一描画パイプライン（レイヤー分離）

2. **アセット配置**:

   ```
   assets/
   ├── mainCharacters/{name}/{name}.png, {name}.json
   └── subCharacters/{name}/{name}.png, {name}.json
   ```

3. **JSON定義**:
   - EntityDef: `assets/definitions/entities_*.json`
   - アトラス: Aseprite JSON Array形式（frames + meta.frameTags）

### データ管理

- **セーブデータ**: [USER_DATA_SAVE_AND_LOADING.md](USER_DATA_SAVE_AND_LOADING.md)
- **スキーマ**: `assets/schemas/`

---

## 🔄 設計変更履歴

### 2025-12-18: キャラクター描画システム統一

**変更内容**:

- メインキャラクター: セルアニメ形式 → 256×256 スプライトシート
- サブキャラクター: 可変スプライトシート → 128×128以下 スプライトシート統一
- 描画パイプライン: 形式分岐 → 統一パイプライン（レイヤー分離）

**影響範囲**:

- `game/src/Game/Systems/RenderingSystem.cpp` - 統一描画実装
- `game/include/Game/Components/CoreComponents.h` - Animation構造簡略化
- アセット作成ワークフロー - Aseprite統一エクスポート

**詳細**: [CHARACTER_RENDERING_DESIGN.md](CHARACTER_RENDERING_DESIGN.md)

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
