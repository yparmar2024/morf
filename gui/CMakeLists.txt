include("${PROJECT_SOURCE_DIR}/cmake/ViewerDependencies.cmake")

add_library(morfGui STATIC
    src/viewer/mesh_renderer.cpp
    src/viewer/viewer.cpp
)

target_include_directories(morfGui PUBLIC include)
target_compile_features(morfGui PRIVATE cxx_std_17)
target_link_libraries(morfGui
    PUBLIC
        morfCore
        raylib
    PRIVATE
        libtess2
        morfRaygui
        morfTiming
)
