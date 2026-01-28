# Emscripten Web ビルドスクリプト
# 使用方法: EMSDKプロンプトで実行
# .\build-wasm.ps1

$ErrorActionPreference = "Stop"

Write-Host "=== Cat TD Game - Emscripten Build Script ===" -ForegroundColor Cyan

# プロジェクトルート確認
$ProjectRoot = "Z:\kaken"
if (-not (Test-Path "$ProjectRoot\CMakeLists.txt")) {
    Write-Error "CMakeLists.txt not found in $ProjectRoot"
    exit 1
}

Set-Location $ProjectRoot
Write-Host "Project root: $ProjectRoot" -ForegroundColor Green

# ビルドディレクトリクリーンアップ
$BuildDir = "$ProjectRoot\build-wasm"
if (Test-Path $BuildDir) {
    Write-Host "Cleaning previous build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

# Emscripten環境確認
Write-Host "`n=== Checking Emscripten Environment ===" -ForegroundColor Cyan
try {
    $EmccVersion = & emcc --version 2>&1 | Select-Object -First 1
    Write-Host "Emscripten compiler found: $EmccVersion" -ForegroundColor Green
} catch {
    Write-Error "emcc not found! Please run this script in EMSDK Command Prompt."
    exit 1
}

# アセット確認
Write-Host "`n=== Checking Assets ===" -ForegroundColor Cyan
$AssetsDir = "$ProjectRoot\assets"
$DataDir = "$ProjectRoot\data"

if (Test-Path $AssetsDir) {
    $AssetCount = (Get-ChildItem -Recurse $AssetsDir | Measure-Object).Count
    Write-Host "Assets directory found: $AssetCount files" -ForegroundColor Green
} else {
    Write-Warning "Assets directory not found: $AssetsDir"
}

if (Test-Path $DataDir) {
    $DataCount = (Get-ChildItem -Recurse $DataDir | Measure-Object).Count
    Write-Host "Data directory found: $DataCount files" -ForegroundColor Green
} else {
    Write-Warning "Data directory not found: $DataDir"
}

# CMake設定ステップ
Write-Host "`n=== Running CMake Configuration ===" -ForegroundColor Cyan
& emcmake cmake -B $BuildDir -G Ninja -DCMAKE_BUILD_TYPE=Release

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed!"
    exit 1
}

Write-Host "`nCMake configuration successful!" -ForegroundColor Green

# ビルドステップ
Write-Host "`n=== Building Project ===" -ForegroundColor Cyan
& cmake --build $BuildDir --config Release -- -v

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed!"
    exit 1
}

Write-Host "`nBuild successful!" -ForegroundColor Green

# 成果物確認
Write-Host "`n=== Build Artifacts ===" -ForegroundColor Cyan
$OutputDir = "$BuildDir\game"

if (Test-Path $OutputDir) {
    $Artifacts = Get-ChildItem "$OutputDir\CatTDGame*" -ErrorAction SilentlyContinue
    
    if ($Artifacts) {
        Write-Host "Generated files:" -ForegroundColor Green
        foreach ($File in $Artifacts) {
            $SizeMB = [math]::Round($File.Length / 1MB, 2)
            Write-Host "  $($File.Name) - $SizeMB MB" -ForegroundColor White
        }
        
        # .data ファイル確認
        $DataFile = "$OutputDir\CatTDGame.data"
        if (Test-Path $DataFile) {
            Write-Host "`n✓ CatTDGame.data found - Assets packaged successfully!" -ForegroundColor Green
        } else {
            Write-Warning "✗ CatTDGame.data not found - Assets may not be embedded"
            Write-Host "`nChecking link command..." -ForegroundColor Yellow
            $LinkFile = "$OutputDir\CMakeFiles\CatTDGame.dir\link.txt"
            if (Test-Path $LinkFile) {
                $LinkCmd = Get-Content $LinkFile
                if ($LinkCmd -match "--preload-file") {
                    Write-Host "Link command contains --preload-file flag" -ForegroundColor Green
                    Write-Host $LinkCmd -ForegroundColor Gray
                } else {
                    Write-Warning "Link command missing --preload-file flag!"
                }
            }
        }
    } else {
        Write-Warning "No CatTDGame artifacts found in $OutputDir"
    }
}

# サーバー起動ヘルプ
Write-Host "`n=== Next Steps ===" -ForegroundColor Cyan
Write-Host "To run the game in browser:" -ForegroundColor White
Write-Host "  cd $OutputDir" -ForegroundColor Gray
Write-Host "  python -m http.server 8000" -ForegroundColor Gray
Write-Host "`nThen open: http://localhost:8000/CatTDGame.html" -ForegroundColor White

Write-Host "`n=== Build Complete ===" -ForegroundColor Cyan
