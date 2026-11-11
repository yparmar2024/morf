include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY  https://github.com/raysan5/raylib.git
    GIT_TAG         5.0
    GIT_SHALLOW     TRUE
)
FetchContent_Declare(
    libtess2
    GIT_REPOSITORY https://github.com/memononen/libtess2.git
    GIT_TAG        v1.0.2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib libtess2)

# The pinned libtess2 release has no CMake target.
add_library(libtess2 STATIC
    "${libtess2_SOURCE_DIR}/Source/bucketalloc.c"
    "${libtess2_SOURCE_DIR}/Source/dict.c"
    "${libtess2_SOURCE_DIR}/Source/geom.c"
    "${libtess2_SOURCE_DIR}/Source/mesh.c"
    "${libtess2_SOURCE_DIR}/Source/priorityq.c"
    "${libtess2_SOURCE_DIR}/Source/sweep.c"
    "${libtess2_SOURCE_DIR}/Source/tess.c"
)
target_include_directories(libtess2 PUBLIC "${libtess2_SOURCE_DIR}/Include")

# Raylib 5.0 bundles the Raygui header with its examples.
add_library(morfRaygui INTERFACE)
target_include_directories(morfRaygui INTERFACE "${raylib_SOURCE_DIR}/examples/shapes")
