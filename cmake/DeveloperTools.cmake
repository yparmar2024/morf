# Native tools only; downloaded dependencies are excluded from formatting.
file(GLOB_RECURSE MORF_CXX_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/cli/*.cpp"
    "${PROJECT_SOURCE_DIR}/cli/*.hpp"
    "${PROJECT_SOURCE_DIR}/core/*.cpp"
    "${PROJECT_SOURCE_DIR}/core/*.hpp"
    "${PROJECT_SOURCE_DIR}/gui/*.cpp"
    "${PROJECT_SOURCE_DIR}/gui/*.hpp"
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
if(MORF_CPPCHECK)
    add_custom_target(cppcheck
        COMMAND "${MORF_CPPCHECK}"
            "--project=${PROJECT_BINARY_DIR}/compile_commands.json"
            "-i${PROJECT_BINARY_DIR}"
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
