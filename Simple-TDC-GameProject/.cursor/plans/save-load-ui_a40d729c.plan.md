---
name: save-load-ui
overview: タイトル/ホーム画面に最大5スロットのセーブ・ロードUIを追加し、進行/リソース/設定を保存・復元できるようにする
todos:
  - id: review-savedata
    content: SaveData/ユーザーデータ処理を調査・拡張要否を判断
    status: completed
  - id: ui-title-home
    content: Title/Homeにセーブ/ロードUIとスロット一覧を追加
    status: completed
    dependencies:
      - review-savedata
  - id: impl-save-load
    content: 保存/読込ロジックを組み込み（最大5スロット）
    status: completed
    dependencies:
      - ui-title-home
  - id: error-handling
    content: 失敗時のメッセージと無効化制御を追加
    status: completed
    dependencies:
      - impl-save-load
---

# セーブ/ロードUI・機能追加プラン

1. 既存セーブデータ構造の確認と拡張

- `shared/Data/SaveData` 系を調査し、ステージ進行・所持リソース・設定が保持されるようにフィールドを確認/拡張する。
- `UserDataManager` の保存/読込APIを再利用し、最大5スロットに対応する入出力メソッドを整備。

2. タイトル/ホーム画面にセーブ/ロードUIを追加

- `game/src/Game/Scenes/TitleScene.cpp` および `HomeScene.cpp` に「セーブ」「ロード」メニュー/パネルを追加。
- スロット一覧（1〜5）に日時・進行サマリを表示し、「上書き保存」「読み込み」を選択できるUIを実装。
- スロット欠損時は「空」表示、保存確認の簡易ダイアログを付与。

3. セーブ/ロード処理の組み込み

- 保存時: 現在の進行/所持リソース/設定を `SaveData` に詰め、`UserDataManager` でスロット保存。
- 読込時: 選択スロットをロードし、進行/リソース/設定を `GameApp`/`SettingsManager`/シーン遷移に反映。
- ロード後は適切なシーン（例: HomeScene または StageSelect）へ遷移させる。

4. 表示/ロジックの安全性

- ファイル読み書き失敗時のエラーメッセージをUIに表示。
- 進行状況が無いスロットはロード不可でボタンを無効化。
- JSONパースは既存方針どおり try-catch で保護（SaveData側で担保）。