cmake_minimum_required(VERSION 3.19)

project(SimpleTDCGame_NewArch VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FetchContentモジュールを早期にインクルード
include(FetchContent)

# プラットフォーム検出（Emscripten対応）
if(EMSCRIPTEN OR "${PLATFORM}" STREQUAL "Web")
    set(PLATFORM_WEB TRUE)
    message(STATUS "Platform: Web (Emscripten)")
    add_definitions(-DPLATFORM_WEB)
else()
    set(PLATFORM_WEB FALSE)
    message(STATUS "Platform: Desktop")
endif()

# プロジェクトルートを自動検出（CMakeLists.txt の位置が移動しても game/ を正しく参照）
set(PROJECT_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "Project root directory" FORCE)
if(NOT EXISTS "${PROJECT_ROOT_DIR}/game/CMakeLists.txt")
    get_filename_component(_parent_dir "${PROJECT_ROOT_DIR}" DIRECTORY)
    if(EXISTS "${_parent_dir}/game/CMakeLists.txt")
        set(PROJECT_ROOT_DIR "${_parent_dir}" CACHE PATH "Project root directory" FORCE)
    else()
        message(FATAL_ERROR "game directory not found. Expected at ${PROJECT_ROOT_DIR}/game or ${_parent_dir}/game")
    endif()
endif()

# 共通FetchContentキャッシュ先（ユーザー配置のローカル依存を優先する）
set(FETCHCONTENT_BASE_DIR "${PROJECT_ROOT_DIR}/external/local_cache" CACHE PATH "Local cache for third-party sources" FORCE)
file(MAKE_DIRECTORY "${FETCHCONTENT_BASE_DIR}")

# ビルドタイプ設定
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "=== Cat TD Game - Unified Architecture ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Platform: ${PLATFORM_WEB}")
message(STATUS "Project Root: ${PROJECT_ROOT_DIR}")
message(STATUS "FetchContent Base Dir: ${FETCHCONTENT_BASE_DIR}")

# サブプロジェクト追加
add_subdirectory("${PROJECT_ROOT_DIR}/game" "${CMAKE_CURRENT_BINARY_DIR}/game")

message(STATUS "=== Configuration Complete ===")
