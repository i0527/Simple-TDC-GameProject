cmake_minimum_required(VERSION 3.19)

project(CatTDGame LANGUAGES CXX)

# C++標準設定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 依存関係
# 注意: 親CMakeLists.txtで既にFetchContentがincludeされ、FETCHCONTENT_BASE_DIRが設定されています
include(FetchContent)

# FetchContentの設定（親から継承されますが、明示的に設定）
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# ============================================================================
# 外部ライブラリ依存関係
# 参考: .cursorrules - ビルド/依存セクション
#       - raylib, EnTT, nlohmann/json, ImGui, rlImGui を FetchContent で管理
# ============================================================================

# raylib - 2Dグラフィックス・ゲーム開発ライブラリ
if(NOT TARGET raylib)
    set(_raylib_local "${FETCHCONTENT_BASE_DIR}/raylib-src")
    if(EXISTS "${_raylib_local}/CMakeLists.txt")
        message(STATUS "Using local raylib from: ${_raylib_local}")
        set(FETCHCONTENT_SOURCE_DIR_RAYLIB "${_raylib_local}" CACHE PATH "raylib source directory")
    endif()

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
    )

    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(raylib)
    
    # Visual Studioのランタイムライブラリ設定
    if(MSVC)
        set_property(TARGET raylib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
endif()

# EnTT - Entity Component System フレームワーク
if(NOT TARGET EnTT::EnTT)
    set(_entt_local "${FETCHCONTENT_BASE_DIR}/entt-src")
    if(EXISTS "${_entt_local}/CMakeLists.txt")
        message(STATUS "Using local EnTT from: ${_entt_local}")
        set(FETCHCONTENT_SOURCE_DIR_ENTT "${_entt_local}" CACHE PATH "EnTT source directory")
    endif()

    FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.13.0
    )

    FetchContent_MakeAvailable(entt)
    
    # Visual Studioのランタイムライブラリ設定
    if(MSVC AND TARGET EnTT)
        set_property(TARGET EnTT PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
endif()

# nlohmann/json - JSON パース・シリアライズライブラリ
if(NOT TARGET nlohmann_json::nlohmann_json)
    set(_json_local "${FETCHCONTENT_BASE_DIR}/json-src")
    if(EXISTS "${_json_local}/CMakeLists.txt")
        message(STATUS "Using local nlohmann/json from: ${_json_local}")
        set(FETCHCONTENT_SOURCE_DIR_JSON "${_json_local}" CACHE PATH "nlohmann/json source directory")
    endif()

    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )

    FetchContent_MakeAvailable(json)
    
    # nlohmann/jsonはヘッダーオンリーライブラリ（INTERFACE）のため、MSVC_RUNTIME_LIBRARY設定は不要
endif()

# spdlog - 高速ログライブラリ
if(NOT TARGET spdlog::spdlog)
    set(_spdlog_local "${FETCHCONTENT_BASE_DIR}/spdlog-src")
    if(EXISTS "${_spdlog_local}/CMakeLists.txt")
        message(STATUS "Using local spdlog from: ${_spdlog_local}")
        set(FETCHCONTENT_SOURCE_DIR_SPDLOG "${_spdlog_local}" CACHE PATH "spdlog source directory")
    endif()

    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.13.0
    )

    set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(spdlog)
    
    # spdlog::spdlogはALIASターゲットのため、MSVC_RUNTIME_LIBRARY設定は不要
    # グローバル設定が適用されます
endif()

# ImGui - デバッグUI・エディタ機能（dockingブランチ使用）
if(NOT TARGET imgui_lib)
    set(_imgui_local "${FETCHCONTENT_BASE_DIR}/imgui-src")
    if(EXISTS "${_imgui_local}/CMakeLists.txt" OR EXISTS "${_imgui_local}/imgui.cpp")
        message(STATUS "Using local ImGui from: ${_imgui_local}")
        set(FETCHCONTENT_SOURCE_DIR_IMGUI "${_imgui_local}" CACHE PATH "ImGui source directory")
    endif()

    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG docking
    )

    FetchContent_MakeAvailable(imgui)

    add_library(imgui_lib STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
     )

    target_include_directories(imgui_lib PUBLIC ${imgui_SOURCE_DIR})
    
    # Visual Studioのランタイムライブラリ設定
    if(MSVC)
        set_property(TARGET imgui_lib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
endif()

# rlImGui - Raylib と ImGui の統合ライブラリ
if(NOT TARGET rlimgui_lib)
    set(_rlimgui_local "${FETCHCONTENT_BASE_DIR}/rlimgui-src")
    if(EXISTS "${_rlimgui_local}/CMakeLists.txt" OR EXISTS "${_rlimgui_local}/rlImGui.cpp")
        message(STATUS "Using local rlImGui from: ${_rlimgui_local}")
        set(FETCHCONTENT_SOURCE_DIR_RLIMGUI "${_rlimgui_local}" CACHE PATH "rlImGui source directory")
    endif()

    FetchContent_Declare(
        rlimgui
        GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
        GIT_TAG main
    )

    FetchContent_MakeAvailable(rlimgui)

    add_library(rlimgui_lib STATIC
        ${rlimgui_SOURCE_DIR}/rlImGui.cpp
     )

    target_include_directories(rlimgui_lib PUBLIC
        ${rlimgui_SOURCE_DIR}
    )

    target_link_libraries(rlimgui_lib PUBLIC raylib imgui_lib)
    target_compile_definitions(rlimgui_lib PUBLIC IMGUI_ENABLE_DOCKING IMGUI_ENABLE_VIEWPORTS)
    
    # Visual Studioのランタイムライブラリ設定
    if(MSVC)
        set_property(TARGET rlimgui_lib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
endif()

# ============================================================================
# ソースファイル自動収集
# ============================================================================
# 方針: main.cppをエントリポイントとして、gameディレクトリ内の
#       c, cpp, h, hppファイル全てをビルド対象に含める（再帰的検索）
# 参考: docs/directory_structure.md - ヘッダーとソースは同じディレクトリに配置
# 注意: core/配下は機能別にサブディレクトリに分類されています:
#       - core/api/     : システムAPI層
#       - core/system/  : コアシステム
#       - core/config/  : 設定・型定義
#       - core/init/    : 初期化関連
#       - core/states/  : ステート/シーン
#       - core/ecs/     : ECS関連
# ============================================================================
file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# 新規追加されたファイルを明示的に含める（GLOB_RECURSEの検出漏れを防ぐ）
# オーバーレイシステムとUIコンポーネント層のファイルを明示的に追加
list(APPEND GAME_SOURCES
    # OverlayManager
    "${CMAKE_CURRENT_SOURCE_DIR}/core/system/OverlayManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/system/OverlayManager.hpp"
    # オーバーレイ
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/StageSelectOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/StageSelectOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/FormationOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/FormationOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/EnhancementOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/EnhancementOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/CodexOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/CodexOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/SettingsOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/SettingsOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/GachaOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/GachaOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/LicenseOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/LicenseOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/IOverlay.hpp"
    # UIコンポーネント
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ui/IUIComponent.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ui/components/Card.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ui/components/Card.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ui/components/List.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ui/components/List.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ui/components/Tile.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ui/components/Tile.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ui/components/Panel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ui/components/Panel.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ui/components/Button.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ui/components/Button.hpp"
    # IScene
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/IScene.hpp"
    # HomeScreen
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/HomeScreen.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/HomeScreen.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/TitleScreen.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/TitleScreen.hpp"
    # HomeScreen components
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/home/TabBarManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/home/TabBarManager.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/home/ResourceHeader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/home/ResourceHeader.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/home/ContentContainer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/home/ContentContainer.hpp"
    # Character system
    "${CMAKE_CURRENT_SOURCE_DIR}/core/entities/Character.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/entities/Character.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/entities/CharacterManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/entities/CharacterManager.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/entities/StageManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/entities/StageManager.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/entities/CharacterToECS.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/entities/EntityCreationData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/entities/animation/AnimationData.hpp"
)

# main.cppがエントリポイントであることを確認（必須）
set(MAIN_CPP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp")
list(FIND GAME_SOURCES "${MAIN_CPP_PATH}" MAIN_CPP_INDEX)
if(MAIN_CPP_INDEX EQUAL -1)
    message(FATAL_ERROR "main/main.cpp not found! This file is required as the entry point.")
endif()

# 実行ファイルを作成（収集したすべてのソースとヘッダーを含める）
add_executable(CatTDGame ${GAME_SOURCES} )

# インクルードディレクトリ
# ドキュメント（docs/directory_structure.md）に基づいて、すべてのディレクトリを追加
target_include_directories(CatTDGame
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/main
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${raylib_SOURCE_DIR}/src
)

# ============================================================================
# ライブラリリンク
# ============================================================================
target_link_libraries(CatTDGame
    PRIVATE
        raylib
        EnTT::EnTT
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        rlimgui_lib
        imgui_lib
)

# std::filesystem サポート（GCC/Clangの場合）
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_libraries(CatTDGame PRIVATE stdc++fs)
elseif(MSVC)
    # MSVC: legacy_stdio_definitions.lib をリンク（stdio関数のために必要な場合がある）
    # 標準ライブラリは自動的にリンクされる
endif()

# MSVC特有の設定を追加（CRTランタイムの問題解決）
if(MSVC)
    # マルチスレッド対応CRTライブラリ
    target_compile_options(CatTDGame PRIVATE /MD$<$<CONFIG:Debug>:d>)
endif()

# ============================================================================
# コンパイラ警告設定・ランタイムライブラリ設定
# ============================================================================
if(MSVC)
    # Visual Studioのランタイムライブラリ設定（標準ライブラリのリンクに必要）
    # すべてのターゲットに対して一括設定
    foreach(LANG C CXX)
        set(CMAKE_${LANG}_FLAGS_DEBUG "${CMAKE_${LANG}_FLAGS_DEBUG} /MDd")
        set(CMAKE_${LANG}_FLAGS_RELEASE "${CMAKE_${LANG}_FLAGS_RELEASE} /MD")
        # グローバルに警告を抑制（すべてのターゲットに適用）
        set(CMAKE_${LANG}_FLAGS "${CMAKE_${LANG}_FLAGS} /W0 /WX-")
    endforeach()
    
    set_property(GLOBAL PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
    set_property(TARGET CatTDGame PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    # 警告を出力ログに出さない設定 (/W0: 警告を無効化, /WX-: 警告をエラーとして扱わない)
    # C4576を警告として扱い、警告抑制設定で抑制できるようにする
    # spdlogの警告を抑制するマクロ定義を追加
    target_compile_definitions(CatTDGame PRIVATE _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
    target_compile_options(CatTDGame PRIVATE /W0 /WX- /wd4576 /utf-8)
else()
    target_compile_options(CatTDGame PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ============================================================================
# アセット・データディレクトリのコピー（ビルド後）
# 参考: .cursorrules - assets/ と data/ は POST_BUILD でバンドル
# ============================================================================
# プロジェクトルートディレクトリを取得（親ディレクトリ）
get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)

# アセットディレクトリを実行ファイルの隣にコピー
# ディレクトリが存在する場合のみコピー
if(EXISTS "${PROJECT_ROOT_DIR}/assets")
    add_custom_command(TARGET CatTDGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_ROOT_DIR}/assets
            $<TARGET_FILE_DIR:CatTDGame>/assets
        COMMENT "Copying assets to build directory..."
    )
else()
    message(WARNING "assets directory not found at ${PROJECT_ROOT_DIR}/assets")
endif()

# データディレクトリも同様にコピー
if(EXISTS "${PROJECT_ROOT_DIR}/data")
    add_custom_command(TARGET CatTDGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_ROOT_DIR}/data
            $<TARGET_FILE_DIR:CatTDGame>/data
        COMMENT "Copying data to build directory..."
    )
else()
    message(WARNING "data directory not found at ${PROJECT_ROOT_DIR}/data")
endif()

# ============================================================================
# ビルド設定完了
# ============================================================================
# 収集されたソースファイル数を表示
list(LENGTH GAME_SOURCES SOURCE_COUNT)
message(STATUS "Found ${SOURCE_COUNT} source file(s) in game/ directory")
message(STATUS "CatTDGame configured successfully")
message(STATUS "  - Entry point: main/main.cpp")
message(STATUS "  - Source collection: All .c, .cpp, .h, .hpp files in game/")
