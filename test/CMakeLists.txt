cmake_minimum_required(VERSION 3.19)

# Test executables
project(SimpleTDC_Tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# HatSlime Rendering test
add_executable(TestHatSlimeRendering TestHatSlimeRendering.cpp)
target_include_directories(TestHatSlimeRendering
    PRIVATE
        ${CMAKE_SOURCE_DIR}/shared/include
)
target_link_libraries(TestHatSlimeRendering
    PRIVATE
        SimpleTDC_Shared
)

# Compiler warnings
if(MSVC)
    target_compile_options(TestHatSlimeRendering PRIVATE /W4 /utf-8)
else()
    target_compile_options(TestHatSlimeRendering PRIVATE -Wall -Wextra -Wpedantic)
endif()

message(STATUS "Test executables configured")
