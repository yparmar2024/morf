# Add libraries
add_library(morfCore STATIC
    src/command/diff.cpp
    src/command/merge.cpp

    src/compare/engine.cpp

    src/formats/model.cpp
    src/formats/obj/handler.cpp
)

# Provide header file paths
target_include_directories(morfCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link gui library
target_link_libraries(morfCore PRIVATE morfGui)

# Require C++17 for compile
target_compile_features(morfCore PUBLIC cxx_std_17)