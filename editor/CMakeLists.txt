cmake_minimum_required(VERSION 3.19)

# Editor Executable
project(SimpleTDCEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ソースファイル
set(EDITOR_SOURCES
    src/main_editor.cpp
    src/Editor/Application/EditorApp.cpp
    src/Editor/Windows/PreviewWindow.cpp
    src/Editor/Windows/PropertyPanel.cpp
    # CharacterFactoryが依存するGraphics実装
    ${CMAKE_SOURCE_DIR}/game/src/Game/Graphics/GridSheetProvider.cpp
    ${CMAKE_SOURCE_DIR}/game/src/Game/Graphics/AsepriteJsonAtlasProvider.cpp
)

# 実行ファイルを作成
add_executable(SimpleTDCEditor ${EDITOR_SOURCES})

# インクルードディレクトリ
target_include_directories(SimpleTDCEditor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 依存関係
include(FetchContent)

# ImGui（rlImGui互換の v1.89.9 に固定）
if(NOT TARGET imgui_lib)
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
endif()

if(NOT TARGET rlimgui_lib)
    FetchContent_Declare(
        rlimgui
        GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
        GIT_TAG main
    )

    FetchContent_GetProperties(rlimgui)
    if(NOT rlimgui_POPULATED)
        FetchContent_Populate(rlimgui)
    endif()

    add_library(rlimgui_lib STATIC
        ${rlimgui_SOURCE_DIR}/rlImGui.cpp
    )

    target_include_directories(rlimgui_lib PUBLIC
        ${rlimgui_SOURCE_DIR}
    )

    target_link_libraries(rlimgui_lib PUBLIC raylib imgui_lib)
else()
    message(STATUS "Reusing existing rlimgui_lib target")
endif()

# Shared Layer とリンク
target_link_libraries(SimpleTDCEditor
    PRIVATE
        SimpleTDC_Shared
        raylib
        imgui_lib
        rlimgui_lib
)

# コンパイラ警告
if(MSVC)
    target_compile_options(SimpleTDCEditor PRIVATE /W4 /utf-8)
else()
    target_compile_options(SimpleTDCEditor PRIVATE -Wall -Wextra -Wpedantic)
endif()

# assets ディレクトリをコピー
add_custom_command(TARGET SimpleTDCEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:SimpleTDCEditor>/assets
)

message(STATUS "Editor Executable configured")
