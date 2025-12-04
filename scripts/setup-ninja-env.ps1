# setup-ninja-env.ps1 - Setup Ninja build environment with MSVC
#
# Usage:
#   .\scripts\setup-ninja-env.ps1
#
# This script:
# 1. Ensures Ninja is available (downloads if needed)
# 2. Sets up Visual Studio environment (vcvarsall.bat)
# 3. Configures and builds the project with Ninja

param(
    [string]$Config = "Debug",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

function Write-InfoMessage {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Cyan
}

function Write-SuccessMessage {
    param([string]$Message)
    Write-Host "[SUCCESS] $Message" -ForegroundColor Green
}

function Write-ErrorMessage {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$ToolsDir = Join-Path $ProjectRoot ".tools"
$BinDir = Join-Path $ToolsDir "bin"
$NinjaBinary = Join-Path $BinDir "ninja.exe"

# Ensure Ninja is available
Write-InfoMessage "Checking for Ninja..."
if (-not (Test-Path $NinjaBinary)) {
    Write-InfoMessage "Ninja not found. Running bootstrap script..."
    & "$ScriptDir\bootstrap-ninja.ps1"
}

# Add Ninja to PATH
$env:PATH = "$BinDir;$env:PATH"
Write-InfoMessage "Ninja version: $(ninja --version)"

# Find Visual Studio installation
Write-InfoMessage "Searching for Visual Studio installation..."
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    Write-ErrorMessage "vswhere.exe not found. Please install Visual Studio 2019 or later."
    exit 1
}

$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    Write-ErrorMessage "Visual Studio with C++ tools not found."
    exit 1
}

Write-InfoMessage "Visual Studio found at: $vsPath"

# Setup Visual Studio environment
$vcvarsall = Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat"
if (-not (Test-Path $vcvarsall)) {
    Write-ErrorMessage "vcvarsall.bat not found at: $vcvarsall"
    exit 1
}

Write-InfoMessage "Setting up Visual Studio environment..."

# Run vcvarsall and capture environment variables
$tempFile = [System.IO.Path]::GetTempFileName()
cmd /c "`"$vcvarsall`" x64 && set > `"$tempFile`""

Get-Content $tempFile | ForEach-Object {
    if ($_ -match '^([^=]+)=(.*)$') {
        $name = $matches[1]
        $value = $matches[2]
        Set-Item -Path "env:$name" -Value $value
    }
}
Remove-Item $tempFile

Write-SuccessMessage "Visual Studio environment configured"

# Clean if requested
if ($Clean) {
    Write-InfoMessage "Cleaning build directory..."
    if (Test-Path "$ProjectRoot\build") {
        Remove-Item -Path "$ProjectRoot\build\*" -Recurse -Force -ErrorAction SilentlyContinue
    }
}

# Configure with CMake
Write-InfoMessage "Configuring project with Ninja..."
Push-Location $ProjectRoot
try {
    $preset = if ($Config -eq "Debug") { "ninja-debug" } else { "ninja-release" }
    cmake --preset=$preset
    
    if ($LASTEXITCODE -ne 0) {
        Write-ErrorMessage "CMake configuration failed"
        exit 1
    }
    
    Write-SuccessMessage "Configuration completed successfully"
    
    # Build
    Write-InfoMessage "Building project..."
    cmake --build build --config $Config
    
    if ($LASTEXITCODE -ne 0) {
        Write-ErrorMessage "Build failed"
        exit 1
    }
    
    Write-SuccessMessage "Build completed successfully"
    Write-InfoMessage "Executables are in: build\bin\$Config"
}
finally {
    Pop-Location
}
