@echo off
REM VS 2022 ジェネレータでのビルド用バッチファイル（Ninja を使わない）
REM このスクリプトは VS 2022 ジェネレータを使用してビルドを実行します

REM VS 2022 x64 Native Tools 環境をロード
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM プロジェクトルートに移動
cd /d z:\retopo\Simple-TDC-GameProject

REM PowerShell でビルドスクリプトを実行（vs2022-debug プリセットを明示指定）
powershell -NoProfile -Command "Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process; ./scripts/build.ps1 -Clean -Preset vs2022-debug"

REM 終了
pause
