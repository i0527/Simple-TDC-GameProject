# Simple TDC Game - 初回セットアップスクリプト (PowerShell)
#
# このスクリプトは git clone 直後に実行することで、以下を自動化します：
# - CMake プリセット確認
# - 初回ビルド実行
# - WebUI セットアップ確認
# - ゲーム起動準備
#
# 使用方法:
#   PowerShell から実行
#   .\setup.ps1

param(
    [switch]$SkipBuild = $false,
    [switch]$Verbose = $false
)

# ============================================================================
# 色定義
# ============================================================================
$Colors = @{
    Green = 'Green'
    Red = 'Red'
    Yellow = 'Yellow'
    Cyan = 'Cyan'
    Blue = 'Blue'
}

function Write-Header($text) {
    Write-Host "`n" + ("="*60) -ForegroundColor $Colors.Cyan
    Write-Host $text -ForegroundColor $Colors.Cyan -NoNewline
    Write-Host (" "*([Math]::Max(0, 60 - $text.Length))) + "`n" -ForegroundColor $Colors.Cyan
    Write-Host ("="*60) -ForegroundColor $Colors.Cyan
    Write-Host ""
}

function Write-Success($text) {
    Write-Host "✅ $text" -ForegroundColor $Colors.Green
}

function Write-Warning($text) {
    Write-Host "⚠️  $text" -ForegroundColor $Colors.Yellow
}

function Write-Error2($text) {
    Write-Host "❌ $text" -ForegroundColor $Colors.Red
}

function Write-Info($text) {
    Write-Host "ℹ️  $text" -ForegroundColor $Colors.Blue
}

# ============================================================================
# メイン処理
# ============================================================================

Write-Header "Simple TDC Game - Initial Setup"

# プロジェクトルート取得
$projectRoot = (Get-Location).Path
Write-Success "Project root: $projectRoot"
Write-Host ""

# プラットフォーム情報
$osInfo = Get-ComputerInfo -Property CsSystemType
Write-Info "Platform: Windows ($([Environment]::ProcessorCount) cores)"
Write-Host ""

# ============================================================================
# CMake チェック
# ============================================================================

Write-Info "Checking CMake..."

$cmakePath = (Get-Command cmake -ErrorAction SilentlyContinue).Source
if ($cmakePath) {
    Write-Success "CMake found: $cmakePath"
} else {
    Write-Error2 "CMake not found. Please install CMake first."
    Write-Info "Download from: https://cmake.org/download/"
    exit 1
}

Write-Host ""

# ============================================================================
# ビルドディレクトリチェック
# ============================================================================

$buildDir = Join-Path $projectRoot "build"

if (-not (Test-Path $buildDir)) {
    Write-Info "Creating build directory: $buildDir"
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    Write-Success "Build directory created"
} else {
    Write-Success "Build directory exists: $buildDir"
}

Write-Host ""

# ============================================================================
# CMake Configure
# ============================================================================

Write-Header "Running CMake Configure"

$configCmd = @(
    "cmake",
    "-B", $buildDir,
    "-S", $projectRoot,
    "-DCMAKE_BUILD_TYPE=Release"
)

Write-Info "Command: $($configCmd -join ' ')"
Write-Host ""

$process = Start-Process -FilePath $configCmd[0] -ArgumentList $configCmd[1..($configCmd.Count-1)] -WorkingDirectory $projectRoot -PassThru -Wait

if ($process.ExitCode -eq 0) {
    Write-Success "CMake configure completed successfully"
} else {
    Write-Error2 "CMake configure failed"
    exit 1
}

Write-Host ""

# ============================================================================
# Node.js セットアップ確認
# ============================================================================

Write-Header "Checking Node.js Setup"

$nodeDir = Join-Path $buildDir ".nodejs"
$nodeExe = Join-Path $nodeDir "bin" "node.exe"

if (Test-Path $nodeDir) {
    Write-Success "Node.js directory found: $nodeDir"
    
    if (Test-Path $nodeExe) {
        Write-Success "Node.js executable: $nodeExe"
    } else {
        Write-Warning "Node.js executable not found yet: $nodeExe"
        Write-Info "Node.js will be downloaded during CMake configure"
    }
} else {
    Write-Warning "Node.js directory not found: $nodeDir"
    Write-Info "Node.js will be downloaded during CMake configure"
}

Write-Host ""

# ============================================================================
# CMake Build
# ============================================================================

if ($SkipBuild) {
    Write-Warning "Skipping build (--SkipBuild flag set)"
} else {
    Write-Header "Running CMake Build"
    
    $buildCmd = @(
        "cmake",
        "--build", $buildDir,
        "--config", "Release"
    )
    
    Write-Info "Command: $($buildCmd -join ' ')"
    Write-Warning "This may take several minutes on first run..."
    Write-Host ""
    
    $process = Start-Process -FilePath $buildCmd[0] -ArgumentList $buildCmd[1..($buildCmd.Count-1)] -WorkingDirectory $buildDir -PassThru -Wait
    
    if ($process.ExitCode -eq 0) {
        Write-Success "CMake build completed successfully"
    } else {
        Write-Error2 "CMake build failed"
        exit 1
    }
    
    Write-Host ""
}

# ============================================================================
# WebUI セットアップ確認
# ============================================================================

Write-Header "Checking WebUI Setup"

$webUIDir = Join-Path $projectRoot "tools" "webui-editor"
$nodeModules = Join-Path $webUIDir "node_modules"

if (Test-Path $nodeModules) {
    Write-Success "WebUI node_modules found: $nodeModules"
} else {
    Write-Warning "WebUI node_modules not found yet: $nodeModules"
    Write-Info "npm install will be run during CMake build"
}

Write-Host ""

# ============================================================================
# ゲーム実行ファイル確認
# ============================================================================

Write-Header "Checking Game Executable"

$exePath = Join-Path $buildDir "bin" "Release" "SimpleTDCGame.exe"

if (Test-Path $exePath) {
    Write-Success "Game executable found: $exePath"
} else {
    Write-Warning "Game executable not found yet: $exePath"
}

Write-Host ""

# ============================================================================
# 次のステップ表示
# ============================================================================

Write-Header "Next Steps"

Write-Info "セットアップが完了しました！"
Write-Host ""

Write-Info "ゲーム実行:"
if (Test-Path $exePath) {
    Write-Host "  $exePath" -ForegroundColor $Colors.Cyan
} else {
    Write-Host "  $exePath" -ForegroundColor $Colors.Cyan
}

Write-Host ""

Write-Info "WebUI エディター:"
Write-Host "  ブラウザで http://localhost:3000 にアクセス" -ForegroundColor $Colors.Cyan
Write-Host "  （ゲーム起動時に自動起動します）" -ForegroundColor $Colors.Cyan

Write-Host ""

Write-Info "ドキュメント:"
Write-Host "  - CMake + Node.js 統合: docs/CMAKE_NODEJS_INTEGRATION.md" -ForegroundColor $Colors.Cyan
Write-Host "  - WebUI ガイド: tools/webui-editor/README.md" -ForegroundColor $Colors.Cyan
Write-Host "  - 開発マニュアル: docs/DEVELOPER_MANUAL.md" -ForegroundColor $Colors.Cyan

Write-Host ""
Write-Success "Setup completed!"
Write-Host ""
