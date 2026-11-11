add_library(morfCore STATIC
    src/compare/engine.cpp
    src/formats/model.cpp
    src/formats/obj/handler.cpp
)

target_include_directories(morfCore PUBLIC include)
target_compile_features(morfCore PUBLIC cxx_std_17)
target_link_libraries(morfCore PUBLIC morfTiming)
