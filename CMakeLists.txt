# Minimum CMake version to run
cmake_minimum_required(VERSION 3.14)

# Project name and version
project(morf VERSION 1.0.0 LANGUAGES CXX)

# Set global project standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# Add dependent subdirectories
add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(cli)