---
name: ""
overview: デバッグ読み込み関連を整理し、通常ビルドはサンプルJSONを安定使用できる状態にクリーンアップする。
todos:
  - id: clean-toggle
    content: main_new.cppにsample/debug切替フラグを設置しデフォルトsample
    status: completed
  - id: clean-assets
    content: debug JSONはフラグ経由でのみ使用、ログにモード表示
    status: completed
  - id: clean-refine
    content: 不要コメント/残骸除去と簡易確認
    status: completed
---

# Phase2 クリーンアップ計画（デバッグ読込整理）

1) 読み込み経路の整理

- `src/main_new.cpp` にデータファイル切替用の簡易フラグ/定数を設け、デフォルトは sample JSON とする。
- デバッグJSONを使う場合はフラグ変更のみで済むようにし、コード内の一時的な差し替え箇所を除去。

2) デバッグ資産の扱い明確化

- `assets/definitions/*_debug.json` は残しつつ、通常フローで読み込まれないようにする（フラグでのみ有効）。
- ログに「debugモード使用中」を明示するよう調整。

3) 後片付け

- 不要なコメント・残骸を除去し、フォーマットを整える。
- CMake/実行フローへの影響がないことを確認。

4) 軽い確認

- デフォルト（sample）でロード成功を確認。
- フラグをdebugにしてfallbackや警告が期待通り出ることを確認。
