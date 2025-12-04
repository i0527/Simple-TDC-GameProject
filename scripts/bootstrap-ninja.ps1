# bootstrap-ninja.ps1 - Download Ninja build tool to project-local .tools\bin
#
# Usage:
#   .\scripts\bootstrap-ninja.ps1
#
# Environment variables:
#   NINJA_VERSION - Version of Ninja to download (default: v1.11.1)
#
# Example:
#   # Download default version
#   .\scripts\bootstrap-ninja.ps1
#
#   # Download specific version
#   $env:NINJA_VERSION = "v1.12.0"; .\scripts\bootstrap-ninja.ps1
#

param(
    [string]$Version = ""
)

$ErrorActionPreference = "Stop"

# Configuration
if ($Version) {
    $NinjaVersion = $Version
} elseif ($env:NINJA_VERSION) {
    $NinjaVersion = $env:NINJA_VERSION
} else {
    $NinjaVersion = "v1.11.1"
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$ToolsDir = Join-Path $ProjectRoot ".tools"
$BinDir = Join-Path $ToolsDir "bin"
$NinjaBinary = Join-Path $BinDir "ninja.exe"

# Helper functions
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

# Check if ninja is already available
function Test-ExistingNinja {
    # Check system PATH
    $ninjaPath = Get-Command ninja -ErrorAction SilentlyContinue
    if ($ninjaPath) {
        $ninjaVer = & ninja --version 2>$null
        if (-not $ninjaVer) { $ninjaVer = "unknown" }
        Write-InfoMessage "Ninja is already available at: $($ninjaPath.Source) (version $ninjaVer)"
        Write-InfoMessage "Skipping download. To force download, remove ninja from PATH or uninstall it."
        return $true
    }

    # Check if we already downloaded it
    if (Test-Path $NinjaBinary) {
        $ninjaVer = & $NinjaBinary --version 2>$null
        if (-not $ninjaVer) { $ninjaVer = "unknown" }
        Write-InfoMessage "Ninja already exists in .tools\bin (version $ninjaVer)"
        Write-InfoMessage "To update, delete $NinjaBinary and re-run this script."
        return $true
    }

    return $false
}

# Download and install ninja
function Install-Ninja {
    $downloadUrl = "https://github.com/ninja-build/ninja/releases/download/$NinjaVersion/ninja-win.zip"
    $tempDir = Join-Path $env:TEMP "ninja-bootstrap-$(Get-Random)"
    $zipFile = Join-Path $tempDir "ninja-win.zip"

    try {
        # Create temp directory
        New-Item -ItemType Directory -Path $tempDir -Force | Out-Null

        Write-InfoMessage "Downloading Ninja $NinjaVersion from GitHub..."
        Write-InfoMessage "URL: $downloadUrl"

        # Download
        [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12 -bor [Net.SecurityProtocolType]::Tls13
        Invoke-WebRequest -Uri $downloadUrl -OutFile $zipFile -UseBasicParsing

        # Create tools directory
        if (-not (Test-Path $BinDir)) {
            New-Item -ItemType Directory -Path $BinDir -Force | Out-Null
        }

        # Extract
        Write-InfoMessage "Extracting ninja.exe to $BinDir..."
        Expand-Archive -Path $zipFile -DestinationPath $BinDir -Force

        # Verify installation
        if (Test-Path $NinjaBinary) {
            $ninjaVer = & $NinjaBinary --version 2>$null
            if (-not $ninjaVer) { $ninjaVer = "unknown" }
            Write-SuccessMessage "Ninja $ninjaVer installed successfully to $NinjaBinary"
        } else {
            Write-ErrorMessage "Failed to install Ninja. Binary not found."
            exit 1
        }
    }
    finally {
        # Cleanup temp directory
        if (Test-Path $tempDir) {
            Remove-Item -Path $tempDir -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
}

# Print usage instructions
function Write-UsageInstructions {
    Write-Host ""
    Write-InfoMessage "To use the downloaded Ninja, add .tools\bin to your PATH:"
    Write-Host ""
    Write-Host "  `$env:PATH = `"$BinDir;`$env:PATH`""
    Write-Host ""
    Write-InfoMessage "Or configure cmake --preset ninja after adding to PATH:"
    Write-Host ""
    Write-Host "  cmake --preset ninja"
    Write-Host "  cmake --build --preset ninja-release"
    Write-Host ""
}

# Main
function Main {
    Write-InfoMessage "Ninja Bootstrap Script (Windows)"
    Write-InfoMessage "================================="
    Write-InfoMessage "Project root: $ProjectRoot"
    Write-InfoMessage "Requested version: $NinjaVersion"
    Write-Host ""

    if (Test-ExistingNinja) {
        exit 0
    }

    Install-Ninja
    Write-UsageInstructions
}

Main
