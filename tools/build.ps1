<#
.SYNOPSIS
    Simple-TDC-GameProject ビルドスクリプト (PowerShell版)

.DESCRIPTION
    Visual Studio 2022 (VS17) Build Tools を使用してプロジェクトをビルドします。

.PARAMETER BuildType
    ビルドタイプ: Debug または Release (デフォルト: Debug)

.PARAMETER Clean
    ビルドディレクトリをクリーンしてから再構築

.PARAMETER Run
    ビルド後に実行

.EXAMPLE
    .\build.ps1
    .\build.ps1 -BuildType Release
    .\build.ps1 -Clean -Run
#>

param(
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Debug",

    [switch]$Clean,

    [switch]$Run
)

$ErrorActionPreference = "Stop"

# プロジェクトルート
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir

Write-Host "============================================" -ForegroundColor Cyan
Write-Host " Cat TD Game - $BuildType Build (PowerShell)" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# Visual Studio を検索
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (-not (Test-Path $vsWhere)) {
    Write-Host "[ERROR] vswhere.exe が見つかりません" -ForegroundColor Red
    Write-Host "Visual Studio 2022 または Build Tools をインストールしてください"
    exit 1
}

# VS2022を優先的に検索（バージョン17.x）、見つからない場合は最新版を使用
$vsPath = & $vsWhere -version "[17.0,18.0)" -latest -property installationPath
if (-not $vsPath) {
    $vsPath = & $vsWhere -latest -property installationPath
}

if (-not $vsPath) {
    Write-Host "[ERROR] Visual Studio が見つかりません" -ForegroundColor Red
    exit 1
}

Write-Host "[INFO] Visual Studio Path: $vsPath" -ForegroundColor Green

# Developer PowerShell環境をロード
$vcvarsall = Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat"

if (-not (Test-Path $vcvarsall)) {
    Write-Host "[ERROR] vcvarsall.bat が見つかりません" -ForegroundColor Red
    exit 1
}

# vcvarsallを呼び出して環境変数を取得
Write-Host "[INFO] 開発環境を初期化中..." -ForegroundColor Green
$pinfo = New-Object System.Diagnostics.ProcessStartInfo
$pinfo.FileName = "cmd.exe"
$pinfo.Arguments = "/c `"$vcvarsall`" x64 && set"
$pinfo.RedirectStandardOutput = $true
$pinfo.UseShellExecute = $false
$pinfo.CreateNoWindow = $true

$process = New-Object System.Diagnostics.Process
$process.StartInfo = $pinfo
$process.Start() | Out-Null
$output = $process.StandardOutput.ReadToEnd()
$process.WaitForExit()

# 環境変数を設定
$output -split "`r?`n" | ForEach-Object {
    if ($_ -match "^([^=]+)=(.*)$") {
        [Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
    }
}

# CMake確認
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    Write-Host "[ERROR] CMake が見つかりません" -ForegroundColor Red
    exit 1
}

# Ninja確認
$ninja = Get-Command ninja -ErrorAction SilentlyContinue
if ($ninja) {
    Write-Host "[INFO] Ninja を使用します" -ForegroundColor Green
    $generator = "Ninja"
    $generatorArgs = @()
} else {
    Write-Host "[WARNING] Ninja が見つかりません。Visual Studio ジェネレータを使用します" -ForegroundColor Yellow
    $generator = "Visual Studio 17 2022"
    $generatorArgs = @("-A", "x64")
}

# ビルドディレクトリ
$buildDir = if ($BuildType -eq "Debug") {
    Join-Path $ProjectRoot "build_cli"
} else {
    Join-Path $ProjectRoot "build_release"
}

# クリーン処理
if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "[INFO] ビルドディレクトリをクリーン中..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $buildDir
}

# ビルドディレクトリ作成
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

Write-Host ""
Write-Host "[INFO] CMake構成中..." -ForegroundColor Green
Write-Host "============================================"

$cmakeArgs = @("-G", $generator) + $generatorArgs + @(
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-B", $buildDir,
    "-S", $ProjectRoot
)

& cmake @cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[ERROR] CMake構成に失敗しました" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "[INFO] ビルド中..." -ForegroundColor Green
Write-Host "============================================"

& cmake --build $buildDir --config $BuildType --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[ERROR] ビルドに失敗しました" -ForegroundColor Red
    exit 1
}

$exePath = Join-Path $buildDir "game\CatTDGame.exe"

Write-Host ""
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "[SUCCESS] ビルド完了!" -ForegroundColor Green
Write-Host "============================================" -ForegroundColor Cyan
Write-Host "実行ファイル: $exePath"
Write-Host ""

# 実行オプション
if ($Run) {
    if (Test-Path $exePath) {
        Write-Host "[INFO] ゲームを起動中..." -ForegroundColor Green
        Push-Location (Split-Path -Parent $exePath)
        & $exePath
        Pop-Location
    } else {
        Write-Host "[ERROR] 実行ファイルが見つかりません: $exePath" -ForegroundColor Red
        exit 1
    }
}
