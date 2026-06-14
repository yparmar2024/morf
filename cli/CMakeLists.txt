# Create executables
add_executable(morf
    main.cpp
    ../core/src/parser.cpp
    ../core/src/diff.cpp
)

# Provide header file paths
target_include_directories(morf PRIVATE
    ../core/include
)