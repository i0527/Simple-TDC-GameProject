CatTD Game - Web版（再配布用）

■ 必要なもの
  - ブラウザ（Chrome, Firefox, Edge など）
  - Python 3（https://www.python.org/downloads/）

■ 起動方法（このフォルダをどこに置いても同じ手順で起動できます）

  【Windows】
  - start.bat をダブルクリック
  - または コマンドプロンプト/PowerShell で:
    start.bat
    または
    powershell -ExecutionPolicy Bypass -File start.ps1

  【macOS / Linux】
  - ターミナルで:
    chmod +x start.sh
    ./start.sh

  起動すると自動でブラウザが開き、ゲームが表示されます。
  終了する場合は、サーバーを起動したウィンドウで Ctrl+C を押してください。

■ 手動で遊ぶ場合
  1. このフォルダで「python -m http.server 18000」を実行
  2. ブラウザで http://localhost:18000/CatTDGame.html を開く

■ 含まれるファイル
  CatTDGame.html  - エントリページ
  CatTDGame.js    - ゲームロジック
  CatTDGame.wasm  - WebAssembly バイナリ
  CatTDGame.data  - アセットデータ
