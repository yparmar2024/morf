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

# Fetch libtess2 library
FetchContent_Declare(
    libtess2
    GIT_REPOSITORY https://github.com/memononen/libtess2.git
    GIT_TAG        v1.0.2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(libtess2)

# Manually compile libtess2's premake
add_library(libtess2 STATIC                                                                                                                                                                                                             
        ${libtess2_SOURCE_DIR}/Source/bucketalloc.c                                                                                                                                                                                         
        ${libtess2_SOURCE_DIR}/Source/dict.c                                                                                                                                                                                                
        ${libtess2_SOURCE_DIR}/Source/geom.c                                                                                                                                                                                                
        ${libtess2_SOURCE_DIR}/Source/mesh.c                                                                                                                                                                                                
        ${libtess2_SOURCE_DIR}/Source/priorityq.c                                                                                                                                                                                           
        ${libtess2_SOURCE_DIR}/Source/sweep.c                                                                                                                                                                                               
        ${libtess2_SOURCE_DIR}/Source/tess.c                                                                                                                                                                                                
    )                                                                                                                                                                                                                                       
target_include_directories(libtess2 PUBLIC ${libtess2_SOURCE_DIR}/Include) 

# Add libraries
add_library(morfGui STATIC
    src/application/application.cpp
    src/application/session.cpp
    src/geometry/renderer.cpp
    src/geometry/bounds.cpp
)

# Expose our headers to consumers and fetch raygui
target_include_directories(morfGui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${raylib_SOURCE_DIR}/examples/shapes
        ${libtess2_SOURCE_DIR}/Include
)

# Link raylib
target_link_libraries(morfGui
    PUBLIC
        raylib
    PRIVATE
        libtess2
)