# build-with-ninja.ps1 - Quick build script using Ninja
#
# Usage:
#   .\scripts\build-with-ninja.ps1 [-Config Debug|Release]
#
# This script automatically:
# - Ensures Ninja is available
# - Sets up MSVC environment
# - Builds the project

param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug"
)

$ErrorActionPreference = "Stop"

Write-Host "====================================" -ForegroundColor Cyan
Write-Host "Ninja Build Script" -ForegroundColor Cyan
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

# Setup environment and build
& "$PSScriptRoot\setup-ninja-env.ps1" -Config $Config

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "====================================" -ForegroundColor Green
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "====================================" -ForegroundColor Green
Write-Host ""
Write-Host "Executables location: build\bin\$Config" -ForegroundColor Cyan
