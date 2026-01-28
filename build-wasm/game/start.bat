@echo off
setlocal
REM CatTD Game - Web版起動スクリプト（どこからでも実行可能）
REM このスクリプトがあるディレクトリに移動してから HTTP サーバーを起動します。

cd /d "%~dp0"

echo.
echo [CatTD Game] Web版を起動しています...
echo サーバー: http://localhost:18000/
echo ゲームURL: http://localhost:18000/CatTDGame.html
echo.
echo ブラウザが開いたら遊べます。終了するにはこのウィンドウで Ctrl+C を押してください。
echo.

REM 少し待ってからブラウザを開く（サーバー起動を待つ）
start /b cmd /c "timeout /t 2 /nobreak >nul && start http://localhost:18000/CatTDGame.html"

python -m http.server 18000
if errorlevel 1 (
    echo.
    echo [エラー] Python が見つかりません。Python 3 をインストールしてください。
    echo https://www.python.org/downloads/
    pause
    exit /b 1
)
