# Create executables
add_executable(polydiff
    main.cpp
    ../core/src/parser.cpp
)

# Provide header file paths
target_include_directories(polydiff PRIVATE
    ../core/include
)