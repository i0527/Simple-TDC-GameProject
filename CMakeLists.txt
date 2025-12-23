cmake_minimum_required(VERSION 3.19)

project(SimpleTDCGame_NewArch VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 共通FetchContentキャッシュ先（ユーザー配置のローカル依存を優先する）
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/external/local_cache CACHE PATH "Local cache for third-party sources")
file(MAKE_DIRECTORY ${FETCHCONTENT_BASE_DIR})

# ビルドタイプ設定
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "=== Simple TDC Game - New Architecture ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")

# サブプロジェクト追加
add_subdirectory(shared)
add_subdirectory(game)
add_subdirectory(editor)
add_subdirectory(test)

message(STATUS "=== Configuration Complete ===")
