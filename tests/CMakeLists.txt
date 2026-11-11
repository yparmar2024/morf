include(FetchContent)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG 2b60af89e23d28eefc081bc930831ee9d45ea58b # v3.8.1; CMake 3.16 compatible
)
FetchContent_MakeAvailable(Catch2)
# Keep Catch2's compiled feature support consistent with its C++17 consumers.
target_compile_features(Catch2 PUBLIC cxx_std_17)

add_executable(morf_core_tests
    io_tests.cpp
    merge_tests.cpp
    mesh_tests.cpp
    obj_tests.cpp
)
target_link_libraries(morf_core_tests PRIVATE morfCore Catch2::Catch2WithMain)
target_compile_definitions(morf_core_tests PRIVATE MORF_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
# Catch2 runs the individual cases; this registration also supports CMake 3.16.
add_test(NAME core COMMAND morf_core_tests)
set_tests_properties(core PROPERTIES TIMEOUT 30)

find_package(Git REQUIRED)
foreach(scenario IN ITEMS help arguments input_errors headless literal_paths init init_existing init_comment init_partial init_failure git_drivers lfs)
    add_test(NAME "cli.${scenario}"
        COMMAND "${CMAKE_COMMAND}"
            "-DMORF_EXE=$<TARGET_FILE:morf>"
            "-DGIT_EXE=${GIT_EXECUTABLE}"
            "-DFIXTURES=${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
            "-DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/work"
            "-DSCENARIO=${scenario}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cli_tests.cmake"
    )
    set_tests_properties("cli.${scenario}" PROPERTIES TIMEOUT 45)
endforeach()
