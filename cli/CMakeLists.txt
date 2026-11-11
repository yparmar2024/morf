# Find the threading support used to load independent input models.
find_package(Threads REQUIRED)

# Create CLI executable
add_executable(morf
    main.cpp
    src/command/init.cpp
    src/command/diff.cpp
    src/command/merge.cpp
)

# Provide header file paths
target_include_directories(morf PRIVATE include)

# Link core and gui libraries
target_link_libraries(morf PRIVATE morfCore morfGui Threads::Threads)
