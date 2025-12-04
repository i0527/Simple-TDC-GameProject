# build.ps1 - Windows専用ビルドスクリプト
#
# Usage:
#   .\scripts\build.ps1 [-Config Release|Debug] [-Clean] [-Preset ninja-release|ninja-debug|vs2022-release|vs2022-debug]
#
# このスクリプトは:
#   1. Ninjaジェネレータを優先使用（高速ビルド）
#   2. Ninjaが利用できない場合はVisual Studioジェネレータにフォールバック
#   3. 必要に応じてNinjaを自動ダウンロード

param(
    [ValidateSet("Release", "Debug")]
    [string]$Config = "Release",
    [ValidateSet("ninja-release", "ninja-debug", "vs2022-release", "vs2022-debug", "vs2019-release", "vs2019-debug", "auto")]
    [string]$Preset = "auto",
    [switch]$Clean
)

$ErrorActionPreference = "Continue"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$BuildDir = Join-Path $ProjectRoot "build"
$ToolsBinDir = Join-Path $ProjectRoot ".tools\bin"
$NinjaBinary = Join-Path $ToolsBinDir "ninja.exe"

# ヘルパー関数
function Write-InfoMessage {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Cyan
}

function Write-SuccessMessage {
    param([string]$Message)
    Write-Host "[SUCCESS] $Message" -ForegroundColor Green
}

function Write-WarningMessage {
    param([string]$Message)
    Write-Host "[WARNING] $Message" -ForegroundColor Yellow
}

function Write-ErrorMessage {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

# ビルドディレクトリのクリーン
function Clean-BuildDir {
    if (Test-Path $BuildDir) {
        Write-InfoMessage "Cleaning build directory..."
        Remove-Item -Path $BuildDir -Recurse -Force
    }
}

# Ninjaの利用可能性チェック
function Test-NinjaAvailable {
    # システムPATHをチェック
    $ninja = Get-Command ninja -ErrorAction SilentlyContinue
    if ($ninja) {
        Write-InfoMessage "Found Ninja in PATH: $($ninja.Source)"
        return $true
    }

    # .tools/binをチェック
    if (Test-Path $NinjaBinary) {
        Write-InfoMessage "Found Ninja in .tools/bin: $NinjaBinary"
        $env:PATH = "$ToolsBinDir;$env:PATH"
        return $true
    }

    return $false
}

# Ninjaのダウンロード
function Install-NinjaIfNeeded {
    Write-InfoMessage "Ninja not found, attempting to download..."
    
    $bootstrapScript = Join-Path $ScriptDir "bootstrap-ninja.ps1"
    if (-not (Test-Path $bootstrapScript)) {
        Write-WarningMessage "bootstrap-ninja.ps1 not found"
        return $false
    }

    try {
        & $bootstrapScript
        
        if (Test-Path $NinjaBinary) {
            Write-SuccessMessage "Ninja downloaded successfully"
            $env:PATH = "$ToolsBinDir;$env:PATH"
            return $true
        }
    }
    catch {
        Write-WarningMessage "Failed to download Ninja: $_"
    }

    return $false
}

# Visual Studioジェネレータの検出
function Get-VSGenerator {
    # VS2022を優先
    $vs2022 = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -version "[17.0,18.0)" -property installationPath 2>$null
    if ($vs2022) {
        Write-InfoMessage "Found Visual Studio 2022"
        return "vs2022"
    }

    # VS2019
    $vs2019 = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -version "[16.0,17.0)" -property installationPath 2>$null
    if ($vs2019) {
        Write-InfoMessage "Found Visual Studio 2019"
        return "vs2019"
    }

    Write-WarningMessage "No Visual Studio found"
    return $null
}

# CMake設定
function Invoke-CMakeConfigure {
    param([string]$Preset)

    Write-InfoMessage "Configuring with preset: $Preset"
    
    Push-Location $ProjectRoot
    try {
        & cmake --preset $Preset
        $exitCode = $LASTEXITCODE
        
        if ($exitCode -ne 0) {
            Write-WarningMessage "CMake configure failed with exit code: $exitCode"
            return $false
        }
        return $true
    }
    catch {
        Write-WarningMessage "CMake configure exception: $_"
        return $false
    }
    finally {
        Pop-Location
    }
}

# CMakeビルド
function Invoke-CMakeBuild {
    param([string]$Preset)

    Write-InfoMessage "Building with preset: $Preset"
    
    Push-Location $ProjectRoot
    try {
        & cmake --build --preset $Preset
        $exitCode = $LASTEXITCODE
        
        return ($exitCode -eq 0)
    }
    catch {
        Write-WarningMessage "CMake build exception: $_"
        return $false
    }
    finally {
        Pop-Location
    }
}

# メイン処理
function Main {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor White
    Write-Host "  Simple-TDC-GameProject Build Script  " -ForegroundColor White
    Write-Host "  Windows Only                         " -ForegroundColor White
    Write-Host "========================================" -ForegroundColor White
    Write-Host ""
    Write-InfoMessage "Configuration: $Config"
    Write-InfoMessage "Project root: $ProjectRoot"
    Write-Host ""

    # クリーン処理
    if ($Clean) {
        Clean-BuildDir
    }

    # プリセットが指定されている場合
    if ($Preset -ne "auto") {
        $configPreset = $Preset
        $buildPreset = $Preset
        
        if (Invoke-CMakeConfigure -Preset $configPreset) {
            if (Invoke-CMakeBuild -Preset $buildPreset) {
                Write-Host ""
                Write-SuccessMessage "Build completed successfully!"
                return
            }
        }
        
        Write-ErrorMessage "Build failed with preset: $Preset"
        exit 1
    }

    # 自動モード: Ninjaを優先
    Write-InfoMessage "Auto-detecting best build configuration..."

    # Ninjaの準備
    $hasNinja = Test-NinjaAvailable
    if (-not $hasNinja) {
        $hasNinja = Install-NinjaIfNeeded
    }

    # Step 1: Ninjaを試行
    if ($hasNinja) {
        Write-InfoMessage "Trying Ninja generator..."
        
        if ($Config -eq "Release") {
            $configPreset = "ninja-release"
            $buildPreset = "ninja-release"
        } else {
            $configPreset = "ninja-debug"
            $buildPreset = "ninja-debug"
        }

        if (Invoke-CMakeConfigure -Preset $configPreset) {
            if (Invoke-CMakeBuild -Preset $buildPreset) {
                Write-Host ""
                Write-SuccessMessage "Build completed successfully with Ninja!"
                return
            }
        }

        Write-WarningMessage "Ninja build failed, trying Visual Studio..."
        Clean-BuildDir
    }

    # Step 2: Visual Studioにフォールバック
    Write-InfoMessage "Trying Visual Studio generator..."
    
    $vsPreset = Get-VSGenerator
    if ($vsPreset) {
        if ($Config -eq "Release") {
            $configPreset = "$vsPreset-release"
            $buildPreset = "$vsPreset-release"
        } else {
            $configPreset = "$vsPreset-debug"
            $buildPreset = "$vsPreset-debug"
        }

        if (Invoke-CMakeConfigure -Preset $configPreset) {
            if (Invoke-CMakeBuild -Preset $buildPreset) {
                Write-Host ""
                Write-SuccessMessage "Build completed successfully with Visual Studio!"
                return
            }
        }
    }

    Write-Host ""
    Write-ErrorMessage "All build methods failed!"
    Write-ErrorMessage "Please ensure you have one of the following installed:"
    Write-ErrorMessage "  - Visual Studio 2019/2022 with C++ workload"
    Write-ErrorMessage "  - Ninja build system (will be auto-downloaded if available)"
    exit 1
}

Main
