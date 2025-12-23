$defRoot = "assets/definitions/entities/characters"
$rawRoot = "assets/raw/characters"
$iconRoot = "assets/textures/icons/characters"

New-Item -ItemType Directory -Force $defRoot | Out-Null
New-Item -ItemType Directory -Force $rawRoot | Out-Null
New-Item -ItemType Directory -Force $iconRoot | Out-Null

function Process-CharFolder([string]$tierPath, [string]$tierName) {
    if (-not (Test-Path $tierPath)) { return }
    Get-ChildItem $tierPath -Directory | ForEach-Object {
        $name = $_.Name
        $src = $_.FullName
        $defDir = Join-Path $defRoot $name
        New-Item -ItemType Directory -Force $defDir | Out-Null
        $legacyDef = Join-Path $defRoot "$name.json"
        if (Test-Path $legacyDef) {
            Move-Item -Force $legacyDef (Join-Path $defDir 'entity.json')
        }
        if (Test-Path (Join-Path $src 'animations.json')) {
            Move-Item -Force (Join-Path $src 'animations.json') (Join-Path $defDir 'animation.json')
        }
        if (Test-Path (Join-Path $src 'metadata.json')) {
            Move-Item -Force (Join-Path $src 'metadata.json') (Join-Path $defDir 'metadata.json')
        }
        $rawDir = Join-Path $rawRoot (Join-Path $tierName $name)
        New-Item -ItemType Directory -Force $rawDir | Out-Null
        if (Test-Path (Join-Path $src 'source')) {
            Move-Item -Force (Join-Path $src 'source') $rawDir
        }
        if (Test-Path (Join-Path $src 'README.md')) {
            Move-Item -Force (Join-Path $src 'README.md') $rawDir
        }
        $iconsSrc = Join-Path $src 'icons'
        if (Test-Path $iconsSrc) {
            $iconDest = Join-Path $iconRoot (Join-Path $tierName $name)
            New-Item -ItemType Directory -Force $iconDest | Out-Null
            Get-ChildItem $iconsSrc -File | ForEach-Object { Move-Item -Force $_.FullName $iconDest }
            Remove-Item -Force $iconsSrc
        }
    }
}

Process-CharFolder "assets/characters/main" "main"
Process-CharFolder "assets/characters/sub" "sub"
