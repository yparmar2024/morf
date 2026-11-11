include("${PROJECT_SOURCE_DIR}/cmake/ViewerDependencies.cmake")

add_library(morfGui STATIC
    src/application/application.cpp
    src/application/session.cpp
    src/geometry/bounds.cpp
    src/geometry/renderer.cpp
)

target_include_directories(morfGui PUBLIC include)
target_compile_features(morfGui PRIVATE cxx_std_17)
target_link_libraries(morfGui
    PUBLIC
        raylib
    PRIVATE
        libtess2
        morfRaygui
        morfTiming
)
