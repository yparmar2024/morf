find_package(Threads REQUIRED)

add_executable(morf
    main.cpp
    src/command/diff.cpp
    src/command/init.cpp
    src/command/merge.cpp
)

target_include_directories(morf PRIVATE include)
target_compile_features(morf PRIVATE cxx_std_17)
target_link_libraries(morf PRIVATE morfGui morfCore morfTiming Threads::Threads)
