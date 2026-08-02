# Create CLI executable
add_executable(morf main.cpp)

# Link core libraries
target_link_libraries(morf PRIVATE morfCore)