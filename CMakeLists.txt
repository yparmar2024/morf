cmake_minimum_required(VERSION 3.16)

# Set the default before project() initializes the generator's cache entries.
if(CMAKE_GENERATOR MATCHES "Makefiles|Ninja")
    if(DEFINED ENV{CMAKE_EXPORT_COMPILE_COMMANDS})
        set(morf_export_commands_default "$ENV{CMAKE_EXPORT_COMPILE_COMMANDS}")
    else()
        set(morf_export_commands_default ON)
    endif()
    set(CMAKE_EXPORT_COMPILE_COMMANDS "${morf_export_commands_default}" CACHE BOOL
        "Export the compilation database for development tools")
endif()

project(morf VERSION 1.0.0 LANGUAGES C CXX)

get_property(morf_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT morf_multi_config AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Shared, header-only diagnostics for core, CLI, and GUI.
add_library(morfTiming INTERFACE)
target_include_directories(morfTiming INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/support/include")
target_compile_definitions(morfTiming INTERFACE MORF_BUILD_TYPE="$<CONFIG>")
target_compile_features(morfTiming INTERFACE cxx_std_17)

add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(cli)

include(cmake/DeveloperTools.cmake)
