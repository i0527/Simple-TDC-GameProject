---
name: stage-select-domain
overview: ステージ選択シーンとドメイン/サブドメイン管理を導入し、タイトル→ステージ選択→ゲームの流れにし、次ステージ遷移をドメイン順に行えるようにする
todos:
  - id: title-ui
    content: TitleScene UIの最小ボタン化とBGM表示/ミュート
    status: completed
  - id: transition-slide
    content: トランジションにスライド追加しPush/Pop対応
    status: completed
  - id: td-wave-loop
    content: TDGameSceneのWave進行ループと勝敗UI/遷移
    status: completed
  - id: render-anim
    content: sprite_sheet描画と簡易アニメ/エフェクト追加
    status: completed
  - id: combat-tune
    content: 停止距離と射程の調整・優先順位TODO
    status: completed
  - id: audio-resource
    content: 音源欠損警告とリソース解放フック
    status: completed
  - id: tests-note
    content: 最小テスト項目メモ追加
    status: completed
---

# ステージ選択導入・ドメイン管理プラン

1) データ構造とロード対応

- `StageDef` に `domain`(string) と `subdomain`(int) を追加し、ソート用に利用する。
- `StageLoader` をトップレベル配列（`[ {stage...}, ... ]`）にも対応させ、`domain/subdomain` を読み取る。StageManagerは従来の呼び出しを維持。
- デバッグ用に `stages_debug.json`（トップ配列形式）へ新規ステージを2件追加し、既存ステージにも `domain/subdomain` を付与（例: domain="World1", subdomain=1/2/3）。

2) ステージ選択シーンの追加

- 新シーン `StageSelectScene` を追加し、StageManager/DefinitionRegistryからステージ一覧を取得、`domain`→`subdomain` 昇順で表示。
- UI: シンプルなリスト（またはグリッド）に `domain` と `subdomain` を併記し、選択・決定のみ（プレビューなし）。選択中のハイライト/カーソル、決定キー/クリック対応。
- 選択結果を保持し、TitleSceneから遷移し、決定後に TDGameScene へステージIDを渡す。

3) 遷移フローの変更

- TitleSceneの開始アクションを TDGameScene 直行ではなく StageSelectScene へ遷移するよう変更。
- StageSelectScene から TDGameScene 生成時に選択ステージIDを渡す。戻る操作でタイトルに戻れるようにする。

4) ゲーム側の次ステージ遷移

- TDGameScene に現在ステージID/ドメイン情報を受け取る口を用意し、Victory時に同一`domain`内で `subdomain` 昇順の次ステージがあれば遷移、なければステージ選択に戻る。
- Nextボタン/Nキーは有効な次ステージがあるときのみ活性化し、ない場合は非活性（またはステージ選択へ戻す）。

5) 簡易テスト項目

- StageLoader がトップ配列で読み込めること（ログ/実行確認）。
- ステージ選択でリストがドメイン→サブドメイン順に並ぶこと、選択→開始で該当ステージがロードされること。
- Victory時、次ステージがあれば遷移、なければ選択画面に戻ること。
- 戻る操作でタイトルに戻れること。