# Minimum CMake version to run
cmake_minimum_required(VERSION 3.16)

# Fetch Raylib library
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY  https://github.com/raysan5/raylib.git
    GIT_TAG         5.0
    GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(raylib)

# Add libraries
add_library(morfGui STATIC
    src/Application.cpp
    src/Renderer.cpp
)

# Provide header file paths
target_include_directories(morfGui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/core/include
)

# Link external and core libraries
target_link_libraries(morfGui
    PUBLIC
        raylib
    PRIVATE
        morfCore
)