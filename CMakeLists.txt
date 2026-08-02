# Minimum CMake version to run
cmake_minimum_required(VERSION 3.16)

# Project name and version
project(morf VERSION 1.0.0 LANGUAGES C CXX)

# Set global project standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add dependent subdirectories
add_subdirectory(gui)
add_subdirectory(core)
add_subdirectory(cli)