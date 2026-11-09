# Minimum CMake version to run
cmake_minimum_required(VERSION 3.16)

# Project name and version
project(morf VERSION 1.0.0 LANGUAGES C CXX)

# Set global project standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Shared, header-only diagnostics for core, CLI, and GUI.
add_library(morfTiming INTERFACE)
target_include_directories(morfTiming INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/support/include")
target_compile_definitions(morfTiming INTERFACE MORF_BUILD_TYPE="$<CONFIG>")

# Add dependent subdirectories
add_subdirectory(gui)
add_subdirectory(core)
add_subdirectory(cli)

include(cmake/DeveloperTools.cmake)
