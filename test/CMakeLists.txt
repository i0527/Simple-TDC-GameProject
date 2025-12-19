cmake_minimum_required(VERSION 3.19)

# Test executables
project(SimpleTDC_Tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GridSheetProvider test
add_executable(TestGridSheetProvider TestGridSheetProvider.cpp)
target_include_directories(TestGridSheetProvider
    PRIVATE
        ${CMAKE_SOURCE_DIR}/shared/include
        ${CMAKE_SOURCE_DIR}/game/include
)
target_link_libraries(TestGridSheetProvider
    PRIVATE
        SimpleTDC_Shared
)

# AnimationSystem test
add_executable(TestAnimationSystem TestAnimationSystem.cpp)
target_include_directories(TestAnimationSystem
    PRIVATE
        ${CMAKE_SOURCE_DIR}/shared/include
        ${CMAKE_SOURCE_DIR}/game/include
        ${CMAKE_SOURCE_DIR}/build/_deps/entt-src/src
)
target_link_libraries(TestAnimationSystem
    PRIVATE
        SimpleTDC_Shared
        EnTT::EnTT
)

# Compiler warnings
if(MSVC)
    target_compile_options(TestGridSheetProvider PRIVATE /W4 /utf-8)
    target_compile_options(TestAnimationSystem PRIVATE /W4 /utf-8)
else()
    target_compile_options(TestGridSheetProvider PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(TestAnimationSystem PRIVATE -Wall -Wextra -Wpedantic)
endif()

message(STATUS "Test executables configured")
