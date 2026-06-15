# Add libraries
add_library(morfCore STATIC
    src/parser.cpp
    src/diff.cpp
)

# Provide header file paths
target_include_directories(morfCore PUBLIC
    include
)

# Require C++17 for compile
target_compile_features(morfCore PUBLIC cxx_std_17)