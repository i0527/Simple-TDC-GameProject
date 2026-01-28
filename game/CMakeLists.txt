cmake_minimum_required(VERSION 3.19)

project(CatTDGame LANGUAGES CXX)

# C++標準設定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# プラットフォーム検出
# ============================================================================
if(EMSCRIPTEN OR "${PLATFORM}" STREQUAL "Web")
    set(PLATFORM_WEB TRUE)
    message(STATUS "Platform: Web (Emscripten)")
    add_definitions(-DPLATFORM_WEB)
else()
    set(PLATFORM_WEB FALSE)
    message(STATUS "Platform: Desktop")
endif()

# ============================================================================
# 静的ビルドオプション（Desktop のみ）
# ============================================================================
if(NOT PLATFORM_WEB)
    option(BUILD_STATIC "Build with static linking" OFF)

    if(MSVC AND BUILD_STATIC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC runtime library")
        message(STATUS "Static build enabled: Using /MT runtime")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC runtime library")
    endif()
endif()

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
    
    if(PLATFORM_WEB)
        set(PLATFORM "Web" CACHE STRING "" FORCE)
    elseif(BUILD_STATIC)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    endif()
    
    FetchContent_MakeAvailable(raylib)
    
    # Visual Studioのランタイムライブラリ設定
    if(MSVC AND NOT PLATFORM_WEB)
        if(BUILD_STATIC)
            set_property(TARGET raylib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            set_property(TARGET raylib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
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
endif()

# spdlog - 高速ログライブラリ（Webビルドでは不要）
if(NOT PLATFORM_WEB AND NOT TARGET spdlog::spdlog)
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
    if(BUILD_STATIC)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    endif()
    FetchContent_MakeAvailable(spdlog)
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
    if(MSVC AND NOT PLATFORM_WEB)
        if(BUILD_STATIC)
            set_property(TARGET imgui_lib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            set_property(TARGET imgui_lib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
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
    if(MSVC AND NOT PLATFORM_WEB)
        if(BUILD_STATIC)
            set_property(TARGET rlimgui_lib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            set_property(TARGET rlimgui_lib PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
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
    # PlayerDataManager
    "${CMAKE_CURRENT_SOURCE_DIR}/core/system/PlayerDataManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/system/PlayerDataManager.hpp"
    # GameplayDataAPI
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/GameplayDataAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/GameplayDataAPI/GameplayDataAPI.Core.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/GameplayDataAPI/GameplayDataAPI.Character.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/GameplayDataAPI/GameplayDataAPI.ItemPassive.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/GameplayDataAPI/GameplayDataAPI.Stage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/GameplayDataAPI/GameplayDataAPI.PlayerData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/GameplayDataAPI/GameplayDataAPI.Consistency.cpp"
    # BaseSystemAPI
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/BaseSystemAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/BaseSystemAPI/BaseSystemAPI.Core.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/BaseSystemAPI/BaseSystemAPI.Log.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/BaseSystemAPI/BaseSystemAPI.Resource.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/BaseSystemAPI/BaseSystemAPI.Render.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/BaseSystemAPI/BaseSystemAPI.Audio.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/BaseSystemAPI/BaseSystemAPI.Timing.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/BaseSystemAPI/BaseSystemAPI.Window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/BaseSystemAPI/BaseSystemAPI.Collision.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/BaseSystemAPI/BaseSystemAPI.RenderUi.cpp"
    # AudioControlAPI
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/AudioControlAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/AudioControlAPI.cpp"
    # InputSystemAPI
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/InputSystemAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/InputSystemAPI/InputSystemAPI.Core.cpp"
    # SetupAPI
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/SetupAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/SetupAPI/SetupAPI.Core.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/SetupAPI/SetupAPI.Wave.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/SetupAPI/SetupAPI.EcsSpawn.cpp"
    # BattleSetupAPI
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/BattleSetupAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/BattleSetupAPI/BattleSetupAPI.Core.cpp"
    # BattleSetupData
    "${CMAKE_CURRENT_SOURCE_DIR}/core/config/BattleSetupData.hpp"
    # UISystemAPI
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/UISystemAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/UISystemAPI/UISystemAPI.Core.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/UISystemAPI/UISystemAPI.Events.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/UISystemAPI/UISystemAPI.Draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/UISystemAPI/UISystemAPI.ThemeAssets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/UISystemAPI/UISystemAPI.Effects.cpp"
    # ECSystemAPI
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/ECSystemAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/ECSystemAPI/ECSystemAPI.Core.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/ECSystemAPI/ECSystemAPI.Entity.cpp"
    # Scene/Overlay制御API
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/DebugUIAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/DebugUIAPI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/SceneOverlayControlAPI.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/SceneOverlayControlAPI/SceneOverlayControlAPI.Core.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/SceneOverlayControlAPI/SceneOverlayControlAPI.Update.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/SceneOverlayControlAPI/SceneOverlayControlAPI.Render.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/SceneOverlayControlAPI/SceneOverlayControlAPI.StateLifecycle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/api/SceneOverlayControlAPI/SceneOverlayControlAPI.OverlayStack.cpp"
    # オーバーレイ
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/StageSelectOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/StageSelectOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/FormationOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/FormationOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/CharacterEnhancementOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/CharacterEnhancementOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/CodexOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/CodexOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/SettingsOverlay.Core.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/SettingsOverlay.Logic.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/SettingsOverlay.Render.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/SettingsOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/GachaOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/GachaOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/LicenseOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/LicenseOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/BattleResultOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/BattleResultOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/PauseOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/PauseOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/CustomStageEnemyQueueOverlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/CustomStageEnemyQueueOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/EnhancementOverlay.Core.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/EnhancementOverlay.Update.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/EnhancementOverlay.Render.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/EnhancementOverlay.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/EnhancementOverlayInternal.hpp"
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
    # InitScene
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/InitScene.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/InitScene.hpp"
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
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/home/ITabContent.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/home/TabContent.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/states/overlays/home/TabContent.hpp"
    # Character system
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/Character.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/Character.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/CharacterLoader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/CharacterLoader.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/CharacterManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/CharacterManager.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/CharacterStatCalculator.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/EntityCreationData.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/ItemPassiveLoader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/ItemPassiveLoader.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/ItemPassiveManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/ItemPassiveManager.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/StageLoader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/StageLoader.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/StageManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/StageManager.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ecs/entities/animation/AnimationData.hpp"
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
        rlimgui_lib
        imgui_lib
)

# Webビルドではspdlogをリンクしない
if(NOT PLATFORM_WEB)
    target_link_libraries(CatTDGame PRIVATE spdlog::spdlog)
endif()

# ============================================================================
# プラットフォーム固有の設定
# ============================================================================

if(PLATFORM_WEB)
    # Emscripten向け設定
    message(STATUS "Configuring for Emscripten build")
    
    target_compile_definitions(CatTDGame PRIVATE PLATFORM_WEB)
    
    # プロジェクトルートディレクトリを取得（親CMakeLists.txtから継承）
    if(NOT DEFINED PROJECT_ROOT_DIR)
        get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
    endif()
    
    message(STATUS "Project root directory: ${PROJECT_ROOT_DIR}")
    
    # アセットとデータディレクトリの存在確認
    set(ASSETS_DIR "${PROJECT_ROOT_DIR}/assets")
    set(DATA_DIR "${PROJECT_ROOT_DIR}/data")
    
    # Windowsパスをスラッシュに変換
    file(TO_CMAKE_PATH "${ASSETS_DIR}" ASSETS_DIR_NORMALIZED)
    file(TO_CMAKE_PATH "${DATA_DIR}" DATA_DIR_NORMALIZED)
    
    # Emscriptenリンクフラグ（基本設定）
    set(EMSCRIPTEN_LINK_FLAGS)
    list(APPEND EMSCRIPTEN_LINK_FLAGS "-sUSE_GLFW=3")
    list(APPEND EMSCRIPTEN_LINK_FLAGS "-sALLOW_MEMORY_GROWTH=1")
    list(APPEND EMSCRIPTEN_LINK_FLAGS "-sWASM=1")
    list(APPEND EMSCRIPTEN_LINK_FLAGS "-sASYNCIFY")
    list(APPEND EMSCRIPTEN_LINK_FLAGS "-sINITIAL_MEMORY=268435456")
    list(APPEND EMSCRIPTEN_LINK_FLAGS "-sFORCE_FILESYSTEM=1")
    list(APPEND EMSCRIPTEN_LINK_FLAGS "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','FS','IDBFS']")
    list(APPEND EMSCRIPTEN_LINK_FLAGS "-lidbfs.js")
    
    # シェルファイルの設定
    set(SHELL_FILE "${PROJECT_ROOT_DIR}/tools/shell_minimal.html")
    if(EXISTS "${SHELL_FILE}")
        file(TO_CMAKE_PATH "${SHELL_FILE}" SHELL_FILE_NORMALIZED)
        list(APPEND EMSCRIPTEN_LINK_FLAGS "--shell-file=${SHELL_FILE_NORMALIZED}")
        message(STATUS "Using custom shell file: ${SHELL_FILE_NORMALIZED}")
    else()
        message(WARNING "Shell file not found: ${SHELL_FILE}, using default")
    endif()
    
    # アセットのプリロード
    if(EXISTS "${ASSETS_DIR}")
        list(APPEND EMSCRIPTEN_LINK_FLAGS "--preload-file=${ASSETS_DIR_NORMALIZED}@/assets")
        message(STATUS "Preloading assets from: ${ASSETS_DIR_NORMALIZED}")
    else()
        message(WARNING "Assets directory not found: ${ASSETS_DIR}")
    endif()
    
    if(EXISTS "${DATA_DIR}")
        list(APPEND EMSCRIPTEN_LINK_FLAGS "--preload-file=${DATA_DIR_NORMALIZED}@/data")
        message(STATUS "Preloading data from: ${DATA_DIR_NORMALIZED}")
    else()
        message(WARNING "Data directory not found: ${DATA_DIR}")
    endif()
    
    # ビルドタイプ別のフラグ
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND EMSCRIPTEN_LINK_FLAGS "-sASSERTIONS=1")
        list(APPEND EMSCRIPTEN_LINK_FLAGS "-sSTACK_OVERFLOW_CHECK=2")
        list(APPEND EMSCRIPTEN_LINK_FLAGS "-gsource-map")
        list(APPEND EMSCRIPTEN_LINK_FLAGS "--source-map-base=http://localhost:8000/")
        message(STATUS "Debug build: Enabled assertions and source maps")
    else()
        # 最適化を有効化（Closure Compilerは無効）
        list(APPEND EMSCRIPTEN_LINK_FLAGS "-O3")
        list(APPEND EMSCRIPTEN_LINK_FLAGS "-sEXPORT_NAME=Module")
        list(APPEND EMSCRIPTEN_LINK_FLAGS "-sMODULARIZE=0")
        message(STATUS "Release build: Enabled optimizations (Closure disabled)")
    endif()
    
    # リンクオプションを適用
    target_link_options(CatTDGame PRIVATE ${EMSCRIPTEN_LINK_FLAGS})
    
    # 出力ファイル名を設定
    set_target_properties(CatTDGame PROPERTIES
        OUTPUT_NAME "CatTDGame"
        SUFFIX ".html"
    )
    
    # デバッグ用: リンクフラグを表示
    string(REPLACE ";" " " LINK_FLAGS_STR "${EMSCRIPTEN_LINK_FLAGS}")
    message(STATUS "Emscripten link flags: ${LINK_FLAGS_STR}")
    
elseif(MSVC)
    # MSVC特有の設定
    if(BUILD_STATIC)
        target_compile_options(CatTDGame PRIVATE /MT$<$<CONFIG:Debug>:d>)
    else()
        target_compile_options(CatTDGame PRIVATE /MD$<$<CONFIG:Debug>:d>)
    endif()
    
    foreach(LANG C CXX)
        if(BUILD_STATIC)
            set(CMAKE_${LANG}_FLAGS_DEBUG "${CMAKE_${LANG}_FLAGS_DEBUG} /MTd")
            set(CMAKE_${LANG}_FLAGS_RELEASE "${CMAKE_${LANG}_FLAGS_RELEASE} /MT")
        else()
            set(CMAKE_${LANG}_FLAGS_DEBUG "${CMAKE_${LANG}_FLAGS_DEBUG} /MDd")
            set(CMAKE_${LANG}_FLAGS_RELEASE "${CMAKE_${LANG}_FLAGS_RELEASE} /MD")
        endif()
        set(CMAKE_${LANG}_FLAGS "${CMAKE_${LANG}_FLAGS} /W0 /WX-")
    endforeach()
    
    if(BUILD_STATIC)
        set_property(GLOBAL PROPERTY MSVC_RUNTIME_LIBRARY MultiThreaded)
        set_property(TARGET CatTDGame PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        set_property(GLOBAL PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
        set_property(TARGET CatTDGame PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
    
    target_compile_definitions(CatTDGame PRIVATE _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
    target_compile_options(CatTDGame PRIVATE /W0 /WX- /wd4576 /utf-8)
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_link_libraries(CatTDGame PRIVATE stdc++fs)
    target_compile_options(CatTDGame PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ============================================================================
# アセット・データディレクトリのコピー（Desktop のみ）
# ============================================================================
if(NOT PLATFORM_WEB)
    get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)

    if(EXISTS "${PROJECT_ROOT_DIR}/assets")
        add_custom_command(TARGET CatTDGame POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_ROOT_DIR}/assets
                $<TARGET_FILE_DIR:CatTDGame>/assets
            COMMENT "Copying assets to build directory..."
        )
    endif()

    if(EXISTS "${PROJECT_ROOT_DIR}/data")
        add_custom_command(TARGET CatTDGame POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_ROOT_DIR}/data
                $<TARGET_FILE_DIR:CatTDGame>/data
            COMMENT "Copying data to build directory..."
        )
    endif()
endif()

# ============================================================================
# ビルド設定完了
# ============================================================================
# 収集されたソースファイル数を表示
list(LENGTH GAME_SOURCES SOURCE_COUNT)
message(STATUS "Found ${SOURCE_COUNT} source file(s) in game/ directory")
message(STATUS "CatTDGame configured successfully")
message(STATUS "  - Entry point: main/main.cpp")
message(STATUS "  - Platform: ${PLATFORM_WEB}")
message(STATUS "  - Build Type: ${CMAKE_BUILD_TYPE}")
