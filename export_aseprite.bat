@echo off
REM Export Aseprite files to PNG and JSON for sub characters

set ASEPRITE="C:\Program Files\Aseprite\Aseprite.exe"
if not exist %ASEPRITE% (
    echo Aseprite not found at %ASEPRITE%
    echo Please install Aseprite and update the path if necessary.
    pause
    exit /b 1
)

echo Exporting HatSlime...
%ASEPRITE% -b assets/characters/sub/HatSlime/source/HatSlimeWalking.aseprite --sheet assets/characters/sub/HatSlime/HatSlime.png --data assets/characters/sub/HatSlime/animations.json --format json-array

echo Exporting LanterfishAnglerfish...
%ASEPRITE% -b assets/characters/sub/LanterfishAnglerfish/source/LanterfishAnglerfishWalking.aseprite --sheet assets/characters/sub/LanterfishAnglerfish/LanterfishAnglerfish.png --data assets/characters/sub/LanterfishAnglerfish/animations.json --format json-array

echo Exporting LongTailedTit...
%ASEPRITE% -b assets/characters/sub/LongTailedTit/source/LongTailedTit.aseprite --sheet assets/characters/sub/LongTailedTit/LongTailedTit.png --data assets/characters/sub/LongTailedTit/animations.json --format json-array

echo Exporting Orca...
%ASEPRITE% -b assets/characters/sub/Orca/source/OrcaWalking.aseprite --sheet assets/characters/sub/Orca/Orca.png --data assets/characters/sub/Orca/animations.json --format json-array

echo Exporting Rainbow...
%ASEPRITE% -b assets/characters/sub/Rainbow/source/RainbowWalking.aseprite --sheet assets/characters/sub/Rainbow/Rainbow.png --data assets/characters/sub/Rainbow/animations.json --format json-array

echo Exporting SeaHorse...
%ASEPRITE% -b assets/characters/sub/SeaHorse/source/SeaHorseWalking.aseprite --sheet assets/characters/sub/SeaHorse/SeaHorse.png --data assets/characters/sub/SeaHorse/animations.json --format json-array

echo Exporting Whale...
%ASEPRITE% -b assets/characters/sub/Whale/source/WhaleWalking.aseprite --sheet assets/characters/sub/Whale/Whale.png --data assets/characters/sub/Whale/animations.json --format json-array

echo Exporting YodarehakiDragonfish...
%ASEPRITE% -b assets/characters/sub/YodarehakiDragonfish/source/YodarehakiDragonfishWalking.aseprite --sheet assets/characters/sub/YodarehakiDragonfish/YodarehakiDragonfish.png --data assets/characters/sub/YodarehakiDragonfish/animations.json --format json-array

echo Export completed.
pause