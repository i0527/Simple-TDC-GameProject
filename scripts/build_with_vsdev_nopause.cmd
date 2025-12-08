@echo off
REM Non-interactive helper: load Visual Studio dev env then build the target
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64
cmake --build --preset ninja-release --target SimpleTDCGame_NewArch
