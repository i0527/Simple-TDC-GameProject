#!/usr/bin/env python3
"""
Simple TDC Game - 初回セットアップスクリプト

このスクリプトは git clone 直後に実行することで、以下を自動化します：
- CMake プリセット確認
- 初回ビルド実行
- WebUI セットアップ確認
- ゲーム起動準備

使用方法:
  python setup.py

Windows:
  python setup.py

Mac/Linux:
  python3 setup.py
"""

import os
import sys
import platform
import subprocess
import shutil
from pathlib import Path


class Colors:
    """ターミナル出力色定義"""
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    END = '\033[0m'
    BOLD = '\033[1m'


def print_header(text):
    """ヘッダー出力"""
    print(f"\n{Colors.BOLD}{Colors.CYAN}{'='*60}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}{text:^60}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}{'='*60}{Colors.END}\n")


def print_success(text):
    """成功メッセージ出力"""
    print(f"{Colors.GREEN}✅ {text}{Colors.END}")


def print_warning(text):
    """警告メッセージ出力"""
    print(f"{Colors.YELLOW}⚠️  {text}{Colors.END}")


def print_error(text):
    """エラーメッセージ出力"""
    print(f"{Colors.RED}❌ {text}{Colors.END}")


def print_info(text):
    """情報メッセージ出力"""
    print(f"{Colors.BLUE}ℹ️  {text}{Colors.END}")


def get_project_root():
    """プロジェクトルートを取得"""
    script_dir = Path(__file__).parent.absolute()
    return script_dir


def check_cmake():
    """CMakeがインストール済みか確認"""
    print_info("CMake をチェック中...")
    
    result = shutil.which("cmake")
    if result:
        print_success("CMake found: " + result)
        return True
    else:
        print_error("CMake not found. Please install CMake first.")
        print_info("Download from: https://cmake.org/download/")
        return False


def check_build_dir(project_root):
    """ビルドディレクトリをチェック"""
    build_dir = project_root / "build"
    
    if not build_dir.exists():
        print_info(f"Creating build directory: {build_dir}")
        build_dir.mkdir(exist_ok=True)
        return False
    else:
        print_success(f"Build directory exists: {build_dir}")
        return True


def run_cmake_configure(project_root):
    """CMake configure を実行"""
    print_header("Running CMake Configure")
    
    build_dir = project_root / "build"
    
    try:
        # CMake configure
        cmd = [
            "cmake",
            "-B", str(build_dir),
            "-S", str(project_root),
            "-DCMAKE_BUILD_TYPE=Release"
        ]
        
        print_info(f"Command: {' '.join(cmd)}")
        print()
        
        result = subprocess.run(cmd, cwd=str(project_root), check=False)
        
        if result.returncode == 0:
            print_success("CMake configure completed successfully")
            return True
        else:
            print_error("CMake configure failed")
            return False
            
    except Exception as e:
        print_error(f"CMake configure error: {str(e)}")
        return False


def run_cmake_build(project_root):
    """CMake build を実行"""
    print_header("Running CMake Build")
    
    build_dir = project_root / "build"
    
    try:
        cmd = [
            "cmake",
            "--build", str(build_dir),
            "--config", "Release"
        ]
        
        print_info(f"Command: {' '.join(cmd)}")
        print_warning("This may take several minutes on first run...")
        print()
        
        result = subprocess.run(cmd, cwd=str(build_dir), check=False)
        
        if result.returncode == 0:
            print_success("CMake build completed successfully")
            return True
        else:
            print_error("CMake build failed")
            return False
            
    except Exception as e:
        print_error(f"CMake build error: {str(e)}")
        return False


def check_nodejs_setup(project_root):
    """Node.js セットアップを確認"""
    print_header("Checking Node.js Setup")
    
    nodejs_dir = project_root / "build" / ".nodejs"
    
    if nodejs_dir.exists():
        print_success(f"Node.js directory found: {nodejs_dir}")
        
        # プラットフォーム別の実行ファイル確認
        if platform.system() == "Windows":
            node_exe = nodejs_dir / "bin" / "node.exe"
        else:
            node_exe = nodejs_dir / "bin" / "node"
        
        if node_exe.exists():
            print_success(f"Node.js executable: {node_exe}")
            return True
        else:
            print_warning(f"Node.js executable not found: {node_exe}")
            return False
    else:
        print_warning(f"Node.js directory not found: {nodejs_dir}")
        print_info("Node.js will be downloaded during CMake configure")
        return False


def check_webui_setup(project_root):
    """WebUI セットアップを確認"""
    print_header("Checking WebUI Setup")
    
    webui_dir = project_root / "tools" / "webui-editor"
    node_modules = webui_dir / "node_modules"
    
    if node_modules.exists():
        print_success(f"WebUI node_modules found: {node_modules}")
        return True
    else:
        print_warning(f"WebUI node_modules not found: {node_modules}")
        print_info("npm install will be run during CMake build")
        return False


def check_game_executable(project_root):
    """ゲーム実行ファイルを確認"""
    print_header("Checking Game Executable")
    
    if platform.system() == "Windows":
        exe_path = project_root / "build" / "bin" / "Release" / "SimpleTDCGame.exe"
    else:
        exe_path = project_root / "build" / "bin" / "SimpleTDCGame"
    
    if exe_path.exists():
        print_success(f"Game executable found: {exe_path}")
        return True
    else:
        print_warning(f"Game executable not found: {exe_path}")
        return False


def print_next_steps(project_root):
    """次のステップを表示"""
    print_header("Next Steps")
    
    if platform.system() == "Windows":
        exe_path = project_root / "build" / "bin" / "Release" / "SimpleTDCGame.exe"
    else:
        exe_path = project_root / "build" / "bin" / "SimpleTDCGame"
    
    print_info("セットアップが完了しました！")
    print()
    print_info("ゲーム実行:")
    if exe_path.exists():
        print(f"  {exe_path}")
    else:
        print(f"  {exe_path}")
    
    print()
    print_info("WebUI エディター:")
    print("  ブラウザで http://localhost:3000 にアクセス")
    print("  （ゲーム起動時に自動起動します）")
    
    print()
    print_info("ドキュメント:")
    print("  - CMake + Node.js 統合: docs/CMAKE_NODEJS_INTEGRATION.md")
    print("  - WebUI ガイド: tools/webui-editor/README.md")
    print("  - 開発マニュアル: docs/DEVELOPER_MANUAL.md")


def main():
    """メイン処理"""
    print_header("Simple TDC Game - Initial Setup")
    
    project_root = get_project_root()
    print_success(f"Project root: {project_root}")
    print()
    
    # プラットフォーム情報
    system_info = platform.system()
    processor = platform.processor()
    print_info(f"Platform: {system_info} ({processor})")
    print()
    
    # CMake チェック
    if not check_cmake():
        print_error("Setup failed: CMake not installed")
        return 1
    
    print()
    
    # ビルドディレクトリチェック
    check_build_dir(project_root)
    
    print()
    
    # CMake configure
    if not run_cmake_configure(project_root):
        print_error("Setup failed: CMake configure error")
        return 1
    
    print()
    
    # Node.js セットアップ確認
    check_nodejs_setup(project_root)
    
    print()
    
    # CMake build
    if not run_cmake_build(project_root):
        print_error("Setup failed: CMake build error")
        return 1
    
    print()
    
    # WebUI セットアップ確認
    check_webui_setup(project_root)
    
    print()
    
    # ゲーム実行ファイル確認
    check_game_executable(project_root)
    
    print()
    
    # 次のステップ表示
    print_next_steps(project_root)
    
    print()
    print_success("Setup completed!")
    print()
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
