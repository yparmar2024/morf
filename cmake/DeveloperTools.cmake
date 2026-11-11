# Native tools only; downloaded dependencies are excluded from formatting.
file(GLOB_RECURSE MORF_CXX_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/cli/*.cpp"
    "${PROJECT_SOURCE_DIR}/cli/*.hpp"
    "${PROJECT_SOURCE_DIR}/core/*.cpp"
    "${PROJECT_SOURCE_DIR}/core/*.hpp"
    "${PROJECT_SOURCE_DIR}/gui/*.cpp"
    "${PROJECT_SOURCE_DIR}/gui/*.hpp"
    "${PROJECT_SOURCE_DIR}/support/*.hpp"
    "${PROJECT_SOURCE_DIR}/tests/*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/*.hpp"
)
list(SORT MORF_CXX_FILES)

find_program(MORF_CLANG_FORMAT NAMES clang-format)
if(MORF_CLANG_FORMAT)
    add_custom_target(format
        COMMAND "${MORF_CLANG_FORMAT}" -i --style=file ${MORF_CXX_FILES}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Formatting Morf C++ sources and sorting includes"
        VERBATIM
    )
    add_custom_target(format-check
        COMMAND "${MORF_CLANG_FORMAT}" --dry-run --Werror --style=file ${MORF_CXX_FILES}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Checking Morf C++ formatting"
        VERBATIM
    )
endif()

find_program(MORF_CPPCHECK NAMES cppcheck)
if(MORF_CPPCHECK AND CMAKE_EXPORT_COMPILE_COMMANDS AND CMAKE_GENERATOR MATCHES "Makefiles|Ninja")
    execute_process(
        COMMAND "${MORF_CPPCHECK}" --version
        OUTPUT_VARIABLE morf_cppcheck_version_output
        RESULT_VARIABLE morf_cppcheck_version_result
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(morf_cppcheck_version_result EQUAL 0 AND morf_cppcheck_version_output MATCHES "Cppcheck ([0-9]+\\.[0-9]+(\\.[0-9]+)?)")
        set(morf_cppcheck_version "${CMAKE_MATCH_1}")
    endif()

    if(NOT morf_cppcheck_version OR morf_cppcheck_version VERSION_LESS 2.7)
        message(WARNING "Cppcheck 2.7 or newer is required for multiple file filters; the cppcheck target is disabled.")
    else()
        # Exact source paths avoid wildcard differences between Cppcheck versions.
        set(morf_cppcheck_filters)
        foreach(source IN LISTS MORF_CXX_FILES)
            list(APPEND morf_cppcheck_filters "--file-filter=${source}")
        endforeach()
        set(morf_cppcheck_dependencies)
        foreach(dependency IN ITEMS raylib libtess2 Catch2)
            FetchContent_GetProperties(${dependency} SOURCE_DIR dependency_source)
            if(dependency_source)
                list(APPEND morf_cppcheck_dependencies "--suppress=*:${dependency_source}/*")
            endif()
        endforeach()
        add_custom_target(cppcheck
            COMMAND "${MORF_CPPCHECK}"
                "--project=${PROJECT_BINARY_DIR}/compile_commands.json"
                ${morf_cppcheck_filters}
                ${morf_cppcheck_dependencies}
                --enable=warning,portability
                --error-exitcode=1
                --inline-suppr
                --std=c++17
                "--suppressions-list=${PROJECT_SOURCE_DIR}/.cppcheck-suppressions"
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            COMMENT "Checking Morf C++ for defects"
            VERBATIM
        )
    endif()
endif()
