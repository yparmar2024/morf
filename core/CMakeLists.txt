add_library(morfCore STATIC
    src/compare/mesh_diff.cpp
    src/formats/obj/obj_io.cpp
    src/geometry/aabb.cpp
    src/geometry/mesh.cpp
    src/merge/comparison_session.cpp
)

target_include_directories(morfCore PUBLIC include)
target_compile_features(morfCore PUBLIC cxx_std_17)
target_link_libraries(morfCore PUBLIC morfTiming)
