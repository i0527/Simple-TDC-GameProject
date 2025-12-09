param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Config = "Debug",
    [ValidateSet("ALL", "SimpleTDCGame", "SimpleTDCEditor", "SimpleTDC_Shared")]
    [string]$Target = "ALL",
    [ValidateSet("minimal", "normal", "detailed", "diagnostic")]
    [string]$Verbosity = "minimal",
    [switch]$Reconfigure,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

function Write-Info { param([string]$Message) Write-Host "[INFO] $Message" -ForegroundColor Cyan }
function Write-Warn { param([string]$Message) Write-Host "[WARN] $Message" -ForegroundColor Yellow }
function Write-ErrorMsg { param([string]$Message) Write-Host "[ERROR] $Message" -ForegroundColor Red }

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$BuildDir = Join-Path $ProjectRoot "build/output"
$AssetsSrc = Join-Path $ProjectRoot "assets"
$CMakeCache = Join-Path $BuildDir "CMakeCache.txt"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-ErrorMsg "cmake が見つかりません (PATH を確認してください)"
    exit 1
}

if ($Clean -and (Test-Path $BuildDir)) {
    Write-Info "Cleaning $BuildDir"
    Remove-Item -Path $BuildDir -Recurse -Force
}

# Configure (Visual Studio 2022, x64)
if ($Reconfigure -or -not (Test-Path $CMakeCache)) {
    Write-Info "Configuring (generator: Visual Studio 17 2022, arch: x64, config: $Config)"
    & cmake -S $ProjectRoot -B $BuildDir -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=$Config
    if ($LASTEXITCODE -ne 0) { Write-ErrorMsg "CMake configure failed"; exit $LASTEXITCODE }
}
else {
    Write-Info "Configure step skipped (cache exists). Use -Reconfigure to force."
}

$BuildTarget = if ($Target -eq "ALL") { "ALL_BUILD" } else { $Target }
$MsbuildVerbosity = switch ($Verbosity) {
    "minimal" { "minimal" }
    "normal" { "normal" }
    "detailed" { "detailed" }
    "diagnostic" { "diagnostic" }
}

Write-Info "Building target: $BuildTarget (config: $Config, verbosity: $Verbosity)"
& cmake --build $BuildDir --config $Config --target $BuildTarget -- /m /verbosity:$MsbuildVerbosity
if ($LASTEXITCODE -ne 0) { Write-ErrorMsg "Build failed"; exit $LASTEXITCODE }

# Asset copy into the built binary folder (matches POST_BUILD behavior but ensures presence)
$BinDir = Join-Path $BuildDir "bin"
$ConfigBinDir = Join-Path $BinDir $Config
$AssetDest = Join-Path $ConfigBinDir "assets"
if (Test-Path $AssetsSrc) {
    Write-Info "Copying assets to $AssetDest"
    if (-not (Test-Path $ConfigBinDir)) { New-Item -ItemType Directory -Path $ConfigBinDir | Out-Null }
    Copy-Item -Path $AssetsSrc -Destination $AssetDest -Recurse -Force
}
else {
    Write-Warn "Assets directory not found at $AssetsSrc"
}

Write-Host ""
Write-Info "Build completed successfully."
Write-Info "Output: $ConfigBinDir"
Write-Info "Target(s): $Target"
