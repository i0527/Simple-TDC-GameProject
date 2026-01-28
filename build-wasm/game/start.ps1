# CatTD Game - Web版起動スクリプト（PowerShell / どこからでも実行可能）
# このスクリプトがあるディレクトリに移動してから HTTP サーバーを起動します。

Set-Location $PSScriptRoot

if (-not (Get-Command python -ErrorAction SilentlyContinue)) {
    Write-Host ""
    Write-Host "[エラー] Python が見つかりません。Python 3 をインストールしてください。" -ForegroundColor Red
    Write-Host "https://www.python.org/downloads/"
    Read-Host "Enter キーで終了"
    exit 1
}

Write-Host ""
Write-Host "[CatTD Game] Web版を起動しています..." -ForegroundColor Cyan
Write-Host "サーバー: http://localhost:18000/"
Write-Host "ゲームURL: http://localhost:18000/CatTDGame.html"
Write-Host ""
Write-Host "ブラウザが開いたら遊べます。終了するにはこのウィンドウで Ctrl+C を押してください。" -ForegroundColor Yellow
Write-Host ""

# 2秒後にブラウザを開く（サーバー起動を待つ）
$job = Start-Job -ScriptBlock {
    Start-Sleep -Seconds 2
    Start-Process "http://localhost:18000/CatTDGame.html"
}

try {
    python -m http.server 18000
}
finally {
    Stop-Job $job -ErrorAction SilentlyContinue
    Remove-Job $job -ErrorAction SilentlyContinue
}
