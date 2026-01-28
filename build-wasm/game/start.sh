#!/bin/sh
# CatTD Game - Web版起動スクリプト（macOS / Linux / どこからでも実行可能）
# このスクリプトがあるディレクトリに移動してから HTTP サーバーを起動します。

cd "$(dirname "$0")"

echo ""
echo "[CatTD Game] Web版を起動しています..."
echo "サーバー: http://localhost:18000/"
echo "ゲームURL: http://localhost:18000/CatTDGame.html"
echo ""
echo "ブラウザが開いたら遊べます。終了するにはこのターミナルで Ctrl+C を押してください。"
echo ""

# 2秒後にブラウザを開く（サーバー起動を待つ）
(sleep 2 && (
  command -v xdg-open >/dev/null 2>&1 && xdg-open "http://localhost:18000/CatTDGame.html"
  command -v open     >/dev/null 2>&1 && open "http://localhost:18000/CatTDGame.html"
)) &

if ! command -v python3 >/dev/null 2>&1 && ! command -v python >/dev/null 2>&1; then
  echo "[エラー] Python が見つかりません。Python 3 をインストールしてください。"
  echo "https://www.python.org/downloads/"
  exit 1
fi

if command -v python3 >/dev/null 2>&1; then
  exec python3 -m http.server 18000
else
  exec python -m http.server 18000
fi
