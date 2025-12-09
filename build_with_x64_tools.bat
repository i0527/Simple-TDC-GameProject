@echo off
REM x64 Native Tools Command Prompt for VS 2022 でのビルド用バッチファイル
REM このスクリプトは VS 2022 の C++ 開発環境をロードしてビルドを実行します

REM VS 2022 x64 Native Tools 環境をロード
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM プロジェクトルートに移動
cd /d z:\retopo\Simple-TDC-GameProject

REM PowerShell でビルドスクリプトを実行
powershell -NoProfile -Command "Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process; ./scripts/build.ps1 -Clean -Config Debug"

REM 終了
pause
