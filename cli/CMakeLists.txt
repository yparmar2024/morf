# Create CLI executable
add_executable(morf main.cpp)

# Link GUI libraries
target_link_libraries(morf PRIVATE morfGui)